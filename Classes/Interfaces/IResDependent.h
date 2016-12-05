#ifndef __IRESDEPENDENT_H__
#define __IRESDEPENDENT_H__

class IResDependent
{
public:
	virtual ~IResDependent(void) { }
	
	// Clients should not register/deregister during this call. The collection is not guarded.
	virtual void resolutionDidChange(void) = 0;
};
#endif // __IRESDEPENDENT_H__
