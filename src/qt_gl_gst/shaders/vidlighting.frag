#extension GL_ARB_texture_rectangle : enable

vec4 yuv2rgb(void);

varying float v_LightIntensity;

void main(void)
{
    vec3  colour;

    colour  = yuv2rgb();
    colour *= v_LightIntensity;
    gl_FragColor = vec4 (colour, 1.0);
}

