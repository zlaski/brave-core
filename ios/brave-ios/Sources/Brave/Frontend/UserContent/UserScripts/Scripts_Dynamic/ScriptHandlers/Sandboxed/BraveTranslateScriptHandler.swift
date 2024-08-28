// Copyright 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

import BraveCore
import Foundation
import NaturalLanguage
import Shared
import SwiftUI
import Translation
import WebKit
import os.log

protocol BraveTranslateScriptHandlerDelegate: NSObject {
  func updateTranslateURLBar(tab: Tab?, state: TranslateURLBarButton.TranslateState)
  func showTranslateToast(tab: Tab?, state: TranslateURLBarButton.TranslateState)
  func showTranslateOnboarding(tab: Tab?, completion: @escaping (_ translateEnabled: Bool) -> Void)
}

class BraveTranslateScriptLanguageDetectionHandler: NSObject, TabContentScript {
  private weak var tab: Tab?
  private weak var delegate: BraveTranslateScriptHandler?
  private static let namespace = "translate_\(uniqueID)"

  init(tab: Tab, delegate: BraveTranslateScriptHandler) {
    self.tab = tab
    self.delegate = delegate
    super.init()
  }

  static let scriptName = "BraveTranslateLanguageDetectionScript"
  static let scriptId = UUID().uuidString
  static let messageHandlerName = "LanguageDetectionTextCaptured"
  static let scriptSandbox = WKContentWorld.world(name: "BraveTranslateContentWorld")
  static let userScript: WKUserScript? = {
    return WKUserScript(
      source:
        "",
      injectionTime: .atDocumentStart,
      forMainFrameOnly: true,
      in: scriptSandbox
    )
  }()

  func userContentController(
    _ userContentController: WKUserContentController,
    didReceiveScriptMessage message: WKScriptMessage,
    replyHandler: @escaping (Any?, String?) -> Void
  ) {

    // In the future we'd get this from: components/language/ios/browser/language_detection_java_script_feature.mm

    guard let body = message.body as? [String: Any] else {
      Logger.module.error("Invalid Brave Translate Language Detection Message")
      return
    }

    do {
      let message = try JSONDecoder().decode(
        Message.self,
        from: JSONSerialization.data(withJSONObject: body, options: .fragmentsAllowed)
      )

      if message.hasNoTranslate {
        delegate?.currentLanguageInfo.pageLanguage = delegate?.currentLanguageInfo.currentLanguage
      } else {
        delegate?.currentLanguageInfo.pageLanguage = Locale.Language(identifier: message.htmlLang)
      }

      replyHandler(nil, nil)
    } catch {
      Logger.module.error("Brave Translate Language Detection Error: \(error)")
      replyHandler(nil, "Translation Language Detection Error")
    }
  }

  private struct Message: Codable {
    let frameId: String
    let hasNoTranslate: Bool
    let htmlLang: String
    let httpContentLanguage: String
  }
}

class BraveTranslateScriptHandler: NSObject, TabContentScript {
  private weak var tab: Tab?
  private weak var delegate: BraveTranslateScriptHandlerDelegate?
  private let recognizer = NLLanguageRecognizer()
  private static let namespace = "translate_\(uniqueID)"

  private var url: URL?
  private var isTranslationReady = false
  private var translationController: UIHostingController<BraveTranslateContainerView>!
  private var translationSession: BraveTranslateSession?
  private var urlObserver: NSObjectProtocol?
  fileprivate var currentLanguageInfo = BraveTranslateLanguageInfo()

  struct RequestMessage: Codable {
    let method: String
    let url: URL
    let headers: [String: String]
    let body: String
  }

  init(tab: Tab, delegate: BraveTranslateScriptHandlerDelegate) {
    self.tab = tab
    self.delegate = delegate
    self.url = tab.url
    super.init()

    translationController = UIHostingController(
      rootView: BraveTranslateContainerView(
        onTranslationSessionUpdated: { [weak self] session in
          self?.translationSession = session
        },
        languageInfo: currentLanguageInfo
      )
    )

    urlObserver = tab.webView?.observe(
      \.url,
      options: [.new],
      changeHandler: { [weak self] _, change in
        guard let self = self, let url = change.newValue else { return }
        if self.url != url {
          self.url = url
          self.isTranslationReady = false
          self.delegate?.updateTranslateURLBar(tab: self.tab, state: .unavailable)
        }
      }
    )
  }

