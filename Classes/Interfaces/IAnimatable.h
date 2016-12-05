#ifndef __IANIMATABLE_H__
#define __IANIMATABLE_H__

#include <string>

class SceneController;

class IAnimatable
{
public:
    IAnimatable(void) : mScene(s_Scene) { }
	virtual ~IAnimatable(void);
#if DEBUG
    virtual std::string getAnimatableName(void) { return mAnimatableName; }
    void setAnimatableName(const char* value) { mAnimatableName = value; }
#else
    virtual std::string getAnimatableName(void) { return ""; }
    void setAnimatableName(const char* value) { /* ignore */ }
#endif
	virtual bool isComplete(void) const { return false; }
	virtual void* getTarget(void) const { return 0; }
	virtual void advanceTime(float dt) = 0;
    
    static SceneController* getAnimatablesScene();
	static void setAnimatablesScene(SceneController* value);
	static void relinquishAnimatablesScene(SceneController* scene);
    
#if DEBUG
    std::string mAnimatableName;
#endif
    
private:
    SceneController* mScene;
    static SceneController* s_Scene;
};
#endif // __IANIMATABLE_H__
