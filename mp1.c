#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP1                   
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997    
 *  DUE DATE : 9/26            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to understand the
 *             midpoint-line algorithm and its similarities/diffs
 *             with Bresenham's algorithm.  We will also learn the
 *             basics of GLUT. 
 *                                                                     
 *  INPUTS   : Mouse/keyboard only. 
 */

/*
 * Constants
 */

#define TRUE 1
#define FALSE 0
#define MAX_SEGMENTS 20 
#define POS_X 0
#define NEG_X 1
#define POS_Y 2
#define NEG_Y 3
#define ON_PK_LESS 0
#define ON_PK_GREATER 1


/*
 * Globals
 */

int endpoint = 0;
int startPointX[MAX_SEGMENTS];
int startPointY[MAX_SEGMENTS];
int endPointX[MAX_SEGMENTS];
int endPointY[MAX_SEGMENTS];
int whichPoint = -1;
int windowWidth = 300;
int windowHeight = 300;
int lineWidth = 1;

/*
 * Forward Declarations
 */

void Display(void);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SetCameraPosition(int width, int height);
void MidpointLine(int x1, int y1, int x2, int y2);
void MidpointLineTest(int x1, int y1, int x2, int y2);
void beep(void);


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
   glutInitWindowPosition (0, 0);
   glutCreateWindow ("mp1");
   glutDisplayFunc(Display); 
   glutMouseFunc(Mouse);
   glutKeyboardFunc(Keyboard);
   glutReshapeFunc(SetCameraPosition);
   SetCameraPosition(windowWidth, windowHeight);
   glutMainLoop();
}

/*
 * Keyboard() processes input from the keyboard. "q" and
 * "Q" cause the application to exit.  Digits "1" through
 * "9" set the line width.  All other intput is ignored. 
 * 
 * Inputs:       key, x, y 
 * Outputs:      none
 * Side effects: terminates application on "Q" and "q" 
 */

void Keyboard(unsigned char key, int x, int y) {
  char tmp[2];
  key = toupper(key);
  if (key == 'Q') {
    exit(0);
  } else if (isdigit(key)) {
    if (key == '0') {
      beep();
    } else {
      tmp[0] = key;
      tmp[1] = '\0';
      sscanf(tmp, "%i", &lineWidth);
      glutPostRedisplay();
    } 
  } else {
    beep();
  }  
}

/*
 * Mouse() is called when there's input from the mouse.  This 
 * function determines whether the right or left mouse button 
 * was clicked.  Left clicks either set the starting endpoint
 * of the next line to be drawn, or set the ending endpoint and 
 * draw the line.  These endpoints are saved in the global arrays
 * startPointX, startPointY, endpointX, and endPointY.  The global
 * integer whichPoint indexes into these arrays. After a line has been 
 * drawn, whichPoint is incremented.  The Display() procedure also
 * also uses these global arrays and the current value of whichPoint
 * when the window needs redrawing.  For this reason, the value of 
 * whichPoint always represents the index of the last plotted segment.
 * whichPoint is then initialized to -1.  After the first segment has
 * been drawn, whichPoint becomes 0.  After the second 1, and so on.
 *
 * The global boolean variable endpoint tells Mouse() whether the
 * next left click is for a starting point or and ending point. This
 * variable is initialized to (0=FALSE) and subsequently set to 
 * (endpoint+1)mod2, so that it toggles between FALSE/TRUE, or starting
 * and ending, respectively.
 *
 * Right clicks reset whichPoint to -1 and clear the window. 
 * 
 * Inputs:       key, x, y 
 * Outputs:      none
 * Side effects: terminates application on "Q" and "q" 
 */

