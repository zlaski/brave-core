# Copyright (c) 2019 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# you can obtain one at http://mozilla.org/MPL/2.0/.

import os

def url_mappings(src_path):
  br_resources_path = os.path.join(src_path, 'brave', 'ui', 'webui', 'resources')
  return [
    ('chrome://brave-resources/', br_resources_path)
  ]
