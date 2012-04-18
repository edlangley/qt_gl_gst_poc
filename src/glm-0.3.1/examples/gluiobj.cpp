//***************************************************************************
//  
//  GLUI View - A General Purpose Viewer. Modifications by Jeff Rogers.
//  
//  Further modifications by Frederic Devernay
//
//  Based on Paul Rademacher's work. Thanks for GLUI!
//  7/10/98 Paul Rademacher (rademach@cs.unc.edu)
//***************************************************************************
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
# ifdef _WIN32
#   include <windows.h>
# endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <GLUI/glui.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glui.h>
#endif
#include "glm.h"

#include "config.h"

float xy_aspect;
int   last_x,
	  last_y;
float rotationX = 0.0,
	  rotationY = 0.0;

// These are the live variables passed into GLUI **
int   main_window;
float scale_obj = 1.0;  //object variable
int   wireframe = 0;
int   show_axis = 1;
int   smooth = 1;      
int   material = 1;
int   textured = 1;
int   two_sided = 1; 
char  number[4];
int   lighting  = 1;
int   light1    = 1;
int   light2    = 1;
GLuint mode = 0;
int count = 0;

int   show_text = 1;
float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float obj_pos[] = { 0.0, 0.0, 0.0 };
char text[80];
char filename[1024];

// Pointers to the windows and some of the controls we'll create *
GLUI *glui;              //right Menu
GLUI_Rotation *view_rot; //Rotation control pointer
GLUI_Translation *trans_xy;
GLUI_Translation *trans_z;
GLUI_Checkbox *SmoothBox;
GLUI_Checkbox *MatBox;
GLUI_Checkbox *TextureBox;
GLUI_Checkbox *SideBox;
GLUI_EditText *EditText;


//******** User IDs for callbacks *******
#define RESET_OBJECTS_ID     300
#ifdef AVL
#define FLIP_TEXTURES_ID     301
#endif
#define DELETE_MODEL_ID      302
#define FILE_NAME            303

//Prototypes
static void control_cb( int control );
static void myGlutKeyboard(unsigned char Key, int x, int y);
static void myGlutMenu( int value );
static void myGlutIdle( void );
static void myGlutMouse(int button, int button_state, int x, int y );
static void myGlutMotion(int x, int y );
static void myGlutReshape( int x, int y );
static void myGlutDisplay( void );
static void reset_objects(void);
static void draw_axis( float scale_obj );
static void draw_model(void);
static void check_file(int);


//GLM Model Variable************************************************
GLMmodel* pmodel = NULL;


//************************************** main() *******************
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);

    if (argc > 1) {		/* make sure at least 2 args, program and file */
	strncpy(filename, argv[argc - 1],sizeof(filename));	/* get the last arg as the file always */

    } else {			/* user only entered program name, help them */

	printf("Usage: %s <obj filename>\n", argv[0]);
	exit(0);
    }

  //*************************************
  //   Initialize GLUT and create window  
  //*************************************

  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition( 0, 0 );
  glutInitWindowSize(800, 600);
 
  main_window = glutCreateWindow( "GLUI View" );
  glutDisplayFunc( myGlutDisplay );
  GLUI_Master.set_glutReshapeFunc( myGlutReshape );  
  GLUI_Master.set_glutKeyboardFunc( myGlutKeyboard );
  //GLUI_Master.set_glutSpecialFunc( NULL );
  //GLUI_Master.set_glutMouseFunc( myGlutMouse );
  //glutMotionFunc( myGlutMotion );

  /****************************************/
  /*       Set up OpenGL lights           */
  /****************************************/

  GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
  GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
  GLfloat light0_position[] = {0.0f, 0.0f, 2.0f, 0.0f};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable (GL_TEXTURE_2D);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

  //*************************************
  //          Enable z-buferring          
  //*************************************

  glEnable(GL_DEPTH_TEST);
  printf( "\nGLUI version: %3.2f\n", GLUI_Master.get_version() );


  if (!pmodel)
  {
      pmodel = glmReadOBJ(filename);
      if (!pmodel) exit(0);
      glmUnitize(pmodel);  
      glmVertexNormals(pmodel, 90.0, GL_TRUE);
  }




  //** Create the right side subwindow **
  glui = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_RIGHT );
  
  //**** Control for render params ****
  //main panel
  GLUI_Panel *display_panel = glui->add_rollout( "Display Options", true );

  glui->add_checkbox_to_panel(display_panel, "Wireframe", &wireframe);	 //wireframe check box	
  glui->add_checkbox_to_panel(display_panel, "Draw axis", &show_axis );  //draw axis check boxes
 
  SideBox = glui->add_checkbox_to_panel(display_panel, "Two Sided", &two_sided);	     //textured check box	
  SmoothBox = glui->add_checkbox_to_panel(display_panel, "Smooth Shading", &smooth);	 //smooth check box	
  MatBox = glui->add_checkbox_to_panel(display_panel, "Materials");	 //material check box	
  MatBox->set_int_val(1);
  TextureBox = glui->add_checkbox_to_panel(display_panel, "Textured");	 //textured check box	
  TextureBox->set_int_val(1);
 
  glui->add_separator_to_panel(display_panel);
