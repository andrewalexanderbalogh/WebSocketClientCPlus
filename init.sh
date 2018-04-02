#!/bin/bash

# Our top level project directory
export WEBSOCKET_PROJ_DIR="$(pwd)"

# The websocket library to use, and up to the commit we know works
if [ ! -d easywsclient ] ;  then
    git clone https://github.com/dhbaird/easywsclient.git
fi
cd easywsclient
git reset --hard 9b87dc4

cd $WEBSOCKET_PROJ_DIR

# A JSON parser/serializer library
if [ ! -d rapidjson ] ; then
    git clone https://github.com/Tencent/rapidjson.git
fi
cd rapidjson
git reset --hard 8022a5f7

cd $WEBSOCKET_PROJ_DIR

# Copy over just the files that are needed
cp easywsclient/easywsclient.cpp third_party/easywsclient/easywsclient.cpp
cp easywsclient/easywsclient.hpp third_party/easywsclient/easywsclient.hpp
cp -R rapidjson/include/rapidjson third_party
