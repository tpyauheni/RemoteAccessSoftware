# Remote Access Software

Required to read bytecode from serial port and execute it. To compile source code to bytecode see [tpyauheni/RemoteAccessHardware](https://github.com/tpyauheni/RemoteAccessHardware)

## Building

This repository relies on Win32 API, so it should be built using Microsoft Visual C/C++ compiler.

To build executable file with default options, using default Visual Studio 2022 installation paths:
1. Open command prompt. You can do so, by pressing `Win` and `R` at the same time, and then typing `cmd` to `Run` prompt. Then simply press `Enter`.
2. Copy `"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat"`, paste it to command prompt and press `Enter`.
3. Copy `"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.38.33130/bin/Hostx64/x86/cl.exe" /EHsc /Od src\\*.cpp`, paste it to command prompt and press `Enter`.

After that you can see, that files `main.obj` and `main.exe` appeared in the same folder. `main.exe` is Remote Access Client executable, that you can run through explorer, or by typing `main` in command prompt. `main.obj` is simply file with compiled code, you can freely remove it after build.