  deinit {
    translationController?.willMove(toParent: nil)
    translationController?.removeFromParent()
    translationController = nil
  }

  static let scriptName = "BraveTranslateScript"
  static let scriptId = UUID().uuidString
  static let messageHandlerName = "TranslateMessage"
  static let scriptSandbox = WKContentWorld.world(name: "BraveTranslateContentWorld")
  static let userScript: WKUserScript? = {
    guard var script = loadUserScript(named: scriptName) else {
      return nil
    }

    // HACKS! Need a better way to do this.
    // Chromium Scripts do NOT have a secure message handler and cannot be sandboxed the same way!
    return WKUserScript(
      source:
        script
        .replacingOccurrences(of: "$<brave_translate_script>", with: namespace)
        .replacingOccurrences(
          of: "$<brave_translate_api_key>",
          with: kBraveServicesKey
        )
        .replacingOccurrences(of: "$<message_handler>", with: messageHandlerName),
      injectionTime: .atDocumentEnd,
      forMainFrameOnly: true,
      in: scriptSandbox
    )
  }()

  @MainActor
  private func executePageFunction(name functionName: String) async -> String? {
    guard let webView = tab?.webView else { return nil }

    let (result, error) = await webView.evaluateSafeJavaScript(
      functionName: "window.__firefox__.\(BraveTranslateScriptHandler.namespace).\(functionName)",
      contentWorld: BraveTranslateScriptHandler.scriptSandbox,
      asFunction: true
    )

    if let error = error {
      Logger.module.error("Unable to execute page function \(functionName) error: \(error)")
      return nil
    }

    guard let result = result as? String else {
      Logger.module.error("Invalid Page Result")
      return nil
    }

    return result
  }

  @MainActor
  @discardableResult
  func executeChromiumFunction(_ functionName: String, args: [Any] = []) async -> Any? {
    guard let webView = tab?.webView else { return nil }

    let (result, error) = await webView.evaluateSafeJavaScript(
      functionName: "window.__gCrWeb.\(functionName)",
      args: args,
      contentWorld: BraveTranslateScriptHandler.scriptSandbox,
      asFunction: true
    )

    if let error = error {
      Logger.module.error("Unable to execute page function \(functionName) error: \(error)")
      return nil
    }

    return result
  }

  private func getPageSource() async -> String? {
    return await executePageFunction(name: "getPageSource")
  }

  @MainActor
  func guessLanguage() async -> Locale.Language? {
    //    await executeChromiumFunction("languageDetection.detectLanguage")
    //
    //    // Language was identified by the Translate Script
    //    if let currentLanguage = currentLanguageInfo.currentLanguage.languageCode?.identifier,
    //      let pageLanguage = currentLanguageInfo.pageLanguage?.languageCode?.identifier
    //    {
    //      if currentLanguage == pageLanguage {
    //        currentLanguageInfo.pageLanguage = nil
    //      }
    //      return currentLanguageInfo.pageLanguage
    //    }

    // Language identified via our own Javascript
    if let languageCode = await executePageFunction(name: "getPageLanguage") {
      return Locale.Language(identifier: languageCode)
    }

    // Language identified by running the NL detection on the page source
    if let rawPageSource = await executePageFunction(name: "getRawPageSource") {
      recognizer.reset()
      recognizer.processString(rawPageSource)

      if let dominantLanguage = recognizer.dominantLanguage, dominantLanguage != .undetermined {
        return Locale.Language(identifier: dominantLanguage.rawValue)
      }
    }

    return nil
  }

  func getLanguageInfo() async -> BraveTranslateLanguageInfo {
    let pageLanguage = await guessLanguage()
    return .init(currentLanguage: Locale.current.language, pageLanguage: pageLanguage)
  }

