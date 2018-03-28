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
```bash
# Debug build
/opt/clion-2017.3.4/bin/cmake/bin/cmake --build /home/andrew/Public/WebSocketClientCPlus/cmake-build-debug --target WebSocketClientCPlus -- -j 2

# Release build
/opt/clion-2017.3.4/bin/cmake/bin/cmake --build /home/andrew/Public/WebSocketClientCPlus/cmake-build-release --target WebSocketClientCPlus -- -j 2
``` 

The program executable will be available as _cmake-build-debug(release)/WebSocketClientCPlus_