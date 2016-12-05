#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
uniform sampler2D u_texture;
uniform sampler2D u_noiseTex;
uniform float u_threshold;

void main()
{
	float noiseSample = texture2D(u_noiseTex, v_texCoord).r;
    float noiseThreshold = noiseSample - u_threshold;
    vec4 diffuseSample = noiseSample < u_threshold
        ? vec4(0,0,0,0)
        : texture2D(u_texture, v_texCoord);
    diffuseSample *= noiseThreshold < 0.25
        ? vec4(1.0,1.0,1.0,2.0 * noiseThreshold) // Highlight dissolve edge
        : vec4(1.0,1.0,1.0,1.0);
    gl_FragColor = diffuseSample * v_fragmentColor;
}
