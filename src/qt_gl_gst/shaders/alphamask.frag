// GLSL shader which alpha blends output of video conversion using another texture as
// an alpha mask, using an average of rgb
// This shader must be linked with another containing the yuv2rgb function
// to handle the video data first

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect alphaTexture;

vec4 yuv2rgb(void);

void main(void)
{
	vec4 alphaColour;
	float alphaAverage;
	vec4 rgbColour = yuv2rgb();

	alphaColour = texture2DRect(alphaTexture, gl_TexCoord[1].xy);
	alphaAverage = alphaColour.r + alphaColour.g + alphaColour.b;
	alphaAverage /= 3.0;

	gl_FragColor = vec4(rgbColour.rgb, alphaAverage);
/*
	if(alphaAverage == 0.0)
	{
		gl_FragColor = vec4(rgbColour.rgb, 1.0);
	}
	else
	{
		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
	}
*/
	//gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
