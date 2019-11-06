// Notify the background script as soon as the content script has loaded.
// chrome.tabs.insertCSS may sometimes fail to inject CSS in a newly navigated
// page when using the chrome.webNavigation API.
// See: https://bugs.chromium.org/p/chromium/issues/detail?id=331654#c15
// The RenderView should always be ready when the content script begins, so
// this message is used to trigger CSS insertion instead.
chrome.runtime.sendMessage({
  type: 'pageContentReadyForInjection'
})

const unique = require('unique-selector').default

let target: EventTarget | null
let genericExceptions: Array<string> | undefined = undefined
let classIdBuffer: { classes: string[], ids: string[] } = { classes: [], ids: [] }

// Ensures selectors are not queried from the Rust library more than once
const queriedIds = new Set()
const queriedClasses = new Set()
const regexWhitespace = /\s/

// Stores selectors received from the Rust library
const receivedSelectors: string[] = []

// Checks the entire DOM for elements that match selectors received from the Rust library
let runCounter = 0
const hideThirdPartyAds = () => {
  for (const selector of receivedSelectors) {
    document.querySelectorAll(selector).forEach(node => {
      if (node instanceof HTMLElement) {
        if (isDomBlocked(node, window.location.href)) {
          node.setAttribute('first-party-content', 'false')
        } else {
          node.setAttribute('first-party-content', 'true')
        }
      }
    })
  }
  runCounter += 1
  if (runCounter > 3) {
    clearInterval(partyChecker)
  }
}

const partyChecker = setInterval(hideThirdPartyAds, 1000)

// Gets any new unique classes or ids on a set of mutations from the MutationObserver
const getClassesAndIds = function (addedNodes: Element[]) {
  const ids = []
  const classes = []

  for (const node of addedNodes) {
    let nodeId = node.id
    if (nodeId && nodeId.length !== 0) {
      nodeId = nodeId.trim()
      if (!queriedIds.has(nodeId) && nodeId.length !== 0) {
        ids.push(nodeId)
        queriedIds.add(nodeId)
      }
    }
    let nodeClass = node.className
    if (nodeClass && nodeClass.length !== 0 && !regexWhitespace.test(nodeClass)) {
      if (!queriedClasses.has(nodeClass)) {
        classes.push(nodeClass)
        queriedClasses.add(nodeClass)
      }
    } else {
      let nodeClasses = node.classList
      if (nodeClasses) {
        let j = nodeClasses.length
        while (j--) {
          const nodeClassJ = nodeClasses[j]
          if (queriedClasses.has(nodeClassJ) === false) {
            classes.push(nodeClassJ)
            queriedClasses.add(nodeClassJ)
          }
        }
      }
    }
  }
  return { classes, ids }
}

// Runs on nodes received by the MutationObserver
const handleNewNodes = (newNodes: Element[]) => {
  const { classes, ids } = getClassesAndIds(newNodes)
  // If genericExceptions hasn't been filled by the Rust library yet, the
  // classes and ids need to be buffered and sent later
  if (!(genericExceptions)) {
    classIdBuffer.classes.push(...classes)
    classIdBuffer.ids.push(...ids)
  } else {
    chrome.runtime.sendMessage({
      type: 'classIdStylesheet',
      classes,
      ids,
      exceptions: genericExceptions
    })
  }
}

function applyCosmeticFilterMutationObserver () {
  let targetNode = document.documentElement
  let observer = new MutationObserver(mutations => {
    const nodeList: Element[] = []
    for (const mutation of mutations) {
      for (let nodeIndex = 0; nodeIndex < mutation.addedNodes.length; nodeIndex++) {
        nodeList.push(mutation.addedNodes[nodeIndex] as Element)
      }
    }
    handleNewNodes(nodeList)
  })
  let observerConfig = {
    childList: true,
    subtree: true
  }
  observer.observe(targetNode, observerConfig)
}

let allNodes = Array.from(document.querySelectorAll('[id],[class]'))
handleNewNodes(allNodes)
applyCosmeticFilterMutationObserver()

