# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.

import os
import sys

import brave_node
import chromium_presubmit_overrides
import git_cl
import override_utils

USE_PYTHON3 = True
PRESUBMIT_VERSION = '2.0.0'

# pylint: disable=line-too-long


# Adds support for chromium_presubmit_config.json5 and some helpers.
def CheckToModifyInputApi(input_api, _output_api):
    chromium_presubmit_overrides.modify_input_api(input_api)
    return []


def CheckPatchFormatted(input_api, output_api):
    # Use git cl format to format supported files with Chromium formatters.
    git_cl_format_cmd = [
        '-C',
        input_api.change.RepositoryRoot(),
        'cl',
        'format',
        '--presubmit',
        '--python',
    ]

    # Make sure the passed --upstream branch is applied to git cl format.
    if input_api.change.UpstreamBranch():
        git_cl_format_cmd.extend(
            ['--upstream', input_api.change.UpstreamBranch()])

    # Do a dry run if --fix was not passed.
    if not input_api.PRESUBMIT_FIX:
        git_cl_format_cmd.append('--dry-run')

    # Pass a path where the current PRESUBMIT.py file is located.
    git_cl_format_cmd.append(input_api.PresubmitLocalPath())

    # Run git cl format and get return code.
    git_cl_format_code, _ = git_cl.RunGitWithCode(
        git_cl_format_cmd, suppress_stderr=not input_api.PRESUBMIT_FIX)

    is_format_required = git_cl_format_code == 2

    if not is_format_required or input_api.PRESUBMIT_FIX:
        # Use prettier to format other files.
        prettier_files_to_check = (
            r'.+\.ts$',
            r'.+\.js$',
        )
        prettier_files_to_skip = input_api.DEFAULT_FILES_TO_SKIP

        prettier_sources = lambda affected_file: input_api.FilterSourceFile(
            affected_file,
            files_to_check=prettier_files_to_check,
            files_to_skip=prettier_files_to_skip)

        node_args = [
            brave_node.PathInNodeModules('prettier', 'bin-prettier'),
            '--write' if input_api.PRESUBMIT_FIX else '--check',
        ]

        def PerformPrettierAction(files):
            try:
                brave_node.RunNode(node_args + files)
                return True
            except RuntimeError as err:
                if 'Forgot to run Prettier?' in str(err):
                    return False
                # Raise on unexpected output. Could be node or prettier issues.
                raise

        if sys.platform == 'win32':
            # Run prettier per file on Windows, because command line length is
            # restricted.
            for f in input_api.AffectedSourceFiles(prettier_sources):
                if not PerformPrettierAction([f.AbsoluteLocalPath()]):
                    is_format_required = True
                    break
        else:
            is_format_required = not PerformPrettierAction([
                f.AbsoluteLocalPath()
                for f in input_api.AffectedSourceFiles(prettier_sources)
            ])

    if is_format_required:
        if input_api.PRESUBMIT_FIX:
            raise RuntimeError('--fix was passed, but format has failed')
        short_path = input_api.basename(input_api.change.RepositoryRoot())
        return [
            output_api.PresubmitError(
                f'The {short_path} directory requires source formatting. '
                'Please run: npm run presubmit -- --fix')
        ]
    return []


def CheckChangeLintsClean(input_api, output_api):
    return input_api.canned_checks.CheckChangeLintsClean(input_api,
                                                         output_api,
                                                         lint_filters=[])


def CheckPylint(input_api, output_api):
    extra_paths_list = os.environ['PYTHONPATH'].split(';' if sys.platform ==
                                                      'win32' else ':')
    return input_api.canned_checks.RunPylint(
        input_api,
        output_api,
        pylintrc=input_api.os_path.join(input_api.PresubmitLocalPath(),
                                        '.pylintrc'),
        extra_paths_list=extra_paths_list,
        version='2.7')