  func startTranslation() {
    Task { @MainActor [weak self] in
      guard let self = self,
        let currentLanguage = currentLanguageInfo.currentLanguage.languageCode?.identifier,
        let pageLanguage = currentLanguageInfo.pageLanguage?.languageCode?.identifier,
        currentLanguage != pageLanguage
      else {
        return
      }

      // Normalize Chinese if necessary to Chinese (Simplified). zh-TW = Traditional
      // We don't current use the components/language/ios/browser/language_detection_java_script_feature.mm
      // Supported List of Languages is from: components/translate/core/browser/translate_language_list.cc
      // So we have to do this for now.
      await executeChromiumFunction(
        "translate.startTranslation",
        args: [
          pageLanguage == "zh" ? "zh-CN" : pageLanguage,
          currentLanguage == "zh" ? "zh-CN" : currentLanguage,
        ]
      )

      self.delegate?.updateTranslateURLBar(tab: self.tab, state: .active)
    }
  }

  func revertTranslation() {
    Task { @MainActor [weak self] in
      guard let self = self,
        self.isTranslationReady,
        self.tab?.translationState == .active
      else {
        return
      }

      await executeChromiumFunction("translate.revertTranslation")
      self.delegate?.updateTranslateURLBar(tab: self.tab, state: .available)
    }
  }

  func presentUI(on controller: UIViewController) {
    guard isTranslationReady else {
      // TODO: Show Error to the user!
      return
    }

    if translationController?.parent != controller {
      controller.addChild(translationController)
      tab?.webView?.addSubview(translationController.view)
      translationController.didMove(toParent: controller)
      tab?.webView?.sendSubviewToBack(translationController.view)
    }
  }

  func userContentController(
    _ userContentController: WKUserContentController,
    didReceiveScriptMessage message: WKScriptMessage,
    replyHandler: @escaping (Any?, String?) -> Void
  ) {
    guard translationController != nil else {
      Logger.module.error("Invalid Translation Controller")
      return
    }

    guard let body = message.body as? [String: Any] else {
      Logger.module.error("Invalid Brave Translate Message")
      return
    }

    if body["command"] as? String == "request" {
      Task {
        do {
          let message = try JSONDecoder().decode(
            RequestMessage.self,
            from: JSONSerialization.data(withJSONObject: body, options: .fragmentsAllowed)
          )

          let translationSession = self.translationSession ?? BraveTranslateSession()

          // The message is for HTML or CSS request
          if self.translationSession == nil
            && BraveTranslateSession.isPhraseTranslationRequest(message)
          {
            throw BraveTranslateError.otherError
          }

          let (data, response) = try await translationSession.translate(message)

          guard let response = response as? HTTPURLResponse else {
            throw BraveTranslateError.invalidTranslationResponse
          }

          replyHandler(
            [
              "value": [
                "statusCode": response.statusCode,
                "responseType": "",
                "response": String(data: data, encoding: .utf8) ?? "",
                "headers": response.allHeaderFields.map({ "\($0): \($1)" }).joined(
                  separator: "\r\n"
                ),
              ]
            ],
            nil
          )
        } catch {
          Logger.module.error("Brave Translate Error: \(error)")
          replyHandler(nil, "Translation Error")
        }
      }

      return
    }

    if body["command"] as? String == "ready" {
      isTranslationReady = true

      Task { @MainActor [weak self] in
        guard let self = self,
          let delegate = self.delegate
        else {
          return
        }

        //        await executeChromiumFunction("translate.installCallbacks")

        try Task.checkCancellation()
        let languageInfo = await getLanguageInfo()

        try Task.checkCancellation()
        self.currentLanguageInfo.pageLanguage = languageInfo.pageLanguage
        self.currentLanguageInfo.currentLanguage = languageInfo.currentLanguage

        try Task.checkCancellation()

        guard let pageLanguage = languageInfo.pageLanguage?.languageCode?.identifier,
          let currentLanguage = languageInfo.currentLanguage.languageCode?.identifier,
          pageLanguage != currentLanguage
        else {
          delegate.updateTranslateURLBar(tab: self.tab, state: .unavailable)
          delegate.showTranslateToast(tab: self.tab, state: .unavailable)
          return
        }

        delegate.updateTranslateURLBar(tab: self.tab, state: .available)
        delegate.showTranslateToast(tab: self.tab, state: .available)
      }
    }

    if body["command"] as? String == "status" {
      print(body)
    }

    replyHandler(nil, nil)
  }
}

