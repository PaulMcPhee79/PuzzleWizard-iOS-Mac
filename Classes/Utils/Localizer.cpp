
#include "Localizer.h"
#include <SceneControllers/SceneController.h>
#include <Utils/TextUtils.h>
#include <Utils/Utils.h>
#include <Utils/BridgingUtility.h>
#include <algorithm>
#include <assert.h>
#ifdef __ANDROID__
    #include <CMJniUtils.h>
#endif
USING_NS_CC;
using namespace std;

typedef map<string, string> StrMap;
typedef map<string,CCTexture2D*> ContentMap;

static const char* kFontKeyPrefix = "fonts/";
static const char* kFontImageSuffix = ""; //"_0";

static Localizer::LocaleType s_Locale = Localizer::INVALID_LOCALE;
static StrMap* s_LocaleMaps[Localizer::kNumLocales] = {};
static ContentMap s_ContentManager;
static StrMap* s_LocaleStrings = NULL;

const Localizer::LocaleType Localizer::kLocales[Localizer::kNumLocales] = {
    Localizer::EN,
    Localizer::CN,
    Localizer::DE,
    Localizer::ES,
    Localizer::FR,
    Localizer::IT,
    Localizer::JP,
    Localizer::KR
};

static SceneController* s_Scene = NULL;
void Localizer::setScene(SceneController* owner, SceneController* setting)
{
    // Don't allow previous owners to NULL new owners (usually via their destructors)
    if (s_Scene == NULL || s_Scene == owner || setting)
        s_Scene = setting;
}

