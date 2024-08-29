// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

import BraveCore
import BraveShared
import BraveShields
import BraveUI
import BraveWallet
import CertificateUtilities
import Data
import Favicon
import Foundation
import Growth
import LocalAuthentication
import MarketplaceKit
import Preferences
import SafariServices
import Shared
import UniformTypeIdentifiers
import WebKit
import os.log

// Helper types to properly discern between core types and qualifiers within a PageTransition
// mimics ui/base/page_transition_types.cc
extension CWVNavigationType: CustomDebugStringConvertible {
  // CWVNavigationType is marked with NS_OPTIONS_SET in the Obj-C side which turns this into a
  // struct with static lets for each case on the Swift side, but since `CWVNavigationTypeLink` is
  // equal to `0`, no symbol is generated for it because its implied that 0 should be an empty set.
  // This adds it back since link is a core type which should be treated like an enum case
  static let link: CWVNavigationType = .init(rawValue: 0)

  /// The core navigation type, which only one value will exist from the list of CWVNavigationType
  /// cases
  var coreType: CWVNavigationType {
    .init(rawValue: self.rawValue & ~CWVNavigationType.qualifierMask.rawValue)
  }

  /// Qualifiers that are stored within this type.
  var qualifiers: CWVNavigationType {
    .init(rawValue: self.rawValue & CWVNavigationType.qualifierMask.rawValue)
  }

  public static func == (lhs: CWVNavigationType, rhs: CWVNavigationType) -> Bool {
    assert(lhs.rawValue <= CWVNavigationType.lastCore.rawValue, "\(lhs) is a qualifier, not a core type, replace with a `contains()` check`")
    assert(rhs.rawValue <= CWVNavigationType.lastCore.rawValue, "\(rhs) is a qualifier, not a core type, replace with a `contains()` check`")
    return lhs.coreType.rawValue == rhs.coreType.rawValue
  }

  public func contains(_ member: CWVNavigationType) -> Bool {
    assert(member.rawValue > CWVNavigationType.lastCore.rawValue, "\(member) is a core type, not a qualifier, replace with an equality check")
    return qualifiers.rawValue & member.rawValue != 0
  }

  // FIXME: Probably need to implement rest of Set-methods

  var isMainFrame: Bool {
    self != .autoSubframe && self != .manualSubframe
  }

  var isRedirect: Bool {
    rawValue & CWVNavigationType.isRedirectMask.rawValue != 0
  }

  var isNewNavigation: Bool {
    self != .reload && contains(.forwardBack)
  }

  var isWebTriggerable: Bool {
    switch coreType {
    case .link, .autoSubframe, .manualSubframe, .formSubmit:
      return true
    default:
      return false
    }
  }

  public var debugDescription: String {
    switch coreType {
    case .link: return "link"
    case .typed: return "typed"
    case .autoBookmark: return "auto_bookmark"
    case .autoSubframe: return "auto_subframe"
    case .manualSubframe: return "manual_subframe"
    case .generated: return "generated"
    case .autoToplevel: return "auto_toplevel"
    case .formSubmit: return "form_submit"
    case .reload: return "reload"
    case .keyword: return "keyword"
    case .keywordGenerated: return "keyword_generated"
    default: return "<unknown>"
    }
  }
}

extension URLRequest {
  /// Allow local requests only if the request is privileged.
  /// If the request is internal or unprivileged, we should deny it.
  var isInternalUnprivileged: Bool {
    guard let url = url else {
      return true
    }

    if let url = InternalURL(url) {
      return !url.isAuthorized
    } else {
      return false
    }
  }
}

extension WKNavigationType: CustomDebugStringConvertible {
  public var debugDescription: String {
    switch self {
    case .linkActivated: return "linkActivated"
    case .formResubmitted: return "formResubmitted"
    case .backForward: return "backForward"
    case .formSubmitted: return "formSubmitted"
    case .other: return "other"
    case .reload: return "reload"
    @unknown default:
      return "Unknown(\(rawValue))"
    }
  }
}

extension UTType {
  static let textCalendar = UTType(mimeType: "text/calendar")!  // Not the same as `calendarEvent`
  static let mobileConfiguration = UTType(mimeType: "application/x-apple-aspen-config")!
}

extension BrowserViewController: CWVNavigationDelegate {
  public func webViewDidCommitNavigation(_ webView: CWVWebView) {
    guard let tab = tab(for: webView) else { return }
    
    // Reset the stored http request now that load has committed.
    tab.upgradedHTTPSRequest = nil

    // Set the committed url which will also set tab.url
    tab.committedURL = webView.lastCommittedURL

    // Need to evaluate Night mode script injection after url is set inside the Tab
    tab.nightMode = Preferences.General.nightModeEnabled.value
    tab.clearSolanaConnectedAccounts()

    // Providers need re-initialized when changing origin to align with desktop in
    // `BraveContentBrowserClient::RegisterBrowserInterfaceBindersForFrame`
    // https://github.com/brave/brave-core/blob/1.52.x/browser/brave_content_browser_client.cc#L608
    if let provider = braveCore.braveWalletAPI.ethereumProvider(
      with: tab,
      isPrivateBrowsing: tab.isPrivate
    ) {
      // The Ethereum provider will fetch allowed accounts from it's delegate (the tab)
      // on initialization. Fetching allowed accounts requires the origin; so we need to
      // initialize after `commitedURL` / `url` are updated above
      tab.walletEthProvider = provider
      tab.walletEthProvider?.initialize(eventsListener: tab)
    }
    if let provider = braveCore.braveWalletAPI.solanaProvider(
      with: tab,
      isPrivateBrowsing: tab.isPrivate
    ) {
      tab.walletSolProvider = provider
      tab.walletSolProvider?.initialize(eventsListener: tab)
    }

    rewards.reportTabNavigation(tabId: tab.rewardsId)

    // The toolbar and url bar changes can not be
    // on different tab than selected. Or the webview
    // previews and etc will effect the status
    guard tabManager.selectedTab === tab else {
      return
    }

    updateUIForReaderHomeStateForTab(tab)
//    updateBackForwardActionStatus(for: webView as? BraveWebView)
  }

  public func webViewDidStartProvisionalNavigation(_ webView: CWVWebView) {
    if tabManager.selectedTab?.webView !== webView {
      return
    }
    toolbarVisibilityViewModel.toolbarState = .expanded

    // check if web view is loading a different origin than the one currently loaded
    if let selectedTab = tabManager.selectedTab,
       selectedTab.url?.origin != webView.visibleURL.origin
    {

      // new site has a different origin, hide wallet icon.
      tabManager.selectedTab?.isWalletIconVisible = false
      // new site, reset connected addresses
      tabManager.selectedTab?.clearSolanaConnectedAccounts()
      // close wallet panel if it's open
      if let popoverController = self.presentedViewController as? PopoverController,
         popoverController.contentController is WalletPanelHostingController
      {
        self.dismiss(animated: true)
      }
    }

    displayPageZoom(visible: false)

    // If we are going to navigate to a new page, hide the reader mode button. Unless we
    // are going to a about:reader page. Then we keep it on screen: it will change status
    // (orange color) as soon as the page has loaded.
    if !webView.visibleURL.isInternalURL(for: .readermode) {
      topToolbar.updateReaderModeState(ReaderModeState.unavailable)
      hideReaderModeBar(animated: false)
    }

    resetRedirectChain(webView)

    // Append source URL to redirect chain
    appendUrlToRedirectChain(webView)
  }

  fileprivate func resetRedirectChain(_ webView: CWVWebView) {
    if let tab = tab(for: webView) {
      tab.redirectChain = []
    }
  }

  fileprivate func appendUrlToRedirectChain(_ webView: CWVWebView) {
    // The redirect chain MUST be sorted by the order of redirects with the
    // first URL being the source URL.
    if let tab = tab(for: webView) {
      tab.redirectChain.append(webView.visibleURL)
    }
  }

  public func webView(
    _ webView: CWVWebView,
    decidePolicyFor navigationAction: CWVNavigationAction,
    decisionHandler: @escaping (CWVNavigationActionPolicy) -> Void
  ) {
    Task { @MainActor in
      let policy = await self.webView(webView, decidePolicyFor: navigationAction)
      decisionHandler(policy)
    }
  }

