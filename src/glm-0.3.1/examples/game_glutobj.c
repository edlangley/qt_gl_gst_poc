#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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
#include "glm.h"

#define bool int
#define true 1
#define false 0

/* Ortho (if used) */
double _left = 0.0;		/* ortho view volume params */
double _right = 0.0;
double _bottom = 0.0;
double _top = 0.0;
double _zNear = -10.0;
double _zFar = 10.0;

/* Mouse Interface  */
int _mouseX = 0;		/* mouse control variables */
int _mouseY = 0;
bool _mouseLeft = false;
bool _mouseMiddle = false;
bool _mouseRight = false;
double _dragPosX = 0.0;
double _dragPosY = 0.0;
double _dragPosZ = 0.0;

/* Modelview Matrix */
double _matrix[16];		/* model matrix and supporting linear algebra functions */
double _matrixI[16];
double vlen(double x, double y, double z);
void pos(double *px, double *py, double *pz, const int x, const int y,
	 const int *viewport);
void getMatrix();
void invertMatrix(const GLdouble * m, GLdouble * out);

/* Game Mode Vars */
int game_mode = 0;		/*  GLUT GameMode ON/OFF */
int game_fullscreen = 0;	/*  Fullscreen Mode ON/OFF (When g_gamemode Is OFF) */
bool game_keys[256];		/*  Keys Array */

/* Global Variables for this app */
GLMmodel *pmodel = NULL;	/* the loaded model */
char filename[256];
GLuint mode = 0;		/* viewing mode */
int ww, wh;			/* window width and height */
int wireframe = 0;		/* Draw modes */
int show_axis = 1;
int smooth = 1;
int material = 1;
int textured = 1;
int two_sided = 1;
int show_help = 1;		/* help toggle */
int stereo = 0;			/* stereo init variable */
int centerZ = -3;

#ifdef SMOOTH_HINT
int smooth_hint = 0;

#endif				/*  */
int Help_Font = (int) GLUT_BITMAP_8_BY_13;
int linestart = 10;		/* start point on y axis for text lines */
int linespace = 20;		/* spac betwwen text lines */

/* Lighting Variables */
GLfloat light_ambient[] = {
    0.0, 0.0, 0.0, 1.0
};

GLfloat light_diffuse[] = {
    1.0, 1.0, 1.0, 1.0
};

GLfloat light_specular[] = {
    1.0, 1.0, 1.0, 1.0
};

GLfloat light_position[] = {
    1.0, 1.0, 1.0, 0.0
};

GLfloat mat_ambient[] = {
    0.7, 0.7, 0.7, 1.0
};

GLfloat mat_diffuse[] = {
    0.8, 0.8, 0.8, 1.0
};

GLfloat mat_specular[] = {
    1.0, 1.0, 1.0, 1.0
};

GLfloat high_shininess[] = {
    100.0
};


/* Stereo Variables */
GLfloat EyeSep = 0.1;		/* Eye Setup TODO */
GLfloat EyeBack = 0.0;
GLfloat FocusX = 0.0;
GLfloat FocusY = 0.0;
GLfloat FocusZ = -3.0;

/* Standard Glut interface functions                                     */
void Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Display(void);
void SpecialKeys(int a_keys, int x, int y);
void SpecialKeysUp(int key, int x, int y);

/* Model and help display functoins */
void DrawModel(void);
void DrawAxis(float scale);
void HelpDisplay(GLint ww, GLint wh);
void HelpRenderBitmapString(float x, float y, void *font, char *string);

/*****************************************************************************/
/*Main Functions**************************************************************/
/*****************************************************************************/
int main(int argc, char **argv)
{
    int i;
    if (argc > 1) {		/* make sure at least 2 args, program and file */
	strcpy(filename, argv[argc - 1]);	/* get the last arg as the file always */
	for (i = 0; i < argc; i++) {	/* check for startup params */
	    if (strstr(argv[i], "-s"))
		stereo = 1;
	}
    }

    else {			/* user only entered program name, help them */

	printf("Usage: %s [-s] <obj filename>\n", argv[0]);
	exit(0);
    }
    glutInit(&argc, argv);
    if (stereo)
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH |
			    GLUT_STEREO);

    else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    if (game_mode) {
	glutGameModeString("1024x768:32@75");	/*  Select The 640x480 In 16bpp Mode */
	if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
	    glutEnterGameMode();	/*  Enter Full Screen */

	else
	    game_mode = 0;	/*  Cannot Enter Game Mode, Switch To Windowed */
    }
    if (!game_mode) {
	glutInitWindowSize(1024, 768);	/*  Window Size If We Start In Windowed Mode */
	glutCreateWindow("Wavefront Obj File Viewer");
    }
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    getMatrix();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearAccum(0.0, 0.0, 0.0, 0.0);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutSpecialFunc(SpecialKeys);	/*  Register Special Keys Handler */
    glutSpecialUpFunc(SpecialKeysUp);	/*  Called When A Special Key Released */
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

