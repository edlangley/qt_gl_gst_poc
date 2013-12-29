#include "shaderlists.h"

// Arrays containing lists of shaders which can be linked and used together:

GLShaderModule BrickGLESShaderList[NUM_SHADERS_BRICKGLES] =
{
    { "shaders/brick.vert", QGLShader::Vertex },
    { "shaders/brick.frag", QGLShader::Fragment }
};

/* I420 */
GLShaderModule VidI420NoEffectNormalisedShaderList[NUM_SHADERS_VIDI420_NOEFFECT_NORMALISED] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbI420-normalisedtexcoords-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbI420-normalisedtexcoords.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidI420LitNormalisedShaderList[NUM_SHADERS_VIDI420_LIT_NORMALISED] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbI420-normalisedtexcoords-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbI420-normalisedtexcoords.frag", QGLShader::Fragment },
#endif
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment }
};

GLShaderModule VidI420NoEffectShaderList[NUM_SHADERS_VIDI420_NOEFFECT] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbI420-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbI420.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidI420ColourHilightShaderList[NUM_SHADERS_VIDI420_COLOURHILIGHT] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbI420-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbI420.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilight.frag", QGLShader::Fragment }
};

GLShaderModule VidI420ColourHilightSwapShaderList[NUM_SHADERS_VIDI420_COLOURHILIGHTSWAP] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbI420-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbI420.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilightswap.frag", QGLShader::Fragment }
};

GLShaderModule VidI420AlphaMaskShaderList[NUM_SHADERS_VIDI420_ALPHAMASK] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbI420-recttex.frag", QGLShader::Fragment },
    { "shaders/alphamask-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbI420.frag", QGLShader::Fragment },
    { "shaders/alphamask.frag", QGLShader::Fragment },
#endif
    { "shaders/alphamask.vert", QGLShader::Vertex }
};

#ifndef RECTTEX_EXT_NEEDED // temp to reduce error output
/* UYVY */
GLShaderModule VidUYVYNoEffectNormalisedShaderList[NUM_SHADERS_VIDUYVY_NOEFFECT_NORMALISED] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbUYVY-normalisedtexcoords-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbUYVY-normalisedtexcoords.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYLitNormalisedShaderList[NUM_SHADERS_VIDUYVY_LIT_NORMALISED] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbUYVY-normalisedtexcoords-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbUYVY-normalisedtexcoords.frag", QGLShader::Fragment },
#endif
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYNoEffectShaderList[NUM_SHADERS_VIDUYVY_NOEFFECT] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbUYVY-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbUYVY.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYColourHilightShaderList[NUM_SHADERS_VIDUYVY_COLOURHILIGHT] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbUYVY-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbUYVY.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilight.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYColourHilightSwapShaderList[NUM_SHADERS_VIDUYVY_COLOURHILIGHTSWAP] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbUYVY-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbUYVY.frag", QGLShader::Fragment },
#endif
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilightswap.frag", QGLShader::Fragment }
};

GLShaderModule VidUYVYAlphaMaskShaderList[NUM_SHADERS_VIDUYVY_ALPHAMASK] =
{
#if RECTTEX_EXT_NEEDED
    { "shaders/yuv2rgbUYVY-recttex.frag", QGLShader::Fragment },
    { "shaders/alphamask-recttex.frag", QGLShader::Fragment },
#else
    { "shaders/yuv2rgbUYVY.frag", QGLShader::Fragment },
    { "shaders/alphamask.frag", QGLShader::Fragment },
#endif
    { "shaders/alphamask.vert", QGLShader::Vertex }
};

#endif