  @MainActor public func webView(
    _ webView: CWVWebView,
    decidePolicyFor navigationAction: CWVNavigationAction
  ) async -> CWVNavigationActionPolicy {
    guard var requestURL = navigationAction.request.url else {
      return .cancel
    }
    if InternalURL.isValid(url: requestURL) {
      if navigationAction.navigationType.contains(.forwardBack), navigationAction.request.isInternalUnprivileged,
         !navigationAction.navigationType.isMainFrame // FIXME: Test
//         navigationAction.targetFrame?.isMainFrame == false
          || navigationAction.request.cachePolicy == .useProtocolCachePolicy
      {
        Logger.module.warning("Denying unprivileged request: \(navigationAction.request)")
        return .cancel
      }

      return .allow
    }

    if requestURL.scheme == "about" {
      return .allow
    }

    if requestURL.isBookmarklet {
      return .cancel
    }

    // Universal links do not work if the request originates from the app, manual handling is required.
    if let mainDocURL = navigationAction.request.mainDocumentURL,
       let universalLink = UniversalLinkManager.universalLinkType(for: mainDocURL, checkPath: true)
    {
      switch universalLink {
      case .buyVPN:
        presentCorrespondingVPNViewController()
        return .cancel
      }
    }

    // First special case are some schemes that are about Calling. We prompt the user to confirm this action. This
    // gives us the exact same behaviour as Safari.
    let tab = tab(for: webView)

    if ["sms", "tel", "facetime", "facetime-audio"].contains(requestURL.scheme) {
      let shouldOpen = await handleExternalURL(
        requestURL,
        tab: tab,
        navigationAction: navigationAction
      )
      return shouldOpen ? .allow : .cancel
    }

    // Second special case are a set of URLs that look like regular http links, but should be handed over to iOS
    // instead of being loaded in the webview.
    // In addition we are exchaging actual scheme with "maps" scheme
    // So the Apple maps URLs will open properly
    if let mapsURL = isAppleMapsURL(requestURL), mapsURL.enabled {
      let shouldOpen = await handleExternalURL(
        mapsURL.url,
        tab: tab,
        navigationAction: navigationAction
      )
      return shouldOpen ? .allow : .cancel
    }

    if #available(iOS 17.4, *), !ProcessInfo.processInfo.isiOSAppOnVisionOS {
      // Accessing `MarketplaceKitURIScheme` on Vision OS results in a crash
      if requestURL.scheme == MarketplaceKitURIScheme {
        if let queryItems = URLComponents(url: requestURL, resolvingAgainstBaseURL: false)?
          .queryItems,
           let adpURL = queryItems.first(where: {
             $0.name.caseInsensitiveCompare("alternativeDistributionPackage") == .orderedSame
           })?.value?.asURL,
           navigationAction.navigationType.isMainFrame,
//           navigationAction.sourceFrame.isMainFrame,
//           adpURL.baseDomain == navigationAction.sourceFrame.request.url?.baseDomain
            adpURL.baseDomain == navigationAction.request.url?.baseDomain // FIXME: Test
        {
          return .allow
        }
        return .cancel
      }
    }

    if isStoreURL(requestURL) {
      let shouldOpen = await handleExternalURL(
        requestURL,
        tab: tab,
        navigationAction: navigationAction
      )
      return shouldOpen ? .allow : .cancel
    }

    // Handles custom mailto URL schemes.
    if requestURL.scheme == "mailto" {
      let shouldOpen = await handleExternalURL(
        requestURL,
        tab: tab,
        navigationAction: navigationAction
      )
      return shouldOpen ? .allow : .cancel
    }

    // handles Decentralized DNS
    if let decentralizedDNSHelper = self.decentralizedDNSHelperFor(url: requestURL),
       navigationAction.navigationType == .autoToplevel
    {
      topToolbar.locationView.loading = true
      let result = await decentralizedDNSHelper.lookup(
        domain: requestURL.schemelessAbsoluteDisplayString
      )
      topToolbar.locationView.loading = tabManager.selectedTab?.loading ?? false
      guard !Task.isCancelled else {  // user pressed stop, or typed new url
        return .cancel
      }
      switch result {
      case .loadInterstitial(let service):
        showWeb3ServiceInterstitialPage(service: service, originalURL: requestURL)
        return .cancel
      case .load(let resolvedURL):
        if resolvedURL.isIPFSScheme,
           let resolvedIPFSURL = braveCore.ipfsAPI.resolveGatewayUrl(for: resolvedURL)
        {
          requestURL = resolvedIPFSURL
        } else {
          requestURL = resolvedURL
        }
      case .none:
        break
      }
    }

    let isPrivateBrowsing = privateBrowsingManager.isPrivateBrowsing
    tab?.rewardsReportingState.isNewNavigation = navigationAction.navigationType.isNewNavigation // FIXME: Test
    tab?.currentRequestURL = requestURL

    // Website redirection logic
    if requestURL.isWebPage(includeDataURIs: false),
       navigationAction.navigationType.isMainFrame,
       navigationAction.navigationType.isRedirect, // FIXME: Test?
       let redirectURL = WebsiteRedirects.redirect(for: requestURL)
    {

      tab?.loadRequest(URLRequest(url: redirectURL))
      return .cancel
    }

    let signpostID = ContentBlockerManager.signpost.makeSignpostID()
    let state = ContentBlockerManager.signpost.beginInterval("decidePolicyFor", id: signpostID)

    // before loading any ad-block scripts
    // await the preparation of the ad-block services
    await LaunchHelper.shared.prepareAdBlockServices(
      adBlockService: self.braveCore.adblockService
    )

    if let mainDocumentURL = navigationAction.request.mainDocumentURL {
      if mainDocumentURL != tab?.currentPageData?.mainFrameURL {
        // Clear the current page data if the page changes.
        // Do this before anything else so that we have a clean slate.
        tab?.currentPageData = PageData(mainFrameURL: mainDocumentURL)
      }

      // Handle the "forget me" feature on navigation
      if let tab = tab, //navigationAction.targetFrame?.isMainFrame == true {
         navigationAction.navigationType.isMainFrame { // FIXME: Test
        // Cancel any forget data requests
        tabManager.cancelForgetData(for: mainDocumentURL, in: tab)

        // Forget any websites that have "forget me" enabled
        // if we navigated away from the previous domain
        if let currentURL = tab.url,
           !InternalURL.isValid(url: currentURL),
           let currentETLDP1 = currentURL.baseDomain,
           mainDocumentURL.baseDomain != currentETLDP1
        {
          tabManager.forgetDataIfNeeded(for: currentURL, in: tab)
        }
      }

      let domainForMainFrame = Domain.getOrCreate(
        forUrl: mainDocumentURL,
        persistent: !isPrivateBrowsing
      )

      if let tab = tab,
         let modifiedRequest = getInternalRedirect(
          from: navigationAction,
          in: tab,
          domainForMainFrame: domainForMainFrame
         )
      {
        tab.isInternalRedirect = true
        tab.loadRequest(modifiedRequest)

        if let url = modifiedRequest.url {
          Self.log.debug(
            "Redirected to `\(url.absoluteString, privacy: .private)`"
          )
        }

        ContentBlockerManager.signpost.endInterval(
          "decidePolicyFor",
          state,
          "Redirected navigation"
        )
        return .cancel
      } else {
        tab?.isInternalRedirect = false
      }

      // Set some additional user scripts
//      if navigationAction.targetFrame?.isMainFrame == true {
      if navigationAction.navigationType.isMainFrame { // FIXME: Test
        tab?.setScripts(scripts: [
          // Add de-amp script
          // The user script manager will take care to not reload scripts if this value doesn't change
          .deAmp: braveCore.deAmpPrefs.isDeAmpEnabled,

          // Add request blocking script
          // This script will block certian `xhr` and `window.fetch()` requests
          .requestBlocking: requestURL.isWebPage(includeDataURIs: false)
            && domainForMainFrame.globalBlockAdsAndTrackingLevel.isEnabled,

          // The tracker protection script
          // This script will track what is blocked and increase stats
          .trackerProtectionStats: requestURL.isWebPage(includeDataURIs: false)
            && domainForMainFrame.globalBlockAdsAndTrackingLevel.isEnabled,

          // Add Brave search result ads processing script
          // This script will process search result ads on the Brave search page.
            .searchResultAd: BraveAds.shouldSupportSearchResultAds()
          && BraveSearchManager.isValidURL(requestURL) && !isPrivateBrowsing,
        ])
      }

      // Check if custom user scripts must be added to or removed from the web view.
      // FIXME: Figure out how to do this check with CWVNavigationAction
//      if let targetFrame = navigationAction.targetFrame {
//        tab?.currentPageData?.addSubframeURL(
//          forRequestURL: requestURL,
//          isForMainFrame: targetFrame.isMainFrame
//        )
//        let scriptTypes =
//        await tab?.currentPageData?.makeUserScriptTypes(
//          domain: domainForMainFrame,
//          isDeAmpEnabled: braveCore.deAmpPrefs.isDeAmpEnabled
//        ) ?? []
//        tab?.setCustomUserScript(scripts: scriptTypes)
//      }
    }

