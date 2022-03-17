/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import Table, { Cell, Row } from 'brave-ui/components/dataTables/table/index'
import Profile, { Provider } from '../profile/index'
import Tokens from '../tokens/index'
import { getLocale } from 'brave-ui/helpers'
import { TrashIcon } from 'brave-ui/components/icons'

import * as style from './style'

interface ProfileCell {
  verified: boolean
  name: string
  src: string
  provider?: Provider
}

type DonationType = 'donation' | 'tip' | 'recurring'

export interface DetailRow {
  profile: ProfileCell
  contribute: {
    tokens: string
    converted: string
  }
  url: string
  type: DonationType
  text?: string | React.ReactNode
  onRemove?: () => void
}

export interface Props {
  id?: string
  children?: React.ReactNode
  rows?: DetailRow[]
  numItems?: number
  allItems?: boolean
  onShowAll?: () => void
  headerColor?: boolean
}

export default class TableDonation extends React.PureComponent<Props, {}> {
  getTypeContent (row: DetailRow): Cell {
    switch (row.type) {
      case 'recurring':
        return {
          content: (
            <>
              <style.type>{getLocale('recurring')}</style.type>
              <style.remove>
                <button onClick={row.onRemove}>
                  <style.removeIcon><TrashIcon /></style.removeIcon>{getLocale('remove')}
                </button>
              </style.remove>
            </>
          )
        }
      case 'donation':
        return {
          content: (
            <>
              <style.tipDate>{row.text}</style.tipDate>
            </>
          )
        }
      case 'tip':
        return {
          content: (
            <>
              <style.type>{getLocale('tipOnLike')}</style.type>
              <style.date>{row.text}</style.date>
            </>
          )
        }
    }
  }

  getRows (rows?: DetailRow[]): Row[] | undefined {
    if (!rows) {
      return
    }

    return rows.map((row: DetailRow): Row => {
      return {
        content: [
          {
            content: (
              <style.link>
                <a
                  href={row.url}
                  target={'_blank'}
                  rel='noopener noreferrer'
                >
                  <Profile
                    title={row.profile.name}
                    provider={row.profile.provider}
                    verified={row.profile.verified}
                    src={row.profile.src}
                  />
                </a>
              </style.link>
            )
          },
          this.getTypeContent(row),
          {
            content: (
              <Tokens
                value={row.contribute.tokens}
                converted={row.contribute.converted}
                size={'small'}
              />
            ),
            customStyle: {
              'text-align': 'right',
              'padding': '0 7px 0 10px',
              'max-width': '80px'
            }
          }
        ]
      }
    })
  }

  get headers (): Cell[] {
    const { headerColor } = this.props

    let customStyle = {}

    if (headerColor) {
      customStyle = {
        'border': 'none',
        'border-bottom': '1px solid #696FDC',
        'padding': '8px 0',
        'color': '#696FDC'
      }
    }

    return [
      {
        content: getLocale('site'),
        customStyle
      },
      {
        content: getLocale('date'),
        customStyle
      },
      {
        content: getLocale('tokens'),
        customStyle: Object.assign({
          'text-align': 'right',
          'padding-right': '7px'
        }, customStyle)
      }
    ]
  }

  render () {
    const { id, children, rows, allItems, onShowAll } = this.props
    const numItems = this.props.numItems || 0

    return (
      <div id={id}>
        <Table
          children={children}
          rows={this.getRows(rows)}
          header={this.headers}
        />
        {
          !allItems && numItems > 0
            ? <style.toggleWrap>
              <button onClick={onShowAll}>
                {getLocale('seeAllItems', { numItems })}
              </button>
            </style.toggleWrap>
            : null
        }
      </div>
    )
  }
}
