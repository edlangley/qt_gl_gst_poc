

mediump vec4 yuv2rgb(void);

varying highp float v_LightIntensity;

void main(void)
{
    highp vec4  colour;

    colour  = yuv2rgb();
    colour *= v_LightIntensity;
//    gl_FragColor = colour;
    gl_FragColor = vec4(colour.rgb, 1.0);
}