    // Brave Search logic.

    if navigationAction.navigationType.isMainFrame,
       BraveSearchManager.isValidURL(requestURL),
       let webView = tabManager[webView]?.webView
    {

      if let braveSearchResultAdManager = tab?.braveSearchResultAdManager,
         braveSearchResultAdManager.isSearchResultAdClickedURL(requestURL),
         navigationAction.navigationType == .link
      {
        braveSearchResultAdManager.maybeTriggerSearchResultAdClickedEvent(requestURL)
        tab?.braveSearchResultAdManager = nil
      } else {
        // The Brave-Search-Ads header should be added with a negative value when all
        // of the following conditions are met:
        //   - The current tab is not a Private tab
        //   - Brave Rewards is enabled.
        //   - The "Search Ads" is opted-out.
        //   - The requested URL host is one of the Brave Search domains.
        if !isPrivateBrowsing && rewards.isEnabled
            && !rewards.ads.isOptedInToSearchResultAds()
            && navigationAction.request.allHTTPHeaderFields?["Brave-Search-Ads"] == nil
        {
          var modifiedRequest = URLRequest(url: requestURL)
          modifiedRequest.setValue("?0", forHTTPHeaderField: "Brave-Search-Ads")
          tab?.loadRequest(modifiedRequest)
          ContentBlockerManager.signpost.endInterval(
            "decidePolicyFor",
            state,
            "Redirected to search"
          )
          return .cancel
        }

        let domain = Domain.getOrCreate(forUrl: requestURL, persistent: !isPrivateBrowsing)
        let adsBlockingShieldUp = domain.globalBlockAdsAndTrackingLevel.isEnabled
        tab?.braveSearchResultAdManager = BraveSearchResultAdManager(
          url: requestURL,
          rewards: rewards,
          isPrivateBrowsing: isPrivateBrowsing,
          isAggressiveAdsBlocking: domain.globalBlockAdsAndTrackingLevel.isAggressive
            && adsBlockingShieldUp
        )
      }

      // We fetch cookies to determine if backup search was enabled on the website.
      let profile = self.profile
      let cookies = await webView.underlyingWebView?.configuration.websiteDataStore.httpCookieStore.allCookies() ?? []
      tab?.braveSearchManager = BraveSearchManager(
        profile: profile,
        url: requestURL,
        cookies: cookies
      )
      if let braveSearchManager = tab?.braveSearchManager {
        braveSearchManager.fallbackQueryResultsPending = true
        braveSearchManager.shouldUseFallback { backupQuery in
          guard let query = backupQuery else {
            braveSearchManager.fallbackQueryResultsPending = false
            return
          }

          if query.found {
            braveSearchManager.fallbackQueryResultsPending = false
          } else {
            braveSearchManager.backupSearch(with: query) { completion in
              braveSearchManager.fallbackQueryResultsPending = false
              tab?.injectResults()
            }
          }
        }
      }
    } else {
      tab?.braveSearchResultAdManager = nil
      tab?.braveSearchManager = nil
    }

    // This is the normal case, opening a http or https url, which we handle by loading them in this WKWebView. We
    // always allow this. Additionally, data URIs are also handled just like normal web pages.

    if ["http", "https", "data", "blob", "file"].contains(requestURL.scheme) {
      // FIXME: Test
      if navigationAction.navigationType.isMainFrame { //navigationAction.targetFrame?.isMainFrame == true {
        // FIXME: Find a way to do this with CWVWebView
//        tab?.updateUserAgent(webView, newURL: requestURL)

        if let etldP1 = requestURL.baseDomain,
           tab?.proceedAnywaysDomainList.contains(etldP1) == false
        {
          let domain = Domain.getOrCreate(forUrl: requestURL, persistent: !isPrivateBrowsing)

          let shouldBlock = await AdBlockGroupsManager.shared.shouldBlock(
            requestURL: requestURL,
            sourceURL: requestURL,
            resourceType: .document,
            domain: domain
          )

          if shouldBlock, let url = requestURL.encodeEmbeddedInternalURL(for: .blocked) {
            let request = PrivilegedRequest(url: url) as URLRequest
            tab?.loadRequest(request)
            ContentBlockerManager.signpost.endInterval(
              "decidePolicyFor",
              state,
              "Blocked navigation"
            )
            return .cancel
          }
        }
      }

      pendingRequests[requestURL.absoluteString] = navigationAction.request

      // Adblock logic,
      // Only use main document URL, not the request URL
      // If an iFrame is loaded, shields depending on the main frame, not the iFrame request

      // Weird behavior here with `targetFram` and `sourceFrame`, on refreshing page `sourceFrame` is not nil (it is non-optional)
      //  however, it is still an uninitialized object, making it an unreliable source to compare `isMainFrame` against.
      //  Rather than using `sourceFrame.isMainFrame` or even comparing `sourceFrame == targetFrame`, a simple URL check is used.
      // No adblocking logic is be used on session restore urls. It uses javascript to retrieve the
      // request then the page is reloaded with a proper url and adblocking rules are applied.
      if let mainDocumentURL = navigationAction.request.mainDocumentURL,
         mainDocumentURL.schemelessAbsoluteString == requestURL.schemelessAbsoluteString,
         !(InternalURL(requestURL)?.isSessionRestore ?? false),
         navigationAction.navigationType.isMainFrame // FIXME: Test
//         navigationAction.sourceFrame.isMainFrame
//          || navigationAction.targetFrame?.isMainFrame == true
      {
        // Identify specific block lists that need to be applied to the requesting domain
        let domainForShields = Domain.getOrCreate(
          forUrl: mainDocumentURL,
          persistent: !isPrivateBrowsing
        )

        // Load rule lists
        let ruleLists = await AdBlockGroupsManager.shared.ruleLists(for: domainForShields)
        tab?.contentBlocker.set(ruleLists: ruleLists)
      }

      let documentTargetURL: URL? =
      // FIXME: Test
      navigationAction.request.mainDocumentURL ?? /*navigationAction.targetFrame?.request
        .mainDocumentURL ??*/ requestURL  // Should be the same as the sourceFrame URL
      if let documentTargetURL = documentTargetURL {
        let domainForShields = Domain.getOrCreate(
          forUrl: documentTargetURL,
          persistent: !isPrivateBrowsing
        )
        let isScriptsEnabled = !domainForShields.isShieldExpected(
          .noScript,
          considerAllShieldsOption: true
        )

        // Due to a bug in iOS WKWebpagePreferences.allowsContentJavaScript does NOT work!
        // https://github.com/brave/brave-ios/issues/8585
        //
        // However, the deprecated API WKWebViewConfiguration.preferences.javaScriptEnabled DOES work!
        // Even though `configuration` is @NSCopying, somehow this actually updates the preferences LIVE!!
        // This follows the same behaviour as Safari
        //
        // - Brandon T.
        //
        // FIXME: Find a way to do this with CWVWebView
//        preferences.allowsContentJavaScript = isScriptsEnabled
//        webView.configuration.preferences.javaScriptEnabled = isScriptsEnabled
      }

      // Cookie Blocking code below
      if let tab = tab {
        tab.setScript(script: .cookieBlocking, enabled: Preferences.Privacy.blockAllCookies.value)
      }

      // Reset the block alert bool on new host.
      if let newHost: String = requestURL.host, let oldHost: String = webView.visibleURL.host, // webView.url?.host,
         newHost != oldHost
      {
        self.tabManager.selectedTab?.alertShownCount = 0
        self.tabManager.selectedTab?.blockAllAlerts = false
      }

      // FIXME: Find a way to do this with CWVWebView
//      if navigationAction.shouldPerformDownload {
//        self.shouldDownloadNavigationResponse = true
//      }

      ContentBlockerManager.signpost.endInterval("decidePolicyFor", state)
      return .allow
    }

