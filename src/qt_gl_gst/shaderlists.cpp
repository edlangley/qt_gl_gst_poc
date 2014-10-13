#include "shaderlists.h"

// Arrays containing lists of shaders which can be linked and used together:

GLShaderModule BrickGLESShaderList[NUM_SHADERS_BRICKGLES] =
{
    { "shaders/brick.vert", QGLShader::Vertex },
    { "shaders/brick.frag", QGLShader::Fragment }
};

#ifdef VIDI420_SHADERS_NEEDED
/* I420 */
GLShaderModule VidI420NoEffectNormalisedShaderList[NUM_SHADERS_VIDI420_NOEFFECT_NORMALISED] =
{
    { "shaders/yuv2rgbI420-normalisedtexcoords"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidI420LitNormalisedShaderList[NUM_SHADERS_VIDI420_LIT_NORMALISED] =
{
    { "shaders/yuv2rgbI420-normalisedtexcoords"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment }
};

GLShaderModule VidI420NoEffectShaderList[NUM_SHADERS_VIDI420_NOEFFECT] =
{
    { "shaders/yuv2rgbI420"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidI420LitShaderList[NUM_SHADERS_VIDI420_LIT] =
{
    { "shaders/yuv2rgbI420"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment }
};

GLShaderModule VidI420ColourHilightShaderList[NUM_SHADERS_VIDI420_COLOURHILIGHT] =
{
    { "shaders/yuv2rgbI420"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilight.frag", QGLShader::Fragment }
};

GLShaderModule VidI420ColourHilightSwapShaderList[NUM_SHADERS_VIDI420_COLOURHILIGHTSWAP] =
{
    { "shaders/yuv2rgbI420"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilightswap.frag", QGLShader::Fragment }
};

GLShaderModule VidI420AlphaMaskShaderList[NUM_SHADERS_VIDI420_ALPHAMASK] =
{
    { "shaders/yuv2rgbI420"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/alphamask"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/alphamask.vert", QGLShader::Vertex }
};
#endif

#ifdef VIDUYVY_SHADERS_NEEDED
/* UYVY */
GLShaderModule VidUYVYNoEffectNormalisedShaderList[NUM_SHADERS_VIDUYVY_NOEFFECT_NORMALISED] =
{
    { "shaders/yuv2rgbUYVY-normalisedtexcoords"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYLitNormalisedShaderList[NUM_SHADERS_VIDUYVY_LIT_NORMALISED] =
{
    { "shaders/yuv2rgbUYVY-normalisedtexcoords"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYNoEffectShaderList[NUM_SHADERS_VIDUYVY_NOEFFECT] =
{
    { "shaders/yuv2rgbUYVY"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYLitShaderList[NUM_SHADERS_VIDUYVY_LIT] =
{
    { "shaders/yuv2rgbUYVY"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYColourHilightShaderList[NUM_SHADERS_VIDUYVY_COLOURHILIGHT] =
{
    { "shaders/yuv2rgbUYVY"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilight.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYColourHilightSwapShaderList[NUM_SHADERS_VIDUYVY_COLOURHILIGHTSWAP] =
{
    { "shaders/yuv2rgbUYVY"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilightswap.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYAlphaMaskShaderList[NUM_SHADERS_VIDUYVY_ALPHAMASK] =
{
    { "shaders/yuv2rgbUYVY"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/alphamask"VIDCONV_FRAG_SHADER_SUFFIX".frag", QGLShader::Fragment },
    { "shaders/alphamask.vert", QGLShader::Vertex }
};
#endif
