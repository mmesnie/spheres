#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP5                        
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997         
 *  DUE DATE : 11/21                                            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to understand deterministic, 
 *             self-similar fractals and how to generate them.  We should
 *             learn how to caculate the fractal similarity dimension and
 *             be able to use the built-in OpenGL transformation routines.
 *                                                                     
 *  INPUTS   : Mouse/keyboard.
 */


/*************
 * Constants *
 *************/

#define MAX_POINTS 40
#define MIN_LEVEL 1
#define MAX_LEVEL 6
#define true 1
#define false 0
#define WIDTH 600
#define HEIGHT 600
#define START_X 100 
#define START_Y 300 
#define END_X 500 
#define END_Y 300 

/*
 * Test Initiator
 */

#define TEST_X_START 50 
#define TEST_Y_START 50 
#define TEST_X_END 100 
#define TEST_Y_END 100 


/*********
 * Types *
 *********/

typedef int bool;


/*********************
 * Globals Variables *
 *********************/

int Px[MAX_POINTS];               /* x coords. of generator vertices */
int Py[MAX_POINTS];               /* y coords. of generator vertices */
int numPoints = 0;                /* num. generator vertices*/
int windowWidth = WIDTH;          /* width of app. window */
int windowHeight = HEIGHT;        /* height of app. window */
int recursionLevel = 1;           /* level of recursion */
bool gridOn = false;              /* grid on/off */
bool enteringPoints = false;      /* entering gen. points or not */


/************************
 * Forward Declarations *
 ************************/

void Display(void);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SetCameraPosition(int width, int height);
void beep(void);
void drawFractal(int x1, int y1, int x2, int y2, int depth);
void drawGrid();


/****************
 * Subroutines  *
 ****************/

/*
 * main() initializes GLUT, creates the main window, 
 * and loops for events. 
 *
 * Inputs:       argc, argv 
 * Outputs:      none
 * Side effects: main window created
 */

void main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize (windowWidth, windowHeight); 
   glutCreateWindow ("mp4");
   glutDisplayFunc(Display); 
   glutMouseFunc(Mouse);
   glutKeyboardFunc(Keyboard);
   glutReshapeFunc(SetCameraPosition);
   SetCameraPosition(windowWidth, windowHeight);
   glutMainLoop();
}


/*
 * Keyboard() processes input from the keyboard.
 * 
 * Inputs:       key, x, y 
 * Outputs:      none
 * Side effects: Terminates application on 'Q' and 'q' 
 *               Digits 1-6 set the level of recursion.
 *               Spacebar toggles grid. 
 */

void Keyboard(unsigned char key, int x, int y) {
  int level;
  bool redraw = false;

  key = toupper(key);
  if (key == 'Q') {
    exit(0);
  } else if (key == ' ') {
    if (gridOn) {
      gridOn = false;
      redraw = true;
    } else {
      gridOn = true;
      redraw = true;
    }
  } else if (isdigit(key)) {
    level = key - '0';
    if ((level>=MIN_LEVEL)&(level<=MAX_LEVEL)) {
      recursionLevel = level;
      redraw = true;
    } else {
      beep();
    }
  } else {
    beep();
  }  
  if (redraw) {
    glutPostRedisplay();
  }
}

/*
 * Mouse() is called when there's input from the mouse. 
 * 
 * Inputs:       button, state, x, y 
 * Outputs:      none
 * Side Effects: Enters new generator points. 
 *
 * The coordindates of the generator points are stored
 * in the arrays Px[] and Py[].  Clicking on the START
 * vertex clears all points.  Clicking the END vertex
 * completes the generator.
 *
 */

void Mouse(int button, int state, int x, int y) {
  bool redraw = false;

  y = windowHeight - y;

  /*
   * force placement to closest multiple of 50
   * if grid is on
   */

  if (gridOn) {
    x = 50*(x/50+(x%50+25)/50); 
    y = 50*(y/50+(y%50+25)/50); 
  }

  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      if ((abs(START_X-x)<=3)&(abs(START_Y-y)<=3)) {
        enteringPoints = true;
        numPoints = 0;
        Px[numPoints] = START_X;
        Py[numPoints] = START_Y;
        numPoints++;
        redraw = true; 
      } else if (enteringPoints) {
        if ((abs(END_X-x)<=3)&(abs(END_Y-y)<=3)) {
          Px[numPoints] = END_X;
          Py[numPoints] = END_Y;
          numPoints++;
          enteringPoints = false; redraw = true; } else {
          if ((numPoints+1) == MAX_POINTS) {
            Px[numPoints] = END_X;
            Py[numPoints] = END_Y;
            enteringPoints = false;
          } else {
            Px[numPoints] = x;
            Py[numPoints] = y;
          }
          numPoints++;
          redraw = true; 
        }
      }
    }
  } else {
    beep();
  } 
  if (redraw) {
    glutPostRedisplay();
  }
}


