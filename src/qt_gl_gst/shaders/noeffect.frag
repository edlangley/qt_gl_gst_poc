// GLES shader which displays YUV420P video data correctly as a texture
// This shader must be linked with another containing yuv2rgb function
// to handle the video data first

//#extension GL_ARB_texture_rectangle : enable

mediump vec4 yuv2rgb(void);

void main(void)
{
	gl_FragColor = yuv2rgb();
	//gl_FragColor = vec4 (1.0, 1.0, 0.0, 1.0);
}
