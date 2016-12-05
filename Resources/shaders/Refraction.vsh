
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_scrollCoord;

uniform mediump vec2 u_displacementScroll;
uniform mediump float u_scroll;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_scrollCoord;

uniform vec2 u_displacementScroll;
uniform float u_scroll;
#endif

void main()
{
    gl_Position = CC_MVPMatrix * a_position;
	v_fragmentColor = a_color;
	v_texCoord = u_displacementScroll + a_texCoord;
	v_scrollCoord = vec2(a_texCoord.x + u_scroll, a_texCoord.y + u_scroll);
}
