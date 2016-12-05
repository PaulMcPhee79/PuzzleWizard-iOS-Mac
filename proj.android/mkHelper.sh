///mkHelper.sh
#!/bin/sh

echo 'LOCAL_PATH := $(call my-dir)'
echo "" 
echo 'include $(CLEAR_VARS)'
echo "" 
echo 'LOCAL_MODULE := game_shared'
echo"" 
echo 'LOCAL_MODULE_FILENAME := libgame'
echo "" 
echo 'LOCAL_CPPFLAGS += -DCHEEKY_MOBILE -DDEBUG=0 -DGOD_MODE=0 -DCM_LOW_POWER=1 -DCM_BETA=0 -DCM_EVLOG=0 -DCM_SMALL_CACHES=1'
#echo "" 
#echo 'LOCAL_LDFLAGS += -g -rdynamic'
echo "" 
echo 'LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog'
echo "" 
echo "LOCAL_SRC_FILES := hellocpp/main.cpp \\"
echo "CMJniUtils.cpp \\"
echo "CMNative.cpp \\"

while read data; do
    echo "                   ../$data \\" 
done

echo "" 
echo 'LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes $(LOCAL_PATH) $(LOCAL_PATH)/../../breakpad/src $(LOCAL_PATH)/../../breakpad/src/common/android/include'
echo "" 
echo 'LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static cocosdenshion_static cocos_extension_static breakpad_client'
echo "" 
echo 'include $(BUILD_SHARED_LIBRARY)'
echo ""
echo 'include /Users/paulmcphee/iphone-apps/cocos2dx/cocos2d-x-2.2.6/samples/Cpp/PuzzleHound/breakpad/android/google_breakpad/Android.mk'
echo "" 
echo '$(call import-module,CocosDenshion/android) \'
echo '$(call import-module,cocos2dx) \'
echo '$(call import-module,extensions)'


# HockeyApp / Breakpad
# echo "" 
# echo 'include $(CLEAR_VARS)'
# echo 'LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog'
# echo 'LOCAL_MODULE := CMNative'
# echo 'LOCAL_SRC_FILES := CMNative.cpp'  
# echo 'LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../../breakpad/src $(LOCAL_PATH)/../../breakpad/src/common/android/include'
# echo 'LOCAL_STATIC_LIBRARIES += breakpad_client'

# echo 'include $(BUILD_SHARED_LIBRARY)'

# If NDK_MODULE_PATH is defined, import the module, otherwise do a direct
# includes. This allows us to build in all scenarios easily.
# echo 'ifneq ($(NDK_MODULE_PATH),)'
# echo ' $(call import-module,google_breakpad)'
# echo 'else'
# echo '  include $(LOCAL_PATH)/../../breakpad/android/google_breakpad/Android.mk'
# echo 'endif'
# echo 'include /Users/paulmcphee/iphone-apps/cocos2dx/cocos2d-x-2.2.6/samples/Cpp/PuzzleHound/breakpad/android/google_breakpad/Android.mk'
