AM_CFLAGS = $(WARN_CFLAGS)

lib_LTLIBRARIES = libglm.la

include_HEADERS = glm.h
noinst_HEADERS = glmint.h

libglm_la_CFLAGS = $(GL_CFLAGS) $(AM_CFLAGS)
libglm_la_SOURCES = glm.c glm_util.c glmimg.c glmimg_jpg.c glmimg_png.c glmimg_sdl.c glmimg_sim.c glmimg_devil.c
libglm_la_LIBADD = $(GL_LIBS) $(IPC_LIBS) $(SUPPORT_LIBS)
libglm_la_LDFLAGS = -version-info 0:0:0