chrome.runtime.onMessage.addListener((msg, sender, sendResponse) => {
  const action = typeof msg === 'string' ? msg : msg.type
  switch (action) {
    // Used for manual right-click cosmetic rule creation
    case 'getTargetSelector': {
      sendResponse(unique(target))
      break
    }
    // When the Rust library has prepared a set of generic exceptions, the page
    // can safely request class and id based selector rules
    case 'cosmeticFilterGenericExceptions': {
      genericExceptions = msg.exceptions
      if (classIdBuffer.classes.length !== 0 || classIdBuffer.ids.length !== 0) {
        chrome.runtime.sendMessage({
          type: 'classIdStylesheet',
          classes: classIdBuffer.classes,
          ids: classIdBuffer.ids,
          exceptions: genericExceptions
        })
        classIdBuffer.classes = []
        classIdBuffer.ids = []
      }
      break
    }
    // The background script injects CSS generated from the rules, with
    // `[first-party-content="false"]` appended to each selector. It also
    // sends them here to allow the content script to query selectors that
    // have been received to check party-ness.
    case 'filterFirstPartyCosmeticSelectors': {
      console.error('filtering first party selectors from ' + msg.selectors.length + ' total selectors')
      receivedSelectors.push(...msg.selectors)
    }
  }
})




const parseDomain = require('parse-domain')

interface BlockTestAccumulator {
  thirdParty: boolean
  firstParty: boolean
  blacklisted: boolean
  networkedStyle: boolean
  iframeBlocked: boolean
}

const isFirstPartyTo = (sourceDomain: any, target: string) => {
    const targetDomain = parseDomain(target)
    if (!targetDomain) {
        if (target.startsWith('/')) {
            return true
        }
        console.log("Warn: not a real domain: ", target)
    }
    if (sourceDomain && targetDomain) {
        return sourceDomain.tld == targetDomain.tld && sourceDomain.domain == targetDomain.domain
    }
    return undefined
}

const nodeBlockTest = (node: HTMLElement, sourceUrl: any, acc: BlockTestAccumulator) => {
    if (node.hasAttribute) {
        const idAttribute = node.getAttribute('id')
        if (idAttribute) {
          if (idAttribute.startsWith('google_ads_iframe_') || idAttribute.startsWith('div-gpt-ad') || idAttribute.startsWith('adfox_')) {
            acc.blacklisted = true
          }
        }
        const srcAttribute = node.getAttribute('src')
        if (srcAttribute) {
          const firstParty = isFirstPartyTo(sourceUrl, srcAttribute)
          if (firstParty !== undefined) {
            if (firstParty === true) {
              acc.firstParty = true
            } else if (firstParty === false) {
              acc.thirdParty = true
            }
          }
        }
        const styleAttribute = node.getAttribute('style')
        if (styleAttribute) {
          if (styleAttribute.includes('url(') || styleAttribute.includes('\\')) {
            acc.networkedStyle = true
          }
        }
        const srcdocAttribute = node.getAttribute('srcdoc')
        if (srcdocAttribute) {
          if (srcdocAttribute === '') {
            acc.thirdParty = true
          } else {
            const parser = new DOMParser()
            const iframeDom = parser.parseFromString(srcdocAttribute, "text/html")
            if (isDomBlocked(iframeDom.documentElement, sourceUrl)) {
              acc.iframeBlocked = true
            }
          }
        }
    }
    return acc
}

// A DOM tree is blocked if it affects at least one third-party resource and exactly
// zero 1st party resources.
const isDomBlocked = (domTree: HTMLElement, sourceUrl: any) => {
    const contents = isNodeTreeBlocked(domTree, parseDomain(sourceUrl), {thirdParty: false, firstParty: false, blacklisted: false, networkedStyle: false, iframeBlocked: false})
    return (contents.thirdParty && !contents.firstParty) || contents.blacklisted || contents.networkedStyle || contents.iframeBlocked
}

const isNodeTreeBlocked = (node: HTMLElement, sourceUrl: any, acc: BlockTestAccumulator) => {
    acc = nodeBlockTest(node, sourceUrl, acc)
    let childNode = node.firstChild
    while(childNode) {
        acc = isNodeTreeBlocked(childNode as HTMLElement, sourceUrl, acc)
        childNode = childNode.nextSibling
    }
    return acc
}
