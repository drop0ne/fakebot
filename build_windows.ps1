$cl = Get-Command cl -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "Microsoft C++ compiler not found in PATH."
    Write-Host "Open x64 Native Tools Command Prompt for Visual Studio or Developer PowerShell first."
    exit 1
}

New-Item -ItemType Directory -Force -Path build | Out-Null

& cl /std:c++20 /EHsc /W4 /I src `
    src/main.cpp `
    src/ChatBot.cpp `
    src/RuleLoader.cpp `
    src/TextUtils.cpp `
    /Fe:build/FakeBot.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed."
    exit $LASTEXITCODE
}

Write-Host "Build complete: build/FakeBot.exe"
