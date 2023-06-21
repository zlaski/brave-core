// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { render } from 'react-dom'
// import { Provider } from 'react-redux'
import { initLocale } from 'brave-ui'
import { BrowserRouter, NavLink, Redirect, Route, Switch } from 'react-router-dom'

// assets
import faveiconUrl from '../assets/svg-icons/brave-icon.svg'

// utils
import { loadTimeData } from '../../common/loadTimeData'
// import * as Lib from '../common/async/lib'
import { removeDeprecatedLocalStorageKeys } from '../common/constants/local-storage-keys'

// actions
// import * as WalletActions from '../common/actions/wallet_actions'

// contexts
// import { LibContext } from '../common/context/lib.context'
// import { ApiProxyContext } from '../common/context/api-proxy.context'

// components
// import BraveCoreThemeProvider from '../../common/BraveCoreThemeProvider'
// import Container from './container'
// import {
//   store,
//   // walletPageApiProxy
// } from './store'

// style
// import walletDarkTheme from '../theme/wallet-dark'
// import walletLightTheme from '../theme/wallet-light'
import 'emptykit.css'

import { setIconBasePath } from '@brave/leo/react/icon'
import { WalletRoutes } from '../constants/types'
setIconBasePath('chrome://resources/brave-icons')

function App () {
  // const [
    // initialThemeType,
    // setInitialThemeType] = React.useState<chrome.braveTheme.ThemeType>()
  // React.useEffect(() => {
  //   chrome.braveTheme.getBraveThemeType(setInitialThemeType)
  // }, [])

  React.useEffect(() => {
    /** Sets FAVICON for Wallet Pages */
    let link = document.querySelector("link[rel~='icon']") as HTMLLinkElement
    if (!link) {
      link = document.createElement('link')
      link.rel = 'icon'
      document.getElementsByTagName('head')[0].appendChild(link)
    }
    link.href = faveiconUrl
  }, [])

  React.useEffect(() => {
    removeDeprecatedLocalStorageKeys()
  }, [])

  console.log({
    history,
    location: window.location
  })

  return (
    <div style={{ height: '100vh', width: '100vw', color: 'black' }}>
    APP
      <BrowserRouter>
        <Routes />
      </BrowserRouter>
    </div>
  )
}

function Routes() {
  // const history = useHistory()

  return (
    <Switch>
      <Route path={WalletRoutes.Unlock} exact>
        Unlock
        <NavLink
          to={WalletRoutes.Portfolio}
          key={WalletRoutes.Portfolio}
          isActive={() => false}
          activeClassName='active'
        >
          Unlock
        </NavLink>
        {/* <button
          onClick={() => {
            alert('unlock')
            history.push(WalletRoutes.Portfolio)
          }}
        >
          Unlock
        </button> */}
      </Route>
      <Route path={WalletRoutes.Portfolio} exact>
        Portfolio
        <NavLink
          to={WalletRoutes.Accounts}
          key={WalletRoutes.Accounts}
          isActive={() => false}
          activeClassName='active'
        >
          Accounts
        </NavLink>
        {/* <button onClick={() => history.push(WalletRoutes.Accounts)}>
          Accounts
        </button> */}
      </Route>
      <Route path={WalletRoutes.Accounts} exact>
        Accounts
        {/* <button onClick={() => history.push(WalletRoutes.Portfolio)}>
          Portfolio
        </button> */}
        <NavLink
          to={WalletRoutes.Portfolio}
          key={WalletRoutes.Portfolio}
          isActive={() => false}
          activeClassName='active'
        >
          Portfolio
        </NavLink>
      </Route>
      <Redirect to={WalletRoutes.Unlock} />
    </Switch>
  )
}

function initialize () {
  initLocale(loadTimeData.data_)
  // store.dispatch(WalletActions.initialize({}))
  render(<App />, document.getElementById('root'))
}

document.addEventListener('DOMContentLoaded', initialize)