void Mouse(int button, int state, int x, int y) {
 
  y = windowHeight - y;
  switch(button) {
    case GLUT_LEFT_BUTTON: 
      switch(state) {
        case GLUT_UP: 
          break;
        case GLUT_DOWN: 
          
          /*
           * return with a "beep" if we've already plotted MAX_SEGMENTS
           */
  
          if (whichPoint+1 == MAX_SEGMENTS) {
            beep();
            return;
          } 
          switch(endpoint) {
            case TRUE:

              /* ending point of segment */

              glColor3f(1.0, 0.0, 0.0);
              endPointX[whichPoint+1] = x;
              endPointY[whichPoint+1] = y;
              whichPoint++;
              break;
            case FALSE:

              /* starting point of segment */
               
              glColor3f(0.0, 1.0, 0.0);
              startPointX[whichPoint+1] = x;
              startPointY[whichPoint+1] = y;
              break;
          }

          /* toggle endpoint between 0/1 or FALSE/TRUE */

          endpoint = (endpoint+1)%2;

          break;
      }
      break;
    case GLUT_RIGHT_BUTTON: 
      switch(state) {
        case GLUT_DOWN: 
          break;
        case GLUT_UP: 
          glClear(GL_COLOR_BUFFER_BIT);
          endpoint = FALSE;
          whichPoint = -1;
          break;
      }
      break;
  }
  glutPostRedisplay();
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
 * Display() is responsible for redrawing the window.  The global
 * integer whichPoint stores the index of the last drawn segment, and
 * indexes into the global arrays startPointX, startPointY, endPointX,
 * and endPointY.  The procedure redraws segments 0 to whichPoint.
 *
 *
 * Inputs:       none 
 * Outputs:      none
 * Side effects: clears and draws the main window
 */

void Display(void)
{
  int i;
  glClear(GL_COLOR_BUFFER_BIT);

  /*
   * If the next point clicked will be an endpoint
   * then a starting point must be out there.
   */
  if (endpoint) {
    glPointSize(3.0);
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POINTS);  
      glVertex2i(startPointX[whichPoint+1], startPointY[whichPoint+1]);
    glEnd();  
  }
  for (i=0;i<=whichPoint;i++) {
    glPointSize(3.0);
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POINTS);  
      glVertex2i(startPointX[i], startPointY[i]);
    glEnd();  
    glPointSize(3.0);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POINTS);  
      glVertex2i(endPointX[i], endPointY[i]);
    glEnd();  
    glPointSize(1.0);
    glColor3f(1.0, 1.0, 1.0);
    MidpointLine(startPointX[i], startPointY[i], endPointX[i], endPointY[i]);
    glFlush();
  }
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
 * MidpointLine() is my implementation of the midpoint line
 * algorithm.  The vertices of the line to plot are stored in
 * the global arrays startPointX, startPointY, endPointX, and
 * endPointY.  <index> indexes into these arrays.
 *
 * The global variable lineWidth stores the width of the line.
 * This is a value between 1 and 9, inclusive.
 *
 * Inputs:       index 
 * Outputs:      none
 * Side effects: draws line segment of width <lineWidth>
 *               from (x1,y1) (x2,y2).
 *
 * The algorithm begins by initializing the constants deltaX,
 * deltaY, twoDeltaX, and twoDeltaY.  The slope is calculated
 * only two determine which quadrant the line is being drawn
 * in. 
 *
 * Once the quadrant has been determined, we use one of the
 * eight variations of the midpoint line algorithm.  Each
 * quadrant has particular values for the following variables:
 * 
 * stepBy: step positively or negatively by x or y
 * yInc: increment y by 1 or -1 when stepping by x 
 * xInc: increment x by 1 or -1 when stepping by y
 * steps: number of steps, either abs(deltaX) or abs(deltaY)
 * incWhen: increment to next pixel when either P(k)
 *          is less than or equal to zero, or when P(k) is 
 *          greater than zero 
 * P[0]: base case for decision function
 * incPkLessThanEqualZero: amount to P[k] by to get P[k+1] when
 *                         P[k] is <= 0 
 * incPkGreaterThanZero: amount to P[k] by to get P[k+1] when
 *                       P[k] is > 0 
 */