/*
 * SetCameraPosition() sets up the viewing area for 
 * the window. 
 *
 * Inputs:       width, height 
 * Outputs:      none
 * Side effects: sets viewing area 
 */

void SetCameraPosition(int width, int height) {
  glutReshapeWindow(WIDTH, HEIGHT);
  glViewport(0, 0, WIDTH, HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) WIDTH, 0.0, (GLdouble) HEIGHT);
  glMatrixMode(GL_MODELVIEW);
}


/*
 * Display() is responsible for redrawing the window.  
 *
 * Inputs:       none 
 * Outputs:      none
 * Side effects: clears and draws the main window
 */

void Display(void)
{
  int i, j;

  /* clear window */ 

  glClear(GL_COLOR_BUFFER_BIT);

  /* draw grid */

  if (gridOn) {
    drawGrid();
  }

  /* draw "start" point */

  glColor3f(0.0,1.0,0.0);
  glPointSize(5.0);
  glBegin(GL_POINTS); 
  glVertex2i(START_X, START_Y);
  glEnd(); 

  /* draw "end" point */

  glColor3f(1.0,0.0,0.0);
  glPointSize(5.0);
  glBegin(GL_POINTS); 
  glVertex2i(END_X, END_Y);
  glEnd(); 


  if (enteringPoints) {

    /* connect generator points */

    glPointSize(1.0);
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_LINE_STRIP); 
    for (i=0;i<numPoints;i++) {
      glVertex2i(Px[i], Py[i]);
    }
    glEnd(); 
  } else if (numPoints>0) {

    /* draw fractal */

    drawFractal(START_X, START_Y, END_X, END_Y, 1);
    glLoadIdentity();
  }


  /* flush all commands */

  glFlush();
}

/*
 * beep() sends an audible character to the console
 *
 * Inputs:       none 
 * Outputs:      none
 * Side effects: beep
 */

void beep() {
  printf("\a");
  fflush(stdout);
}


/*
 * drawFractal() recursively draws a fractal with <depth>
 * levels of recursion. 
 *
 * Inputs:  starting/ending points of initializer and current depth 
 * Outputs: none
 * Side Effects: draws fractal :)
 */

void drawFractal(int x1, int y1, int x2, int y2, int depth) {
  int i, j;
  double scale, theta;
  double M[16];

  /* push matrix since we're going into a level of recursion */

  glPushMatrix();

  /* save top of stack so we can later multiply */
 
  glGetDoublev(GL_MODELVIEW_MATRIX, M);
  glLoadIdentity();

  /* scale */

  scale = (double) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1))/(END_X-START_X);
  glTranslatef(x1, y1, 0);
  glScalef(scale, scale, 1.0);
  glTranslatef(-x1, -y1, 0);
 
  /* rotate */

  if (x2 != x1) {
    theta = (360.0*atan((double)(y2-y1)/(double)(x2-x1)))/(2*3.1415927);
    if (x2<x1) {
      theta += 180;
    }
  } else {
    if (y2 > y1) {
      theta = 90.0;
    } else {
      theta = -90.0;
    }
  }
  glTranslatef(x1, y1, 0);
  glRotatef(theta,0.0,0.0,1.0);
  glTranslatef(-x1, -y1, 0);

  /* translate */

  glTranslatef(x1-START_X, y1-START_Y, 0);

  /* multiply with saved copy */

  glMultMatrixd(M);

  /* if this is not the final depth, call drawFractal
   * on remaining edges, else draw the edges
   */

  if (depth<recursionLevel) {
    for (i=0;i<numPoints-1;i++) {
      drawFractal(Px[i], Py[i], Px[i+1], Py[i+1], depth+1);
    }
  } else {
    glColor3f(1.0,1.0,0.0);
    glBegin(GL_LINE_STRIP);
    for (i=0;i<numPoints;i++) {
      glVertex2i(Px[i], Py[i]);
    }
    glEnd();
  }

  /* pop matrix since we're coming out of a level of recursion */

  glPopMatrix();
}


/*
 * drawGrid() draws the grid of points :)
 *
 * Input: none
 * Output: none
 * Side Effects: grid drawn
 */

void drawGrid() {
  int i, j;

  glColor3f(0.0,1.0,0.0);
  glPointSize(1.0);
  for (i=0;i<HEIGHT;i+=50) {  
    for (j=0;j<WIDTH;j+=50) {  
      glBegin(GL_POINTS); 
        glVertex2i(i,j);
      glEnd();
    }
  }
}
