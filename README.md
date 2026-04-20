# FakeBot

FakeBot is a small C++20 console chatbot reconstructed from the transcript and normalized into a clean project layout.

## Included

- `src/` multi-file C++ source tree
- `CMakeLists.txt` for standard builds
- `build_windows.bat` and `build_windows.ps1` for Windows builds
- `rules.txt` example external rule file
- `FakeBot_single_file.cpp` for a one-file variant

## Build on Windows

### Visual Studio Developer Command Prompt

```bat
build_windows.bat
```

### PowerShell

```powershell
./build_windows.ps1
```

The expected output is `build/FakeBot.exe`.

## Notes

The transcript version required one normalization for coherence:
- `TextUtils.cpp` includes `<sstream>` for `std::istringstream`
