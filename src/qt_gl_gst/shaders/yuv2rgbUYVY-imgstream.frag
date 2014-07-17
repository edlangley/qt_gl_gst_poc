// Perform YUV to RGB conversion on UYVY format interleaved YUV data
// Using the built in IMG streaming texture extension for PowerVR.

#ifdef GL_IMG_texture_stream2
#extension GL_IMG_texture_stream2 : enable
#endif

varying highp vec4 v_texCoord;
uniform samplerStreamIMG u_vidTexture;

mediump vec4 yuv2rgb()
{
	mediump vec4 rgb;
	mediump vec2 v2texcoord = v_texCoord.xy;

	rgb = textureStreamIMG(u_vidTexture, v2texcoord);

	return rgb;
}

