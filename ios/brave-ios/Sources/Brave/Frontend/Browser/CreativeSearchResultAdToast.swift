// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

import DesignSystem
import Foundation
import Shared
import SnapKit
import SwiftUI
import UIKit

struct CreativeSearchResultAdToastUX {
  static let toastHeight: CGFloat = 100.0
  static let toastPadding: CGFloat = 10.0
  static let toastCloseButtonWidth: CGFloat = 12.0
  static let toastLabelFont = UIFont.systemFont(ofSize: 15, weight: .semibold)
  static let toastBackgroundColor = UIColor(braveSystemName: .schemesOnPrimaryFixed)
  static let learnMoreUrl = "https://brave.com"
}

class CreativeSearchResultAdToast: Toast, UITextViewDelegate {
  let tabManager: TabManager

  init(tabManager: TabManager) {
    self.tabManager = tabManager

    super.init(frame: .zero)

    self.clipsToBounds = true
    self.addSubview(
      createView(
        Strings.searchResultAdsClickedToastTitle + " "
      )
    )

    self.toastView.backgroundColor = CreativeSearchResultAdToastUX.toastBackgroundColor

    self.toastView.snp.makeConstraints { make in
      make.left.right.height.equalTo(self)
      self.animationConstraint =
        make.top.equalTo(self).offset(CreativeSearchResultAdToastUX.toastHeight).constraint
    }

    self.snp.makeConstraints { make in
      make.height.equalTo(CreativeSearchResultAdToastUX.toastHeight)
    }
  }

  required init?(coder aDecoder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  fileprivate func createView(
    _ labelText: String
  ) -> UIView {
    let horizontalStackView = UIStackView()
    horizontalStackView.axis = .horizontal
    horizontalStackView.alignment = .center
    horizontalStackView.spacing = CreativeSearchResultAdToastUX.toastPadding

    let label = UITextView()
    label.textAlignment = .left
    label.textColor = .white
    label.font = CreativeSearchResultAdToastUX.toastLabelFont
    label.backgroundColor = CreativeSearchResultAdToastUX.toastBackgroundColor
    label.isEditable = false
    label.isScrollEnabled = false
    label.isSelectable = true
    label.delegate = self

    let learnMoreText = Strings.learnMore.withNonBreakingSpace
    let attributes: [NSAttributedString.Key: Any] = [
      .foregroundColor: UIColor.white,
      .font: CreativeSearchResultAdToastUX.toastLabelFont,
    ]

    let linkAttributes: [NSAttributedString.Key: Any] = [
      .font: CreativeSearchResultAdToastUX.toastLabelFont,
      .foregroundColor: UIColor.white,
      .underlineStyle: 1,
    ]
    label.linkTextAttributes = linkAttributes

    let nsLabelAttributedString = NSMutableAttributedString(
      string: labelText,
      attributes: attributes
    )
    let nsLinkAttributedString = NSMutableAttributedString(
      string: learnMoreText,
      attributes: linkAttributes
    )

    if let url = URL(string: CreativeSearchResultAdToastUX.learnMoreUrl) {
      let learnMoreRange = NSRange(location: 0, length: learnMoreText.count)
      nsLinkAttributedString.addAttribute(.link, value: url, range: learnMoreRange)
      nsLabelAttributedString.append(nsLinkAttributedString)
      label.isUserInteractionEnabled = true
    }
    label.attributedText = nsLabelAttributedString

    horizontalStackView.addArrangedSubview(label)

    if let buttonImage = UIImage(braveSystemNamed: "leo.close") {
      let button = UIButton()
      button.setImage(buttonImage, for: .normal)
      button.imageView?.contentMode = .scaleAspectFit
      button.imageView?.tintColor = .white
      button.imageView?.preferredSymbolConfiguration = .init(
        font: .preferredFont(for: .title3, weight: .regular),
        scale: .small
      )

      button.imageView?.snp.makeConstraints {
        $0.width.equalTo(CreativeSearchResultAdToastUX.toastCloseButtonWidth)
      }

      button.addGestureRecognizer(
        UITapGestureRecognizer(target: self, action: #selector(buttonPressed))
      )

      horizontalStackView.addArrangedSubview(button)
    }

    toastView.addSubview(horizontalStackView)

    horizontalStackView.snp.makeConstraints { make in
      make.centerX.equalTo(toastView)
      make.centerY.equalTo(toastView)
      make.width.equalTo(toastView.snp.width).offset(
        -2 * CreativeSearchResultAdToastUX.toastPadding
      )
    }

    return toastView
  }

  func textView(
    _ textView: UITextView,
    shouldInteractWith url: URL,
    in characterRange: NSRange,
    interaction: UITextItemInteraction
  ) -> Bool {
    self.tabManager.addTabAndSelect(
      URLRequest(url: URL(string: CreativeSearchResultAdToastUX.learnMoreUrl)!),
      isPrivate: false
    )
    dismiss(true)
    return false
  }

  @objc func buttonPressed(_ gestureRecognizer: UIGestureRecognizer) {
    dismiss(true)
  }

  override func showToast(
    viewController: UIViewController? = nil,
    delay: DispatchTimeInterval = SimpleToastUX.toastDelayBefore,
    duration: DispatchTimeInterval?,
    makeConstraints: @escaping (ConstraintMaker) -> Void,
    completion: (() -> Void)? = nil
  ) {
    super.showToast(
      viewController: viewController,
      delay: delay,
      duration: duration,
      makeConstraints: makeConstraints
    )
  }
}
