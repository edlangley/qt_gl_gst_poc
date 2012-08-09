// GLSL shader which makes output texture monochrome except colours
// in a certain range for each component
// This shader must be linked with another containing yuv2rgb function
// to handle the video data first


uniform mediump vec4 u_colrToDisplayMin, u_colrToDisplayMax;

mediump vec4 yuv2rgb(void);

void main(void)
{
	mediump vec4 rgbColour = yuv2rgb();
	mediump float monoComponent;

	if((rgbColour.r > u_colrToDisplayMin.r) && (rgbColour.r < u_colrToDisplayMax.r) &&
	   (rgbColour.r > u_colrToDisplayMin.g) && (rgbColour.r < u_colrToDisplayMax.g) &&
	   (rgbColour.r > u_colrToDisplayMin.b) && (rgbColour.r < u_colrToDisplayMax.b))
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