#ifdef SMOOTH_HINT
    if (smooth_hint) {
	glEnable(GL_LINE_SMOOTH);

	/* glEnable (GL_POLYGON_SMOOTH); */
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	/* glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST); */
    }
#endif				/*  */
    if (!pmodel) {		/* load up the model */
	pmodel = glmReadOBJ(filename);
	if (!pmodel) {
	    printf("\nUsage: objviewV2 <-s> <obj filename>\n");
	    exit(0);
	}
	glmUnitize(pmodel);
	glmVertexNormals(pmodel, 90.0, GL_TRUE);
    }
    glutMainLoop();
    exit(0);
}


/*****************************************************************************/
/*Mouse Functions*************************************************************/
/*****************************************************************************/
void Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    ww = w;
    wh = h;
    _top = 1.0;
    _bottom = -1.0;
    _left = -(double) w / (double) h;
    _right = -_left;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* glOrtho(_left,_right,_bottom,_top,_zNear,_zFar);  Ortho */
    gluPerspective(45, (double) w / (double) h, 0.1, -_zFar);	/* PErspective for stereo */
    glMatrixMode(GL_MODELVIEW);
} void Mouse(int button, int state, int x, int y)
{
    int viewport[4];
    _mouseX = x;
    _mouseY = y;
    if (state == GLUT_UP)
	switch (button) {
	case GLUT_LEFT_BUTTON:
	    _mouseLeft = false;
	    break;
	case GLUT_MIDDLE_BUTTON:
	    _mouseMiddle = false;
	    break;
	case GLUT_RIGHT_BUTTON:
	    _mouseRight = false;
	    break;
	}

    else
	switch (button) {
	case GLUT_LEFT_BUTTON:
	    _mouseLeft = true;
	    break;
	case GLUT_MIDDLE_BUTTON:
	    _mouseMiddle = true;
	    break;
	case GLUT_RIGHT_BUTTON:
	    _mouseRight = true;
	    break;
	}
    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&_dragPosX, &_dragPosY, &_dragPosZ, x, y, viewport);
}
void Motion(int x, int y)
{
    bool changed = false;
    const int dx = x - _mouseX;
    const int dy = y - _mouseY;
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (dx == 0 && dy == 0)
	return;
    if (_mouseMiddle || (_mouseLeft && _mouseRight)) {
	double s = exp((double) dy * 0.01);
	glScalef(s, s, s);
	changed = true;
    }

    else if (_mouseLeft) {
	double ax, ay, az;
	double bx, by, bz;
	double angle;
	ax = dy;
	ay = dx;
	az = 0.0;
	angle = vlen(ax, ay, az) / (double) (viewport[2] + 1) * 180.0;

	/* Use inverse matrix to determine local axis of rotation */
	bx = _matrixI[0] * ax + _matrixI[4] * ay + _matrixI[8] * az;
	by = _matrixI[1] * ax + _matrixI[5] * ay + _matrixI[9] * az;
	bz = _matrixI[2] * ax + _matrixI[6] * ay + _matrixI[10] * az;
	glRotatef(angle, bx, by, bz);
	changed = true;
    }

    else if (_mouseRight) {
	double px, py, pz;
	pos(&px, &py, &pz, x, y, viewport);
	glLoadIdentity();
	glTranslatef(px - _dragPosX, py - _dragPosY, pz - _dragPosZ);
	glMultMatrixd(_matrix);
	_dragPosX = px;
	_dragPosY = py;
	_dragPosZ = pz;
	changed = true;
    }
    _mouseX = x;
    _mouseY = y;
    if (changed) {
	getMatrix();
	glutPostRedisplay();
    }
}


