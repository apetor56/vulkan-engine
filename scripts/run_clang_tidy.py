# work only with Makefile and Ninja cmake generator

import subprocess
import sys
from pathlib import Path

scriptPath = Path(__file__).resolve().parent
rootPath = scriptPath.parent
sourcePath = rootPath / 'source'
buildPath = rootPath / 'build'
clangTidyConfig = rootPath / '.clang-tidy'

def runClangTidy():    
    clangTidyCmd = ['clang-tidy', '--config-file', clangTidyConfig, '-p', buildPath]
    
    if len(sys.argv) > 1:
        clangTidyCmd.extend(sys.argv[1:])
    else:
        for path in sourcePath.rglob('*.cpp'):
            clangTidyCmd.append(path)
            
        for path in sourcePath.rglob('*.hpp'):
            clangTidyCmd.append(path)

    print("Running clang-tidy. It could take a while")
    subprocess.run(clangTidyCmd, text=True)


if __name__ == "__main__":
    runClangTidy()
