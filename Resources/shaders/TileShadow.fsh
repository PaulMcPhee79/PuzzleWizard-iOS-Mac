#ifdef GL_ES
precision mediump float;
#endif

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_occTexCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_occTexCoord;
#endif

uniform sampler2D u_texture;
uniform sampler2D u_occTexture;

void main()
{
	float occSample = 1.0 - texture2D(u_occTexture, v_occTexCoord).r;
	gl_FragColor = texture2D(u_texture, v_texCoord) * occSample * v_fragmentColor;
}