    // Standard schemes are handled in previous if-case.
    // This check handles custom app schemes to open external apps.
    // Our own 'brave' scheme does not require the switch-app prompt.
    if requestURL.scheme?.contains("brave") == false {
      // Do not allow opening external URLs from child tabs
      let shouldOpen = await handleExternalURL(
        requestURL,
        tab: tab,
        navigationAction: navigationAction
      )
      let isSyntheticClick =
      navigationAction.responds(to: Selector(("_syntheticClickType")))
      && navigationAction.value(forKey: "syntheticClickType") as? Int == 0

      // Do not show error message for JS navigated links or redirect
      // as it's not the result of a user action.
      // FIXME: Test
      if !shouldOpen, navigationAction.navigationType == .link && !isSyntheticClick {
        if self.presentedViewController == nil && self.presentingViewController == nil
            && tab?.isExternalAppAlertPresented == false && tab?.isExternalAppAlertSuppressed == false
        {

          return await withCheckedContinuation { continuation in
            // This alert does not need to be a BrowserAlertController because we return a policy
            // without waiting for user action
            let alert = UIAlertController(
              title: Strings.unableToOpenURLErrorTitle,
              message: Strings.unableToOpenURLError,
              preferredStyle: .alert
            )
            alert.addAction(UIAlertAction(title: Strings.OKString, style: .default, handler: nil))
            self.present(alert, animated: true) {
              continuation.resume(returning: shouldOpen ? .allow : .cancel)
            }
          }
        }
      }

      return shouldOpen ? .allow : .cancel
    }

    return .cancel
  }

  public func webView(
    _ webView: CWVWebView,
    decidePolicyFor navigationResponse: CWVNavigationResponse,
    decisionHandler: @escaping (CWVNavigationResponsePolicy) -> Void
  ) {
    Task { @MainActor in
      let policy = await self.webView(webView, decidePolicyFor: navigationResponse)
      decisionHandler(policy)
    }
  }

  @MainActor public func webView(
    _ webView: CWVWebView,
    decidePolicyFor navigationResponse: CWVNavigationResponse
  ) async -> CWVNavigationResponsePolicy {
    let isPrivateBrowsing = privateBrowsingManager.isPrivateBrowsing
    let response = navigationResponse.response
    let responseURL = response.url
    let tab = tab(for: webView)
    let isInvalid: Bool
    if let httpResponse = response as? HTTPURLResponse {
      isInvalid = httpResponse.statusCode >= 400
    } else {
      isInvalid = true
    }

    // Handle invalid upgrade to https
    if isInvalid,
       navigationResponse.isForMainFrame,
       let responseURL = responseURL,
       let tab = tab,
       let originalResponse = handleInvalidHTTPSUpgrade(
        tab: tab,
        responseURL: responseURL
       )
    {
      tab.loadRequest(originalResponse)
      return .cancel
    }

    // Store the response in the tab
    if let responseURL = responseURL {
      tab?.responses[responseURL] = response
    }

    // Check if we upgraded to https and if so we need to update the url of frame evaluations
    if let responseURL = responseURL,
       let domain = tab?.currentPageData?.domain(persistent: !isPrivateBrowsing),
       tab?.currentPageData?.upgradeFrameURL(
        forResponseURL: responseURL,
        isForMainFrame: navigationResponse.isForMainFrame
       ) == true
    {
      let scriptTypes =
      await tab?.currentPageData?.makeUserScriptTypes(
        domain: domain,
        isDeAmpEnabled: braveCore.deAmpPrefs.isDeAmpEnabled
      ) ?? []
      tab?.setCustomUserScript(scripts: scriptTypes)
    }

    if let tab = tab, let responseURL = responseURL,
       let response = response as? HTTPURLResponse
    {
      let internalUrl = InternalURL(responseURL)

      let kHttpClientErrorResponseCodeClass = 4
      let kHttpServerErrorResponseCodeClass = 5

      let responseCodeClass = response.statusCode / 100
      tab.rewardsReportingState.isErrorPage =
      (responseCodeClass == kHttpClientErrorResponseCodeClass
       || responseCodeClass == kHttpServerErrorResponseCodeClass)

      if !tab.rewardsReportingState.wasRestored {
        tab.rewardsReportingState.wasRestored = internalUrl?.isSessionRestore == true
      }
    }

    var request: URLRequest?
    if let url = responseURL {
      request = pendingRequests.removeValue(forKey: url.absoluteString)
    }

    // We can only show this content in the web view if this web view is not pending
    // download via the context menu.
    let canShowInWebView = /*navigationResponse.canShowMIMEType &&*/ (webView != pendingDownloadWebView)
    let forceDownload = webView == pendingDownloadWebView

    let mimeTypesThatRequireSFSafariViewControllerHandling: [UTType] = [
      .textCalendar,
      .mobileConfiguration,
    ]

    // SFSafariViewController only supports http/https links
    if navigationResponse.isForMainFrame, let url = responseURL,
       url.isWebPage(includeDataURIs: false),
       let tab, tab === tabManager.selectedTab,
       let mimeType = response.mimeType.flatMap({ UTType(mimeType: $0) }),
       mimeTypesThatRequireSFSafariViewControllerHandling.contains(mimeType)
    {
      handleLinkWithSafariViewController(url, tab: tab)
      return .cancel
    }

    // If the response has the attachment content-disposition, download it
    let mimeType = response.mimeType ?? MIMEType.octetStream
    let isAttachment =
      (response as? HTTPURLResponse)?.value(
        forHTTPHeaderField: "Content-Disposition"
      )?.starts(with: "attachment") ?? (mimeType == MIMEType.octetStream)

    if isAttachment {
      return .download
    }

    if canShowInWebView {
      return .allow
    }

    // Check if this response should be handed off to Passbook.

    // FIXME: Chromium policy deciders dont allow .download
//    if shouldDownloadNavigationResponse {
//      shouldDownloadNavigationResponse = false
//      if response.mimeType == MIMEType.passbook {
//        return .download
//      }
//    }

    // Check if this response should be downloaded.
    if let cookieStore = tab?.webView?.underlyingWebView?.configuration.websiteDataStore.httpCookieStore,
        let downloadHelper = DownloadHelper(
      request: request,
      response: response,
      cookieStore: cookieStore,
      canShowInWebView: canShowInWebView,
      forceDownload: forceDownload
    ) {
      // Clear the pending download web view so that subsequent navigations from the same
      // web view don't invoke another download.
      pendingDownloadWebView = nil

      let downloadAlertAction: (HTTPDownload) -> Void = { [weak self] download in
        self?.downloadQueue.enqueue(download)
      }

      // Open our helper and cancel this response from the webview.
      if tab === tabManager.selectedTab,
         let downloadAlert = downloadHelper.downloadAlert(from: view, okAction: downloadAlertAction)
      {
        present(downloadAlert, animated: true, completion: nil)
      }

      return .cancel
    }

    // If the content type is not HTML, create a temporary document so it can be downloaded and
    // shared to external applications later. Otherwise, clear the old temporary document.
    if let tab = tab, navigationResponse.isForMainFrame {
      if response.mimeType?.isKindOfHTML == false, let request = request {
        tab.temporaryDocument = TemporaryDocument(
          preflightResponse: response,
          request: request,
          tab: tab
        )
      } else {
        tab.temporaryDocument = nil
      }

      tab.mimeType = response.mimeType
    }

    // If none of our helpers are responsible for handling this response,
    // just let the webview handle it as normal.
    return .allow
  }

  public func webViewDidFinishNavigation(_ webView: CWVWebView) {
    guard let tab = tabManager[webView], let webView = tab.webView else { return }
    // Inject app's IAP receipt for Brave SKUs if necessary
    if !tab.isPrivate {
      Task { @MainActor in
        await BraveSkusAccountLink.injectLocalStorage(webView: webView)
      }
    }

    // Second attempt to inject results to the BraveSearch.
    // This will be called if we got fallback results faster than
    // the page navigation.
    if let braveSearchManager = tab.braveSearchManager {
      // Fallback results are ready before navigation finished,
      // they must be injected here.
      if !braveSearchManager.fallbackQueryResultsPending {
        tab.injectResults()
      }
    } else {
      // If not applicable, null results must be injected regardless.
      // The website waits on us until this is called with either results or null.
      tab.injectResults()
    }

    navigateInTab(tab: tab)
    rewards.reportTabUpdated(
      tab: tab,
      isSelected: tabManager.selectedTab == tab,
      isPrivate: privateBrowsingManager.isPrivateBrowsing
    )
    tab.reportPageLoad(to: rewards, redirectChain: tab.redirectChain)
    // Reset `rewardsReportingState` tab property so that listeners
    // can be notified of tab changes when a new navigation happens.
    tab.rewardsReportingState = RewardsTabChangeReportingState()

    Task {
      await tab.updateEthereumProperties()
      await tab.updateSolanaProperties()
    }

    if webView.visibleURL.isLocal == false {
      // Set rewards inter site url as new page load url.
      tab.rewardsXHRLoadURL = webView.visibleURL
    }

    if tab.walletEthProvider != nil {
      tab.emitEthereumEvent(.connect)
    }

    if let lastCommittedURL = webView.lastCommittedURL {
      maybeRecordBraveSearchDailyUsage(url: lastCommittedURL)
    }

    // Added this method to determine long press menu actions better
    // Since these actions are depending on tabmanager opened WebsiteCount
    updateToolbarUsingTabManager(tabManager)

    recordFinishedPageLoadP3A()
  }

  // FIXME: Need to refactor this logic into BraveWebClient::PrepareErrorPage + CWVWebView's handleSSLErrorWith delegate method
  public func webView(_ webView: CWVWebView, didFailNavigationWithError error: any Error) {
    guard let tab = tab(for: webView), let webView = tab.webView else { return }

    // Handle invalid upgrade to https
    if let responseURL = webView.lastCommittedURL,
       let response = handleInvalidHTTPSUpgrade(
        tab: tab,
        responseURL: responseURL
       )
    {
      tab.loadRequest(response)
      return
    }

    // Ignore the "Frame load interrupted" error that is triggered when we cancel a request
    // to open an external application and hand it over to UIApplication.openURL(). The result
    // will be that we switch to the external app, for example the app store, while keeping the
    // original web page in the tab instead of replacing it with an error page.
    var error = error as NSError
    if error.domain == "WebKitErrorDomain" && error.code == 102 {
      return
    }

    if checkIfWebContentProcessHasCrashed(webView, error: error) {
      return
    }

    if let sslPinningError = tab.sslPinningError {
      error = sslPinningError as NSError
    }

    if error.code == Int(CFNetworkErrors.cfurlErrorCancelled.rawValue) {
      if tab === tabManager.selectedTab {
        updateToolbarCurrentURL(tab.url?.displayURL)
        updateWebViewPageZoom(tab: tab)
      }
      return
    }

    if let url = error.userInfo[NSURLErrorFailingURLErrorKey] as? URL {
      ErrorPageHelper(certStore: profile.certStore).loadPage(error, forUrl: url, inWebView: webView)

      // Submitting same erroneous URL using toolbar will cause progress bar get stuck
      // Reseting the progress bar in case there is an error is necessary
      if tab == self.tabManager.selectedTab {
        self.topToolbar.hideProgressBar()
      }

      // If the local web server isn't working for some reason (Brave cellular data is
      // disabled in settings, for example), we'll fail to load the session restore URL.
      // We rely on loading that page to get the restore callback to reset the restoring
      // flag, so if we fail to load that page, reset it here.
      if InternalURL(url)?.aboutComponent == "sessionrestore" {
        tab.restoring = false
      }
    }
  }

  public func webView(_ webView: CWVWebView, didRequestDownloadWith task: CWVDownloadTask) {
    task.delegate = self
    let temporaryDir = FileManager.default.temporaryDirectory.appending(path: task.suggestedFileName)
    task.startDownloadToLocalFile(atPath: temporaryDir.path())
  }
}

