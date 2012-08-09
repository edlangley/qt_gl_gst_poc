// GLES shader for passing interpolated texture co-ordinates
// to the video fragment shader


uniform highp mat4 u_mvp_matrix;
uniform highp mat4 u_mv_matrix;

attribute highp vec4 a_vertex;
attribute highp vec4 a_texCoord;

varying mediump vec4 v_texCoord;

void main(void)
{
    gl_Position = (u_mvp_matrix * a_vertex);
    v_texCoord = a_texCoord;
}




