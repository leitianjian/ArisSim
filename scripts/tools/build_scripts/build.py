import argparse
import sys
from os.path import abspath, dirname
import os
import platform
import shutil
from glob import glob
from typing import Dict, Optional
import pathlib

from setuptools import distutils



# By appending pytorch_root to sys.path, this module can import other torch
# modules even when run as a standalone script. i.e., it's okay either you
# do `python build_libtorch.py` or `python -m tools.build_libtorch`.
pytorch_root = dirname(dirname(abspath(__file__)))
sys.path.append(pytorch_root)

# from build_scripts.cmake.cmake import CMake
from build_scripts.build_sire import _create_build_env, build_sire
from build_scripts.cmake import which  # type: ignore[import]

if __name__ == "__main__":
    os.environ["USE_MSVC"] = "True"
    os.environ["USE_NINJA"] = "True"
    os.environ["BUILD_TEST"] = "FALSE"
    # Placeholder for future interface. For now just gives a nice -h.
    parser = argparse.ArgumentParser(description="Build Sire", allow_abbrev=True)
    parser.add_argument(
        '-b',
        "--base-path",
        type=pathlib.Path,
        required=True,
        help="Sire base path (CMakeList.txt folder)",
    )
    parser.add_argument(
        "--build-dir",
        type=pathlib.Path,
        help="Sire build directory",
    )
    parser.add_argument(
        '-i',
        "--install-dir",
        type=pathlib.Path,
        help="Sire isntall directory",
    )
    parser.add_argument(
        '-a',
        "--aris-path",
        type=pathlib.Path,
        required=True,
        help="Aris install path",
    )
    parser.add_argument(
        '-f',
        "--fcl-path",
        type=pathlib.Path,
        required=True,
        help="hpp-fcl install path",
    )
    parser.add_argument(
        '-t',
        "--toolchain-path",
        type=pathlib.Path,
        help="toolchain setting path (.cmake file)",
    )
    parser.add_argument('-r', "--rerun-cmake", action="store_true", help="rerun cmake")
    parser.add_argument('-c',
        "--cmake-only",
        action="store_true",
        help="Stop once cmake terminates. Leave users a chance to adjust build options",
    )
    parser.add_argument("--build-demos", action="store_true", help="build demos")
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--release', action='store_true')
    group.add_argument('--debug', action='store_true')
    group.add_argument('--build-all', action='store_true')
    options = parser.parse_args()
    if options.build_demos:
        os.environ["BUILD_DEMOS"] = "TRUE"
    
    my_env = _create_build_env()

    USE_NINJA = which("ninja", env=my_env) is not None
    if "CMAKE_GENERATOR" in my_env:
        USE_NINJA = my_env["CMAKE_GENERATOR"].lower() == "ninja"

    if USE_NINJA:
        my_env["CMAKE_GENERATOR"] = "ninja"

    # print(options, str(options.install_dir))
    if options.build_all:
        options.debug = True
        options.release = True

    if options.debug:
        os.environ['CMAKE_BUILD_TYPE'] = 'Debug'
        build_sire(
            sire_path=str(options.base_path),
            build_dir=None if options.build_dir is None else str(options.build_dir),
            install_dir=None if options.install_dir is None else str(options.install_dir),
            version=None,
            # cmake_python_library=None,
            # build_python=False,
            rerun_cmake=options.rerun_cmake,
            cmake_only=options.cmake_only,
            cmake_toolchain_file=None if options.toolchain_path is None else str(options.toolchain_path),
            env=my_env,
            TARGET_ARIS_PATH=str(options.aris_path),
            TARGET_HPP_FCL_PATH=str(options.fcl_path),
            CMAKE_BUILD_TYPE='Debug'
        )
    if options.release:
        os.environ['CMAKE_BUILD_TYPE'] = 'Release'
        build_sire(
            sire_path=str(options.base_path),
            build_dir=None if options.build_dir is None else str(options.build_dir),
            install_dir=None if options.install_dir is None else str(options.install_dir),
            version=None,
            # cmake_python_library=None,
            # build_python=False,
            rerun_cmake=True if options.debug else options.rerun_cmake,
            cmake_only=options.cmake_only,
            cmake_toolchain_file=None if options.toolchain_path is None else str(options.toolchain_path),
            env=my_env,
            TARGET_ARIS_PATH=str(options.aris_path),
            TARGET_HPP_FCL_PATH=str(options.fcl_path),
            CMAKE_BUILD_TYPE='Release'
        )