def CheckLicense(input_api, output_api):
    """Verifies the Brave license header."""

    files_to_check = input_api.DEFAULT_FILES_TO_CHECK + (r'.+\.gni?$', )
    files_to_skip = input_api.DEFAULT_FILES_TO_SKIP + (
        r"\.storybook/",
        r"ios/browser/api/ledger/legacy_database/core_data_models/",
        r'win_build_output/',
    )

    current_year = int(input_api.time.strftime('%Y'))
    allowed_years = (str(s) for s in reversed(range(2015, current_year + 1)))
    years_re = '(' + '|'.join(allowed_years) + ')'

    # License regexp to match in NEW and MOVED files, it doesn't allow variance.
    # Note: presubmit machinery cannot distinguish between NEW and MOVED files,
    # that's why we cannot force this regexp to have a precise year, also
    # uplifts may fail during year change period, so the year check is relaxed.
    new_file_license_re = input_api.re.compile((
        r'.*? Copyright \(c\) %(year)s The Brave Authors\. All rights reserved\.\n'
        r'.*? This Source Code Form is subject to the terms of the Mozilla Public\n'
        r'.*? License, v\. 2\.0\. If a copy of the MPL was not distributed with this file,\n'
        r'.*? You can obtain one at https://mozilla.org/MPL/2\.0/\.(?: \*/)?\n'
    ) % {'year': years_re}, input_api.re.MULTILINE)

    # License regexp to match in EXISTING files, it allows some variance.
    existing_file_license_re = input_api.re.compile((
        r'.*? Copyright \(c\) %(year)s The Brave Authors\. All rights reserved\.\n'
        r'.*? This Source Code Form is subject to the terms of the Mozilla Public\n'
        r'.*? License, v\. 2\.0\. If a copy of the MPL was not distributed with this(\n.*?)? file,\n?'
        r'.*? (y|Y)ou can obtain one at https?://mozilla.org/MPL/2\.0/\.(?: \*/)?\n'
    ) % {'year': years_re}, input_api.re.MULTILINE)

    # License template for new files. Includes current year.
    expected_license_template = (
        '%(comment)s Copyright (c) %(year)s The Brave Authors. All rights reserved.\n'
        '%(comment)s This Source Code Form is subject to the terms of the Mozilla Public\n'
        '%(comment)s License, v. 2.0. If a copy of the MPL was not distributed with this file,\n'
        '%(comment)s You can obtain one at https://mozilla.org/MPL/2.0/.\n'
    ) % {
        'comment': '#',
        'year': current_year,
    }

    bad_new_files = []
    bad_files = []
    sources = lambda affected_file: input_api.FilterSourceFile(
        affected_file,
        files_to_check=files_to_check,
        files_to_skip=files_to_skip)
    for f in input_api.AffectedSourceFiles(sources):
        contents = input_api.ReadFile(f, 'r')[:1000].replace('\r\n', '\n')
        if not contents:
            continue
        if f.Action() == 'A':  # 'A' means "Added", also includes moved files.
            if not new_file_license_re.search(contents):
                bad_new_files.append(f.LocalPath())
        else:
            if not existing_file_license_re.search(contents):
                bad_files.append(f.LocalPath())

    splitted_expected_license_template = expected_license_template.replace(
        "# ", "").split('\n')
    multiline_comment_expected_license = (
        f'/* {splitted_expected_license_template[0]}\n'
        f' * {splitted_expected_license_template[1]}\n'
        f' * {splitted_expected_license_template[2]}\n'
        f' * {splitted_expected_license_template[3]} */\n')
    assert new_file_license_re.search(expected_license_template)
    assert existing_file_license_re.search(expected_license_template)
    assert new_file_license_re.search(multiline_comment_expected_license)
    assert existing_file_license_re.search(multiline_comment_expected_license)

    # Show this to simplify copy-paste when an invalid license is found.
    expected_licenses = (f'{expected_license_template.replace("#", "//")}\n'
                         f'{multiline_comment_expected_license}\n'
                         f'{expected_license_template}')

    result = []
    if bad_new_files:
        expected_license_message = (
            f'Expected one of license headers in new files:\n'
            f'{expected_licenses}')
        result.append(
            output_api.PresubmitError(expected_license_message,
                                      items=bad_new_files))
    if bad_files:
        expected_license_message = (
            f'Expected one of license headers in existing files:\n'
            f'{expected_licenses.replace(f"{current_year}", "<year>")}')
        result.append(
            output_api.PresubmitPromptWarning(expected_license_message,
                                              items=bad_files))
    return result


def CheckWebDevStyle(input_api, output_api):
    results = []
    try:
        old_sys_path = sys.path[:]
        cwd = input_api.PresubmitLocalPath()
        sys.path += [input_api.os_path.join(cwd, '..', 'tools')]
        # pylint: disable=import-error, import-outside-toplevel
        from web_dev_style import presubmit_support
        with override_utils.override_scope_variable(output_api,
                                                    'PresubmitPromptWarning',
                                                    output_api.PresubmitError):
            results += presubmit_support.CheckStyle(input_api, output_api)
    finally:
        sys.path = old_sys_path
    return results
