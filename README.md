# WebSocketClientCPlus
Lightweight c++ WebSocket Client

## Dependencies
[Cmake](https://cmake.org/)  
[easywsclient](https://github.com/dhbaird/easywsclient)  
[rapidjson](http://rapidjson.org/)

## Project Setup
The necessary third party libraries/files should already be available in the
_/third_party_ directory. Else run the _init.sh/init.bat_ script to gather/update the files.


Cmake is used as the build tool.
Both IDE's of *[CLion](https://www.jetbrains.com/clion/)* and *[Visual Studio 2017](https://www.visualstudio.com/downloads/)* support and include built in CMake implementations.
Else you will need to download _CMake_ itself, and use its CLI appropriately to build the project.

The program executable will be available at:  
CLion Builds: _cmake-build-debug(release)/WebSocketClientCPlus_  
VS 2017 Builds: _C:\Users\\<user\>\CMakeBuilds\...\build\x86-debug(release)\WebSocketClientCPlus.exe_
