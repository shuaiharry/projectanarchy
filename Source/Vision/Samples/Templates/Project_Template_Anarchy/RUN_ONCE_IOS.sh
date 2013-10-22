#!/bin/sh

mkdir -p ./Data/Vision
sudo cp -R -f "$VISION_SDK/Data/Vision/Base" ./Data/Vision

mkdir -p ./Data/Common/iOS
sudo cp -R -f "$VISION_SDK/Data/Common/iOS" ./Data/Common

mkdir -p ./Source/GameApplication/iOS
sudo cp -f "$VISION_SDK/Source/Vision/Runtime/Common/VisSampleApp.cpp" ./Source/GameApplication
sudo cp -f "$VISION_SDK/Source/Vision/Runtime/Common/VisSampleAppCallbacks.cpp" ./Source/GameApplication
sudo cp -R -f "$VISION_SDK/Source/Vision/Runtime/Common/iOS" ./Source/GameApplication