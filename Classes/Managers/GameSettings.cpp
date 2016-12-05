
#include "GameSettings.h"

namespace CMSettings
{
    const int kDefaultWinWidth = 1024;
    const int kDefaultWinHeight = 768;
    const int kMaxVolume = 10;
    const int kDefaultSfxVolume = 10;
#ifdef CHEEKY_MOBILE
    const int kDefaultMusicVolume = 10; //6;
#else
    const int kDefaultMusicVolume = 7;
#endif
    
    const char* B_SETTINGS_INITIALIZED = "cm_init";
    const char* B_FULLSCREEN = "fullscreen";
	const char* B_SFX = "sfx";
	const char* B_MUSIC = "music";
    const char* B_COLOR_BLIND_MODE = "color_blind_mode";
    const char* B_CLOUD_ENABLED = "cloud_enabled";
    const char* B_LOW_POWER_MODE_ENABLED = "low_power_enabled";
    
    const char* I_WIN_WIDTH = "win_width";
	const char* I_WIN_HEIGHT = "win_height";
    const char* I_SFX = "sfx_volume";
	const char* I_MUSIC = "music_volume";
    const char* I_LOCALE = "locale_lang";
    const char* I_RATE_GAME = "rate_game";
    const char* I_INTERRUPTED_PUZZLE_ID = "interrupted_pid";
    
    const char* S_BETA_NAME = "beta_name";
    const char* S_SYSLOCALE = "syslocale";
    const char* S_LICENSE = "rzglkjgH945tkmdFg";
}
