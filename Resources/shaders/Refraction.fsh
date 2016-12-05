#ifdef GL_ES
precision mediump float;
#endif

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_scrollCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_scrollCoord;
#endif

uniform sampler2D u_texture;
uniform sampler2D u_displacementTex;

void main()
{
	float displacement = texture2D(u_displacementTex, v_texCoord).x;
	vec2 texCoord = vec2(v_scrollCoord.x + displacement, v_scrollCoord.y + displacement);
	gl_FragColor = texture2D(u_texture, texCoord); // * v_fragmentColor;
}
