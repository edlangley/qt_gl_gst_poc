// GLSL shader which makes output texture monochrome except colours
// in a certain range for each component
// This shader must be linked with another containing yuv2rgb function
// to handle the video data first

#extension GL_ARB_texture_rectangle : enable

uniform vec4 colrToDisplayMin, colrToDisplayMax;

vec4 yuv2rgb(void);

void main(void)
{
	vec4 rgbColour = yuv2rgb();//vidTexture, yWidth, yHeight);
	float monoComponent;

	if((rgbColour.r > colrToDisplayMin.r) && (rgbColour.r < colrToDisplayMax.r) &&
	   (rgbColour.r > colrToDisplayMin.g) && (rgbColour.r < colrToDisplayMax.g) &&
	   (rgbColour.r > colrToDisplayMin.b) && (rgbColour.r < colrToDisplayMax.b))
	{
		gl_FragColor = rgbColour;

	}
	else
	{
		// monochrome:
		monoComponent = rgbColour.r + rgbColour.g + rgbColour.b;
		monoComponent /= 3.0;

		gl_FragColor = vec4(monoComponent, monoComponent, monoComponent, 1.0);
	}
}
