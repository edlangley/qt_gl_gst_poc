// GLSL shader which makes output texture monochrome except colours
// in a certain range for each component
// This shader must be linked with another containing yuv2rgb function
// to handle the video data first

#extension GL_ARB_texture_rectangle : enable

uniform vec4 u_colrToDisplayMin, u_colrToDisplayMax, u_componentToSwap;

vec4 yuv2rgb(void);

void main(void)
{
	vec4 rgbColour = yuv2rgb();
	vec4 swappedColour;
	float monoComponent;

	if((rgbColour.r > u_colrToDisplayMin.r) && (rgbColour.r < u_colrToDisplayMax.r) &&
	   (rgbColour.r > u_colrToDisplayMin.g) && (rgbColour.r < u_colrToDisplayMax.g) &&
	   (rgbColour.r > u_colrToDisplayMin.b) && (rgbColour.r < u_colrToDisplayMax.b))
	{
		if(u_componentToSwap.b > (u_componentToSwap.r + u_componentToSwap.g))
		{
			swappedColour.b = (rgbColour.r + rgbColour.g)/2;
			swappedColour.r = rgbColour.b;
			swappedColour.g = rgbColour.b;
		}

		// TODO: same for red and green ....
/*
		swappedColour.r = rgbColour.g;
		swappedColour.g = rgbColour.b;
		swappedColour.b = rgbColour.r
*/		

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
