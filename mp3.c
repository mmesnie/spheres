#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#define DOUBLE_BUFFERING
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP3                   
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997    
 *  DUE DATE : 10/26            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to understand the
 *             various line and polygon clipping algorithms presented
 *             in the text and to understand how to implement the 
 *             Nicholl-Lee-Nicholl line clipping algorithm.
 *                                                                     
 *  INPUTS   : Mouse/keyboard only. 
 */


/*
 * Constants
 */

#define TRUE 1
#define FALSE 0
#define MAX_SEGMENTS 20 
#define INF  1000000.0
#define NINF -1000000.0


/*
 * Globals Variables
 */

/* These are the globals used by the clipping
 * routines.  They are for the clipped points
 * and the slopes. 
 */ 

int PxNew, PyNew, QxNew, QyNew;
double M, Mtl, Mtr, Mbl, Mbr;
int deltaX, deltaY;

/* clipping window coordinates */ 

int xMin; int yMin;
int xMax; int yMax;

/* arrays for line segement endpoints */

int numPoints = -1;
int Px[MAX_SEGMENTS];
int Py[MAX_SEGMENTS];
int Qx[MAX_SEGMENTS];
int Qy[MAX_SEGMENTS];
int numClippedPoints = -1;
int PxClipped[MAX_SEGMENTS];
int PyClipped[MAX_SEGMENTS];
int QxClipped[MAX_SEGMENTS];
int QyClipped[MAX_SEGMENTS];

/* misc. */

typedef int boolean;
int lineWidth = 1;
int windowWidth = 300;
int windowHeight = 300;
boolean cleared = FALSE;
int lastX, lastY;
boolean rubberbanding = FALSE;
boolean enteringLine = FALSE;
boolean enteringRect = FALSE;
boolean noRectangle = TRUE;


/*
 * Forward Declarations
 */

void Display(void);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SetCameraPosition(int width, int height);
void beep(void);
void LineClip(int i);
boolean PxDirectlyLeft(int i);
boolean PxDirectlyTop(int i);
boolean PxCenter(int i);
boolean PxTopLeft(int i);
double slope(int deltaY, int deltaX);
void ClipInit(int i);
void MouseMovement(int x, int y);


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
   #ifdef DOUBLE_BUFFERING 
     glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   #else
     glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
   #endif
   glutInitWindowSize (windowWidth, windowHeight); 
   glutCreateWindow ("mp3");
   glutDisplayFunc(Display); 
   glutMouseFunc(Mouse);
   glutMotionFunc(MouseMovement);
   glutPassiveMotionFunc(MouseMovement);
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
 * Side effects: terminates application on "Q" and "q" 
 */