void MidpointLine(int x1, int y1, int x2, int y2) {
  int twoDeltaY, twoDeltaX, deltaX, deltaY;
  int steps, stepBy, k, xInc, yInc, incWhen; 
  int x, y, i;
  double slope; 
  int quadrant;
  int incPkLessThanEqualZero, incPkGreaterThanZero;
  int stepByX;
  int Pk, PkPlus1;

  /*
   * Initialize constants
   */

  deltaX = x2 - x1; twoDeltaX = 2*deltaX;
  deltaY = y2 - y1; twoDeltaY = 2*deltaY;  
  slope = (double) deltaY/deltaX;

  /*
   * Determine what quadrant we're in.
   */

  if (x1 <= x2) {
    if (slope < -1)  {
      quadrant = 7;
    } else if (slope < 0) {
      quadrant = 8;
    } else if (slope < 1) {
      quadrant = 1;
    } else {
      quadrant = 2;
    }
  } else {
    if (slope < -1)  {
      quadrant = 3;
    } else if (slope < 0) {
      quadrant = 4;
    } else if (slope < 1) {
      quadrant = 5;
    } else {
      quadrant = 6;
    }
  }

  /*
   * Initialize things for the given quadrant.
   */

  switch (quadrant) {
    case 1:
      stepBy  = POS_X;
      steps   = deltaX;
      yInc    = 1;
      incWhen = ON_PK_GREATER;
      Pk      = twoDeltaY - deltaX;
      incPkLessThanEqualZero = twoDeltaY;
      incPkGreaterThanZero   = twoDeltaY - twoDeltaX;
      break;
    case 2:
      stepBy  = POS_Y;
      steps   = deltaY;
      xInc    = 1;
      incWhen = ON_PK_GREATER;
      Pk      = twoDeltaX - deltaY;
      incPkLessThanEqualZero = twoDeltaX;
      incPkGreaterThanZero   = twoDeltaX - twoDeltaY;
      break;
    case 3:
      stepBy  = POS_Y;
      steps   = deltaY;
      xInc    = -1;
      incWhen = ON_PK_LESS;
      Pk      = twoDeltaX + deltaY;
      incPkLessThanEqualZero = twoDeltaX + twoDeltaY;
      incPkGreaterThanZero   = twoDeltaX;
      break;
    case 4:
      stepBy  = NEG_X;
      steps   = -deltaX;
      yInc    = 1;
      incWhen = ON_PK_GREATER;
      Pk      = twoDeltaY + deltaX;
      incPkLessThanEqualZero = twoDeltaY;
      incPkGreaterThanZero   = twoDeltaY + twoDeltaX;
      break;
    case 5:
      stepBy  = NEG_X;
      steps   = -deltaX;
      yInc    = -1;
      incWhen = ON_PK_LESS;
      Pk      = twoDeltaY - deltaX;
      incPkGreaterThanZero   = twoDeltaY;
      incPkLessThanEqualZero = twoDeltaY - twoDeltaX;
      break;
    case 6:
      stepBy  = NEG_Y;
      steps   = -deltaY;
      xInc    = -1;
      incWhen = ON_PK_LESS;
      Pk      = twoDeltaX - deltaY;
      incPkGreaterThanZero   = twoDeltaX;
      incPkLessThanEqualZero = twoDeltaX - twoDeltaY;
      break;
    case 7:
      stepBy  = NEG_Y;
      steps   = -deltaY;
      xInc    = 1;
      incWhen = ON_PK_GREATER;
      Pk      = twoDeltaX + deltaY; 
      incPkLessThanEqualZero = twoDeltaX;
      incPkGreaterThanZero   = twoDeltaX + twoDeltaY;
      break;
    case 8:
      stepBy  = POS_X;
      steps   = deltaX;
      yInc    = -1;
      incWhen = ON_PK_LESS;
      Pk      = twoDeltaY + deltaX;
      incPkLessThanEqualZero = twoDeltaY + twoDeltaX;
      incPkGreaterThanZero   = twoDeltaY;
      break;
  }


  /*
   * To avoid too many or operations
   */ 

  if ((stepBy==POS_X)|(stepBy==NEG_X)) {
    stepByX = TRUE;
  } else {
    stepByX = FALSE;
  }

  /*
   *  plot first point
   */

  x = x1; y = y1;
  glBegin(GL_POINTS);  
    glVertex2i(x, y);
  glEnd();  

  /* 
   * plot <steps> points
   */

  for (k=0;k<steps;k++) {
    if (Pk <= 0) {
      PkPlus1 = Pk + incPkLessThanEqualZero; 
      if (incWhen == ON_PK_LESS) {
        if (stepByX) {
          y += yInc;
        } else {
          x += xInc;
        }
      }
    } else {
      PkPlus1 = Pk + incPkGreaterThanZero; 
      if (incWhen == ON_PK_GREATER) {
        if (stepByX) {
          y += yInc;
        } else {
          x += xInc;
        }
      }
    } 
    Pk = PkPlus1;

    /*
     * increment var we're stepping by 
     */

    switch (stepBy) {
      case POS_X:
        x++;
        break;
      case NEG_X:
        x--;
        break;
      case POS_Y:
        y++;
        break;
      case NEG_Y:
        y--;
        break;
    } 


    /*
     * Line width is implemented as discussed on page
     * 147 of the book. For abs(slopes)<=1, we plot extra
     * pixels first above and then below the line.  For 
     * abs(slopes>1), we plot extra pixels horizontally, first
     * on the left and then right on the right. 
     */

    glBegin(GL_POINTS);  
      if (fabs(slope)<=1.0) {
        for (i=0;i<(lineWidth+1)/2.0;i++) { 
          glVertex2i(x, y+i);
        }
        for (i=1;i<lineWidth/2.0;i++) {
          glVertex2i(x, y-i);
        }
      } else {
        for (i=0;i<(lineWidth+1)/2.0;i++) { 
          glVertex2i(x-i, y);
        }
        for (i=1;i<lineWidth/2.0;i++) {
          glVertex2i(x+i, y);
        }
      }
    glEnd();  
  }
}

/*
 * Used for first draft of program
 */ 

void MidpointLineTest(int x1, int y1, int x2, int y2) {
  glLineWidth(lineWidth);
  glBegin(GL_LINE_LOOP);  
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
  glEnd();  
}

