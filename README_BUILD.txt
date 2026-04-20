FakeBot Windows build notes
==========================

Option 1: Visual Studio compiler
- Open "Developer Command Prompt for VS".
- Change into the repo directory.
- Run: build_windows.bat

Option 2: PowerShell
- Open Developer PowerShell for Visual Studio.
- Change into the repo directory.
- Run: .\build_windows.ps1

Option 3: CMake
- mkdir build
- cd build
- cmake ..
- cmake --build . --config Release

Expected output:
- build/FakeBot.exe

Notes:
- The program expects rules.txt to be present next to the working directory when launched.
- The reconstructed source includes <sstream> in TextUtils.cpp for std::istringstream.
