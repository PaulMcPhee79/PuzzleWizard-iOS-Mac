
#include "Transitions.h"
#include <cmath>

// Easing functions from http://dojotoolkit.org and http://www.robertpenner.com/easing                                  

namespace CMTransitions
{
	float(* const LINEAR)(float) = linear;
	float(* const EASE_IN_LINEAR)(float) = easeInLinear;
	float(* const EASE_IN)(float) = easeIn;
	float(* const EASE_OUT)(float) = easeOut;
	float(* const EASE_IN_OUT)(float) = easeInOut;
	float(* const EASE_OUT_IN)(float) = easeOutIn;
	float(* const EASE_IN_BACK)(float) = easeInBack;
	float(* const EASE_OUT_BACK)(float) = easeOutBack;
	float(* const EASE_IN_OUT_BACK)(float) = easeInOutBack;
	float(* const EASE_OUT_IN_BACK)(float) = easeOutInBack;
	float(* const EASE_IN_ELASTIC)(float) = easeInElastic;
	float(* const EASE_OUT_ELASTIC)(float) = easeOutElastic;
	float(* const EASE_IN_OUT_ELASTIC)(float) = easeInOutElastic;
	float(* const EASE_OUT_IN_ELASTIC)(float) = easeOutInElastic;
	float(* const EASE_OUT_BOUNCE)(float) = easeOutInBounce;
	float(* const EASE_IN_BOUNCE)(float) = easeInBounce;
	float(* const EASE_IN_OUT_BOUNCE)(float) = easeInOutBounce;
	float(* const EASE_OUT_IN_BOUNCE)(float) = easeOutInBounce;

	static const double PI = 3.14159265358;

	float linear(float ratio) { return ratio; }

    float easeInLinear(float ratio) { return ratio * ratio; }
    float easeIn(float ratio) { return ratio * ratio * ratio; }
    float easeOut(float ratio) { float invRatio = ratio - 1.0f; return invRatio * invRatio * invRatio + 1.0f; }
    float easeInOut(float ratio) { return ((ratio < 0.5f) ? 0.5f * easeIn(ratio * 2.0f) : 0.5f * easeOut((ratio - 0.5f) * 2.0f) + 0.5f); }
    float easeOutIn(float ratio) { return ((ratio < 0.5f) ? 0.5f * easeOut(ratio * 2.0f) : 0.5f * easeIn((ratio - 0.5f) * 2.0f) + 0.5f); }

    float easeInBack(float ratio) { float s = 1.70158f; return (float)pow((double)ratio, 2.0) * ((s + 1.0f) * ratio - s); }
    float easeOutBack(float ratio) { float invRatio = ratio - 1.0f, s = 1.70158f; return (float)pow((double)invRatio, 2.0) * ((s + 1.0f) * invRatio + s) + 1.0f; }
    float easeInOutBack(float ratio) { return ((ratio < 0.5f) ? 0.5f * easeInBack(ratio * 2.0f) : 0.5f * easeOutBack((ratio - 0.5f) * 2.0f) + 0.5f); }
    float easeOutInBack(float ratio) { return ((ratio < 0.5f) ? 0.5f * easeOutBack(ratio * 2.0f) : 0.5f * easeInBack((ratio - 0.5f) * 2.0f) + 0.5f); }

    float easeInElastic(float ratio)
    {
        if (ratio == 0.0f || ratio == 1.0f) return ratio;
        else
        {
            float p = 0.3f;
            float s = p / 4.0f;
            float invRatio = ratio - 1.0f;
            return -1.0f * (float)pow(2.0, 10.0 * invRatio) * (float)sin(((double)invRatio - (double)s) * PI / p);
        }
    }

    float easeOutElastic(float ratio)
    {
        if (ratio == 0.0f || ratio == 1.0f) return ratio;
        else
        {
            float p = 0.3f;
            float s = p / 4.0f;
            return -1.0f * (float)pow(2.0, -10.0 * ratio) * (float)sin(((double)ratio - (double)s) * PI / p) + 1.0f;
        }
    }

    float easeInOutElastic(float ratio)
    {
        return (ratio < 0.5f) ? 0.5f * easeInElastic(ratio * 2.0f) : 0.5f * easeOutElastic((ratio - 0.5f) * 2.0f) + 0.5f;
    }

    float easeOutInElastic(float ratio)
    {
        return (ratio < 0.5f) ? 0.5f * easeOutElastic(ratio * 2.0f) : 0.5f * easeInElastic((ratio - 0.5f) * 2.0f) + 0.5f;
    }

	float easeOutBounce(float ratio)
    {
        float s = 7.5625f;
        float p = 2.75f;
        float l;

        if (ratio < (1.0f / p))
        {
            l = s * (float)pow((double)ratio, 2.0);
        }
        else
        {
            if (ratio < (2.0f / p))
            {
                ratio -= 1.5f / p;
                l = s * (float)pow((double)ratio, 2.0) + 0.75f;
            }
            else
            {
                ratio -= 2.625f / p;
                l = s * (float)pow((double)ratio, 2.0) + 0.984375f;
            }
        }

        return l;
    }

    float easeInBounce(float ratio)
    {
        return 1.0f - easeOutBounce(1.0f - ratio);
    }

    float easeInOutBounce(float ratio)
    {
        return (ratio < 0.5f) ? 0.5f * easeInBounce(ratio * 2.0f) : 0.5f * easeOutBounce((ratio - 0.5f) * 2.0f) + 0.5f;
    }

    float easeOutInBounce(float ratio)
    {
        return (ratio < 0.5f) ? 0.5f * easeOutBounce(ratio * 2.0f) : 0.5f * easeInBounce((ratio - 0.5f) * 2.0f) + 0.5f;
    }
}