void Localizer::invalidateStateCaches(void)
{
	LocaleType locale = getLocale();
	if (locale == Localizer::INVALID_LOCALE)
		return;

	string texName = fontKeyForLocale(locale).append(kFontImageSuffix);
	ccTexParams params = {GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
	CCTexture2D* tex = s_Scene->textureByName(texName.c_str());
	if (tex)
	{
		tex->setTexParameters(&params);
		tex->generateMipmap();
	}

	texName = FXFontKeyForLocale(locale).append(kFontImageSuffix);
	tex = s_Scene->textureByName(texName.c_str());
	if (tex)
	{
		tex->setTexParameters(&params);
		tex->generateMipmap();
	}
}

void Localizer::initLocalizationStrings(LocaleType srcLocale, LocaleType destLocale)
{
    StrMap* srcLocaleStrings = getStringsForLocale(srcLocale);
    StrMap* destLocaleStrings = getStringsForLocale(destLocale);
    
    if (srcLocaleStrings == NULL || destLocaleStrings == NULL || destLocaleStrings->size() != 0) // Don't init NULL and don't init more than once
        return;
    
    std::string fullpath = CCFileUtils::sharedFileUtils()->fullPathForFilename(pathForLocaleStrings(destLocale).c_str());
    CCString *contents = CCString::createWithContentsOfFile(fullpath.c_str());
    
    std::string line;
    std::string strLeft = contents->getCString();
    
    while (strLeft.length() > 0)
    {
        int pos = (int)strLeft.find('\n');
        
        if (pos != (int)std::string::npos)
        {
            // the data is more than a line.get one line
            line = strLeft.substr(0, pos);
            strLeft = strLeft.substr(pos + 1);
        }
        else
        {
            // get the left data
            line = strLeft;
            strLeft.erase();
        }
        
        replace(line.begin(), line.end(), '^', '\n');
        
        int tokenIndex = (int)line.find(',');
        if (tokenIndex != string::npos)
        {
            string srcStr = line.substr(0, tokenIndex);
            string destStr = line.substr(tokenIndex+1);
            (*srcLocaleStrings)[destStr] = srcStr;
            (*destLocaleStrings)[srcStr] = destStr;
        }
    }
}

void Localizer::initLocalizationContent(LocaleType locale)
{
    if (locale == Localizer::INVALID_LOCALE)
        return;
    
    ccTexParams params = {GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE};
    
    {
        string fontKey = fontKeyForLocale(locale);
        if (s_ContentManager.count(fontKey) != 0)
            return;

        string texName = fontKey;
        texName.append(kFontImageSuffix);
        CCTexture2D* tex = s_Scene->textureByName(texName.c_str());
        if (tex == NULL)
            tex = CCTextureCache::sharedTextureCache()->addImage(texName.append(CM_IMG_FONT_EXT).c_str());

        string fontName = fontKey;
        s_Scene->addFontName(fontName.append(CM_FONT_EXT).c_str(), TextUtils::kBaseFontSize);
        s_ContentManager[fontKey] = tex;
        tex->setTexParameters(&params);
        tex->generateMipmap();
    }
    
    {
        string FXFontKey = FXFontKeyForLocale(locale);
        if (s_ContentManager.count(FXFontKey) != 0)
            return;

        string texName = FXFontKey;
        texName.append(kFontImageSuffix);
        CCTexture2D* FXTex = s_Scene->textureByName(texName.c_str());
        if (FXTex == NULL)
            FXTex = CCTextureCache::sharedTextureCache()->addImage(texName.append(CM_IMG_FONT_EXT).c_str());
        
        string fontName = FXFontKey;
        s_Scene->addFontName(fontName.append(CM_FONT_EXT).c_str(), TextUtils::kBaseFXFontSize);
        s_ContentManager[FXFontKey] = FXTex;
        FXTex->setTexParameters(&params);
        FXTex->generateMipmap();
    }
}

void Localizer::destroyLocalizationContent(void)
{
    CCLabelBMFont::purgeCachedData();
    for (int i = Localizer::EN; i < Localizer::INVALID_LOCALE; ++i)
    {
        purgeLocalizationStrings(static_cast<LocaleType>(i));
        purgeLocalizationContent(static_cast<LocaleType>(i));
    }
}

void Localizer::purgeLocale(LocaleType fromLocale, LocaleType toLocale)
{
    if (fromLocale == Localizer::INVALID_LOCALE || fromLocale == toLocale)
        return;
    
    // Don't remove toLocale's font names in case this is called after a locale transition. If it's
    // not, then the new locale will overwrite the old ones anyway.
    if (s_Scene && toLocale == Localizer::INVALID_LOCALE)
    {
        s_Scene->removeFontName(TextUtils::kBaseFontSize);
        s_Scene->removeFontName(TextUtils::kBaseFXFontSize);
    }
    
    if (string(fontKeyForLocale(fromLocale)).compare(fontKeyForLocale(toLocale)) != 0) // All EU langs share the same content
    {
        CCLabelBMFont::purgeCachedData();
        purgeLocalizationContent(fromLocale);
    }
    
    if (fromLocale != Localizer::EN)
        purgeLocalizationStrings(fromLocale);
}

void Localizer::purgeLocalizationStrings(LocaleType locale)
{
    if (locale == Localizer::INVALID_LOCALE)
    {
        for (int i = 0; i < Localizer::kNumLocales; ++i)
        {
            delete s_LocaleMaps[i], s_LocaleMaps[i] = NULL;
        }
    }
    else
    {
        delete s_LocaleMaps[locale], s_LocaleMaps[locale] = NULL;
    }
}

void Localizer::purgeLocalizationContent(LocaleType locale)
{
    string fontKey = fontKeyForLocale(locale);
    ContentMap::iterator it = s_ContentManager.find(fontKey);
    if (it != s_ContentManager.end())
    {
        CCTextureCache::sharedTextureCache()->removeTexture(it->second);
        s_ContentManager.erase(fontKey);
    }
    
    string FXFontKey = FXFontKeyForLocale(locale);
    it = s_ContentManager.find(FXFontKey);
    if (it != s_ContentManager.end())
    {
        CCTextureCache::sharedTextureCache()->removeTexture(it->second);
        s_ContentManager.erase(FXFontKey);
    }
}

std::string Localizer::pathForLocaleStrings(LocaleType locale)
{
    //return "data/locales/" + locale2String(locale) + ".txt";
    return string(locale2String(locale)).append(".txt");
}

Localizer::LocaleType Localizer::getLocale(void)
{
    return s_Locale;
}

void Localizer::setLocale(LocaleType locale)
{
    if (locale == s_Locale || locale == Localizer::INVALID_LOCALE)
        return;
    
    CCAssert(s_LocaleMaps[locale] != NULL, string("Locale must be initialized before being set: ").append(locale2String(locale)).c_str());
    
    s_LocaleStrings = s_LocaleMaps[locale];
    s_Locale = locale;
}

std::string Localizer::localizeString(const char* value)
{
    return localizeString(value, getLocale());
}

std::string Localizer::localizeString(const char* value, LocaleType locale)
{
    return localizeString(value ? std::string(value) : std::string(""), locale);
}

std::string Localizer::localizeString(const std::string& str)
{
    return localizeString(str, getLocale());
}

std::string Localizer::localizeString(const std::string& str, LocaleType locale)
{
    if (str.empty()) return "";
    
    StrMap* localeStrings = getStringsForLocale(locale);
    if (localeStrings)
    {
        StrMap::iterator it = localeStrings->find(str);
        if (it != localeStrings->end())
            return it->second;
        else
        {
            StrMap* enStrings = getStringsForLocale(Localizer::EN);
            if (enStrings)
            {
                it = enStrings->find(str);
                if (it != enStrings->end())
                {
                    it = localeStrings->find(it->second);
                    if (it != localeStrings->end())
                        return it->second;
                }
            }
        }
    }
    
    return str;
}

std::string Localizer::fontKeyPrefixForLocale(LocaleType locale)
{
    return string(kFontKeyPrefix).append(locale2FontPrefix(locale)).append("-");
}

std::string Localizer::fontKeyForLocale(LocaleType locale)
{
    return CMUtils::strConcatVal(fontKeyPrefixForLocale(locale).c_str(), TextUtils::kBaseFontSize);
}

std::string Localizer::FXFontKeyPrefixForLocale(LocaleType locale)
{
    return string(kFontKeyPrefix).append(locale2FontPrefix(locale)).append("-FX-");
}

std::string Localizer::FXFontKeyForLocale(LocaleType locale)
{
    return CMUtils::strConcatVal(FXFontKeyPrefixForLocale(locale).c_str(), TextUtils::kBaseFXFontSize);
}

std::string Localizer::iconTextureNameForLocale(LocaleType locale)
{
    return string(fontKeyForLocale(locale)).append(kFontImageSuffix);
}

std::string Localizer::FXIconTextureNameForLocale(LocaleType locale)
{
    return string(FXFontKeyForLocale(locale)).append(kFontImageSuffix);
}

std::map<string, string>* Localizer::getStringsForLocale(LocaleType locale)
{
    CCAssert(locale != Localizer::INVALID_LOCALE, string("Invalid locale in Localizer::getStringsForLocale: ").append(locale2String(locale)).c_str());
    if (s_LocaleMaps[locale] == NULL)
        s_LocaleMaps[locale] = new StrMap();
    
    return s_LocaleMaps[locale];
}

void Localizer::setStringsForLocale(LocaleType locale, std::map<string, string>* localeStrings)
{
    if (localeStrings && locale != Localizer::INVALID_LOCALE)
    {
        CCAssert(s_LocaleMaps[locale] == NULL, string("Localizer::setStringsForLocale when locale strings are already present: ").append(locale2String(locale)).c_str());
        s_LocaleMaps[locale] = localeStrings;
    }
}

const char* Localizer::locale2String(LocaleType locale)
{
    switch (locale) {
        case Localizer::EN: return "EN";
        case Localizer::CN: return "CN";
        case Localizer::DE: return "DE";
        case Localizer::ES: return "ES";
        case Localizer::FR: return "FR";
        case Localizer::IT: return "IT";
        case Localizer::JP: return "JP";
        case Localizer::KR: return "KR";
        default: return "INVALID";
    }
}

const char* Localizer::locale2StringLower(LocaleType locale)
{
    switch (locale) {
        case Localizer::EN: return "en";
        case Localizer::CN: return "cn";
        case Localizer::DE: return "de";
        case Localizer::ES: return "es";
        case Localizer::FR: return "fr";
        case Localizer::IT: return "it";
        case Localizer::JP: return "jp";
        case Localizer::KR: return "kr";
        default: return "invalid";
    }
}

const char* Localizer::locale2FontPrefix(LocaleType locale)
{
    switch (locale) {
        case Localizer::EN:
        case Localizer::DE:
        case Localizer::ES:
        case Localizer::FR:
        case Localizer::IT:
            return "EU";
        case Localizer::CN: return "CN";
        case Localizer::JP: return "JP";
        case Localizer::KR: return "KR";
        default: return "INVALID";
    }
}

struct ISOStruct {
    static map<string, Localizer::LocaleType> createMap(void)
    {
        map<string, Localizer::LocaleType> m;
        m["en"] = Localizer::EN;
        m["zh"] = Localizer::CN;
        m["de"] = Localizer::DE;
        m["es"] = Localizer::ES;
        m["fr"] = Localizer::FR;
        m["it"] = Localizer::IT;
        m["ja"] = Localizer::JP;
        m["ko"] = Localizer::KR;
        return m;
    }
    
    static const map<string, Localizer::LocaleType> s_ISOLocaleMap;
};

const map<string, Localizer::LocaleType> ISOStruct::s_ISOLocaleMap = ISOStruct::createMap();

Localizer::LocaleType Localizer::getLocaleTypeFromCurrentUICulture(void)
{
    // 1. Query the Localizable.strings
    {
        string CM_LocaleKey = CMUtils::toLowerEN(BridgingUtility::localizedString("CM_LocaleKey"));
        map<string, Localizer::LocaleType>::const_iterator itMap = ISOStruct::s_ISOLocaleMap.find(CM_LocaleKey);
        if (itMap != ISOStruct::s_ISOLocaleMap.end())
            return itMap->second;
    }
#ifdef __APPLE__
    // 2. Walk through the preferred languages until we find a match
    {
        vector<string> isoCodes = BridgingUtility::getPreferredLanguageCodes();
        for (vector<string>::iterator itVec = isoCodes.begin(); itVec != isoCodes.end(); ++itVec)
        {
            map<string, Localizer::LocaleType>::const_iterator itMap = ISOStruct::s_ISOLocaleMap.find((CMUtils::toLowerEN(*itVec)));
            if (itMap != ISOStruct::s_ISOLocaleMap.end())
                return itMap->second;
        }
    }

    // 3. Finally, fall back to the default language
    return Localizer::EN;
#elif defined(__ANDROID__)
    return CMJniUtils::getLocaleTypeFromCurrentUICulture(); // Defaults to EN
#else
    return Localizer::EN;
#endif
}

std::string Localizer::getLocaleStringFromCurrentUICultureExplicit(void)
{
#ifdef __APPLE__
    vector<string> isoCodes = BridgingUtility::getPreferredLanguageCodes();
    if (isoCodes.size() > 0)
        return CMUtils::toLowerEN(isoCodes[0]);
#endif
    return "";
}

Localizer::LocaleType Localizer::getLaunchLocale(void)
{
    LocaleType locale = Localizer::EN;
#ifdef CHEEKY_MOBILE
    #if !CM_BETA
        CCUserDefault* userDefaults = CCUserDefault::sharedUserDefault();
        Localizer::LocaleType currentUILocale = getLocaleTypeFromCurrentUICulture();
    
        // We only use the saved locale to set the crash log state on mobile devices.
        LocaleType savedLocale = (LocaleType)userDefaults->getIntegerForKey(CMSettings::I_LOCALE, (int)Localizer::INVALID_LOCALE);
        if (savedLocale != currentUILocale)
        {
            const char* params[] = { "Locale", Localizer::locale2String(currentUILocale) };
            BridgingUtility::logEvent(BridgingUtility::EV_NAME_LOCALE_CHANGED, CMUtils::makeStrMap(params));
        }
    
        locale = currentUILocale;
    #endif
#else
    CCUserDefault* userDefaults = CCUserDefault::sharedUserDefault();
    LocaleType currentUILocale = getLocaleTypeFromCurrentUICulture();
    std::string currentSysLocaleStr = getLocaleStringFromCurrentUICultureExplicit();
    std::string savedSysLocaleStr = userDefaults->getStringForKey(CMSettings::S_SYSLOCALE, "");
    if (currentSysLocaleStr.compare(savedSysLocaleStr) != 0)
    {
        // User changed system locale since last launch. Respect their new changes and override any other settings. However, don't save this new
        // setting, else the manually selected in-game setting will override their new system setting on next launch. Only write the system setting
        // on initial launch (ie saved == Localizer::INVALID_LOCALE) and when the user manually sets the in-game locale.
        if (savedSysLocaleStr.empty())
            userDefaults->setStringForKey(CMSettings::S_SYSLOCALE, currentSysLocaleStr.c_str());
        
        locale = currentUILocale;
    }
    else
    {
        locale = (LocaleType)userDefaults->getIntegerForKey(CMSettings::I_LOCALE, (int)currentUILocale);
    }
#endif
    
    return locale;
}
