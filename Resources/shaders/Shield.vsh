#ifdef GL_ES
precision mediump float;
#endif

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_stencilCoord0;
varying mediump vec2 v_stencilCoord1;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_stencilCoord0;
varying vec2 v_stencilCoord1;
#endif

uniform float u_shieldAlpha;
uniform float u_stencilRotation0;
uniform float u_stencilRotation1;

void main()
{
    gl_Position = CC_MVPMatrix * a_position;
	v_fragmentColor = a_color * u_shieldAlpha;
	v_texCoord = a_texCoord;
    
    float s = sin(u_stencilRotation0);
    float c = cos(u_stencilRotation0);
    mat2 rotMatrix = mat2(c, -s, s, c);
    rotMatrix *= 0.5;
    rotMatrix += 0.5;
    rotMatrix = rotMatrix * 2.0 - 1.0;
    v_stencilCoord0 = a_texCoord - 0.5;
    v_stencilCoord0 = v_stencilCoord0 * rotMatrix + 0.5;
    
	s = sin(u_stencilRotation1);
    c = cos(u_stencilRotation1);
    rotMatrix = mat2(c, -s, s, c);
    rotMatrix *= 0.5;
    rotMatrix += 0.5;
    rotMatrix = rotMatrix * 2.0 - 1.0;
    v_stencilCoord1 = a_texCoord - 0.5;
    v_stencilCoord1 = v_stencilCoord1 * rotMatrix + 0.5;
}