/*****************************************************************************/
/*Keyboard input**************************************************************/
/*****************************************************************************/
void Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'h':
    case 'H':{
	    show_help = !show_help;
	    break;
	}
    case 'r':
    case 'R':{
	    int i;

	    /* Resetting Scene */
	    for (i = 0; i < 16; i++) {
		if (i == 0 || i == 5 || i == 10 || i == 15) {
		    _matrix[i] = 1;
		}

		else {
		    _matrix[i] = 0;
		}
	    }
	    glLoadIdentity();
	    glMultMatrixd(_matrix);
	    getMatrix();
	    break;
	}
    case 'a':
    case 'A':{
	    show_axis = !show_axis;
	    break;
	}

#ifdef SMOOTH_HINT
    case 'l':
    case 'L':{
	    smooth_hint = !smooth_hint;
	    if (smooth_hint) {
		glEnable(GL_LINE_SMOOTH);

		/* glEnable (GL_POLYGON_SMOOTH); */
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		/* glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST); */
	    }

	    else {
		glDisable(GL_LINE_SMOOTH);

		/* glDisable (GL_POLYGON_SMOOTH); */
	    }
	    break;
	}

#endif				/*  */
    case 'w':
    case 'W':{
	    wireframe = !wireframe;
	    break;
	}
    case 'd':
    case 'D':{
	    two_sided = !two_sided;
	    break;
	}
    case 's':
    case 'S':{
	    smooth = !smooth;
	    break;
	}
    case 'm':
    case 'M':{
	    material = !material;
	    if (!material && textured)
		textured = 0;
	    break;
	}
    case 't':
    case 'T':{
	    textured = !textured;
	    if (!material && textured)
		material = 1;
	    break;
	}

#ifdef AVL
    case 'f':
    case 'F':{
	    glmFlipModelTextures(pmodel);
	    break;
	}

#endif				/*  */
    case ',':
    case '<':{
	    EyeSep -= 0.05;
	    break;
	}
    case '.':
    case '>':{
	    EyeSep += 0.05;
	    break;
	}
    case '+':
    case '=':
	{
	    FocusZ -= 0.5;
	    break;
	}
    case '-':
    case '_':
	{
	    FocusZ += 0.5;
	    break;
	}
    case 27:
	exit(0);
	break;
    default:{
	    fflush(stdin);
	    break;
	}			/* flush all other input                                 */
    }
    fflush(stdin);		/* flush input */
    glutPostRedisplay();	/* redisplay afterword */
}


/*  Our Keyboard Handler For Special Keys (Like Arrow Keys And Function Keys) */
void SpecialKeys(int a_keys, int x, int y)
{
    switch (a_keys) {
    case GLUT_KEY_F2:

	/*  We Can Switch Between Windowed Mode And Fullscreen Mode Only */
	if (!game_mode) {
	    game_fullscreen = !game_fullscreen;	/*  Toggle g_fullscreen Flag */
	    if (game_fullscreen)
		glutFullScreen();	/*  We Went In Fullscreen Mode */

	    else
		glutReshapeWindow(ww, wh);	/*  We Went In Windowed Mode */
	}
	break;
    default:
	game_keys[a_keys] = true;
	break;
    }
    glutPostRedisplay();
}


/*  Our Keyboard Handler For Special Key Releases. */
void SpecialKeysUp(int key, int x, int y)
{
    game_keys[key] = 0;
}

/*****************************************************************************/
/*Matrix and Vector Utility functions ****************************************/
/*****************************************************************************/
double vlen(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}
void pos(double *px, double *py, double *pz, const int x, const int y,
	 const int *viewport)
{

    /*
       Use the ortho projection and viewport information
       to map from mouse co-ordinates back into world 
       co-ordinates
     */
    *px = (double) (x - viewport[0]) / (double) (viewport[2]);
    *py = (double) (y - viewport[1]) / (double) (viewport[3]);
    *px = _left + (*px) * (_right - _left);
    *py = _top + (*py) * (_bottom - _top);
    *pz = _zNear;
} void getMatrix()
{
    glGetDoublev(GL_MODELVIEW_MATRIX, _matrix);
    invertMatrix(_matrix, _matrixI);
}

/*  From Mesa-2.2\src\glu\project.c */

