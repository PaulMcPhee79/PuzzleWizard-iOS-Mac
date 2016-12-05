#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec2 v_shaderCoord;
varying vec4 v_fragmentColor;
uniform sampler2D u_texture;
uniform sampler2D u_gradientTex;
uniform vec2 u_gradientCoord;

void main()
{
	vec4 texSample = texture2D(u_texture, v_texCoord);
	float gradientSample = texSample.a * texture2D(u_gradientTex, vec2(0.25 * v_shaderCoord.x + u_gradientCoord.x, v_shaderCoord.y)).b;
	gl_FragColor = vec4(texSample.rgb + gradientSample, texSample.a) * v_fragmentColor;
}
