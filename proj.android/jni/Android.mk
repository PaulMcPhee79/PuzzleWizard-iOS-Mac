LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := game_shared

LOCAL_MODULE_FILENAME := libgame

LOCAL_CPPFLAGS += -DCHEEKY_MOBILE -DDEBUG=0 -DGOD_MODE=0 -DCM_LOW_POWER=1 -DCM_BETA=0 -DCM_EVLOG=0 -DCM_SMALL_CACHES=1

LOCAL_LDFLAGS += -g -rdynamic

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_SRC_FILES := hellocpp/main.cpp \
CMJniUtils.cpp \
CMNative.cpp \
                   ../../Classes/AppDelegate.cpp \
                   ../../Classes/CCExtensions/CMScale9Sprite.cpp \
                   ../../Classes/CCExtensions/RenderCache.cpp \
                   ../../Classes/CCExtensions/RenderNode.cpp \
                   ../../Classes/Events/DelayedEvent.cpp \
                   ../../Classes/Events/EventDispatcher.cpp \
                   ../../Classes/Extensions/CMLabelBMFont.cpp \
                   ../../Classes/Extensions/Juggler.cpp \
                   ../../Classes/Extensions/MovieClip.cpp \
                   ../../Classes/Interfaces/IAnimatable.cpp \
                   ../../Classes/Managers/ControlsManager.cpp \
                   ../../Classes/Managers/FileManager.cpp \
                   ../../Classes/Managers/GameSettings.cpp \
                   ../../Classes/Managers/InputManager.cpp \
                   ../../Classes/Managers/LicenseManager.cpp \
                   ../../Classes/Managers/LicenseManagerTester.cpp \
                   ../../Classes/Managers/SpriteLayerManager.cpp \
                   ../../Classes/Managers/TouchManager.cpp \
                   ../../Classes/Prop/CMSprite.cpp \
                   ../../Classes/Prop/CroppedProp.cpp \
                   ../../Classes/Prop/ParticleProp.cpp \
                   ../../Classes/Prop/Prop.cpp \
                   ../../Classes/Prop/UIControls/FXButton.cpp \
                   ../../Classes/Prop/UIControls/IconButton.cpp \
                   ../../Classes/Prop/UIControls/IQSlider.cpp \
                   ../../Classes/Prop/UIControls/MenuButton.cpp \
                   ../../Classes/Prop/UIControls/ProgressBar.cpp \
                   ../../Classes/Prop/UIControls/RateIcon.cpp \
                   ../../Classes/Prop/UIViews/MenuSubview.cpp \
                   ../../Classes/Puzzle/Controllers/GameProgressController.cpp \
                   ../../Classes/Puzzle/Controllers/PuzzleController.cpp \
                   ../../Classes/Puzzle/Controllers/PuzzleOrganizer.cpp \
                   ../../Classes/Puzzle/Controllers/ShieldManager.cpp \
                   ../../Classes/Puzzle/Data/ColorFiller.cpp \
                   ../../Classes/Puzzle/Data/GameProgress.cpp \
                   ../../Classes/Puzzle/Data/HumanPlayer.cpp \
                   ../../Classes/Puzzle/Data/Level.cpp \
                   ../../Classes/Puzzle/Data/MirroredPlayer.cpp \
                   ../../Classes/Puzzle/Data/Player.cpp \
                   ../../Classes/Puzzle/Data/Puzzle.cpp \
                   ../../Classes/Puzzle/Data/PuzzleTile.cpp \
                   ../../Classes/Puzzle/Inputs/PathFinder.cpp \
                   ../../Classes/Puzzle/Inputs/PlayerController.cpp \
                   ../../Classes/Puzzle/Inputs/PlayerControllerIOS.cpp \
                   ../../Classes/Puzzle/Inputs/PlayerControllerOSX.cpp \
                   ../../Classes/Puzzle/Inputs/SearchNode.cpp \
                   ../../Classes/Puzzle/Inputs/TouchPad.cpp \
                   ../../Classes/Puzzle/View/Menu/LevelIcon.cpp \
                   ../../Classes/Puzzle/View/Menu/LevelMenu.cpp \
                   ../../Classes/Puzzle/View/Menu/LevelOverlay.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuBuilder.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuBuilderIOS.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuBuilderOSX.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuCustomDialog.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuDialog.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuDialogManager.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuGridDialog.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuItem/GaugeMenuItem.cpp \
                   ../../Classes/Puzzle/View/Menu/MenuItem/MenuItem.cpp \
                   ../../Classes/Puzzle/View/Menu/PuzzlePage.cpp \
                   ../../Classes/Puzzle/View/Menu/PuzzlePageEntry.cpp \
                   ../../Classes/Puzzle/View/Playfield/AnimPlayerPiece.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/Dissolver.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/PuzzleRibbon.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/Shield.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/SolvedAnimation.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/TileConveyorBelt.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/TileRotator.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/TileShadow.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/TileSwapper.cpp \
                   ../../Classes/Puzzle/View/Playfield/Effects/Twinkle.cpp \
                   ../../Classes/Puzzle/View/Playfield/HUD/HUDCell.cpp \
                   ../../Classes/Puzzle/View/Playfield/HUD/PlayerHUD.cpp \
                   ../../Classes/Puzzle/View/Playfield/HumanPlayerPiece.cpp \
                   ../../Classes/Puzzle/View/Playfield/MirrorPlayerPiece.cpp \
                   ../../Classes/Puzzle/View/Playfield/PlayerPiece.cpp \
                   ../../Classes/Puzzle/View/Playfield/PuzzleBoard.cpp \
                   ../../Classes/Puzzle/View/Playfield/StaticPlayerPiece.cpp \
                   ../../Classes/Puzzle/View/Playfield/TileDecoration.cpp \
                   ../../Classes/Puzzle/View/Playfield/TilePiece.cpp \
                   ../../Classes/Puzzle/View/PuzzleHelper.cpp \
                   ../../Classes/SceneControllers/GameController.cpp \
                   ../../Classes/SceneControllers/MenuController.cpp \
                   ../../Classes/SceneControllers/PlayfieldController.cpp \
                   ../../Classes/SceneControllers/SceneController.cpp \
                   ../../Classes/SceneViews/MenuView.cpp \
                   ../../Classes/SceneViews/PlayfieldView.cpp \
                   ../../Classes/SceneViews/SceneView.cpp \
                   ../../Classes/Testing/TFManager.cpp \
                   ../../Classes/Utils/BGContainer.cpp \
                   ../../Classes/Utils/BridgingUtility.cpp \
                   ../../Classes/Utils/ButtonsProxy.cpp \
                   ../../Classes/Utils/ByteTweener.cpp \
                   ../../Classes/Utils/CloudWrapper.cpp \
                   ../../Classes/Utils/CMLock.cpp \
                   ../../Classes/Utils/DeviceDetails.cpp \
                   ../../Classes/Utils/FloatTweener.cpp \
                   ../../Classes/Utils/fmemopen.c \
                   ../../Classes/Utils/FpsSampler.cpp \
                   ../../Classes/Utils/IntTweener.cpp \
                   ../../Classes/Utils/Jukebox.cpp \
                   ../../Classes/Utils/LangFX.cpp \
                   ../../Classes/Utils/Localizer.cpp \
                   ../../Classes/Utils/PoolIndexer.cpp \
                   ../../Classes/Utils/PWDebug.cpp \
                   ../../Classes/Utils/ReusableCache.cpp \
                   ../../Classes/Utils/SpriteColorer.cpp \
                   ../../Classes/Utils/TextUtils.cpp \
                   ../../Classes/Utils/Transitions.cpp \
                   ../../Classes/Utils/UINavigator.cpp \
                   ../../Classes/Utils/Utils.cpp \
                   ../../Classes/Utils/UtilsNS.cpp \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes $(LOCAL_PATH) $(LOCAL_PATH)/../../breakpad/src $(LOCAL_PATH)/../../breakpad/src/common/android/include

LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static cocosdenshion_static cocos_extension_static breakpad_client

include $(BUILD_SHARED_LIBRARY)

include /Users/paulmcphee/iphone-apps/cocos2dx/cocos2d-x-2.2.6/samples/Cpp/PuzzleHound/breakpad/android/google_breakpad/Android.mk

$(call import-module,CocosDenshion/android) \
$(call import-module,cocos2dx) \
$(call import-module,extensions)
