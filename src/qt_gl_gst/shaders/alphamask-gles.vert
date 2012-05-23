// GLES shader for passing interpolated texture co-ordinates
// to the video fragment shader


uniform highp mat4 u_mvp_matrix;
uniform highp mat4 u_mv_matrix;

attribute highp vec4 a_vertex;
attribute highp vec3 a_alphaTexCoord;
attribute highp vec4 a_texCoord;

varying highp vec3 v_alphaTexCoord;
varying highp vec4 v_texCoord;

void main(void)
{
    gl_Position = (u_mvp_matrix * a_vertex);
    v_alphaTexCoord = a_alphaTexCoord;
    v_texCoord = a_texCoord;
}




