/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_QR_BROWSER_QR_UTIL_H_
#define BRAVE_COMPONENTS_QR_BROWSER_QR_UTIL_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "chrome/services/qrcode_generator/public/cpp/qrcode_generator_service.h"

namespace qr_util {

class QRUtil {
 public:
  QRUtil();
  ~QRUtil();

  using GetQRCodeCallback = base::OnceCallback<void(const std::string&)>;

  bool GetQRCode(const std::string& text,
      GetQRCodeCallback callback);

 private:
  bool TextToBytes32(const std::string& text,
      std::vector<uint8_t>* bytes);

  void OnGetQRCode(GetQRCodeCallback callback,
      const qrcode_generator::mojom::GenerateQRCodeResponsePtr response);

  mojo::Remote<qrcode_generator::mojom::QRCodeGeneratorService>
      qr_code_service_remote_;

  base::WeakPtrFactory<QRUtil> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(QRUtil);
};

}  // namespace qr_util

#endif  // BRAVE_COMPONENTS_QR_BROWSER_QR_UTIL_H_
