// GLSL shader which makes output texture monochrome except colours
// in a certain range for each component
// This shader must be linked with another containing yuv2rgb function
// to handle the video data first


uniform mediump vec4 u_colrToDisplayMin, u_colrToDisplayMax;
uniform mediump vec4 u_componentSwapR, u_componentSwapG, u_componentSwapB;

mediump vec4 yuv2rgb(void);

void main(void)
{
	mediump vec4 rgbColour = yuv2rgb();
	mediump vec4 swapColourSum;
	mediump vec4 swappedColour;
	mediump float monoComponent;

	if((rgbColour.r > u_colrToDisplayMin.r) && (rgbColour.r < u_colrToDisplayMax.r) &&
	   (rgbColour.r > u_colrToDisplayMin.g) && (rgbColour.r < u_colrToDisplayMax.g) &&
	   (rgbColour.r > u_colrToDisplayMin.b) && (rgbColour.r < u_colrToDisplayMax.b))
	{
/*
		swappedColour.r = rgbColour.g;
		swappedColour.g = rgbColour.b;
		swappedColour.b = rgbColour.r;
*/

		swapColourSum = rgbColour * u_componentSwapR;
		//swappedColour.r = (swapColourSum.r + swapColourSum.g + swapColourSum.b)/3;
		swappedColour.r = clamp((swapColourSum.r + swapColourSum.g + swapColourSum.b), 0.0, 1.0);
		
		swapColourSum  = rgbColour * u_componentSwapG;
		//swappedColour.g = (swapColourSum.r + swapColourSum.g + swapColourSum.b)/3;
		swappedColour.g = clamp((swapColourSum.r + swapColourSum.g + swapColourSum.b), 0.0, 1.0);

		swapColourSum  = rgbColour * u_componentSwapB;
		//swappedColour.b = (swapColourSum.r + swapColourSum.g + swapColourSum.b)/3;
		swappedColour.b = clamp((swapColourSum.r + swapColourSum.g + swapColourSum.b), 0.0, 1.0);

		swappedColour.a = 1.0;
		gl_FragColor = swappedColour;
	}
	else
	{
		// monochrome:
		monoComponent = rgbColour.r + rgbColour.g + rgbColour.b;
		monoComponent /= 3.0;

		gl_FragColor = vec4(monoComponent, monoComponent, monoComponent, 1.0);
	}
}
