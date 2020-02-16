#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP2                   
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997    
 *  DUE DATE : 10/10            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to 1) understand the 2D
 *             geometric transformations: Translate, Rotate, and Scale 
 *             2) learn how to combine/multiply these transformation
 *             matrices and 3) learn how to derive these matrices ourself.
 *                                                                     
 *  INPUTS   : Mouse/keyboard only. 
 */

/*
 * Constants
 */

#define TRUE 1
#define FALSE 0
#define MIN_VERTICES 3
#define MAX_VERTICES 20 


/*
 * Globals
 */

int windowWidth = 400;
int windowHeight = 400;
int vertexX[MAX_VERTICES];
int vertexY[MAX_VERTICES];
int rotVertexX = 199;
int rotVertexY = 199;
int refVertexX = 199;
int refVertexY = 399;
int whichVertex = 0;
int finishedBuilding = TRUE;
double identity[16];
double workingMatrix[16];
double PI = 3.1415927;

/*
 * Forward Declarations
 */

void Display(void);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void SetCameraPosition(int width, int height);
void beep(void);
void loadIdentity(double a[16]);
void printMatrix(double a[16]);
void multMatrix(double a[16], double b[16]);
void translate(int x, int y);
void scale(double x, double y);
void rotate(double theta);
void reflectY(void);


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
   glutCreateWindow ("mp2");
   glutDisplayFunc(Display); 
   glutMouseFunc(Mouse);
   glutKeyboardFunc(Keyboard);
   glutSpecialFunc(SpecialKeyboard);
   glutReshapeFunc(SetCameraPosition);
   loadIdentity(workingMatrix);
   SetCameraPosition(windowWidth, windowHeight);
   glutMainLoop();
}

/*
 * Keyboard/SpecialKeyboard process input from the keyboard.
 * Inputs:       key, x, y 
 * Outputs:      none
 *
 * Side effects are as follows:
 *
 * "q" and "Q" cause the application to exit. 
 * <up arrow> translates the polygon 10 pixels upward 
 * <down arrow> translates the polygon 10 pixels downward 
 * <left arrow> translates the polygon 10 pixels leftward 
 * <right arrow> translates the polygon 10 pixels rightward 
 * "</," rotates the polygon 15 degrees counterclockwise 
 * ">/>" rotates the polygon 15 degrees clockwise 
 * "[/{" scales along the y axis by -10% 
 * "]/}" scales along the y axis by +10% 
 * "-/_" scales along the x axis by -10% 
 * "=/+" scales along the x axis by +10% 
 * "i/I" resets all tranformations
 * "f/F" reflects the polygon about the line
 *       defined by the reflection and rotation points
 * 
 */

void SpecialKeyboard(int key, int x, int y) {
  if (!finishedBuilding) {
    beep();
    return;
  }
  if (key == GLUT_KEY_UP) {
    translate(0,10);
    glutPostRedisplay();
  } else if (key == GLUT_KEY_DOWN) {
    translate(0,-10);
    glutPostRedisplay();
  } else if (key == GLUT_KEY_LEFT) {
    translate(-10,0);
    glutPostRedisplay();
  } else if (key == GLUT_KEY_RIGHT) {
    translate(10,0);
    glutPostRedisplay();
  }
}

