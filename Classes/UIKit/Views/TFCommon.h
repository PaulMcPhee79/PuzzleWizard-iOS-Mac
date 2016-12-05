
#define RUISCALE(s) TF_ruiScale(s)
#define EXPUIIMG(p) TF_expandUIImagePath(p)

typedef void(^TFEventHandler)(id);

float TF_ruiScale(float val);
NSString* TF_expandUIImagePath(NSString* imageName);