// MARK: WKNavigationDelegate
extension BrowserViewController {
  private static let log = Logger(subsystem: Bundle.main.bundleIdentifier!, category: "navigation")

  // Recognize an Apple Maps URL. This will trigger the native app. But only if a search query is present.
  // Otherwise it could just be a visit to a regular page on maps.apple.com.
  // Exchaging https/https scheme with maps in order to open URLS properly on Apple Maps
  fileprivate func isAppleMapsURL(_ url: URL) -> (enabled: Bool, url: URL)? {
    if url.scheme == "http" || url.scheme == "https" {
      if url.host == "maps.apple.com" && url.query != nil {
        guard var urlComponents = URLComponents(url: url, resolvingAgainstBaseURL: false) else {
          return nil
        }
        urlComponents.scheme = "maps"

        if let url = urlComponents.url {
          return (true, url)
        }
        return nil
      }
    }
    return (false, url)
  }

  // Recognize a iTunes Store URL. These all trigger the native apps. Note that appstore.com and phobos.apple.com
  // used to be in this list. I have removed them because they now redirect to itunes.apple.com. If we special case
  // them then iOS will actually first open Safari, which then redirects to the app store. This works but it will
  // leave a 'Back to Safari' button in the status bar, which we do not want.
  fileprivate func isStoreURL(_ url: URL) -> Bool {
    let isStoreScheme = ["itms-apps", "itms-appss", "itmss"].contains(url.scheme)
    if isStoreScheme {
      return true
    }

    let isHttpScheme = ["http", "https"].contains(url.scheme)
    let isAppStoreHost = ["itunes.apple.com", "apps.apple.com", "appsto.re"].contains(url.host)
    return isHttpScheme && isAppStoreHost
  }

  // This is the place where we decide what to do with a new navigation action. There are a number of special schemes
  // and http(s) urls that need to be handled in a different way. All the logic for that is inside this delegate
  // method.

  fileprivate func isUpholdOAuthAuthorization(_ url: URL) -> Bool {
    return url.scheme == "rewards" && url.host == "uphold"
  }

  /// Handles a link by opening it in an SFSafariViewController and presenting it on the BVC.
  ///
  /// This is unfortunately neccessary to handle certain downloads natively such as ics/calendar invites and
  /// mobileconfiguration files.
  ///
  /// The user unfortunately has to  dismiss it manually after they have handled the file.
  /// Chrome iOS does the same
  private func handleLinkWithSafariViewController(_ url: URL, tab: Tab) {
    let vc = SFSafariViewController(url: url, configuration: .init())
    vc.modalPresentationStyle = .formSheet
    self.present(vc, animated: true)

    // If the website opened this URL in a separate tab, remove the empty tab
    if tab.url == nil || tab.url?.absoluteString == "about:blank" {
      tabManager.removeTab(tab)
    }
  }
}

// MARK: WKNavigationDelegateHelper

extension BrowserViewController {
  fileprivate func recordFinishedPageLoadP3A() {
    var storage = P3ATimedStorage<Int>.pagesLoadedStorage
    storage.add(value: 1, to: Date())
    UmaHistogramRecordValueToBucket(
      "Brave.Core.PagesLoaded",
      buckets: [
        0,
        .r(1...10),
        .r(11...50),
        .r(51...100),
        .r(101...500),
        .r(501...1000),
        .r(1001...),
      ],
      value: storage.combinedValue
    )
  }

  private func tab(for webView: BraveWebView) -> Tab? {
    tabManager[webView] ?? (webView as? TabWebView)?.tab
  }

  private func tab(for webView: CWVWebView) -> Tab? {
    tabManager[webView] ?? (webView as? TabWebView)?.tab
  }

