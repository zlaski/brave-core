/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

 import * as React from 'react'
 
 export interface Props {
    onActivity?: () => void
    url: string | null
  }

 export default class AdaptiveCaptcha extends React.PureComponent<Props, {}> {


  generateCaptchaURL() {
    const url = this.props.url;
    if (url === null) {
      return ""
    } else {
      return url + "?" + Date.now();
    }
  }

  render() {

    const reloadHandler = () => {
      window.location.reload();
    }
    window.addEventListener("message", reloadHandler);

    return (
          <span>
            <iframe
              id="captchaWindow"
              src={this.generateCaptchaURL()}
              width="100%" height="100%"
              frameBorder="0"
            ></iframe>
          </span>
        );
    }
}