#ifndef __LOCALIZER_UTILS_H__
#define __LOCALIZER_UTILS_H__

#include "cocos2d.h"
class SceneController;
USING_NS_CC;

class Localizer
{
public:
    enum LocaleType { EN = 0, CN, DE, ES, FR, IT, JP, KR, INVALID_LOCALE }; // Update kNumLocales if this length changes
    static const int kNumLocales = 8; // TODO: Try kNumLocales = LocaleType.INVALID_LOCALE;
    static const LocaleType kLocales[kNumLocales];
    
    static void setScene(SceneController* owner, SceneController* setting);
    static void invalidateStateCaches(void);
    static void initLocalizationStrings(LocaleType srcLocale, LocaleType destLocale);
    static void initLocalizationContent(LocaleType locale);
    static void destroyLocalizationContent(void);
    static void purgeLocale(LocaleType fromLocale, LocaleType toLocale = Localizer::INVALID_LOCALE);
    static std::string pathForLocaleStrings(LocaleType locale);
    static LocaleType getLocale(void);
    static void setLocale(LocaleType locale);
    static std::string localizeString(const char* str);
    static std::string localizeString(const char* str, LocaleType locale);
    static std::string localizeString(const std::string& str);
    static std::string localizeString(const std::string& str, LocaleType locale);
    static std::string fontKeyPrefixForLocale(LocaleType locale);
    static std::string fontKeyForLocale(LocaleType locale);
    static std::string FXFontKeyPrefixForLocale(LocaleType locale);
    static std::string FXFontKeyForLocale(LocaleType locale);
    static std::string iconTextureNameForLocale(LocaleType locale);
    static std::string FXIconTextureNameForLocale(LocaleType locale);
    static const char* locale2String(LocaleType locale);
    static const char* locale2StringLower(LocaleType locale);
    static LocaleType getLocaleTypeFromCurrentUICulture(void);
    static std::string getLocaleStringFromCurrentUICultureExplicit(void); // No matches attempted - just returns most preferred lang setting.
    static LocaleType getLaunchLocale(void);

private:
    Localizer();
    ~Localizer(void);
    Localizer(const Localizer& other);
	Localizer& operator=(const Localizer& rhs);
    
    static void purgeLocalizationStrings(LocaleType locale);
    static void purgeLocalizationContent(LocaleType locale);
    static std::map<std::string, std::string>* getStringsForLocale(LocaleType locale);
    static void setStringsForLocale(LocaleType locale, std::map<std::string, std::string>* localeStrings);
    static const char* locale2FontPrefix(LocaleType locale);
};
#endif // __LOCALIZER_UTILS_H__
