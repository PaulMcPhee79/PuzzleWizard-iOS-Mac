#ifndef __TEXT_UTILS_H__
#define __TEXT_UTILS_H__

#include "cocos2d.h"
class SceneController;
USING_NS_CC;

class TextUtils
{
public:
    enum CMFontType { FONT_REGULAR, FONT_FX };
    
    static const int kBaseFontSize = 50; // This should be "100", but it's only a map key and is annoying to update font resources.
    static const int kBaseFXFontSize = 152;
    
    static void setScene(SceneController* owner, SceneController* setting);
    static CCLabelBMFont* create(CMFontType fontType, const char *str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
    static CCLabelBMFont* create(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
    static CCLabelBMFont* createFX(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
    static CCLabelBMFont* createCommonSML(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
    static CCLabelBMFont* createCommonLGE(const std::string& str, int fontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
    static void switchFntFile(const char* fntFile, CCLabelBMFont* label, bool localize = true);
    
private:
    TextUtils();
    ~TextUtils(void);
    TextUtils(const TextUtils& other);
	TextUtils& operator=(const TextUtils& rhs);
    
    static CCLabelBMFont* create(const std::string& str, int fontSize, int baseFontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
    static CCLabelBMFont* createFX(const std::string& str, int fontSize, int baseFontSize, float width, CCTextAlignment alignment, CCPoint imageOffset);
};
#endif // __TEXT_UTILS_H__
