
#include "TextUtils.h"
#include <SceneControllers/SceneController.h>
#include <Extensions/CMLabelBMFont.h>
#include <assert.h>

static const int kBaseFontSizeInternal = 50; // 52
static const int kBaseFXFontSizeInternal = 76; //112;

static SceneController* s_Scene = NULL;
void TextUtils::setScene(SceneController* owner, SceneController* setting)
{
    // Don't allow previous owners to unset new owners (usually via their destructors)
    if (s_Scene == NULL || s_Scene == owner || setting)
        s_Scene = setting;
}

CCLabelBMFont* TextUtils::create(const std::string& str, int fontSize, int baseFontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    if (s_Scene)
    {
        CCLabelBMFont* label = CMLabelBMFont::create(str.c_str(), s_Scene->getFontName(baseFontSize).c_str(), width, alignment, imageOffset);
        label->setScale(fontSize / (float)baseFontSize);
        return label;
    }
    else
        return NULL;
}

CCLabelBMFont* TextUtils::createFX(const std::string& str, int fontSize, int baseFontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    if (s_Scene)
    {
        CCLabelBMFont* label = CCLabelBMFont::create(str.c_str(), s_Scene->getFontName(baseFontSize).c_str(), width, alignment, imageOffset);
        label->setScale(fontSize / (float)baseFontSize);
        return label;
    }
    else
        return NULL;
}

CCLabelBMFont* TextUtils::create(TextUtils::CMFontType fontType, const char *str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    switch (fontType)
    {
        case TextUtils::FONT_REGULAR:
            return create(str, fontSize, width, alignment, imageOffset);
        case TextUtils::FONT_FX:
        default:
            return createFX(str, fontSize, width, alignment, imageOffset);
    }
}

CCLabelBMFont* TextUtils::create(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    return create(s_Scene->localizeString(str), fontSize, kBaseFontSizeInternal, width, alignment, imageOffset);
}

CCLabelBMFont* TextUtils::createFX(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    return createFX(s_Scene->localizeString(str), fontSize, kBaseFXFontSizeInternal, width, alignment, imageOffset);
}

CCLabelBMFont* TextUtils::createCommonSML(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    return create(str, fontSize, 24, width, alignment, imageOffset);
}

CCLabelBMFont* TextUtils::createCommonLGE(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset)
{
    return create(str, fontSize, 40, width, alignment, imageOffset);
}

void TextUtils::switchFntFile(const char* fntFile, CCLabelBMFont* label, bool localize)
{
    if (fntFile && label)
    {
        if (localize)
        {
            std::string localizedString = s_Scene->localizeString(label->getString());
            label->setString("");
            label->setFntFile(fntFile);
            label->setString(localizedString);
        }
        else
        {
            std::string strCache = label->getString();
            label->setString("");
            label->setFntFile(fntFile);
            label->setString(strCache.c_str());
        }
    }
}

