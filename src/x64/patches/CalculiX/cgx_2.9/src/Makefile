CFLAGS = -O3 -Wall -DWIN32 \
  -I. \
  -I../../libSNL/src \
  -I../../glut-3.5/src \
  -I../../../glut-3.7.6-bin-32and64

LFLAGS = \
  -lglu32 -lopengl32 \
  -L../../../glut-3.7.6-bin-32and64/GL -lglut64 \
  -lpthread -lm 

CC=gcc
CPP=g++

# on 32bit systems change lib64 to lib
# -lpthread is needed if mesaGL is using threading, otherwhise it is not needed
# -L/usr/X11/lib64 -lpthread -lX11 -lXext -lXi -lXmu -lXt -lSM -lICE \

include Makefile.inc

SUTIL = \
 uselibSNL.cpp \
 generateTet.cpp \
 ../../libSNL/src/*.cpp

OLIB = $(SLIB:.c=.o)
OULIB = $(ULIB:.c=.o)
OUTIL = $(SUTIL:.c=.o)

cgx: $(OLIB) $(OUTIL) $(OULIB)
	$(CPP)  $(OULIB)  $(OLIB) $(OUTIL)  $(CFLAGS) $(LFLAGS) -o  $@



