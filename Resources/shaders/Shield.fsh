#ifdef GL_ES
precision mediump float;
#endif

const vec2 c_centerPos = vec2(0.5, 0.5);

#ifdef GL_ES
varying mediump vec2 v_texCoord;
varying mediump vec2 v_stencilCoord0;
varying mediump vec2 v_stencilCoord1;
varying lowp vec4 v_fragmentColor;
uniform mediump vec2 u_displacementScroll;
#else
varying vec2 v_texCoord;
varying vec2 v_stencilCoord0;
varying vec2 v_stencilCoord1;
varying vec4 v_fragmentColor;
uniform vec2 u_displacementScroll;
#endif

uniform sampler2D u_texture;
uniform sampler2D u_plasmaTex;
uniform sampler2D u_sin2xTable;
uniform sampler2D u_stencilTex0;
uniform sampler2D u_stencilTex1;

void main()
{
    vec4 stencilSample = texture2D(u_stencilTex0, v_stencilCoord0) * texture2D(u_stencilTex1, v_stencilCoord1);
    vec4 texSample = texture2D(u_texture, v_texCoord) * (stencilSample.r + stencilSample.g);
    vec2 sinSample = texture2D(u_sin2xTable, v_texCoord).rg;
    vec4 plasmaSample = texture2D(u_plasmaTex, u_displacementScroll + sinSample) * stencilSample.r;
    //gl_FragColor = (texSample + plasmaSample) * v_fragmentColor; // For non-cropped shields
	gl_FragColor = (texSample + texSample.r * plasmaSample) * v_fragmentColor; // For cropped shields
}