void Keyboard(unsigned char key, int x, int y) {
  double deltaX, deltaY;

  key = toupper(key);
  if (key == 'Q') {
    exit(0);
  }
  if (!finishedBuilding) {
    beep();
    return;
  }

  /*
   * Rotations, scaling, and reflections
   * translate to the origin, perform the
   * transformation, and then translate the
   * polygon back to its original position.
   */

  if (key == 'I') {
    loadIdentity(workingMatrix);
    glutPostRedisplay();
  } else if ((key == '<')|(key == ',')) {
    translate(-rotVertexX, -rotVertexY);
    rotate(15*2*PI/360);
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else if ((key == '>')|(key == '.')) {
    translate(-rotVertexX, -rotVertexY);
    rotate(-15*2*PI/360);
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else if ((key == '-')|(key == '_')) {
    translate(-rotVertexX, -rotVertexY);
    scale(0.9, 1.0);
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else if ((key == '+')|(key == '=')) {
    translate(-rotVertexX, -rotVertexY);
    scale(1.1, 1.0);
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else if ((key == '{')|(key == '[')) {
    translate(-rotVertexX, -rotVertexY);
    scale(1.0, 0.9);
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else if ((key == '}')|(key == ']')) {
    translate(-rotVertexX, -rotVertexY);
    scale(1.0, 1.1);
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else if ((key == 'f')|(key == 'F')) {
    translate(-rotVertexX, -rotVertexY);
    
    /* 
     * find inverse tangent of slope of
     * line so that we can rotate the
     * line of reflection onto the x axis
     */

    deltaY = rotVertexY - refVertexY;
    deltaX = rotVertexX - refVertexX;
    rotate(-atan(deltaY/deltaX));
    reflectY();
    rotate(atan(deltaY/deltaX));
    translate(rotVertexX, rotVertexY);
    glutPostRedisplay();
  } else {
    beep();
  }
}

/*
 * Mouse() is called when there's input from the mouse.
 *
 * Inputs:       which button, what state, x coord, y coord 
 * Outputs:      none
 * Side effects: If a left button was clicked, and a polygon
 *               is not currently under construction, it
 *               assigns this points as the first vertex in
 *               the polygon, sets a flag that a polygon is
 *               being built, and stores this point into the
 *               arrays of vertices.  If a polygon is currently
 *               under construction, it either adds the point
 *               to the list of vertices, or completes the polygon
 *               if the point is within 3 pixels of the starting
 *               vertex and at least <MIN_VERTICES-1> vertices
 *               have already been input for the polygon.
 *
 *               If the middle or right buttons are clicked then
 *               the reflection and rotation points, respectively,
 *               are set to the current mouse position.
 *                
 *              
 */

void Mouse(int button, int state, int x, int y) {
  int deltaX, deltaY;
  int i;

  y = windowHeight - y;
  switch(button) {
    case GLUT_LEFT_BUTTON: 
      switch(state) {
        case GLUT_UP: 
          break;
        case GLUT_DOWN: 
          if (finishedBuilding) {
            finishedBuilding = FALSE;
            whichVertex = 0; 
            loadIdentity(workingMatrix);
          }
          vertexX[whichVertex] = x;
          vertexY[whichVertex] = y;
          whichVertex++;

          if (whichVertex>1) { 

            /*
             * See if this point is close to the
             * first vertex or if we have already 
             * input MAX_VERTICES. 
             */

            deltaX = abs(x - vertexX[0]); 
            deltaY = abs(y - vertexY[0]); 

            if (((deltaX<=3)&(deltaY<=3)&(whichVertex>=MIN_VERTICES))
                |(whichVertex==MAX_VERTICES)) {
              finishedBuilding = TRUE;
              whichVertex--;
            }
          }
          glutPostRedisplay();
          break;
      }  
      break;
    case GLUT_RIGHT_BUTTON: 
      switch(state) {
        case GLUT_DOWN: 
          rotVertexX = x;
          rotVertexY = y; 
          if ((refVertexX == rotVertexX)&(refVertexY == rotVertexY)) {
            refVertexX++;
          } 
          glutPostRedisplay();
          break;
        case GLUT_UP: 
          break;
      }
      break;
    case GLUT_MIDDLE_BUTTON: 
      switch(state) {
        case GLUT_DOWN: 
          refVertexX = x;
          refVertexY = y; 
          if ((refVertexX == rotVertexX)&(refVertexY == rotVertexY)) {
            refVertexX++;
          } 
          glutPostRedisplay();
          break;
        case GLUT_UP: 
          break;
      }
      break;
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
  windowWidth  = width; windowHeight = height;
}


/*
 * Display() is responsible for redrawing the window.
 *
 * Inputs:       none 
 * Outputs:      none
 * Side effects: Clears and redraws the main window. If a polygon is
 *               under construction, we plot the first point with a point
 *               size of three, and then connect the remaining points
 *               with a line strip.  Otherwise, we draw the entire
 *               polygon as defined by the set of vertices.  The polygon,
 *               line strip, and points are drawn in yellow. 
 *
 *               The reflection and rotation points are draws with
 *               a points size of 3 in cyan and magenta, respectively.
 * 
 */

void Display(void)
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0, 1.0, 0.0);
  glPointSize(3.0);
  if (!finishedBuilding) {
    glBegin(GL_POINTS);
      glVertex2i(vertexX[0], vertexY[0]);
    glEnd();
    glBegin(GL_LINE_STRIP);
      for (i=1;i<=whichVertex;i++) {
        glVertex2i(vertexX[i-1], vertexY[i-1]);
      }
    glEnd();
  } else {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(workingMatrix);
    glBegin(GL_POLYGON);
      for (i=1;i<=whichVertex;i++) {
        glVertex2i(vertexX[i-1], vertexY[i-1]);
      }
    glEnd(); 
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPointSize(3.0);
  glColor3f(0.0, 1.0, 1.0);
  glBegin(GL_POINTS);
    glVertex2i(refVertexX, refVertexY);
  glEnd();
  glColor3f(1.0, 0.0, 1.0);
  glBegin(GL_POINTS);
    glVertex2i(rotVertexX, rotVertexY);
  glEnd();
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
 * Translate polygon by multiplying a standard translation
 * matrix (tmp) with the working matrix containing 
 * all transformations up to this points.
 *
 * Input: x and y translations
 * Output: new global matrix <workingMatrix> 
 */

void translate(int x, int y) {
  double tmp[16]; 
  loadIdentity(tmp);
  tmp[12] = (double) x; 
  tmp[13] = (double) y; 
  multMatrix(tmp, workingMatrix);  
}


/*
 * Scale polygon by multiplying a standard scaling 
 * matrix (tmp) with the working matrix containing 
 * all transformations up to this points.
 *
 * Input: amount to scale in x and y directions 
 * Output: new global matrix <workingMatrix> 
 */

void scale(double x, double y) {
  double tmp[16]; 
  loadIdentity(tmp);
  tmp[0] = x; 
  tmp[5] = y; 
  multMatrix(tmp, workingMatrix);  
}


/*
 * Rotate polygon by multiplying a standard rotation 
 * matrix (tmp) with the working matrix containing 
 * all transformations up to this points.
 *
 * Input: degrees to rotate (theta)
 * Output: new global matrix <workingMatrix> 
 */

void rotate(double theta) {
  double tmp[16]; 
  loadIdentity(tmp);
  tmp[0] = cos(theta); tmp[4] = -sin(theta); 
  tmp[1] = sin(theta); tmp[5] =  cos(theta); 
  multMatrix(tmp, workingMatrix);  
}


/*
 * Reflect polygon by multiplying a standard reflection
 * matrix (tmp) with the working matrix containing 
 * all transformations up to this points. This reflects
 * about the x axis by negating the y value.
 *
 * Input: none 
 * Output: new global matrix <workingMatrix> 
 */

void reflectY(void) {
  double tmp[16]; 
  loadIdentity(tmp);
  tmp[5] =  -1.0;
  multMatrix(tmp, workingMatrix);
}


/*
 * Load the identity matrix into matrix a[]
 */

void loadIdentity(double a[16]) {
  int i;
  for (i=0;i<16;i++) {
    a[i] = 0.0;  
  }
  a[0]  = 1.0;  
  a[5]  = 1.0;  
  a[10] = 1.0;  
  a[15] = 1.0;  
}


/*
 * Multiply matrices a[] and b[], storing the
 * result in matrix b[]. 
 */

void multMatrix(double a[16], double b[16]) {
  int i, j, k;
  double sum;
  double tmp[16];

  for (i=0;i<4;i++) { 
    for (j=0;j<4;j++) { 
      sum = 0;
      for (k=0;k<4;k++) { 
        sum += a[k*4+i]*b[j*4+k]; 
      }
      tmp[i+j*4] = sum;
    }
  }
  for (i=0;i<16;i++) { 
    b[i] = tmp[i];
  }
}


/*
 * Print elements in matrix
 */

void printMatrix(double a[16]) {
  int i, j;
  
  for (i=0;i<4;i++) {
    for (j=0;j<4;j++) {
      printf("%5.3lf ", a[i+j*4]);
    }
    printf("\n");
  }
  printf("\n");
  fflush(stdout);
}