/*

Derivation of Midpoint-Line Algorithm (quadrant 1)
--------------------------------------------------


Let p(k) = f(x[k]+1),y[k]+1/2) 
         = m(x[k]+1) + b - y[k] - 1/2
         = m*x[k] + m + b - y[k] - 1/2


multiply by 2*deltaX to eliminate the calculation of m and 1/2:

         = 2*deltaY*x[k] + 2*deltaY + 2*deltaX*b - 2*deltaX*y[k] - deltaX 
         = 2*deltaY*x[k] - 2*deltaX*y[k] + 2*deltaY + deltaX(2b-1)


substite p[k+1] to evaluate at the next term and develop a recurrence:


p(k+1)   = 2*deltaY*x[k+1] - 2*deltaX*y[k+1] + 2*deltaY + deltaX(2b-1)


subtract p(k) from p(k+1):


p(k+1) - p(k) = 2*deltaY(x[k+1]-x[k]) - 2*deltaX(y[k+1]-y[k])
              = 2*deltaY(x[k] + 1 - x[k]) - 2*deltaX(y[k+1]-y[k])
              = 2*deltaY - 2*deltaX(y[k+1]-y[k])

therefore:


p(k+1) = p(k) + 2*deltaY - 2*deltaX(y[k+1]-y[k])
       = p(k) + 2*deltaY            for p[k] <=0 
       = p(k) + 2*deltaY - 2*deltaX for p[k] >0 

To calculate p(0), subsitute x(0),y(0) into p(k):

p(0) = 2*deltaY*x[0] - 2*deltaX*y[0] + 2*deltaY + deltaX(2b-1)
     = 2*deltaY*x[0] - 2*deltaX*y[0] + 2*deltaY + 2*deltaX*b - deltaX
     = 2*deltaY*x[0] - 2*deltaX*y[0] + 2*deltaX*b + 2*deltaY - deltaX
     = 2(deltaY*x[0] - deltaX*y[0] + deltaX*b) + 2*deltaY - deltaX
     = 2(f(x(0),y(0)) + 2*deltaY - deltaX
     = 2*deltaY - deltaX


Derivation of Midpoint-Line Algorithm (quadrant 2)
--------------------------------------------------


For quadrant two, I just reversed the roles of x and y relative
to quadrant one:

    p(0)   = 2*deltaX - deltaY
    p(k+1) = p(k) + 2*deltaX            for p[k] >0 
           = p(k) + 2*deltaX - 2*deltaY for p[k] <=0 


Derivation of Midpoint-Line Algorithm (quadrant 3)
--------------------------------------------------

For quadrant 3, my decision parameter tested for x[k]-1/2
instead of x[k]+1/2 (as in quad 2).  The x coordinate is
then *decremented* by 1 if p[k] <= 0.

 
Derivation of Midpoint-Line Algorithm (quadrant 4)
--------------------------------------------------

For quadrant 4, I just reversed the roles of x and y relative 
to quadrant 3.  I also decrement in units of x, and update the
pixel on p[k]>=0. 


Derivation of Midpoint-Line Algorithm (quadrant 5)
--------------------------------------------------

Quadrant 5 is the same as quadrant 4, except that I decrement
the y pixel value on pk<=0 instead of incrementing on pk>0. 


Derivation of Midpoint-Line Algorithm (quadrant 6)
--------------------------------------------------

Quadrant 6 is just quadrant 5 with the x and y's reversed. 


Derivation of Midpoint-Line Algorithm (quadrant 7)
--------------------------------------------------

Quadrant 7 is the same as quadrant 6, except that I test for
x[k] + 1/2 instead of x[k] - 1/2.  This changes the sign
of the incremental 2*deltaY in the decision parameter and 
deltaY in the initial condition.


Derivation of Midpoint-Line Algorithm (quadrant 8)
--------------------------------------------------

Quadrant 8 is quadrant 7 with x and y reversed, and I step
by in unit steps of x, instead of negative unit steps in
y.  The updating is on pk<=0.  


Comment
-------

There are many similarities between quadrants.  For example, quadrants 
1, 4, 5, and 8 all have slope magnitudes less than 1.  And quadrants 
2, 3, 6, and 7 all have magnitudes greater than 1.  So I could have used these
facts to initialize what variable I'm stepping by.  Other similarties exist
between quadrants and when when a pixel is incremented (i.e. on pk<=0 or pk>0).
However, I chose not to bundle all of these quadrants into categories because 
I think it makes the code less readable.  You'll notice that in my initializ-
ation section, you can see exactly what paramaters are required in each
quadrant.  I also chose this strategy to minimize the number of conditional 
statements within the while loop.
*/
