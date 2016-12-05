#ifndef __TRANSITIONS_H__
#define __TRANSITIONS_H__

#include <cmath>

// Easing functions from http://dojotoolkit.org and http://www.robertpenner.com/easing                                  

typedef float(*TRANS_FUNC)(float);

namespace CMTransitions
{
	extern float(* const LINEAR)(float);
	extern float(* const EASE_IN_LINEAR)(float);
	extern float(* const EASE_IN)(float);
	extern float(* const EASE_OUT)(float);
	extern float(* const EASE_IN_OUT)(float);
	extern float(* const EASE_OUT_IN)(float);
	extern float(* const EASE_IN_BACK)(float);
	extern float(* const EASE_OUT_BACK)(float);
	extern float(* const EASE_IN_OUT_BACK)(float);
	extern float(* const EASE_OUT_IN_BACK)(float);
	extern float(* const EASE_IN_ELASTIC)(float);
	extern float(* const EASE_OUT_ELASTIC)(float);
	extern float(* const EASE_IN_OUT_ELASTIC)(float);
	extern float(* const EASE_OUT_IN_ELASTIC)(float);
	extern float(* const EASE_OUT_BOUNCE)(float);
	extern float(* const EASE_IN_BOUNCE)(float);
	extern float(* const EASE_IN_OUT_BOUNCE)(float);
	extern float(* const EASE_OUT_IN_BOUNCE)(float);

	float linear(float ratio);

    float easeInLinear(float ratio);
    float easeIn(float ratio);
    float easeOut(float ratio);
    float easeInOut(float ratio);
    float easeOutIn(float ratio);

    float easeInBack(float ratio);
    float easeOutBack(float ratio);
    float easeInOutBack(float ratio);
    float easeOutInBack(float ratio);

    float easeInElastic(float ratio);
    float easeOutElastic(float ratio);
    float easeInOutElastic(float ratio);
    float easeOutInElastic(float ratio);

	float easeOutBounce(float ratio);
    float easeInBounce(float ratio);
    float easeInOutBounce(float ratio);
    float easeOutInBounce(float ratio);
}

#endif // __TRANSITIONS_H__
