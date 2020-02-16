#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP4                        
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997         
 *  DUE DATE : 11/7                                            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to understand the
 *             various splines presented in the book, derive the
 *             cubic Bezier Spline basis-matrix, derive the equations
 *             for forward difference calculations, and implement the
 *             drawing of Bezier Splines using forward differences.
 *                                                                     
 *  INPUTS   : Mouse/keyboard.
 */


/*************
 * Constants *
 *************/

#define MAX_POINTS 50
#define PRECISION 50 
#define MAX_SPLINES 1 + (MAX_POINTS - 4)/3
#define true 1
#define false 0


/*********
 * Types *
 *********/

typedef int bool;


/*********************
 * Globals Variables *
 *********************/

int Px[MAX_POINTS];               /* x coords. of control points */
int Py[MAX_POINTS];               /* y coords. of control points */
int Sx[MAX_SPLINES][PRECISION+1]; /* x coords. of spline points */
int Sy[MAX_SPLINES][PRECISION+1]; /* y coords. of spline points */
int splineCount;                  /* num. of splines */    
int numPoints = 0;                /* num. control points */
bool firstOrder;                  /* first order continuity */ 
int windowWidth = 300;            /* width of app. window */
int windowHeight = 300;           /* height of app. window */
bool movingPoint = false;         /* a point is currently being moved */   
int pointBeingMoved;              /* index of point being moved */   


/************************
 * Forward Declarations *
 ************************/

void Display(void);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SetCameraPosition(int width, int height);
void beep(void);
void CalcSpline(int p0, int p1, int p2, int p3, int spline[]);


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
 * Keyboard() processes input from the keyboard. "q" and
 * "Q" cause the application to exit. 
 * 
 * Inputs:       key, x, y 
 * Outputs:      none
 * Side effects: terminates application on 'Q' and 'q' 
 *               toggles first order cont. on ' ' 
 */

void Keyboard(unsigned char key, int x, int y) {
  key = toupper(key);
  if (key == 'Q') {
    exit(0);
  } else if (key == ' ') {
    if (firstOrder) {
      firstOrder = false;
    } else {
      firstOrder = true;
    }
    glutPostRedisplay();
  } else {
    beep();
  }  
}

/*
 * Mouse() is called when there's input from the mouse. 
 * 
 * Inputs:       button, state, x, y 
 * Outputs:      none
 * Side Effects: Either adds a control point, deletes all
 *               control points, or moves a control point. 
 *               May also be a no-op if moving a point
 *               and any key other than the middle key was
 *               clicked.
 *
 * The left button is used for entering control points.
 * The middle button is used for moving control points.
 * The right button is used for clearing control points.
 *
 * The coordindates of the control points are stored
 * in the arrays Px[] and Py[].  
 *
 * The global variable movingPoint is a boolean telling
 * whether or not a point is currently being moved. If
 * a point is being moved, the next click must be from
 * the middle button - all others are ignored.
 */

