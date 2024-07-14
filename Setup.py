from pathlib import Path
import subprocess

def setup():
    rootPath = Path(__file__).resolve().parent
    presets = {
        '1': 'visual-studio-msvc-debug',
        '2': 'visual-studio-clangcl-debug',
        '3': 'mingw-makefiles-clang-debug',
        '4': 'mingw-makefiles-gcc-debug'
    }
    menu = (
        "Select configuration:\n"
        "   Generator                Compiler\n"
        "1) Visual Studio 17 2022    msvc143\n"
        "2) Visual Studio 17 2022    clang-cl\n"
        "3) MinGW Makefile           clang++\n"
        "4) MinGW Makefile           g++\n"
    )
    
    userInput = input(menu)
    preset = presets.get(userInput)
    subprocess.run(['cmake', '-S', rootPath, '--preset', preset])

if __name__ == "__main__":
    setup()
