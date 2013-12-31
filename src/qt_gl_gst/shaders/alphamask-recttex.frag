// GLSL shader which alpha blends output of video conversion using another texture as
// an alpha mask, using an average of rgb
// This shader must be linked with another containing the yuv2rgb function
// to handle the video data first

uniform sampler2DRect u_alphaTexture;

varying vec3 v_alphaTexCoord;

vec4 yuv2rgb(void);

void main(void)
{
	vec4 alphaColour;
	float alphaAverage;
	vec4 rgbColour = yuv2rgb();

	alphaColour = texture2DRect(u_alphaTexture, v_alphaTexCoord.xy);
	alphaAverage = alphaColour.r + alphaColour.g + alphaColour.b;
	alphaAverage /= 3.0;

	gl_FragColor = vec4(rgbColour.rgb, alphaAverage);
}