/*  Compute the inverse of a 4x4 matrix.  Contributed by scotter@lafn.org */
void invertMatrix(const GLdouble * m, GLdouble * out)
{

/* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)
    GLdouble det;
    GLdouble d12, d13, d23, d24, d34, d41;
    GLdouble tmp[16];		/* Allow out == in. */

    /* Inverse = adjoint / det. (See linear algebra texts.) */

    /* pre-compute 2x2 dets for last two rows when computing */
    /* cofactors of first two rows. */
    d12 = (m31 * m42 - m41 * m32);
    d13 = (m31 * m43 - m41 * m33);
    d23 = (m32 * m43 - m42 * m33);
    d24 = (m32 * m44 - m42 * m34);
    d34 = (m33 * m44 - m43 * m34);
    d41 = (m34 * m41 - m44 * m31);
    tmp[0] = (m22 * d34 - m23 * d24 + m24 * d23);
    tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
    tmp[2] = (m21 * d24 + m22 * d41 + m24 * d12);
    tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);

    /* Compute determinant as early as possible using these cofactors. */
    det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];

    /* Run singularity test. */
    if (det == 0.0) {

	/* printf("invert_matrix: Warning: Singular matrix.\n"); */
/* 	  memcpy(out,_identity,16*sizeof(double)); */
    }

    else {
	GLdouble invDet = 1.0 / det;

	/* Compute rest of inverse. */
	tmp[0] *= invDet;
	tmp[1] *= invDet;
	tmp[2] *= invDet;
	tmp[3] *= invDet;
	tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
	tmp[5] = (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
	tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
	tmp[7] = (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

	/* Pre-compute 2x2 dets for first two rows when computing */
	/* cofactors of last two rows. */
	d12 = m11 * m22 - m21 * m12;
	d13 = m11 * m23 - m21 * m13;
	d23 = m12 * m23 - m22 * m13;
	d24 = m12 * m24 - m22 * m14;
	d34 = m13 * m24 - m23 * m14;
	d41 = m14 * m21 - m24 * m11;
	tmp[8] = (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
	tmp[9] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
	tmp[10] = (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
	tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
	tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
	tmp[13] = (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
	tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
	tmp[15] = (m31 * d23 - m32 * d13 + m33 * d12) * invDet;
	memcpy(out, tmp, 16 * sizeof(GLdouble));
    }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}


/*****************************************************************************/
/*OpenGL Display Functions****************************************************/
/*****************************************************************************/
void DrawModel(void)
{
    mode = GLM_NONE;		/* reset mode */
    if (smooth)
	mode = mode | GLM_SMOOTH;

    else
	mode = mode | GLM_FLAT;
    if (two_sided)
	mode = mode | GLM_2_SIDED;
    if (material)
	mode = mode | GLM_MATERIAL;

    else
	mode = mode | GLM_COLOR;
    if (textured && material)
	mode = mode | GLM_TEXTURE;
    glPushMatrix();
    if (pmodel)
	glmDraw(pmodel, mode);
    glPopMatrix();
}
void DrawAxis(float scale)
{
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glScalef(scale, scale, scale);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(.8f, 0.05f, 0.0);
    glVertex3f(1.0, 0.25f, 0.0);	/*  Letter X */
    glVertex3f(0.8f, .25f, 0.0);
    glVertex3f(1.0, 0.05f, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);	/*  X axis */
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);	/*  Y axis */
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);	/*  Z axis */
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
} void HelpDisplay(GLint ww, GLint wh)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    /*  switch to projection mode */
    glMatrixMode(GL_PROJECTION);

    /*  save previous matrix which contains the  */
    /* settings for the perspective projection */
    glPushMatrix();

    /*  reset matrix */
    glLoadIdentity();

    /*  set a 2D orthographic projection */
    gluOrtho2D(0, ww, 0, wh);

    /*  invert the y axis, down is positive */
    glScalef(1, -1, 1);

    /*  mover the origin from the bottom left corner */
    /*  to the upper left corner */
    glTranslatef(0, -wh, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    linestart = 10;
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font, "Help Menu");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font, "---------");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "H/h = Toggle Help Menu");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "F2  = Toggle Full Screen");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "Esc = Exits Program");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "R/r = Reset Position");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "C/c = Toggle Axis");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "W/w = Toggle Wireframe");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "D/d = Toggle Double Sided Polygons");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "S/s = Toggle Smooth Shading");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "M/m = Toggle Materials");
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "T/t = Toggle Textures");

    /* HelpRenderBitmapString(30,linestart+=linespace,(void *)Help_Font, "L/l = Toggle Line Smoothing");    */
