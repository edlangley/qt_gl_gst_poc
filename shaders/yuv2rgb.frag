// Perform YUV to RGB conversion on I420 format planar YUV data
// Using formula:
// R = 1.164(Y - 16) + 1.596(V - 128)
// G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
// B = 1.164(Y - 16)                  + 2.018(U - 128)

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect vidTexture;
uniform float yHeight, yWidth;

// YUV offset (reciprocals of 255 based offsets above)
const vec3 offset = vec3(-0.0625, -0.5, -0.5);
// RGB coefficients 
const vec3 rCoeff = vec3(1.164,  0.000,  1.596);
const vec3 gCoeff = vec3(1.164, -0.391, -0.813);
const vec3 bCoeff = vec3(1.164,  2.018,  0.000);

vec4 yuv2rgb(void)
{
	vec3 yuv, rgb;
	vec4 texCoord;

	texCoord = gl_TexCoord[0];
	
	// lookup Y
	yuv.r = texture2DRect(vidTexture, texCoord.xy).r;
	// lookup U
	// co-ordinate conversion algorithm for i420:
	//	x /= 2.0; if modulo2(y) then x += width/2.0;
	texCoord.x /= 2.0;	
	if((texCoord.y - floor(texCoord.xy)) == 0.0)
	{
		texCoord.x += (yWidth/2.0);
	}
	texCoord.y = yHeight+(texCoord.y/4.0);
	yuv.g = texture2DRect(vidTexture, texCoord.xy).r;
	// lookup V
	texCoord.y += yHeight/4.0;
	yuv.b = texture2DRect(vidTexture, texCoord.xy).r;

	// Convert
	yuv += offset;
	rgb.r = dot(yuv, rCoeff);
	rgb.g = dot(yuv, gCoeff);
	rgb.b = dot(yuv, bCoeff);

	return vec4(rgb, 1.0);
}
