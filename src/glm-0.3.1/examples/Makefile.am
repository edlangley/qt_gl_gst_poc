AM_CFLAGS = $(WARN_CFLAGS)

if HAVE_GLUT
GLUTPROGS = smooth glutobj game_glutobj
endif

if HAVE_GLUI
GLUIPROGS = gluiobj
endif

noinst_PROGRAMS = $(GLUTPROGS) $(GLUIPROGS)

smooth_CFLAGS = -I$(top_srcdir)/glm $(GLUT_CFLAGS)
smooth_SOURCES = dirent32.h gltb.c gltb.h gltx.c gltx.h smooth.c trackball.c trackball.h
smooth_LDADD = ../glm/libglm.la $(GLUT_LIBS)

gluiobj_CXXFLAGS = -I$(top_srcdir)/glm $(GLUI_CFLAGS)
gluiobj_SOURCES = gluiobj.cpp
gluiobj_LDADD = ../glm/libglm.la $(GLUI_LIBS)

glutobj_CFLAGS = -I$(top_srcdir)/glm $(GLUT_CFLAGS)
glutobj_SOURCES = glutobj.c
glutobj_LDADD = ../glm/libglm.la $(GLUT_LIBS)

game_glutobj_CFLAGS = -I$(top_srcdir)/glm $(GLUT_CFLAGS)
game_glutobj_SOURCES = game_glutobj.c
game_glutobj_LDADD = ../glm/libglm.la $(GLUT_LIBS)
