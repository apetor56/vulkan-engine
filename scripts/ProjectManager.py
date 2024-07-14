import os
from pathlib import Path
import subprocess
import sys
import shutil

scriptPath = Path(__file__).resolve()
rootPath = scriptPath.parent.parent
buildPath = rootPath / 'build'

def help():
    info = (
        "\nProject Manager script usage:\n"
        "py ProjectManager.py [option]\n\n"
        "Running script without option for first time will configure and build project.\n\n"
        "Available options:\n"
        "--help       show this message\n"
        "--configure  configure project with cmake presets (see CMakePresets.json)\n"
        "--build      build project and executable. If project is not configured, then this option do it as well\n"
        "--clean      remove entire build directory\n"
        "--rebuild    rebuild only project dependencies, not external files\n"
    )
    print(info)

def buildDirExist():
    return os.path.exists(buildPath)

def configure():
    if buildDirExist():
        print(f'Configuration already is selected. To select new cofiguration prompt: ')
        print(f'py {__file__} --clean')
        print(f'py {__file__} --configure')
        exit()
    
    menu = (
        "Select configuration:\n"
        "   Generator                Compiler\n"
        "1) Visual Studio 17 2022    msvc143\n"
        "2) Visual Studio 17 2022    clang-cl\n"
        "3) MinGW Makefile           clang++\n"
        "4) MinGW Makefile           g++\n"
    )
    
    userInput = input(menu)

    presets = {
        '1': 'visual-studio-msvc-debug',
        '2': 'visual-studio-clangcl-debug',
        '3': 'mingw-makefiles-clang-debug',
        '4': 'mingw-makefiles-gcc-debug'
    }

    preset = presets.get(userInput)
    if preset:
        subprocess.run(['cmake', '-S', rootPath, '--preset', preset])
    else:
        print("Invalid selection. Please run the script again and select a valid option.")
        
def build():
    if not buildDirExist():
        configure();
    
    subprocess.run(['cmake', '--build', buildPath])
        
def clean():
    subprocess.run(['rm', '-rf', buildPath])
    
def rebuild():
    shutil.rmtree(buildPath / 'source')
    build()
    
def handleCommandArgs():
    arg = sys.argv[1].lower()
    if arg == '--help' or arg == '-h':
        help()
    elif arg == '--configure':
        configure()
    elif arg == '--build':
        build()
    elif arg == '--clean':
        clean()
    elif arg == '--rebuild':
        rebuild()
    else:
        print('Passed unknown argument')

if __name__ == "__main__":
    if len(sys.argv) == 2:
        handleCommandArgs()
    else:
        configure()
        build()