#ifdef AVL
  //Flip Textures button
  glui->add_button_to_panel(display_panel, "Flip Textures", FLIP_TEXTURES_ID, control_cb);
#endif


 //Scale spinner
  glui->add_separator_to_panel(display_panel);
  GLUI_Spinner *scale_spinner = glui->add_spinner_to_panel(display_panel, "Scale:", GLUI_SPINNER_FLOAT, &scale_obj);
  scale_spinner->set_float_limits( .005f, 4.0 );    //Set limits for scale
  scale_spinner->set_alignment( GLUI_ALIGN_RIGHT ); //Set alignment

  //Flip Textures button
  //DEBUGGING Delete command glui->add_button_to_panel(display_panel, "Delete Model", DELETE_MODEL_ID, control_cb);

  EditText = glui->add_edittext("File", GLUI_EDITTEXT_TEXT, filename, FILE_NAME, check_file);



  //Rotate Ball
  view_rot = glui->add_rotation( "Rotate Objects", view_rotate );
  view_rot->set_spin( 0.99 ); //set slight degradation in spin

  //Move XY 
  trans_xy =  glui->add_translation( "Move Objects XY", GLUI_TRANSLATION_XY, obj_pos );
  trans_xy->set_speed( .005 );

  //Move Z
  trans_z =   glui->add_translation( "Move Objects Z", GLUI_TRANSLATION_Z, &obj_pos[2] );
  trans_z->set_speed( .005 );

  
  //Reset positions button
  glui->add_statictext( "" );
  glui->add_button("Reset Positions", RESET_OBJECTS_ID, control_cb);


  // A 'quit' button
  glui->add_statictext( "" );
  glui->add_statictext( "" );
  glui->add_button( "Quit", 0,(GLUI_Update_CB)exit );


  // Link windows to GLUI, and register idle callback
  glui->set_main_gfx_window( main_window );

  // We register the idle callback with GLUI, *not* with GLUT 

  //GLUI_Master.set_glutIdleFunc( myGlutIdle );

  // Regular GLUT main loop 
  
  glutMainLoop();
}


///************************************** control_cb() *******************
// GLUI control callback                                                 
static void control_cb( int control )
{
  int i;
  if( control == RESET_OBJECTS_ID)
  {

      //Resetting Object Rotation Matrices
      for (i = 0; i < 16; i++)
      {
        if (i == 0 || i == 5 || i == 10 || i == 15)
        {
            view_rotate[i] =1;
        }
        else
        {   
            view_rotate[i] =0 ;
        }

      }
      //Resetting obj_positions
      obj_pos[0] = obj_pos[1] = obj_pos[2] = 0;
	  
	  view_rot->reset();
	  trans_xy->set_x(0.0);
	  trans_xy->set_y(0.0);
      trans_z->set_z(0.0);
  }
#ifdef AVL
  if( control == FLIP_TEXTURES_ID)
  {
      glmFlipModelTextures(pmodel);
  }
#endif
  /*DEBUGGING DELETE COMMAND
  if( control == DELETE_MODEL_ID)
  {
      if (pmodel) glmDelete(pmodel);
      
      pmodel = NULL;
      

      pmodel = glmReadOBJ("data/dual.obj");
      
            
      if (!pmodel) exit(0);
      glmUnitize(pmodel);  
      glmVertexNormals(pmodel, 90.0, GL_TRUE);
  }
  */
}


///*************************************** myGlutKeyboard() **********
static void myGlutKeyboard(unsigned char Key, int x, int y)
{
  switch(Key)
  {
  case 27: 
  case 'q':
    exit(0);
    break;
  };
  
  glutPostRedisplay();
}


///**************************************** myGlutMenu() ***********
static void myGlutMenu( int value )
{
  myGlutKeyboard( value, 0, 0 );
}


