#ifndef __ILOCALIZABLE_H__
#define __ILOCALIZABLE_H__

class ILocalizable
{
public:
    static const int kNonLocalizableTag = 123456789;
    
	virtual ~ILocalizable(void) { }
	
	// Clients should not register/deregister during this call. The collection is not guarded.
	virtual void localeDidChange(const char* fontKey, const char* FXFontKey) = 0;
};
#endif // __ILOCALIZABLE_H__
