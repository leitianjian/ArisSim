import os
import platform
import struct
import sys
from itertools import chain
from typing import cast, Iterable, List, Dict, Optional
from . import which

IS_WINDOWS = platform.system() == "Windows"
IS_DARWIN = platform.system() == "Darwin"
IS_LINUX = platform.system() == "Linux"
CMAKE_EXE_PATH = ""

# IS_CONDA = (
#     "conda" in sys.version
#     or "Continuum" in sys.version
#     or any(x.startswith("CONDA") for x in os.environ)
# )
# CONDA_DIR = os.path.join(os.path.dirname(sys.executable), "..")

IS_64BIT = struct.calcsize("P") == 8

BUILD_DIR = "build"
INSTALL_DIR = "install"

def check_env_flag(name: str, default: str = "", env: Dict[str, str] = os.environ) -> bool:
    return env.get(name, default).upper() in ["ON", "1", "YES", "TRUE", "Y"]


def check_negative_env_flag(name: str, default: str = "", env: Dict[str, str] = os.environ) -> bool:
    return env.get(name, default).upper() in ["OFF", "0", "NO", "FALSE", "N"]


def gather_paths(env_vars: Iterable[str], env: Dict[str, str] = os.environ) -> List[str]:
    return list(chain(*(env.get(v, "").split(os.pathsep) for v in env_vars)))


def lib_paths_from_base(base_path: str) -> List[str]:
    return [os.path.join(base_path, s) for s in ["lib/x64", "lib", "lib64"]]



# We promised that CXXFLAGS should also be affected by CFLAGS
if "CFLAGS" in os.environ and "CXXFLAGS" not in os.environ:
    os.environ["CXXFLAGS"] = os.environ["CFLAGS"]

