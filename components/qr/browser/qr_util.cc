/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include <utility>

#include "base/strings/string_number_conversions.h"
#include "brave/components/qr/browser/qr_util.h"
#include "brave/vendor/bip39wally-core-native/include/wally_bip39.h"
#include "ui/base/webui/web_ui_util.h"

namespace qr_util {

QRUtil::QRUtil() : weak_ptr_factory_(this) {
}

QRUtil::~QRUtil() {
}

bool QRUtil::GetQRCode(const std::string& text,
                       QRUtil::GetQRCodeCallback callback) {
  std::vector<uint8_t> seed;

  if (!QRUtil::TextToBytes32(text, &seed)) {
    return false;
  }

  qr_code_service_remote_ = qrcode_generator::LaunchQRCodeGeneratorService();
  qr_code_service_remote_.set_disconnect_handler(
      base::BindOnce(&QRUtil::OnGetQRCode,
                     weak_ptr_factory_.GetWeakPtr(),
                     std::move(callback), nullptr));
  qrcode_generator::mojom::QRCodeGeneratorService* generator =
      qr_code_service_remote_.get();
  qrcode_generator::mojom::GenerateQRCodeRequestPtr request =
      qrcode_generator::mojom::GenerateQRCodeRequest::New();

  const std::string text_hex = base::HexEncode(seed.data(), seed.size());
  request->data = text_hex;
  request->should_render = true;
  request->render_dino = false;
  request->render_module_style =
      qrcode_generator::mojom::ModuleStyle::DEFAULT_SQUARES;
  request->render_locator_style =
      qrcode_generator::mojom::LocatorStyle::DEFAULT_SQUARE;

  generator->GenerateQRCode(
      std::move(request),
      base::BindOnce(&QRUtil::OnGetQRCode,
                     weak_ptr_factory_.GetWeakPtr(),
                     std::move(callback)));

  return true;
}

bool QRUtil::TextToBytes32(const std::string& text,
                           std::vector<uint8_t>* bytes) {
  DCHECK(bytes);

  size_t written;
  const size_t kDefaultSeedSize = 32;

  bytes->resize(kDefaultSeedSize);
  if (bip39_mnemonic_to_bytes(nullptr, text.c_str(), bytes->data(),
                              bytes->size(), &written) != WALLY_OK) {
    return false;
  }

  return true;
}

void QRUtil::OnGetQRCode(QRUtil::GetQRCodeCallback callback,
    const qrcode_generator::mojom::GenerateQRCodeResponsePtr response) {
  const std::string data_url = webui::GetBitmapDataUrl(response->bitmap);
  qr_code_service_remote_.reset();
  // callback(base::Value(data_url));
}

}  // namespace qr_util