void Mouse(int button, int state, int x, int y) {
  bool redraw = false;
  int i;

  y = windowHeight - y;
  switch(button) {
    case GLUT_LEFT_BUTTON: 
      switch(state) {
        case GLUT_UP: 
          break;
        case GLUT_DOWN: 
          if ((numPoints<MAX_POINTS)&(!movingPoint)) {
            Px[numPoints] = x;
            Py[numPoints] = y;
            numPoints++;
            redraw = true; 
          } else {
            beep();
          } 
          break;
      }
      break;
    case GLUT_RIGHT_BUTTON: 
      switch(state) {
        case GLUT_UP: 
          break;
        case GLUT_DOWN: 
          if (!movingPoint) {
            numPoints = 0;
            movingPoint = false;
            redraw = true;
          } else {
            beep();
          }
          break;
      }
      break;
    case GLUT_MIDDLE_BUTTON: 
      switch(state) {
        case GLUT_UP: 
          break;
        case GLUT_DOWN: 
          if (!movingPoint) { 

            /* check if it's close to an existing points */

            for (i=0;i<numPoints;i++) {
              if ((abs(Px[i]-x)<4)&(abs(Py[i]-y)<4)) {
                movingPoint = true;
                pointBeingMoved = i; 
                redraw = true;
                break;
              }
            }
          } else {

            /* store new coords. for control point */

            Px[pointBeingMoved] = x;
            Py[pointBeingMoved] = y;
            movingPoint = false;
            redraw = true;
          }
          break;
        }
      break;
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
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) width, 0.0, (GLdouble) height);
  glMatrixMode(GL_MODELVIEW);
  windowWidth  = width;
  windowHeight = height;
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

  
  /* draw control points */

  glColor3f(0.0,1.0,1.0);
  glPointSize(5.0);
  if (movingPoint) {

    /* draw control points up to point being moved in cyan */

    for (i=0;i<pointBeingMoved;i++) {
      glBegin(GL_POINTS); 
      glVertex2i(Px[i], Py[i]);  
      glEnd(); 
    }

    /* draw control point being moved in white */

    glColor3f(1.0,1.0,1.0);
    glBegin(GL_POINTS); 
    glVertex2i(Px[i], Py[i]);  
    glEnd(); 

    /* draw remaining control point in cyan */

    glColor3f(0.0,1.0,1.0);
    for (i=pointBeingMoved+1;i<numPoints;i++) {
      glBegin(GL_POINTS); 
      glVertex2i(Px[i], Py[i]);  
      glEnd(); 
    }
  } else {

    /* draw all control point in cyan */

    for (i=0;i<numPoints;i++) {
      glBegin(GL_POINTS); 
      glVertex2i(Px[i], Py[i]);  
      glEnd(); 
    }
  }

  /* connect control points */

  glPointSize(1.0);
  glBegin(GL_LINE_STRIP); 
  for (i=0;i<numPoints;i++) {
    glVertex2i(Px[i], Py[i]);
  }
  glEnd(); 

  /* calc splines */

  splineCount = 0;
  for (i=0;i<=numPoints-4;i+=3) {
    if (firstOrder&(i>0)) {
      CalcSpline(Px[i], Px[i]+Px[i]-Px[i-1], Px[i+2], Px[i+3], Sx[splineCount]);
      CalcSpline(Py[i], Py[i]+Py[i]-Py[i-1], Py[i+2], Py[i+3], Sy[splineCount]);
    } else {
      CalcSpline(Px[i], Px[i+1], Px[i+2], Px[i+3], Sx[splineCount]);
      CalcSpline(Py[i], Py[i+1], Py[i+2], Py[i+3], Sy[splineCount]);
    }
    splineCount++;
  }

  /* draw splines */

  glColor3f(1.0,1.0,0.0);
  for (i=0;i<splineCount;i++) {
    glBegin(GL_LINE_STRIP); 
    for (j=0;j<=PRECISION;j++) {
      glVertex2i(Sx[i][j], Sy[i][j]);
    }
    glEnd();
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
 * CalcSpline() calculates the coordinates of a spline from
 * its 4 control points.  These coordinates are saved in the
 * global arrays Sx[][] and Sy[][].  The first dimension is
 * the spline, and the second is a point within the spline.
 * This subroutine accepts a pointer to the *second* dimension.
 *
 * The calculation used is a forward difference as described
 * in the text.
 */


void CalcSpline(int p0, int p1, int p2, int p3, int spline[]) {
  int i;
  double a, b, c, d;
  double deltaU = 1.0/PRECISION;
  double Xk, deltaXk, delta2Xk; 

  a = -p0+3*p1-3*p2+p3;
  b =  3*p0-6*p1+3*p2;
  c = -3*p0+3*p1;
  d =  p0;

  Xk = d; 
  deltaXk = a*pow(deltaU,3) + b*pow(deltaU,2) + c*deltaU;
  delta2Xk = 6*a*pow(deltaU,3) + 2*b*pow(deltaU,2);
  spline[0] = Xk; 
  for (i=1;i<=PRECISION;i++) {
    Xk += deltaXk;
    deltaXk += delta2Xk;
    delta2Xk += 6*a*pow(deltaU,3);
    spline[i] = Xk; 
  }
}