void Keyboard(unsigned char key, int x, int y) {
  key = toupper(key);
  if (key == 'Q') {
    exit(0);
  } else if (key == ' ') {
    if (rubberbanding) {
      rubberbanding = FALSE;
      cleared = FALSE;
      glutPostRedisplay();
    } else {

      /* Update now instead of waiting for a mouse movement */   

      rubberbanding = TRUE;
      MouseMovement(x, y);
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
 * Px, Py, endpointX, and Qy.  The global
 * integer numPoints indexes into these arrays. After a line has been 
 * drawn, numPoints is incremented.  The Display() procedure also
 * also uses these global arrays and the current value of numPoints
 * when the window needs redrawing.  For this reason, the value of 
 * numPoints always represents the index of the last plotted segment.
 * numPoints is then initialized to -1.  After the first segment has
 * been drawn, numPoints becomes 0.  After the second 1, and so on.
 *
 * The global boolean variable enteringLine tells Mouse() whether the
 * next left click is for a starting point or and ending point.
 *
 * Right clicks reset numPoints to -1 and clear the window. 
 *
 * Middle clicks set the starting/endpoints vertices for the clipping
 * region.  These points are save in (xMin, yMin) and (xMax, yMax).
 * 
 * Inputs:       key, x, y 
 * Outputs:      none
 * Side effects: terminates application on "Q" and "q",
 *               and " " toggles rubberbanding.
 */

void Mouse(int button, int state, int x, int y) {
  int tmp;
  boolean update;
  int i;
 
  y = windowHeight - y;
  update = FALSE;
  switch(button) {
    case GLUT_LEFT_BUTTON: 
      switch(state) {
        case GLUT_UP: 
          break;
        case GLUT_DOWN: 
          
          /*
           * return with a "beep" if we've already plotted MAX_SEGMENTS
           */
  
          if ((numPoints+1 == MAX_SEGMENTS)|(enteringRect)) {
            beep();
            return;
          } 
          switch(enteringLine) {
            case TRUE:

              /* ending point of segment */

              glColor3f(1.0, 0.0, 0.0);
              Qx[numPoints+1] = x;
              Qy[numPoints+1] = y;
              numPoints++;
              LineClip(numPoints);
              enteringLine = FALSE;
              cleared = FALSE;
              break;
            case FALSE:

              /* starting point of segment */
              
              glColor3f(0.0, 1.0, 0.0);
              Px[numPoints+1] = x;
              Py[numPoints+1] = y;
              enteringLine = TRUE;
              break;
          }
          update = TRUE;
          break;
      }
      break;
    case GLUT_RIGHT_BUTTON: 
      switch(state) {
        case GLUT_DOWN: 
          break;
        case GLUT_UP: 

          /* clear everything */

          glClear(GL_COLOR_BUFFER_BIT);
          enteringLine = FALSE;
          numPoints = -1;
          numClippedPoints = -1;
          update = TRUE;
          break;
      }
      break;
    case GLUT_MIDDLE_BUTTON: 
      switch(state) {
        case GLUT_DOWN: 
          if (enteringLine) {
            beep();
            return;
          } 
          switch(enteringRect) { 

            /* starting corner */

            case FALSE:
              xMin = x; yMin = y;
              enteringRect = TRUE;
              break;

            /* ending corner */

            case TRUE:
              xMax = x; yMax = y;
              if (xMin > xMax) {
                tmp = xMin;
                xMin = xMax;
                xMax = tmp;
              } 
              if (yMin > yMax) {
                tmp = yMin;
                yMin = yMax;
                yMax = tmp;
              } 
              enteringRect = FALSE;
              noRectangle = FALSE;
              cleared = FALSE; 
              numClippedPoints = -1; 
              for (i=0;i<=numPoints;i++) {
                LineClip(i);
              }
              update = TRUE;
              break;
          }
          update = TRUE;
          break;
        case GLUT_UP: 
          break;
        }
      break;
  }
  if (update) {
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
  cleared = FALSE;

  /* call mouse movement procedure if rubberbanding is ON */

  if (rubberbanding) {
    MouseMovement(lastX, lastY);  
  }
}


/*
 * Display() is responsible for redrawing the window.  The global
 * integer numPoints stores the index of the last drawn segment, and
 * indexes into the global arrays Px, Py, Qx,
 * and Qy.  The procedure redraws segments 0 to numPoints.
 *
 * In addition to line segments, Display also draws the clipping
 * rectangle and the clipped line segments.
 *
 * Inputs:       none 
 * Outputs:      none
 * Side effects: clears and draws the main window
 */

void Display(void)
{
  int i;

  /*
   * "cleared" is a boolean used by MouseMovement() to let Display()
   * know that it already cleared the screen and has started
   * drawing.
   */

  if (!cleared) {
    glClear(GL_COLOR_BUFFER_BIT);
  }

  /*
   * If the next point to be clicked is an endpoint
   * then draw the starting point.
   */

  glColor3f(0.0, 1.0, 0.0);
  if (enteringLine) {
    glPointSize(3.0);
    glBegin(GL_POINTS);  
      glVertex2i(Px[numPoints+1], Py[numPoints+1]);
    glEnd();  
  }

  /*
   * Draw all line segments in green.
   */

  glPointSize(1.0);
  for (i=0;i<=numPoints;i++) {
    glBegin(GL_LINES);
      glVertex2i(Px[i], Py[i]);
      glVertex2i(Qx[i], Qy[i]);
    glEnd();
  }

  /*
   * Draw either a blue point or a blue rectangle.
   */

  glColor3f(0.0, 0.0, 1.0);
  if (enteringRect) {
    glPointSize(3.0);
    glBegin(GL_POINTS);  
      glVertex2i(xMin, yMin);
    glEnd();  
  } else {
    if (!noRectangle) {
      glPointSize(1.0);
      glBegin(GL_LINE_LOOP);
        glVertex2i(xMin, yMin);
        glVertex2i(xMin, yMax);
        glVertex2i(xMax, yMax);
        glVertex2i(xMax, yMin);
      glEnd();
    } 
  }

  /*
   * Draw all clipped line segments in red.
   */

  glPointSize(1.0);
  glColor3f(1.0, 0.0, 0.0);
  for (i=0;i<=numClippedPoints;i++) {
    glBegin(GL_LINES);
      glVertex2i(PxClipped[i], PyClipped[i]);
      glVertex2i(QxClipped[i], QyClipped[i]);
    glEnd();
  }
  #ifdef DOUBLE_BUFFERING
  glutSwapBuffers();
  #else
  glFlush();
  #endif
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
 * LineClip() determines which of the cases the lines needs 
 * to be clipped for and then calls one of the clipping
 * routines.
 *
 * Inputs:       index to the current line segment "i"  
 * Outputs:      none
 * Side effects: calls clipping routines
 */

void LineClip(int i) {
  boolean cut;
  boolean yReflected;
  boolean xReflected;
  int Xc, Yc;

  /* 
   * Proceed no further if there is no clipping rectangle.
   */

  if (noRectangle) {
    return;
  }

  /*
   * The center of the rectangle will be needed for
   * reflections.
   */

  Xc = (xMax+xMin)/2;
  Yc = (yMax+yMin)/2;
  xReflected = FALSE;
  yReflected = FALSE;

  if (((Px[i]<xMin)|(Px[i]>xMax))&(Py[i]>=yMin)&(Py[i]<=yMax)) {

    /*
     * Px is directly left or directly right.  If directly
     * right, reflect about Y axis and treat as thought it's
     * directly left.
     */

    if (Px[i]>xMax) {
      /* reflect about y axis */
      Px[i] -= 2*(Px[i]-Xc);
      Qx[i] -= 2*(Qx[i]-Xc);
      yReflected = TRUE;
    } 
    cut = PxDirectlyLeft(i);
  } else if (((Py[i]>yMax)||(Py[i]<yMin))&(Px[i]>=xMin)&(Px[i]<=xMax)) {
 
    /*
     * Px is directly top or directly bottom.  If directly
     * bottom, reflect about X axis and treat as thought it's
     * directly top.
     */

    if (Py[i]<yMin) {
      /* reflect about x axis */
      Py[i] += 2*(Yc-Py[i]);
      Qy[i] += 2*(Yc-Qy[i]);
      xReflected = TRUE;
    } 
    cut = PxDirectlyTop(i);
  } else if ((Px[i]<=xMax)&(Px[i]>=xMin)&(Py[i]<=yMax)&(Py[i]>=yMin)) {

    /* 
     * Px is in the center.
     */

    cut = PxCenter(i);
  } else {

    /*
     * Px is in one of the four corners.  If necessary reflect about
     * the x and/or y axis to get in the top left corner.
     */

    if (((Px[i]<xMin)||(Px[i]>xMax))&((Py[i]>yMax)||(Py[i]<yMax))) {
      if (Px[i]>xMax) {
        Px[i] -= 2*(Px[i]-Xc);
        Qx[i] -= 2*(Qx[i]-Xc);
        yReflected = TRUE;
      }
      if (Py[i]<yMin) {
        Py[i] += 2*(Yc-Py[i]);
        Qy[i] += 2*(Yc-Qy[i]);
        xReflected = TRUE;
      }
      cut = PxTopLeft(i);
    } else {
      /* should never get this far...*/
      beep();
    }
  }

  /*
   * If the point was not cut (return of FALSE), then
   * update the unclipped point line segment arrays and
   * do some reverse reflections if necessary.
   */

  if (!cut) {
    numClippedPoints++;
    PxClipped[numClippedPoints] = PxNew;
    PyClipped[numClippedPoints] = PyNew;
    QxClipped[numClippedPoints] = QxNew;
    QyClipped[numClippedPoints] = QyNew; 
    if (yReflected||xReflected) {
      if (yReflected) {
        PxClipped[numClippedPoints] += 2*(Xc-PxClipped[numClippedPoints]);
        QxClipped[numClippedPoints] += 2*(Xc-QxClipped[numClippedPoints]);
      }
      if (xReflected) {
        PyClipped[numClippedPoints] -= 2*(PyClipped[numClippedPoints]-Yc);
        QyClipped[numClippedPoints] -= 2*(QyClipped[numClippedPoints]-Yc);
      }
    }
  }

  /* unreflect unclipped points if reflected */

  if (yReflected) {
    Px[i] += 2*(Xc-Px[i]);
    Qx[i] += 2*(Xc-Qx[i]);
  }
  if (xReflected) {
    Py[i] -= 2*(Py[i]-Yc);
    Qy[i] -= 2*(Qy[i]-Yc);
  }
}

/*
 * All the line clipping routines first call ClipInit
 * to calculate the slopes and initialize other variables.
 */

boolean PxDirectlyLeft(int i) {

  ClipInit(i);

  if (Qx[i]<xMin) {
    /* trivially reject */
    return TRUE; 
  } 

  PxNew = xMin;
  PyNew = Py[i] + M*(xMin-Px[i]);

  if ((M>Mtl)|(M<Mbl)) {
    /* reject */
    return TRUE;
  } else if ((M==Mtl)|(M==Mbl)) { 
    if (M == 0) {
      /* on top of top/bottom border */
      if (Qx[i] > xMax) {
        QxNew = xMax;
      } 
      PyNew = Py[i];
    } else {
      /* intersects top/bottom left corner */
      QxNew = xMin;
      if (M>0) {
        PyNew = yMax; QyNew = yMax;
      } else {
        PyNew = yMin; QyNew = yMin;
      }
    }
  } else if ((M<Mtl)&(M>=Mtr)) { 
    /* intersects left and top borders */
    if (Qy[i]>yMax) { 
      QyNew = yMax;
      QxNew = Px[i] + (1/M)*(yMax-Py[i]);
    }
  } else if (M>Mbr) { 
    /* intersects left and right borders */
    if (Qx[i] > xMax) {
      QxNew = xMax;
      QyNew = Py[i] + M*(xMax-Px[i]);
    }
  } else if (M>Mbl) { 
    /* intersects left and bottom borders */
    if (Qy[i]<yMin) { 
      QyNew = yMin;
      QxNew = (int) Px[i] + (1/M)*(yMin-Py[i]);
    }
  } 
  return FALSE;
}

boolean PxDirectlyTop(int i) {

  /* 
   * Cases as shown in figure 6-12, but for
   * the top border.
   */

  if (Qy[i]>=yMax) {
    /* trivially reject */ 
    return TRUE; 
  }
  ClipInit(i);

  PyNew = yMax;
  PxNew = Px[i] - 1/M*(Py[i]-yMax);

  if (Qx[i]<Px[i]) {

    /* cases for when endpoint is to the left of startpoint */
   
    if (M<Mtl) {
      /* reject */
      return TRUE; 
    } else if (M==Mtl) {
      /* intersects left corner */
      PxNew = xMin;
      QxNew = xMin;
      PyNew = yMax;
      QyNew = yMax;
    } else if (M<Mbl) {
      /* intersects top and left borders */
      if (Qx[i] < xMin) {
        QxNew = xMin;
        QyNew = Py[i] - M*(Px[i]-xMin);
      } 
    } else {
      /* intersects top and bottom borders */
      if (Qy[i] < yMin) {
        QyNew = yMin;
        QxNew = Px[i] - 1/M*(Py[i]-yMin);
      }
    }
  } else {

    /* cases for when endpoint is to the right of startpoint (or even with) */

    if (M>Mtr) {
      /* reject */
      return TRUE;
    } else if ((M==Mtr)||(M==Mtl)) {
      if (M==NINF) {
        /* on top of top right/left */
        if (Qy[i]>yMin) {
          Qy[i] = yMin;
        }
      } else {
        /* intersects right corner */
        PxNew = xMax;
        QxNew = xMax;
        PyNew = yMax;
        QyNew = yMax;
      }
    } else if (M > Mbr) {
      /* intersects top and right border */
      if (Qx[i] > xMax) {
        QxNew = xMax;
        QyNew = Py[i] + M*(xMax-Px[i]);
      } 
    } else {
      /* intersects top and bottom (this case also handles the
       * vertical lines that intersect the left/right borders
       */
      if (Qy[i] < yMin) {
        QyNew = yMin;
        QxNew = Px[i] - 1/M*(Py[i]-yMin);
      }
    } 
  } 
  return FALSE;
}

boolean PxTopLeft(int i) {
  typedef enum {right, left, bottom, top} BORDER;
  BORDER firstBorder;
  BORDER secondBorder;
  double MtlTobr, MpTobr;

  ClipInit(i);
  if ((Qx[i]<xMin)||(Qy[i]>yMax)) {
    return TRUE; 
  } 

  /* MtlToBr is the slope from the top left
   * corner of the clipping rect. to the bottom
   * right.  We compare this with the slope from
   * the starting point of the line to the bottom
   * right corner to see which case from figure
   * 6-13 we're dealing with. 
   */

  MtlTobr = slope(yMin-yMax, xMax-xMin);
  MpTobr  = slope(yMin-Py[i], xMax-Px[i]);
  if (MpTobr<MtlTobr) {

    /* case as shown in figure 6-13 b */

    if (M>Mtr) {
      return TRUE; 
    } else if (M>Mbr) {
      firstBorder = top;
      secondBorder = right;
    } else if (M>Mtl) {
      firstBorder = top;
      secondBorder = bottom;
    } else if (M>Mbl) {
      firstBorder = left;
      secondBorder = bottom;
    } else { 
      return TRUE; 
    }
  } else {

    /* case as shown in figure 6-13 a */

    if (M>Mtr) {
      return TRUE; 
    } else if (M>Mtl) {
      firstBorder = top;
      secondBorder = right;
    } else if (M>Mbr) {
      firstBorder = left;
      secondBorder = right;
    } else if (M>Mbl) {
      firstBorder = left;
      secondBorder = bottom;
    } else {
      return TRUE; 
    }
  }

  /* calculate intersections */ 

  switch(firstBorder) {
    case left:
         PxNew = xMin;
         PyNew = Py[i] + M*(xMin-Px[i]);
         break;
    case top:
         PyNew = yMax;
         PxNew = Px[i] - 1/M*(Py[i]-yMax);
         break;
  }
  switch(secondBorder) {
    case right:
         if (Qx[i]>xMax) {
           QxNew = xMax;
           QyNew = Py[i] + M*(xMax-Px[i]);
         }
         break;
    case bottom:
         if (Qy[i]<yMin) {
           QyNew = yMin;
           QxNew = Px[i] - 1/M*(Py[i]-yMin);
         }
         break;
  }
  return FALSE;
}

boolean PxCenter(int i) {
  typedef enum {right, left, bottom, top} BORDER;
  BORDER whichBorder;

  /*
   * Cases from figure 6-11 in book
   */
  
  ClipInit(i);
  if ((Qx[i]>=xMin)&(Qx[i]<=xMax)&(Qy[i]<=yMax)&(Qy[i]>=yMin)) {
    /* trivially accept */
    return FALSE; 
  } 

  if (Qy[i]>=Py[i]) {
    if (M>Mtr) {
      whichBorder = top;
    } else if (M>=0) {
      whichBorder = right;
      if ((M==0)&(Qx[i]<Px[i])) {  
        whichBorder = left;
      }
    } else if (M>=Mtl) {
      whichBorder = left;
    } else {
      whichBorder = top;
    } 
  } else {
    if (M>Mbl) {
      whichBorder = bottom;
    } else if (M>=0) {
      whichBorder = left;
    } else if (M>=Mbr) {
      whichBorder = right;
    } else {
      whichBorder = bottom;
    } 
  }

  /* calculate intersections */

  switch(whichBorder) {
    case left:
         QxNew = xMin;
         QyNew = Py[i] - M*(Px[i]-xMin);
         break;
    case right:
         QxNew = xMax;
         QyNew = Py[i] + M*(xMax-Px[i]);
         break;
    case bottom:
         QyNew = yMin;
         QxNew = Px[i] - 1/M*(Py[i]-yMin);
         break;
    case top:
         QyNew = yMax;
         QxNew = Px[i] + 1/M*(yMax-Py[i]);
         break;
  }
  return FALSE;
}

/*
 * Subroutine to calculate the slope
 * given deltaX and deltaY.
 */

double slope(int deltaY, int deltaX) {
  if (deltaX == 0) {
    if (deltaY < 0) {
      return NINF;
    } else {
      return INF;
    }  
  } else {
    return (double) deltaY/deltaX; 
  }
}

/*
 * Slopes and other initialization for clipping 
 * routines.
 */

void ClipInit(int i) {
  deltaX = Qx[i]-Px[i];
  deltaY = Qy[i]-Py[i];
  M   = slope(deltaY, deltaX); 
  Mtl = slope(yMax-Py[i], xMin-Px[i]); 
  Mbl = slope(yMin-Py[i], xMin-Px[i]); 
  Mtr = slope(yMax-Py[i], xMax-Px[i]); 
  Mbr = slope(yMin-Py[i], xMax-Px[i]); 
  PxNew = Px[i];
  PyNew = Py[i];
  QxNew = Qx[i];
  QyNew = Qy[i];
}

/*
 * MouseMovement takes care of the rubberbanding.
 * This function is called after mouse motion and
 * after window resizing/exposure. 
 *
 * Input: x and y coordinates of mouse
 */

void MouseMovement(int x, int y) {
  if (rubberbanding) {
    if (enteringRect) {
      glClear(GL_COLOR_BUFFER_BIT);
      cleared = TRUE;
      glColor3f(0.0, 0.0, 1.0);
     glBegin(GL_LINE_LOOP);
        glVertex2i(xMin, yMin);
        glVertex2i(xMin, windowHeight-y);
        glVertex2i(x, windowHeight-y);
        glVertex2i(x, yMin);
      glEnd();
      glutPostRedisplay();
    } else if (enteringLine) {
      glClear(GL_COLOR_BUFFER_BIT);
      cleared = TRUE;
      glColor3f(0.0, 1.0, 0.0);
      glBegin(GL_LINES); 
        glVertex2i(Px[numPoints+1], Py[numPoints+1]);
        glVertex2i(x, windowHeight-y);
      glEnd();
      glutPostRedisplay();
    }

    /*
     * We need lastX and lastY in case of an exposure
     * or resize.  The reshape function calls MouseMovement()
     * after a reshape and passes it lastX and lastY so that
     * either the rubberbanding rectangle of line segment can 
     * be redrawn. 
     */

    lastX = x;
    lastY = y;
  }
}
