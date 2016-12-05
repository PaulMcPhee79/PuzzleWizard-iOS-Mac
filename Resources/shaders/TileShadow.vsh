
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_occTexCoord;
uniform mediump vec4 u_occRegion;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_occTexCoord;
uniform vec4 u_occRegion;
#endif



void main()
{
    gl_Position = CC_MVPMatrix * a_position;
	v_fragmentColor = a_color;
	v_texCoord = a_texCoord;

	v_occTexCoord = vec2(a_texCoord.x - u_occRegion.x, 1.0 - (a_texCoord.y + u_occRegion.y));
	mat2 scaleMatrix = mat2(1.0 / u_occRegion.z, 0.0, 0.0, 1.0 / u_occRegion.w);
	v_occTexCoord = v_occTexCoord * scaleMatrix;
}