  private func handleExternalURL(
    _ url: URL,
    tab: Tab?,
    navigationAction: CWVNavigationAction
  ) async -> Bool {
    // Do not open external links for child tabs automatically
    // The user must tap on the link to open it.
    // FIXME: Test
    if tab?.parent != nil && navigationAction.navigationType != .link {
      return false
    }

    // Check if the current url of the caller has changed
    if let domain = tab?.url?.baseDomain,
      domain != tab?.externalAppURLDomain
    {
      tab?.externalAppAlertCounter = 0
      tab?.isExternalAppAlertSuppressed = false
    }

    tab?.externalAppURLDomain = tab?.url?.baseDomain

    // Do not try to present over existing warning
    if tab?.isExternalAppAlertPresented == true || tab?.isExternalAppAlertSuppressed == true {
      return false
    }

    // External dialog should not be shown for non-active tabs #6687 - #7835
    let isVisibleTab = tab?.isTabVisible() == true

    if !isVisibleTab {
      return false
    }

    var alertTitle = Strings.openExternalAppURLGenericTitle

    if let displayHost = tab?.url?.withoutWWW.host {
      alertTitle = String(format: Strings.openExternalAppURLTitle, displayHost)
    }

    // Handling condition when Tab is empty when handling an external URL we should remove the tab once the user decides
    let removeTabIfEmpty = { [weak self] in
      if let tab = tab, tab.url == nil {
        self?.tabManager.removeTab(tab)
      }
    }

    // Show the external sceheme invoke alert
    @MainActor
    func showExternalSchemeAlert(
      isSuppressActive: Bool,
      openedURLCompletionHandler: @escaping (Bool) -> Void
    ) {
      // Check if active controller is bvc otherwise do not show show external sceheme alerts
      guard shouldShowExternalSchemeAlert() else {
        openedURLCompletionHandler(false)
        return
      }

      view.endEditing(true)
      tab?.isExternalAppAlertPresented = true

      let popup = AlertPopupView(
        imageView: nil,
        title: alertTitle,
        message: String(format: Strings.openExternalAppURLMessage, url.relativeString),
        titleWeight: .semibold,
        titleSize: 21
      )

      if isSuppressActive {
        popup.addButton(title: Strings.suppressAlertsActionTitle, type: .destructive) {
          [weak tab] () -> PopupViewDismissType in
          openedURLCompletionHandler(false)
          tab?.isExternalAppAlertSuppressed = true
          return .flyDown
        }
      } else {
        popup.addButton(title: Strings.openExternalAppURLDontAllow) {
          [weak tab] () -> PopupViewDismissType in
          openedURLCompletionHandler(false)
          removeTabIfEmpty()
          tab?.isExternalAppAlertPresented = false
          return .flyDown
        }
      }
      popup.addButton(title: Strings.openExternalAppURLAllow, type: .primary) {
        [weak tab] () -> PopupViewDismissType in
        UIApplication.shared.open(url, options: [:]) { didOpen in
          openedURLCompletionHandler(!didOpen)
        }
        removeTabIfEmpty()
        tab?.isExternalAppAlertPresented = false
        return .flyDown
      }
      popup.showWithType(showType: .flyUp)
    }

    func shouldShowExternalSchemeAlert() -> Bool {
      guard let rootVC = currentScene?.browserViewController else {
        return false
      }

      func topViewController(startingFrom viewController: UIViewController) -> UIViewController {
        var top = viewController
        if let navigationController = top as? UINavigationController,
          let vc = navigationController.visibleViewController
        {
          return topViewController(startingFrom: vc)
        }
        if let tabController = top as? UITabBarController,
          let vc = tabController.selectedViewController
        {
          return topViewController(startingFrom: vc)
        }
        while let next = top.presentedViewController {
          top = next
        }
        return top
      }

      let isTopController = self == topViewController(startingFrom: rootVC)
      let isTopWindow = view.window?.isKeyWindow == true
      return isTopController && isTopWindow
    }

    tab?.externalAppAlertCounter += 1

    return await withCheckedContinuation { continuation in
      showExternalSchemeAlert(isSuppressActive: tab?.externalAppAlertCounter ?? 0 > 2) {
        continuation.resume(with: .success($0))
      }
    }
  }
}

// MARK: WKUIDelegate

extension BrowserViewController: CWVUIDelegate {
  public func webView(
    _ webView: CWVWebView,
    createWebViewWith configuration: CWVWebViewConfiguration,
    for action: CWVNavigationAction
  ) -> CWVWebView? {
    guard let parentTab = tabManager[webView] else { return nil }
    guard !action.request.isInternalUnprivileged,
          let navigationURL = action.request.url,
          navigationURL.shouldRequestBeOpenedAsPopup()
    else {
      print("Denying popup from request: \(action.request)")
      return nil
    }

    if let currentTab = tabManager.selectedTab {
      screenshotHelper.takeScreenshot(currentTab)
    }

    // FIXME: Will require refactoring Tab to take CWVWebViewConfiguration's

    // If the page uses `window.open()` or `[target="_blank"]`, open the page in a new tab.
    // IMPORTANT!!: WebKit will perform the `URLRequest` automatically!! Attempting to do
    // the request here manually leads to incorrect results!!
//    let newTab = tabManager.addPopupForParentTab(parentTab, configuration: configuration)
//
//    newTab.url = URL(string: "about:blank")
//
//    toolbarVisibilityViewModel.toolbarState = .expanded
//
//    // Wait until WebKit starts the request before selecting the new tab, otherwise the tab manager may
//    // restore it as if it was a dead tab.
//    var observation: NSKeyValueObservation?
//    observation = newTab.webView?.observe(
//      \.visibleURL,
//       changeHandler: { [weak self] webView, _ in
//         _ = observation  // Silence write but not read warning
//         observation = nil
//         guard let self = self, let tab = self.tabManager[webView] else { return }
//         self.tabManager.selectTab(tab)
//       }
//    )
//
//    return newTab.webView
    return nil
  }

  public func webViewDidClose(_ webView: CWVWebView) {
    guard let tab = tabManager[webView] else { return }
    tabManager.addTabToRecentlyClosed(tab)
    tabManager.removeTab(tab)
  }

  public func webView(
    _ webView: CWVWebView,
    requestMediaCapturePermissionFor type: CWVMediaCaptureType,
    decisionHandler: @escaping (CWVPermissionDecision) -> Void
  ) {
    let titleFormat: String = {
      switch type {
      case .camera:
        return Strings.requestCameraPermissionPrompt
      case .microphone:
        return Strings.requestMicrophonePermissionPrompt
      case .cameraAndMicrophone:
        return Strings.requestCameraAndMicrophonePermissionPrompt
      @unknown default:
        return Strings.requestCaptureDevicePermissionPrompt
      }
    }()
    // FIXME: Test
    guard let host = webView.visibleURL.host else { return }
    let title = String.localizedStringWithFormat(titleFormat, host)
    let alertController = BrowserAlertController(title: title, message: nil, preferredStyle: .alert)
    alertController.addAction(
      .init(
        title: Strings.requestCaptureDevicePermissionAllowButtonTitle,
        style: .default,
        handler: { _ in
          decisionHandler(.grant)
        }
      )
    )
    alertController.addAction(
      .init(
        title: Strings.CancelString,
        style: .cancel,
        handler: { _ in
          decisionHandler(.deny)
        }
      )
    )
    alertController.dismissedWithoutAction = {
      decisionHandler(.prompt)
    }
    present(alertController, animated: true)
  }

