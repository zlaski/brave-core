// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.


const config = require('./config')
const fs = require('fs/promises');
const path = require('path')
const util = require('util');
const exec = util.promisify(require('child_process').exec);

// This function is used to modify build/config/unsafe_buffers_paths.txt, to
// address the issue that the Brave build uses the `-I../../..` flag, which
// allows `src/` files to be included. This alters how `clang` calculates the
// root of the source tree. With this function, all entries in
// unsafe_buffers_paths.txt are duplicated at the end with `src/` prepended to
// them.
async function updateUnsafeBuffersPaths() {
  const unsafeBuffersPath = ['build', 'config', 'unsafe_buffers_paths.txt'];

  // The contents of `unsafe_buffers_paths.txt` are read straight from the
  // repository, permitting to check if any update to the buffers is needed, and
  // avoiding duplicated patching.
  const {stdout, stderr} = await exec(
      [
        'git', '-C', config.srcDir, 'show',
        'HEAD:' + unsafeBuffersPath.join('/')
      ].join(' '),
      {encoding: 'utf8'});

  // The files contents.
  const bufferPaths = stdout.trim();

  // txt files use eol=lf in .gitattributes.
  bufferPathLines = bufferPaths.split('\n')

  // Adding exlusion paths for brave as well, which are also to be corrected
  // for src prefix
  bufferPathLines.push(
      ...['', '# Appended path correction for redirect_cc by update_patches.',
          '# File patched by brave/build/commands/lib/updateUnsafeBuffersPaths.js.',
          '-brave/'])

  updatedPathLines = bufferPathLines.filter(
      (line) => line.startsWith('-') || line.startsWith('+'));
  updatedPathLines = updatedPathLines.map(
      (line) => {return line.slice(0, 1) + 'src/' + line.slice(1)});

  updatedContents = [...bufferPathLines, ...updatedPathLines, ''].join('\n');

  // To avoid invalidating a lot of objects that might already have been built,
  // the file is only updated if there are any changes between the proposed and
  // current contents.
  absoluteFilePath = path.join(config.srcDir, ...unsafeBuffersPath)
  currentFileContents = await fs.readFile(absoluteFilePath, {encoding: 'utf8'})
  if (currentFileContents !== updatedContents) {
    await fs.writeFile(absoluteFilePath, updatedContents)
  }
}

module.exports = updateUnsafeBuffersPaths
