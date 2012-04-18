// Perform YUV to RGB conversion on I420 format planar YUV data
// Using formula:
// R = 1.164(Y - 16) + 1.596(V - 128)
// G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
// B = 1.164(Y - 16)                  + 2.018(U - 128)

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect vidTexture;
uniform float yHeight, yWidth;

// YUV offset 
const vec3 offset = vec3(-0.0625, -0.5, -0.5);
// RGB coefficients 
const vec3 rCoeff = vec3(1.164,  0.000,  1.596);
const vec3 gCoeff = vec3(1.164, -0.391, -0.813);
const vec3 bCoeff = vec3(1.164,  2.018,  0.000);

void main(void)
{
	vec3 yuv, rgb;
	vec4 texCoord;

	texCoord = gl_TexCoord[0];
/*	
	// Get the Y, U and V
	yuv.r = texture2D(vidTexture, texCoord.xy).r;
	texCoord.y = yHeight+(texCoord.y/4.0);
	yuv.g = texture2D(vidTexture, texCoord.xy).g;
	texCoord.y += yHeight/4.0;
	yuv.b = texture2D(vidTexture, texCoord.xy).b;

	// Convert
	yuv += offset;
	rgb.r = dot(yuv, rCoeff);
	rgb.g = dot(yuv, gCoeff);
	rgb.b = dot(yuv, bCoeff);

	gl_FragColor = vec4(rgb, 1.0);
*/

	// Test shader is doing _something_
//	gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); // yellow
/*	float xFract = 1.0/yWidth;
	float yFract = 1.0/yHeight;
	gl_FragColor = vec4(xFract*texCoord.s, yFract*texCoord.t, 0.0, 1.0); // gradient: top left black, top right red, bottom left green, bottom right yellow
*/
	// check Y
	//vec3 lightColour = texture2D(vidTexture, texCoord.xy);
	//gl_FragColor = vec4(lightColour, 1.0);
	//gl_FragColor = texture2DRect(vidTexture, texCoord.xy);
	// check U
	texCoord.y += yHeight/2;
	gl_FragColor = vec4(texture2DRect(vidTexture, texCoord.xy).r, texture2DRect(vidTexture, texCoord.xy).g, texture2DRect(vidTexture, texCoord.xy).b, 1.0);

	// lookup U
	// co-ordinate conversion algorithm: x /= 2.0; if modulo2(y) then x += yWidth/2.0;
/*	texCoord.x /= 2.0;	
	if((texCoord.y - floor(texCoord.y)) == 0.0)
	{
		texCoord.x += (yWidth/2.0);
	}
	texCoord.y = yHeight+(texCoord.y/4.0);

	gl_FragColor = vec4(texture2D(vidTexture, texCoord.xy).r, 0.0, 0.0, 1.0);
*/
}