  public func webView(
    _ webView: CWVWebView,
    contextMenuConfigurationFor element: CWVHTMLElement,
    completionHandler: @escaping (UIContextMenuConfiguration?) -> Void
  ) {
    // Only show context menu for valid links such as `http`, `https`, `data`. Safari does not show it for anything else.
    // This is because you cannot open `javascript:something` URLs in a new page, or share it, or anything else.
    guard let url = element.hyperlink, url.isWebPage() else {
      completionHandler(UIContextMenuConfiguration(identifier: nil, previewProvider: nil, actionProvider: nil))
      return
    }

    let actionProvider: UIContextMenuActionProvider = { _ -> UIMenu? in
      var actions = [UIAction]()

      if let currentTab = self.tabManager.selectedTab {
        let tabType = currentTab.type

        if !tabType.isPrivate {
          let openNewTabAction = UIAction(
            title: Strings.openNewTabButtonTitle,
            image: UIImage(systemName: "plus")
          ) { _ in
            self.addTab(url: url, inPrivateMode: false, currentTab: currentTab)
          }

          openNewTabAction.accessibilityLabel = "linkContextMenu.openInNewTab"
          actions.append(openNewTabAction)
        }

        let openNewPrivateTabAction = UIAction(
          title: Strings.openNewPrivateTabButtonTitle,
          image: UIImage(named: "private_glasses", in: .module, compatibleWith: nil)!.template
        ) { _ in
          if !tabType.isPrivate, Preferences.Privacy.privateBrowsingLock.value {
            self.askForLocalAuthentication { [weak self] success, error in
              if success {
                self?.addTab(url: url, inPrivateMode: true, currentTab: currentTab)
              }
            }
          } else {
            self.addTab(url: url, inPrivateMode: true, currentTab: currentTab)
          }
        }
        openNewPrivateTabAction.accessibilityLabel = "linkContextMenu.openInNewPrivateTab"

        actions.append(openNewPrivateTabAction)

        if UIApplication.shared.supportsMultipleScenes {
          if !tabType.isPrivate {
            let openNewWindowAction = UIAction(
              title: Strings.openInNewWindowTitle,
              image: UIImage(braveSystemNamed: "leo.window")
            ) { _ in
              self.openInNewWindow(url: url, isPrivate: false)
            }

            openNewWindowAction.accessibilityLabel = "linkContextMenu.openInNewWindow"
            actions.append(openNewWindowAction)
          }

          let openNewPrivateWindowAction = UIAction(
            title: Strings.openInNewPrivateWindowTitle,
            image: UIImage(braveSystemNamed: "leo.window.tab-private")
          ) { _ in
            if !tabType.isPrivate, Preferences.Privacy.privateBrowsingLock.value {
              self.askForLocalAuthentication { [weak self] success, error in
                if success {
                  self?.openInNewWindow(url: url, isPrivate: true)
                }
              }
            } else {
              self.openInNewWindow(url: url, isPrivate: true)
            }
          }

          openNewPrivateWindowAction.accessibilityLabel = "linkContextMenu.openInNewPrivateWindow"
          actions.append(openNewPrivateWindowAction)
        }

        let copyAction = UIAction(
          title: Strings.copyLinkActionTitle,
          image: UIImage(systemName: "doc.on.doc"),
          handler: UIAction.deferredActionHandler { _ in
            UIPasteboard.general.url = url as URL
          }
        )
        copyAction.accessibilityLabel = "linkContextMenu.copyLink"
        actions.append(copyAction)

        let copyCleanLinkAction = UIAction(
          title: Strings.copyCleanLink,
          image: UIImage(braveSystemNamed: "leo.broom"),
          handler: UIAction.deferredActionHandler { _ in
            let service = URLSanitizerServiceFactory.get(privateMode: currentTab.isPrivate)
            let cleanedURL = service?.sanitizeURL(url) ?? url
            UIPasteboard.general.url = cleanedURL
          }
        )
        copyCleanLinkAction.accessibilityLabel = "linkContextMenu.copyCleanLink"
        actions.append(copyCleanLinkAction)

        if let braveWebView = webView as? BraveWebView {
          let shareAction = UIAction(
            title: Strings.shareLinkActionTitle,
            image: UIImage(systemName: "square.and.arrow.up")
          ) { _ in
            let touchPoint = braveWebView.lastHitPoint
            let touchRect = CGRect(origin: touchPoint, size: .zero)

            // TODO: Find a way to add fixes #3323 and #2961 here:
            // Normally we use `tab.temporaryDocument` for the downloaded file on the tab.
            // `temporaryDocument` returns the downloaded file to disk on the current tab.
            // Using a downloaded file url results in having functions like "Save to files" available.
            // It also attaches the file (image, pdf, etc) and not the url to emails, slack, etc.
            // Since this is **not** a tab but a standalone web view, the downloaded temporary file is **not** available.
            // This results in the fixes for #3323 and #2961 not being included in this share scenario.
            // This is not a regression, we simply never handled this scenario in both fixes.
            // Some possibile fixes include:
            // - Detect the file type and download it if necessary and don't rely on the `tab.temporaryDocument`.
            // - Add custom "Save to file" functionality (needs investigation).
            self.presentActivityViewController(
              url,
              sourceView: braveWebView,
              sourceRect: touchRect,
              arrowDirection: .any
            )
          }

          shareAction.accessibilityLabel = "linkContextMenu.share"

          actions.append(shareAction)
        }

        let linkPreview = Preferences.General.enableLinkPreview.value

        let linkPreviewTitle =
        linkPreview ? Strings.hideLinkPreviewsActionTitle : Strings.showLinkPreviewsActionTitle
        let linkPreviewAction = UIAction(
          title: linkPreviewTitle,
          image: UIImage(systemName: "eye.fill")
        ) { _ in
          Preferences.General.enableLinkPreview.value.toggle()
        }

        actions.append(linkPreviewAction)
      }

      return UIMenu(title: url.absoluteString.truncate(length: 100), children: actions)
    }

    // FIXME: Link previews
//    let linkPreview: UIContextMenuContentPreviewProvider? = { [unowned self] in
//      if let tab = tabManager[webView] {
//        return LinkPreviewViewController(url: url, for: tab, browserController: self)
//      }
//      return nil
//    }

//    let linkPreviewProvider = Preferences.General.enableLinkPreview.value ? linkPreview : nil
    completionHandler(UIContextMenuConfiguration(
      identifier: nil,
      previewProvider: nil,
      actionProvider: actionProvider
    ))
  }

  public func webView(_ webView: CWVWebView, didLoad favIcons: [CWVFavicon]) {
    print("loaded favicons: \(favIcons)")
    // FIXME: Gotta run this through to download them and update the favicons on disk?
  }

  public func webView(
    _ webView: CWVWebView,
    runJavaScriptAlertPanelWithMessage message: String,
    pageURL url: URL,
    completionHandler: @escaping () -> Void
  ) {
    guard let tab = tabManager[webView], let webView = tab.webView else { return }
    var messageAlert = MessageAlert(
      message: message,
      pageURL: url,
      completionHandler: completionHandler,
      suppressHandler: nil
    )
    handleAlert(webView: webView, alert: &messageAlert) {
      completionHandler()
    }
  }

  public func webView(
    _ webView: CWVWebView,
    runJavaScriptConfirmPanelWithMessage message: String,
    pageURL url: URL,
    completionHandler: @escaping (Bool) -> Void
  ) {
    guard let tab = tabManager[webView], let webView = tab.webView else { return }
    var confirmAlert = ConfirmPanelAlert(
      message: message,
      pageURL: url,
      completionHandler: completionHandler,
      suppressHandler: nil
    )
    handleAlert(webView: webView, alert: &confirmAlert) {
      completionHandler(false)
    }
  }

  public func webView(
    _ webView: CWVWebView,
    runJavaScriptTextInputPanelWithPrompt prompt: String,
    defaultText: String,
    pageURL url: URL,
    completionHandler: @escaping (String?) -> Void
  ) {
    guard let tab = tabManager[webView], let webView = tab.webView else { return }
    var textInputAlert = TextInputAlert(
      message: prompt,
      pageURL: url,
      completionHandler: completionHandler,
      defaultText: defaultText,
      suppressHandler: nil
    )
    handleAlert(webView: webView, alert: &textInputAlert) {
      completionHandler(nil)
    }
  }
}

extension BrowserViewController {
  fileprivate func shouldDisplayJSAlertForWebView(_ webView: BraveWebView) -> Bool {
    // Only display a JS Alert if we are selected and there isn't anything being shown
    return ((tabManager.selectedTab == nil ? false : tabManager.selectedTab!.webView == webView))
      && (self.presentedViewController == nil)
  }

  func suppressJSAlerts(webView: BraveWebView) {
    let script = """
      window.alert=window.confirm=window.prompt=function(n){},
      [].slice.apply(document.querySelectorAll('iframe')).forEach(function(n){if(n.contentWindow != window){n.contentWindow.alert=n.contentWindow.confirm=n.contentWindow.prompt=function(n){}}})
      """
    webView.underlyingWebView?.evaluateSafeJavaScript(
      functionName: script,
      contentWorld: .defaultClient,
      asFunction: false
    )
  }

