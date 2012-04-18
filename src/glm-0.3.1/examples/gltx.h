/*
 *  Simple SGI .rgb (IRIS RGB) image file reader ripped off from
 *  texture.c (written by David Blythe).  See the SIGGRAPH '96
 *  Advanced OpenGL course notes.
 */


/* includes */
# ifdef _WIN32
#   include <windows.h>
# endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif


/* typedefs */

/* GLTXimage: Structure containing a texture image */
typedef struct {
  GLuint   width;			/* width of image */
  GLuint   height;			/* height of image */
  GLuint   components;			/* number of components in image */
  GLubyte* data;			/* image data */
} GLTXimage;


/* gltxDelete: Deletes a texture image
 * 
 * image - properly initialized GLTXimage structure
 */
void
gltxDelete(GLTXimage* image);

/* gltxReadRGB: Reads and returns data from an IRIS RGB image file.
 *
 * name       - name of the IRIS RGB file to read data from
 */
GLTXimage*
gltxReadRGB(char *name);
