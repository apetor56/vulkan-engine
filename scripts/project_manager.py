import os
import subprocess
import sys

scriptPath = os.path.dirname(os.path.abspath(__file__))
rootPath = os.path.dirname(scriptPath)
buildPath = os.path.join(rootPath, 'build')

def selectToolset():
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
    if os.path.exists(buildPath):
        subprocess.run(['cmake', '--build', buildPath])
        exit()
        
def clean():
    subprocess.run(['rm', '-rf', buildPath])
    
def rebuild():
    clean()
    selectToolset()
    build()
    
def handleCommandArgs():
    arg = sys.argv[1].lower()
    if arg == 'build':
        build()
    elif arg == 'clean':
        clean()
    elif arg == 'rebuild':
        rebuild()

if __name__ == "__main__":
    if len(sys.argv) == 2:
        handleCommandArgs()
    else:
        selectToolset()
