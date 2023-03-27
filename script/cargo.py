#!/usr/bin/env python3

# Copyright (c) 2020 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.

import json
import optparse # pylint: disable=deprecated-module
import os
import sys
import subprocess

def run_cargo(command, args, out = subprocess.PIPE):
    # Set environment variables for rustup
    env = os.environ.copy()

    rustup_home = args.rustup_home
    env['RUSTUP_HOME'] = rustup_home
    env['CARGO_HOME'] = rustup_home
    # Enable experimental features in non-nightly builds
    env['RUSTC_BOOTSTRAP'] = '1'

    rustup_bin_dir = os.path.abspath(os.path.join(rustup_home, 'bin'))
    cargo_exe = args.exe

    env['PATH'] = rustup_bin_dir + os.pathsep + env['PATH']

    if args.toolchain:
        toolchains_path = os.path.abspath(
            os.path.join(rustup_home, 'toolchains', args.toolchain, "bin"))
        env['PATH'] = toolchains_path + os.pathsep + env['PATH']

    if args.clang_bin_path is not None and not sys.platform.startswith('win'):
        env['AR'] = os.path.join(args.clang_bin_path, 'llvm-ar')
        env['CC'] = os.path.join(args.clang_bin_path, 'clang')
        env['CXX'] = os.path.join(args.clang_bin_path, 'clang++')

    if args.mac_deployment_target is not None:
        env['MACOSX_DEPLOYMENT_TARGET'] = args.mac_deployment_target

    if args.ios_deployment_target is not None:
        env['IPHONEOS_DEPLOYMENT_TARGET'] = args.ios_deployment_target

    if args.is_debug == "false":
        env['NDEBUG'] = "1"

    rust_flags = args.rust_flags.copy()
    rust_flags.append("-Cpanic=" + args.panic)

    if rust_flags is not None:
        env['RUSTFLAGS'] = " ".join(rust_flags)

    # env['RUSTC_WRAPPER'] = os.path.join(os.path.dirname(__file__), 'rustc_wrapper.py')

    try:
        cargo_args = []
        cargo_args.append(cargo_exe)
        cargo_args.append(command)
        if command == "build":
            cargo_args.append('--build-plan')
            cargo_args.append('-Zunstable-options')
        if args.profile == "release":
            cargo_args.append("--release")
        cargo_args.append("--manifest-path=" + args.manifest_path)
        cargo_args.append("--target-dir=" + args.build_path)
        cargo_args.append("--target=" + args.target)
        if command != "clean" and args.features is not None:
            cargo_args.append("--features=" + args.features)
        # if command != "clean":
        #     cargo_args.append("--frozen")
        # if command != "clean":
            # use deployment target as a proxy for mac/ios target_os
            # if (args.mac_deployment_target is not None
            #         or args.ios_deployment_target is not None):
            #     cargo_args.append("-Z")
            #     cargo_args.append("build-std=panic_" + args.panic + ",std")
            # cargo_args.append('--lib')
        if command == "rustc":
            cargo_args.append('--crate-type=rlib')
            cargo_args.append('--')
            # cargo_args.append('-L' + args.cxx_lib_dir)
            # cargo_args.append('--emit=dep-info,link')
            # cargo_args.append('--print=file-names')
            cargo_args += rust_flags
        subprocess.check_call(cargo_args, env=env, stdout=out)
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e


def build(args):
    # Check the previous args against the current args because cargo doesn't
    # rebuild when env vars change and rerun-if-env-changed doesn't force the
    # dependencies to rebuild
    build_args_cache_file = os.path.join(args.build_path, ".cargo_args")
    previous_args = {}
    clean = False

    if os.path.exists(build_args_cache_file):
        with open(build_args_cache_file, "r", encoding="utf8") as f:
            previous_args = json.load(f)

        if previous_args != args.__dict__:
            clean = True

        for filename in args.other_inputs:
            if (os.path.getmtime(build_args_cache_file) <
                    os.path.getmtime(filename)):
                clean = True

    if clean:
        print('run cargo')
        run_cargo('clean', args)

    try:
        # `cargo rustc` provides options that are not supported by `cargo build`
        with open(os.path.join(args.build_path, 'build.json'), 'w', encoding="utf8") as out:
            print('run build')
            run_cargo('build', args, out)

        print('run rustc')
        run_cargo('rustc', args)
        with open(build_args_cache_file, "w", encoding="utf8") as f:
            json.dump(args.__dict__, f)

        outputs = []
        with open(os.path.join(args.build_path, 'build.json'), 'r', encoding="utf8") as f:
            build_plan = json.load(f)
            for invocation in build_plan['invocations']:
                for target_kind in invocation['target_kind']:
                    if target_kind == 'rlib' or target_kind == 'lib':
                        for output in invocation['outputs']:
                            if output.endswith('.rlib'):
                                outputs.append(output)

        with open(os.path.join(args.build_path, 'cargo.d'), 'w', encoding="utf8") as out:
            out.write("\n".join(outputs))

    except subprocess.CalledProcessError as e:
        print(e.output)
        raise e


def parse_args():
    parser = optparse.OptionParser(description='Cargo')

    parser.add_option('--exe')
    parser.add_option('--rustup_home')
    parser.add_option('--manifest_path')
    parser.add_option('--build_path')
    parser.add_option('--target')
    parser.add_option('--toolchain')
    parser.add_option('--is_debug')
    parser.add_option('--profile')
    parser.add_option('--panic')
    parser.add_option('--cxx_lib')
    parser.add_option('--cxx_lib_dir')
    parser.add_option('--clang_bin_path')
    parser.add_option('--rust_version')
    parser.add_option('--mac_deployment_target')
    parser.add_option('--ios_deployment_target')
    parser.add_option("--rust_flag", action="append",
                                     dest="rust_flags",
                                     default=[])
    parser.add_option('--features')
    parser.add_option('--inputs',
                      action="append",
                      dest="other_inputs",
                      default=[])

    options, _ = parser.parse_args()

    if options.is_debug not in ('false', 'true'):
        raise Exception("is_debug argument was not specified correctly")

    if options.profile not in ('release', 'dev'):
        raise Exception("profile argument was not specified correctly")

    return options


def main():
    build(parse_args())

    return 0


if __name__ == '__main__':
    sys.exit(main())