  func handleAlert<T: JSAlertInfo>(
    webView: BraveWebView,
    alert: inout T,
    completionHandler: @escaping () -> Void
  ) {
    guard let promptingTab = tabManager[webView], !promptingTab.blockAllAlerts else {
      suppressJSAlerts(webView: webView)
      tabManager[webView]?.cancelQueuedAlerts()
      completionHandler()
      return
    }
    promptingTab.alertShownCount += 1
    let suppressBlock: JSAlertInfo.SuppressHandler = { [unowned self] suppress in
      if suppress {
        func suppressDialogues(_: UIAlertAction) {
          self.suppressJSAlerts(webView: webView)
          promptingTab.blockAllAlerts = true
          self.tabManager[webView]?.cancelQueuedAlerts()
          completionHandler()
        }
        // Show confirm alert here.
        let suppressSheet = UIAlertController(
          title: nil,
          message: Strings.suppressAlertsActionMessage,
          preferredStyle: .actionSheet
        )
        suppressSheet.addAction(
          UIAlertAction(
            title: Strings.suppressAlertsActionTitle,
            style: .destructive,
            handler: suppressDialogues
          )
        )
        suppressSheet.addAction(
          UIAlertAction(
            title: Strings.cancelButtonTitle,
            style: .cancel,
            handler: { _ in
              completionHandler()
            }
          )
        )
        if UIDevice.current.userInterfaceIdiom == .pad,
          let popoverController = suppressSheet.popoverPresentationController
        {
          popoverController.sourceView = self.view
          popoverController.sourceRect = CGRect(
            x: self.view.bounds.midX,
            y: self.view.bounds.midY,
            width: 0,
            height: 0
          )
          popoverController.permittedArrowDirections = []
        }
        self.present(suppressSheet, animated: true)
      } else {
        completionHandler()
      }
    }
    alert.suppressHandler = promptingTab.alertShownCount > 1 ? suppressBlock : nil
    if shouldDisplayJSAlertForWebView(webView) {
      let controller = alert.alertController()
      controller.delegate = self
      present(controller, animated: true)
    } else {
      promptingTab.queueJavascriptAlertPrompt(alert)
    }
  }

  func checkIfWebContentProcessHasCrashed(_ webView: CWVWebView, error: NSError) -> Bool {
    if error.code == WKError.webContentProcessTerminated.rawValue
      && error.domain == "WebKitErrorDomain"
    {
      print("WebContent process has crashed. Trying to reload to restart it.")
      webView.reload()
      return true
    }

    return false
  }

  fileprivate func addTab(url: URL, inPrivateMode: Bool, currentTab: Tab) {
    let tab = self.tabManager.addTab(
      URLRequest(url: url),
      afterTab: currentTab,
      isPrivate: inPrivateMode
    )
    if inPrivateMode && !privateBrowsingManager.isPrivateBrowsing {
      self.tabManager.selectTab(tab)
    } else {
      // We're not showing the top tabs; show a toast to quick switch to the fresh new tab.
      let toast = ButtonToast(
        labelText: Strings.contextMenuButtonToastNewTabOpenedLabelText,
        buttonText: Strings.contextMenuButtonToastNewTabOpenedButtonText,
        completion: { buttonPressed in
          if buttonPressed {
            self.tabManager.selectTab(tab)
          }
        }
      )
      self.show(toast: toast)
    }
    self.toolbarVisibilityViewModel.toolbarState = .expanded
  }

  /// Get a possible redirect request from debouncing or query param stripping
  private func getInternalRedirect(
    from navigationAction: CWVNavigationAction,
    in tab: Tab,
    domainForMainFrame: Domain
  ) -> URLRequest? {
    guard let requestURL = navigationAction.request.url else { return nil }

    // For main frame only and if shields are enabled
    guard requestURL.isWebPage(includeDataURIs: false),
      domainForMainFrame.globalBlockAdsAndTrackingLevel.isEnabled,
      navigationAction.navigationType.isMainFrame
    else { return nil }

    // Handle Debounce
    // Only if the site (etld+1) changes
    // We also only handle `http` and `https` requests
    // Lets get the redirect chain.
    // Then we simply get all elements up until the user allows us to redirect
    // (i.e. appropriate settings are enabled for that redirect rule)
    if let debounceService = DebounceServiceFactory.get(privateMode: tab.isPrivate),
      debounceService.isEnabled,
      let currentURL = tab.webView?.lastCommittedURL,
      currentURL.baseDomain != requestURL.baseDomain
    {
      if let redirectURL = debounceService.debounce(requestURL) {
        // For now we only allow the `Referer`. The browser will add other headers during navigation.
        var modifiedRequest = URLRequest(url: redirectURL)

        // Also strip query params if debouncing
        modifiedRequest =
          modifiedRequest.stripQueryParams(
            initiatorURL: tab.committedURL,
            redirectSourceURL: requestURL,
            isInternalRedirect: false
          ) ?? modifiedRequest

        for (headerKey, headerValue) in navigationAction.request.allHTTPHeaderFields ?? [:] {
          guard headerKey == "Referer" else { continue }
          modifiedRequest.setValue(headerValue, forHTTPHeaderField: headerKey)
        }

        Self.log.debug(
          "Debouncing `\(requestURL.absoluteString)`"
        )

        return modifiedRequest
      }
    }

    // Handle query param stripping
    if let request = navigationAction.request.stripQueryParams(
      initiatorURL: tab.committedURL,
      redirectSourceURL: tab.redirectSourceURL,
      isInternalRedirect: tab.isInternalRedirect
    ) {
      Self.log.debug(
        "Stripping query params for `\(requestURL.absoluteString)`"
      )
      return request
    }

    // Attempt to upgrade to HTTPS
    if FeatureList.kBraveHttpsByDefault.enabled, ShieldPreferences.httpsUpgradeLevel.isEnabled,
      let upgradedURL = braveCore.httpsUpgradeExceptionsService.upgradeToHTTPS(for: requestURL)
    {
      guard tab.upgradedHTTPSRequest?.url?.baseDomain != requestURL.baseDomain else {
        // Avoid circular upgrades. This should be nil or we already tried to upgrade this
        // and somehow it went back to http (i.e. server side redirect).
        // We handle this as an invalid https upgrade right away
        tab.upgradedHTTPSRequest = navigationAction.request
        return handleInvalidHTTPSUpgrade(tab: tab, responseURL: upgradedURL)
      }
      Self.log.debug(
        "Upgrading `\(requestURL.absoluteString)` to HTTPS"
      )

      tab.upgradedHTTPSRequest = navigationAction.request
      var request = navigationAction.request
      request.url = upgradedURL
      return request
    }

    return nil
  }

  /// Upon an invalid response, check that we need to roll back any HTTPS upgrade
  /// or show the interstitial page
  private func handleInvalidHTTPSUpgrade(tab: Tab, responseURL: URL) -> URLRequest? {
    // Handle invalid upgrade to https
    guard responseURL.scheme == "https",
      let originalRequest = tab.upgradedHTTPSRequest,
      let originalURL = originalRequest.url,
      responseURL.baseDomain == originalURL.baseDomain
    else {
      return nil
    }

    if FeatureList.kHttpsOnlyMode.enabled, ShieldPreferences.httpsUpgradeLevel.isStrict,
      let url = originalURL.encodeEmbeddedInternalURL(for: .httpBlocked)
    {
      Self.log.debug(
        "Show http blocked interstitial for `\(originalURL.absoluteString)`"
      )

      let request = PrivilegedRequest(url: url) as URLRequest
      return request
    } else {
      Self.log.debug(
        "Revert HTTPS upgrade for `\(originalURL.absoluteString)`"
      )

      tab.upgradedHTTPSRequest = nil
      braveCore.httpsUpgradeExceptionsService.addException(for: originalURL)
      return originalRequest
    }
  }
}

extension P3ATimedStorage where Value == Int {
  fileprivate static var pagesLoadedStorage: Self { .init(name: "paged-loaded", lifetimeInDays: 7) }
}

extension URLRequest {
  /// Strip any query params in the request and return a new request if anything is stripped.
  ///
  /// The `isInternalRedirect` is a true value whenever we redirected the user for debouncing or query-stripping.
  /// It's an optimization because we assume that we stripped and debounced the user fully so there should be no further stripping on the next iteration.
  ///
  /// - Parameters:
  ///   - initiatorURL: The url page the user is coming from before any redirects
  ///   - redirectSourceURL: The last redirect url that happened (the true page the user is coming from)
  ///   - isInternalRedirect: Identifies if we have internally redirected or not. More info in the description
  /// - Returns: A modified request if any stripping is to occur.
  fileprivate func stripQueryParams(
    initiatorURL: URL?,
    redirectSourceURL: URL?,
    isInternalRedirect: Bool
  ) -> URLRequest? {
    guard let requestURL = url,
      let requestMethod = httpMethod
    else { return nil }

    guard
      let strippedURL = (requestURL as NSURL).applyingQueryFilter(
        initiatorURL: initiatorURL,
        redirectSourceURL: redirectSourceURL,
        requestMethod: requestMethod,
        isInternalRedirect: isInternalRedirect
      )
    else { return nil }

    var modifiedRequest = self
    modifiedRequest.url = strippedURL
    return modifiedRequest
  }
}
