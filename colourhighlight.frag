// GLSL shader which makes output texture monochrome except colours
// in a certain range for each component
// This shader must be linked with another containing yuv2rgb function
// to handle the video data first

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect vidTexture;
uniform float yHeight, yWidth;


void main(void)
{
	vec4 rgbColour = yuv2rgb(vidTexture);

	

}