#ifdef AVL
    HelpRenderBitmapString(30, linestart +=
			   linespace, (void *) Help_Font,
			   "F/f = Flip Textures");

#endif				/*  */
    if (stereo) {
	HelpRenderBitmapString(30, linestart +=
			       linespace, (void *) Help_Font,
			       "Stereo Variables");
	HelpRenderBitmapString(30, linestart +=
			       linespace, (void *) Help_Font,
			       "----------------");
	HelpRenderBitmapString(30, linestart +=
			       linespace, (void *) Help_Font,
			       "</, = Decrease Eye Separation");
	HelpRenderBitmapString(30, linestart +=
			       linespace, (void *) Help_Font,
			       ">/. = Increase Eye Separation");
	HelpRenderBitmapString(30, linestart +=
			       linespace, (void *) Help_Font,
			       "+/- = Increase/Decrease Focus Distance");
    }
    glPopMatrix();

    /*  set the current matrix to GL_PROJECTION */
    glMatrixMode(GL_PROJECTION);

    /*  restore previous settings */
    glPopMatrix();

    /*  get back to GL_MODELVIEW matrix */
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
} void HelpRenderBitmapString(float x, float y, void *font, char *string)
{
    char *c;

    /*  set position to start drawing fonts */
    glRasterPos2f(x, y);

    /*  loop all the characters in the string */
    for (c = string; *c != '\0'; c++) {
	glutBitmapCharacter(font, *c);
    }
}
void Display(void)
{

/*    GLint viewport[4]; */
    /*
       int jitter;

       glGetIntegerv (GL_VIEWPORT, viewport);

       glClear(GL_ACCUM_BUFFER_BIT);
       for (jitter = 0; jitter < ACSIZE; jitter++) {
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       accPerspective (50.0, 
       (GLdouble) viewport[2]/(GLdouble) viewport[3], 
       1.0, 15.0, j8[jitter].x, j8[jitter].y, 0.0, 0.0, 1.0);
       DrawModel(); 
       glAccum(GL_ACCUM, 1.0/ACSIZE);
       }
       glAccum (GL_RETURN, 1.0);
       glFlush();
     */
    if (stereo) {

	/* Clear right and left eye buffers */
	glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawBuffer(GL_BACK_RIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Right Eye */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double) ww / (double) wh, 0.1, -_zFar);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDrawBuffer(GL_BACK_RIGHT);
	glLoadIdentity();
	gluLookAt(EyeSep / 2, 0, EyeBack, FocusX, FocusY, FocusZ, 0, 1, 0);
	glTranslatef(0, 0, centerZ);
	glMultMatrixd(_matrix);
	if (show_axis)
	    DrawAxis(1.0f);
	if (wireframe)		/* if Wireframe is checked */
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	/* draw wireframe */

	else			/* else */
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	/* draw filled polygons */
	DrawModel();
	glTranslatef(0, 0, -centerZ);
	glPopMatrix();

	/* Left Eye */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double) ww / (double) wh, 0.1, -_zFar);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDrawBuffer(GL_BACK_LEFT);
	glLoadIdentity();
	gluLookAt(-EyeSep / 2, 0, EyeBack, FocusX, FocusY, FocusZ, 0, 1,
		  0);
	glPushMatrix();
	glTranslatef(0, 0, centerZ);
	glMultMatrixd(_matrix);
	if (show_axis)
	    DrawAxis(1.0f);
	if (wireframe)		/* if Wireframe is checked */
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	/* draw wireframe */

	else			/* else */
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	/* draw filled polygons */
	DrawModel();
	glTranslatef(0, 0, -centerZ);
	glPopMatrix();
	glPopMatrix();
    }

    else {			/* NON stereo mode */

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, 0, centerZ);	/* to center object down Z */
	glMultMatrixd(_matrix);
	if (show_axis)
	    DrawAxis(1.0f);
	if (wireframe)		/* if Wireframe is checked */
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	/* draw wireframe */

	else			/* else */
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	/* draw filled polygons */
	DrawModel();
	glTranslatef(0, 0, -centerZ);	/* to center object down Z */
	glPopMatrix();
    }
    if (show_help)
	HelpDisplay(ww, wh);
    glutSwapBuffers();
}
