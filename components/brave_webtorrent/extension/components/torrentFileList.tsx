/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import { Heading, Table } from 'brave-ui/components'
import { Cell, Row } from 'brave-ui/components/dataTables/table/index'
import { LoaderIcon } from 'brave-ui/components/icons'
import * as prettierBytes from 'prettier-bytes'

// Constants
import { File, TorrentObj } from '../constants/webtorrentState'

interface Props {
  torrentId: string
  torrent?: TorrentObj,
  onSaveAllFiles: (torrent: TorrentObj) => void
}

export default class TorrentFileList extends React.PureComponent<Props, {}> {
  render () {
    // const { torrent, torrentId, onSaveAllFiles } = this.props
    const { torrent, torrentId } = this.props
    if (!torrent || !torrent.files) {
      return (
        <div className='torrentSubhead'>
          <p className='starterText'>
            Click "Start Torrent" to begin your download.
          </p>
        </div>
      )
    }

    const header: Cell[] = [
      {
        content: '#'
      },
      {
        content: 'Name'
      },
      {
        content: 'Save File'
      },
      {
        content: 'Size'
      }
    ]

    const renderFileLink = (file: File, ix: number, isDownload: boolean) => {
      if (isDownload) {
        if (torrent.serverURL) {
          const url = torrent.serverURL + '/' + ix
          return (
            <a href={url} download={file.name}>
              ⇩
            </a>
          )
        } else {
          return <div /> // No download links until the server is ready
        }
      } else {
        // use # for .torrent links, since query params might cause the remote
        // server to return 404
        const suffix = /^https?:/.test(torrentId) ? '#ix=' + ix : '&ix=' + ix
        const href = torrentId + suffix
        return (
          <a href={href} target='_blank'>
            {' '}
            {file.name}{' '}
          </a>
        )
      }
    }

    const rows: Row[] = torrent.files.map((file: File, index: number) => {
      return {
        content: [
          {
            content: index + 1
          },
          {
            content: renderFileLink(file, index, false)
          },
          {
            content: renderFileLink(file, index, true)
          },
          {
            content: prettierBytes(file.length)
          }
        ]
      }
    })

    const saveAllFiles = () => {
      if (!torrent.serverURL || !torrent.files) return

      console.log('torrentFileList.tsx::saveAllFiles called')
      // @ts-ignore
      chrome.fileSystem.chooseEntry({
        type: 'openDirectory'
      }, onChooseEntry)

      function onChooseEntry (entry: any, fileEntries: any) {
        if (chrome.runtime.lastError) {
          console.error(chrome.runtime.lastError)
          // return
        }
        console.log(entry)
        console.log(fileEntries)
        // onSaveAllFiles(entry)
      }

      // @ts-ignore
      chrome.fileSystem.getVolumeList(onGetVolumeList)

      function onGetVolumeList (volumes: any) {
        if (chrome.runtime.lastError) {
          console.error(chrome.runtime.lastError)
          // return
        }
        console.log(volumes)

        // @ts-ignore
        chrome.fileSystem.requestFileSystem({
          volumeId: volumes[0],
          writable: true
        }, onRequestFileSystem)
      }

      function onRequestFileSystem (filesystem: any) {
        if (chrome.runtime.lastError) {
          console.error(chrome.runtime.lastError)
          // return
        }
        console.log(filesystem)
      }

      // torrent.files.forEach((file, ix) => {
      //   const url = torrent.serverURL + '/' + ix + '/' + file.name
      //   chrome.downloads.download({
      //     url: url,
      //     saveAs: false,
      //     conflictAction: "uniquify"
      //   }, (downloadId: number) => {
      //     console.log(downloadId)
      //   })
      // })
    }

    return (
      <div>
        <Heading children='Files' level={2} className='torrentHeading' />
        <a href='#' onClick={saveAllFiles}>
          Save All Files...
        </a>
        <Table header={header} rows={rows}>
          <div className='loadingContainer'>
            <div className='__icon'>
              <LoaderIcon />
            </div>
            Loading the torrent file list
          </div>
        </Table>
      </div>
    )
  }
}
