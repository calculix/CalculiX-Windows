//
// Copyright 2006, Convergent Mechanical Solutions LLC
// all rights reserved
//
#ifndef _GL_GLX_H
#define _GL_GLX_H

#ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE 0x812F
#endif

/*
 * Data structure used by color operations
 */
typedef struct {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;  /* do_red, do_green, do_blue */
	char pad;
} XColor;

typedef int Colormap;

typedef struct {
	Colormap cmap;
} Display;

#endif
