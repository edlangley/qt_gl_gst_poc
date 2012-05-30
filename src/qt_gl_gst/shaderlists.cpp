#include "shaderlists.h"

// Arrays containing lists of shaders which can be linked and used together:

GLShaderModule BrickGLESShaderList[NUM_SHADERS_BRICKGLES] =
{
#if GLES2
    { "shaders/brick-gles.vert", QGLShader::Vertex },
    { "shaders/brick-gles.frag", QGLShader::Fragment }
#else
    { "shaders/brick.vert", QGLShader::Vertex },
    { "shaders/brick.frag", QGLShader::Fragment }
#endif
};

GLShaderModule VidI420NoEffectNormalisedShaderList[NUM_SHADERS_VIDI420NOEFFECT_NORMALISED] =
{
#if GLES2
    { "shaders/noeffect-gles.vert", QGLShader::Vertex },
    { "shaders/noeffect-gles.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-normalisedtexcoords-gles.frag", QGLShader::Fragment }
#else
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-normalisedtexcoords.frag", QGLShader::Fragment }
#endif
};

GLShaderModule VidI420LitNormalisedShaderList[NUM_SHADERS_VIDI420LIT_NORMALISED] =
{
#if GLES2
    { "shaders/noeffect-gles.vert", QGLShader::Vertex },
    { "shaders/noeffect-gles.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-normalisedtexcoords-gles.frag", QGLShader::Fragment }
#else
    { "shaders/vidlighting.vert", QGLShader::Vertex },
    { "shaders/vidlighting.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-normalisedtexcoords.frag", QGLShader::Fragment }
#endif
};

GLShaderModule VidI420NoEffectShaderList[NUM_SHADERS_VIDI420NOEFFECT] =
{
#if GLES2
    { "shaders/noeffect-gles.vert", QGLShader::Vertex },
    { "shaders/noeffect-gles.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-gles.frag", QGLShader::Fragment }
#else
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/noeffect.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb.frag", QGLShader::Fragment }
#endif
};

GLShaderModule VidI420ColourHilightShaderList[NUM_SHADERS_VIDI420COLOURHILIGHT] =
{
#if GLES2
    { "shaders/noeffect-gles.vert", QGLShader::Vertex },
    { "shaders/colourhilight-gles.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-gles.frag", QGLShader::Fragment }
#else
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilight.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb.frag", QGLShader::Fragment }
#endif
};

GLShaderModule VidI420ColourHilightSwapShaderList[NUM_SHADERS_VIDI420COLOURHILIGHTSWAP] =
{
#if GLES2
    { "shaders/noeffect-gles.vert", QGLShader::Vertex },
    { "shaders/colourhilightswap-gles.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-gles.frag", QGLShader::Fragment }
#else
    { "shaders/noeffect.vert", QGLShader::Vertex },
    { "shaders/colourhilightswap.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb.frag", QGLShader::Fragment }
#endif
};

GLShaderModule VidI420AlphaMaskShaderList[NUM_SHADERS_VIDI420ALPHAMASK] =
{
#if GLES2
    { "shaders/alphamask-gles.vert", QGLShader::Vertex },
    { "shaders/alphamask-gles.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb-gles.frag", QGLShader::Fragment }
#else
    { "shaders/alphamask.vert", QGLShader::Vertex },
    { "shaders/alphamask.frag", QGLShader::Fragment },
    { "shaders/yuv2rgb.frag", QGLShader::Fragment }
#endif
};
