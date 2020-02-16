# Mesa Makefile for HPUX

CC = gcc
#CCOPTS = -O2 -g -ansi -pedantic-errors
RM = rm -f

XINCDIR = -I/usr/include/X11
XLIBDIR = -L/usr/X11/lib
MESAINCDIR = -I/usr/include
MESALIBDIR = -L/usr/lib
CFLAGS = $(CCOPTS) $(XINCDIR) $(MESAINCDIR)
#LDLIBS = $(MESALIBDIR) -lglut -lMesaGL -lMesaGLU $(XLIBDIR) -lX11 -lXext -lXmu -lXi -lm
LDLIBS = $(MESALIBDIR) -lglut -lGL -lGLU $(XLIBDIR) -lX11 -lXext -lXmu -lXi -lm

all:	mp0 mp1 mp2 mp3 mp4 mp5 mp6

.c.o:	$*.c
	$(CC) -o $*.o $(CFLAGS) -c $*.c
	
mp0:	mp0.o
	$(CC) -o mp0 mp0.o $(LDLIBS)
	$(RM) mp0.o
	
mp1:	mp1.o
	$(CC) -o mp1 mp1.o $(LDLIBS)
	$(RM) mp1.o
	
mp2:	mp2.o
	$(CC) -o mp2 mp2.o $(LDLIBS)
	$(RM) mp2.o
	
mp3:	mp3.o
	$(CC) -o mp3 mp3.o $(LDLIBS)
	$(RM) mp3.o
	
mp4:	mp4.o
	$(CC) -o mp4 mp4.o $(LDLIBS)
	$(RM) mp4.o
	
mp5:	mp5.o
	$(CC) -o mp5 mp5.o $(LDLIBS)
	$(RM) mp5.o
	
mp6:	mp6.o
	$(CC) -o mp6 mp6.o $(LDLIBS)
	$(RM) mp6.o
	
clean:	;
	$(RM) mp0 mp1 mp2 mp3 mp4 mp5 mp6 \
	mp0.o mp1.o mp2.o mp3.o mp4.o mp5.o mp6.o