private enum BraveTranslateError: Error {
  case invalidURL
  case invalidLanguage
  case sameLanguage
  case invalidPageSource
  case invalidTranslationResponseCode
  case invalidTranslationResponse
  case otherError
}

class BraveTranslateLanguageInfo: NSObject, ObservableObject {
  @Published
  var currentLanguage: Locale.Language

  @Published
  var pageLanguage: Locale.Language?

  override init() {
    currentLanguage = .init(identifier: Locale.current.identifier)
    pageLanguage = nil
    super.init()
  }

  init(currentLanguage: Locale.Language, pageLanguage: Locale.Language?) {
    self.currentLanguage = currentLanguage
    self.pageLanguage = pageLanguage
    super.init()
  }
}

private class BraveTranslateSession {
  static func isPhraseTranslationRequest(
    _ request: BraveTranslateScriptHandler.RequestMessage
  ) -> Bool {
    return request.url.path.starts(with: "/translate_a/")
  }

  func translate(
    _ request: BraveTranslateScriptHandler.RequestMessage
  ) async throws -> (data: Data, response: URLResponse) {
    var urlRequest = URLRequest(url: request.url)
    urlRequest.httpMethod = request.method
    urlRequest.httpBody = request.body.data(using: .utf8)
    request.headers.forEach { (key, value) in
      urlRequest.setValue(value, forHTTPHeaderField: key)
    }

    let session = URLSession(configuration: .ephemeral)
    defer { session.finishTasksAndInvalidate() }
    return try await session.data(for: urlRequest)
  }
}

#if compiler(>=6.0) && !targetEnvironment(simulator)
@available(iOS 18.0, *)
private class BraveTranslateSessionApple: BraveTranslateSession {
  private weak var session: TranslationSession?

  init(session: TranslationSession) {
    self.session = session
  }

  override func translate(
    _ request: BraveTranslateScriptHandler.RequestMessage
  ) async throws -> (data: Data, response: URLResponse) {
    // Do not attempt to translate requests to html and css from Brave-Translate script
    guard Self.isPhraseTranslationRequest(request) else {
      return try await super.translate(request)
    }

    guard let session = session else {
      throw BraveTranslateError.otherError
    }

    let components = URLComponents(string: "https://translate.brave.com?\(request.body)")
    let phrases = components!.queryItems!.map({ "\($0.value ?? "")" })
    let results = try await session.translations(
      from: phrases.map({ .init(sourceText: $0) })
    ).map({ $0.targetText })

    let data = try JSONSerialization.data(withJSONObject: results, options: [])
    let response =
      HTTPURLResponse(
        url: request.url,
        statusCode: 200,
        httpVersion: "HTTP/1.1",
        headerFields: ["Content-Type": "text/html"]
      )
      ?? URLResponse(
        url: request.url,
        mimeType: "text/html",
        expectedContentLength: data.count,
        textEncodingName: "UTF-8"
      )
    return (data, response)
  }
}
#endif

private struct BraveTranslateContainerView: View {
  var onTranslationSessionUpdated: ((BraveTranslateSession) -> Void)?

  @ObservedObject
  var languageInfo: BraveTranslateLanguageInfo

  var body: some View {
    Color.clear
      .osAvailabilityModifiers({ view in
        if #available(iOS 18.0, *) {
          #if compiler(>=6.0) && !targetEnvironment(simulator)
          view
            .translationTask(
              .init(source: languageInfo.pageLanguage, target: languageInfo.currentLanguage),
              action: { session in
                onTranslationSessionUpdated?(BraveTranslateSessionApple(session: session))
              }
            )
          #else
          view.task {
            onTranslationSessionUpdated?(BraveTranslateSession())
          }
          #endif
        } else {
          view.task {
            onTranslationSessionUpdated?(BraveTranslateSession())
          }
        }
      })
  }
}
