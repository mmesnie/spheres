#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP0                   
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997    
 *  DUE DATE : 9/12            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to learn the basics
 *             of OpenGL by drawing a series of squares for the user,
 *             with subsequent square's vertices located equidistant
 *             along the edges of the first square. 
 *                                                                     
 *  INPUTS   : The number of squares (1-50).
 *                     
 */

/*
 * App. defaults
 */
  
#define MAX_SQUARES 50
#define MIN_SQUARES 1 

/*
 * Globals
 */

int numSquares;

/*
 * display() is responsible for clearing the window and
 * drawing <numSquares> squares.
 *
 * Inputs:       none
 * Outputs:      none
 * Side effects: clears and draws the main window
 */

void display(void)
{
   int i;           /* loop counter for squares */ 
   double x0, y0;   /* vertex 1 */
   double x1, y1;   /* vertex 2 */
   double x2, y2;   /* vertex 3 */
   double x3, y3;   /* vertex 4 */
   double length;   /* length of an edge */
   double delta;    /* change in x/y along edge */ 

   /*
    * Initialize first square's vertices
    */

   x0 =  0.5; y0 =  0.5; 
   x1 = -0.5; y1 =  0.5; 
   x2 = -0.5; y2 = -0.5; 
   x3 =  0.5; y3 = -0.5; 

   /*
    * Calculage length of squares's edge and
    * delta, which is the amount to increment
    * either x or y when calculating the vertices
    * of the next square.
    */

   length = fabs(x0-x1); 
   delta  = length/numSquares;

   /*
    * Clear window
    */

   glClear (GL_COLOR_BUFFER_BIT);

   /*
    * Create numSquares squares
    */

   for (i=0;i<numSquares;i++) {

     /*
      * Draw square
      */ 

     glBegin(GL_LINE_LOOP);
     glVertex2f((x0),(y0)); 
     glVertex2f((x1),(y1)); 
     glVertex2f((x2),(y2)); 
     glVertex2f((x3),(y3)); 
     glEnd();

     /* 
      * Calculate vertices of next square
      */

     x0 -= delta;
     y1 -= delta;
     x2 += delta;
     y3 += delta;
   }

   /*
    * flush all commands 
    */

   glFlush ();
}

/*
 * SetCameraPosition() was given with the assignment. 
 * This procedure is responsible for setting up the
 * OpenGL viewing area. 
 *
 * Inputs:       none
 * Outputs:      none
 * Side effects: sets viewing area 
 */

void SetCameraPosition(void) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-0.55, 0.55, -0.55, 0.55, -0.1, 0.1);
  glMatrixMode(GL_MODELVIEW);
  glColor3f (1.0, 1.0, 1.0);
}


/*
 * main() parses the command line, checks for correct
 * input (i.e., valid number of squares), initializes
 * the main window, and waits for events. 
 *
 * Inputs:       argc, argv 
 * Outputs:      none
 * Side effects: a running program :) 
 */

void main(int argc, char** argv)
{
  int i;

   /* 
    * Parse command line
    */
  
   if (argc == 1) {
     printf("usage: %s <num squares>\n", argv[0]);
     exit(1);
   } else {
     for (i=0;i<strlen(argv[1]);i++) {
       if (!isdigit(argv[1][i])) {
         printf("%s: invalid number of squares\n", argv[0]);
         exit(1);
       }
     }
     numSquares = atoi(argv[1]);
   }

   /*
    * Arg check
    */

   if (numSquares > MAX_SQUARES) {
     printf("%s: %i is the maximum number of squares\n", argv[0], MAX_SQUARES);
     exit(1);
   } 
   if (numSquares < MIN_SQUARES) {
     printf("%s: %i is the minimum number of squares\n", argv[0], MIN_SQUARES);
     exit(1);
   } 

   /* 
    * Initialize things and create main window
    */ 

   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize (300, 300); 
   glutInitWindowPosition (0, 0);
   glutCreateWindow ("mp0");
   SetCameraPosition();
   glutDisplayFunc(display); 

   /*
    * Wait for events
    */

   glutMainLoop();
}
