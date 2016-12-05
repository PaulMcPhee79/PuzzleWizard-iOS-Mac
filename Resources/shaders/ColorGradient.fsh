#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
uniform sampler2D u_texture;
uniform sampler2D u_gradientTex;
uniform vec2 u_gradientCoord;

void main()
{
	vec4 texSample = texture2D(u_texture, v_texCoord);
	vec4 gradientSample = texture2D(u_gradientTex, u_gradientCoord);
	//gl_FragColor = texSample * gradientSample * v_fragmentColor;
	//gl_FragColor = (0.5 * texSample + 0.85 * texSample.a * gradientSample); // * v_fragmentColor;
    gl_FragColor = (0.65 * texSample + 0.7 * texSample.a * gradientSample);
}
