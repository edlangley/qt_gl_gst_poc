/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2009 Andrey Nechypurenko <andreynech@gmail.com>
 * Copyright (C) 2010 Nuno Santos <nunosantos@imaginando.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GLCONTEXTID_H
#define __GLCONTEXTID_H

#if defined(Q_OS_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <Wingdi.h>
    #include <GL/glew.h>
    #include <GL/gl.h>
#elif defined (Q_OS_MAC)
    #include <OpenGL/OpenGL.h>
    class NSOpenGLContext;
#else
    #include <X11/Xlib.h>
    //#include <GL/glxew.h>
    #include <GL/gl.h>
    #include <GL/glx.h>
#endif


#if defined(Q_WS_WIN)
    typedef struct _tagGLContextID
    {
        HGLRC contextId;
        HDC dc;
    } GLContextID;
#elif  defined(Q_WS_MAC)
    typedef struct _tagGLContextID
    {
        NSOpenGLContext* contextId;
    } GLContextID;
#elif defined(Q_WS_X11)
    typedef struct _tagGLContextID
    {
        GLXContext contextId;
        Display *display;
        Window wnd;
    } GLContextID;
#endif

#endif // __GLCONTEXTID_H

