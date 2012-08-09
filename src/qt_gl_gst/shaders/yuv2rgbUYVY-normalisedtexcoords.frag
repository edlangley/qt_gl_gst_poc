// Perform YUV to RGB conversion on UYVY format interleaved YUV data
// Using formula:
// R = 1.164(Y - 16) + 1.596(V - 128)
// G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
// B = 1.164(Y - 16)                  + 2.018(U - 128)


uniform lowp sampler2D u_vidTexture;
uniform lowp float u_yHeight, u_yWidth;

varying highp vec4 v_texCoord;

// YUV offset (reciprocals of 255 based offsets above)
const mediump vec3 offset = vec3(-0.0625, -0.5, -0.5);
// RGB coefficients 
const mediump vec3 rCoeff = vec3(1.164,  0.000,  1.596);
const mediump vec3 gCoeff = vec3(1.164, -0.391, -0.813);
const mediump vec3 bCoeff = vec3(1.164,  2.018,  0.000);


mediump vec4 yuv2rgb()
{
	mediump vec3 yuv, rgb;
	mediump vec4 texCoord;

	// For now just show something:
	texCoord.x = v_texCoord.x * u_yWidth;
	texCoord.y = v_texCoord.y * u_yHeight;
	yuv.r = texture2D(u_vidTexture, texCoord.xy).r;
	return vec4(rgb, 1.0);

	
	// lookup Y
	texCoord.x = v_texCoord.x * u_yWidth;
	texCoord.y = v_texCoord.y * u_yHeight;
	texCoord.x = texCoord.x / 2.0;
	if((texCoord.x - floor(texCoord.x)) == 0.0)
	{
		texCoord.x = (v_texCoord.x * 2.0) + 1.0;
	}
	else
	{
		texCoord.x = ((v_texCoord.x + 1.0) * 2.0) - 1.0;
	}
	yuv.r = texture2D(u_vidTexture, texCoord.xy).r;
	
	// lookup U
	texCoord.x = v_texCoord.x * u_yWidth;
	texCoord.y = v_texCoord.y * u_yHeight;
	texCoord.x = floor(texCoord.x / 2.0);
	texCoord.x *= 4.0;
	yuv.g = texture2D(u_vidTexture, texCoord.xy).r;

	// lookup V
	texCoord.x = v_texCoord.x * u_yWidth;
	texCoord.y = v_texCoord.y * u_yHeight;
	texCoord.x = floor(texCoord.x / 2.0);
	texCoord.x *= 4.0;
	texCoord.x += 2.0;
	yuv.g = texture2D(u_vidTexture, texCoord.xy).r;

	// Convert
	yuv += offset;
	rgb.r = dot(yuv, rCoeff);
	rgb.g = dot(yuv, gCoeff);
	rgb.b = dot(yuv, bCoeff);

	return vec4(rgb, 1.0);
}

