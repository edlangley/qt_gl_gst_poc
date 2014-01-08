// GLSL shader which alpha blends output of video conversion using another texture as
// an alpha mask, using an average of rgb
// This shader must be linked with another containing the yuv2rgb function
// to handle the video data first


uniform highp sampler2D u_alphaTexture;

varying highp vec3 v_alphaTexCoord;

mediump vec4 yuv2rgb(void);

void main(void)
{
	highp vec4 alphaColour;
	highp float alphaAverage;
	mediump vec4 rgbColour = yuv2rgb();

	alphaColour = texture2D(u_alphaTexture, v_alphaTexCoord.xy);
	alphaAverage = alphaColour.r + alphaColour.g + alphaColour.b;
	alphaAverage /= 3.0;

	gl_FragColor = vec4(rgbColour.rgb, alphaAverage);
}
