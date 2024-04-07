import os
import sys
from typing import List, Dict, Optional
from setuptools import distutils

def which(exe_file: str, append_paths: List[str] = [], env: Dict[str, str] = os.environ) -> Optional[str]:
    path = env.get("PATH", os.defpath).split(os.pathsep)
    for p in append_paths:
        if os.path.isdir(p):
            path += [p]
    for d in path:
        fname = os.path.join(d, exe_file)
        fnames = [fname]
        if sys.platform == "win32":
            exts = env.get("PATHEXT", "").split(os.pathsep)
            fnames += [fname + ext for ext in exts]
        for name in fnames:
            if os.access(name, os.F_OK | os.X_OK) and not os.path.isdir(name):
                return name
    return None

# if __name__ == '__main__':
#     print(which('cmake', ['C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin']))
#     os.environ["USE_NINJA"] = "False"
#     print(os.environ["USE_NINJA"], os.getenv("USE_NINJA"))
#     print(check_negative_env_flag("USE_NINJA"))
#     print(distutils._msvccompiler._get_vc_env('x64'))
