#!/bin/bash

# Android
cp @1x/Atlases/*.png ../../Resources/android/display/res_low
cp @2x/Atlases/*.png ../../Resources/android/display/res_med
cp @hd/Atlases/*.png ../../Resources/android/display/res_hi

cp @1x/Atlases/*.plist ../../Resources/android/display/res_low/spriteFrames
cp @2x/Atlases/*.plist ../../Resources/android/display/res_med/spriteFrames
cp @hd/Atlases/*.plist ../../Resources/android/display/res_hi/spriteFrames

# Android does not have locale selection dialog
rm ../../Resources/android/display/res_hi/locales-atlas.png
rm ../../Resources/android/display/res_hi/spriteFrames/locales-atlas.plist

# iOS
cp @1x/Atlases/*.png ../../Resources/apple/ios/display/iphone
cp @2x/Atlases/*.png ../../Resources/apple/ios/display/ipad+iphonehd
cp @hd/Atlases/*.png ../../Resources/apple/ios/display/ipadhd

cp @1x/Atlases/*.plist ../../Resources/apple/ios/display/iphone/spriteFrames
cp @2x/Atlases/*.plist ../../Resources/apple/ios/display/ipad+iphonehd/spriteFrames
cp @hd/Atlases/*.plist ../../Resources/apple/ios/display/ipadhd/spriteFrames

# iOS does not have locale selection dialog
rm ../../Resources/apple/ios/display/ipadhd/locales-atlas.png
rm ../../Resources/apple/ios/display/ipadhd/spriteFrames/locales-atlas.plist

# Mac
cp @hd/Atlases/*.png ../../Resources/apple/mac/display/machd

cp @hd/Atlases/*.plist ../../Resources/apple/mac/display/machd/spriteFrames