#if 0
///**************************************** myGlutIdle() ***********
static void myGlutIdle( void )
{
  // According to the GLUT specification, the current window is 
  //     undefined during an idle callback.  So we need to explicitly change
  //     it if necessary 
  if ( glutGetWindow() != main_window ) 
    glutSetWindow(main_window);  

  //  GLUI_Master.sync_live_all();  -- not needed - nothing to sync in this
  //                                     application  

  glutPostRedisplay();
}

///**************************************** myGlutMouse() **********
static void myGlutMouse(int button, int button_state, int x, int y )
{
}


///**************************************** myGlutMotion() **********
static void myGlutMotion(int x, int y )
{
  glutPostRedisplay(); 
}
#endif


///*************************************** myGlutReshape() *************
static void myGlutReshape( int x, int y )
{
  int tx, ty, tw, th;
  GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );

  xy_aspect = (float)tw / (float)th;

  glutPostRedisplay();
}


///************************************************* draw_axis() **********
/// Disables lighting, then draws RGB axis                                
static void draw_axis( float scale )
{
  glPushMatrix();
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glScalef( scale, scale, scale );

  glBegin( GL_LINES );
 
  glColor3f( 1.0, 0.0, 0.0 );
  glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 ); // Letter X
  glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 );     // X axis

  glColor3f( 0.0, 1.0, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 );	 // Y axis

  glColor3f( 0.0, 0.0, 1.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 );	 // Z axis
  glEnd();
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glPopMatrix();

}


///************************************************* draw_model() **********
static void draw_model( void )
{
  
  mode = GLM_NONE; //reset mode

  if (SmoothBox->get_int_val() == 1) //smooth mode
      mode = mode | GLM_SMOOTH;
  else
      mode = mode | GLM_FLAT;

  if (SideBox->get_int_val())
      mode = mode | GLM_2_SIDED;

      
  if(MatBox->get_int_val())         //material mode
      mode = mode | GLM_MATERIAL;
  else
      mode = mode | GLM_COLOR;

  if(TextureBox->get_int_val())     //texture mode
  {
      if(MatBox->get_int_val())
          mode = mode | GLM_TEXTURE;
      else
          TextureBox->set_int_val(0);
      
      if(!material && !textured)
      {
          mode = mode | GLM_MATERIAL;
          mode = mode | GLM_TEXTURE;
          MatBox->set_int_val(1);
          TextureBox->set_int_val(1);
      }
  }

  
  material = MatBox->get_int_val();     //save for analyzing state next time around
  textured = TextureBox->get_int_val();

  glPushMatrix();

  if (pmodel) glmDraw(pmodel, mode); 


  
  glPopMatrix();
}


///**************************************** myGlutDisplay() *****************
static void myGlutDisplay( void )
{
  //set graphics window to gray background
  glClearColor( 0.0, 0.0, 0.0, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -xy_aspect*.04, xy_aspect*.04, -.04, .04, .1, 25.0 );

  glMatrixMode( GL_MODELVIEW );

  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -2.6f );
  glTranslatef( obj_pos[0], obj_pos[1], -obj_pos[2] ); 
  glMultMatrixf( view_rotate );
  glScalef( scale_obj, scale_obj, scale_obj );

 
  if ( show_axis )
    draw_axis(1.0f);
  if (wireframe)							//if Wireframe is checked
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //draw wireframe
  else										//else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //draw filled polygons
 
  draw_model();

  //This is the section that handles text
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glColor3ub( 0, 0, 0 );
  glRasterPos2i( 10, 10 );
  
  //setup the text string
  strcpy(text,filename);

  // Render the live character array 'text'
  int i;
  for( i=0; i<(int)strlen( text ); i++ )
     glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, text[i] );

  glutSwapBuffers(); 
}



//Check file and load if possible
static void check_file(int ID)
{
    FILE *InTest;
    if((strlen(EditText->get_text())) > 0)
    {
      if(InTest = fopen(EditText->get_text(), "r"))
        {
            printf("\nLoading: %s\n", EditText->get_text());
            fclose(InTest);
            if (pmodel) glmDelete(pmodel);
            pmodel = NULL;
            pmodel = glmReadOBJ(EditText->get_text());
      
            if (!pmodel) exit(0);
            glmUnitize(pmodel);  
            glmVertexNormals(pmodel, 90.0, GL_TRUE);
        }
      else
        printf("\n%s does not exist.\n", EditText->get_text());
    }
}
