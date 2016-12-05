#ifndef __GAME_SETTINGS_H__
#define __GAME_SETTINGS_H__

namespace CMSettings
{
    extern const int kDefaultWinWidth;
    extern const int kDefaultWinHeight;
    extern const int kMaxVolume;
    extern const int kDefaultSfxVolume;
    extern const int kDefaultMusicVolume;
    
    extern const char* B_SETTINGS_INITIALIZED;
    extern const char* B_FULLSCREEN;
	extern const char* B_SFX;
	extern const char* B_MUSIC;
    extern const char* B_COLOR_BLIND_MODE;
    extern const char* B_CLOUD_ENABLED;
    extern const char* B_LOW_POWER_MODE_ENABLED;
    
    extern const char* I_WIN_WIDTH;
	extern const char* I_WIN_HEIGHT;
    extern const char* I_SFX;
	extern const char* I_MUSIC;
    extern const char* I_LOCALE;
    extern const char* I_RATE_GAME;
    extern const char* I_INTERRUPTED_PUZZLE_ID;
    
    extern const char* S_BETA_NAME;
    extern const char* S_SYSLOCALE;
    extern const char* S_LICENSE;
}

#endif // __GAME_SETTINGS_H__
