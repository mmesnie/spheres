#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <string.h>
#include <ctype.h>

/*
 *  NAME     : Mike Mesnier          PROGRAM : MP6                        
 *  NET ID   : mesnier               COURSE  : CS318 - Fall 1997         
 *  DUE DATE : 12/12                                            
 *                                                                     
 *  PURPOSE  : The purpose of this program is to understand the various
 *             illumination models presented in the text and to be able
 *             to implement a basic ray-tracing algorithm.
 *                                                                     
 *  INPUTS   : Keyboard.
 */

/*******************
 * Misc. Constants *
 *******************/

#define BOOLEAN short
#define TRUE 1
#define FALSE 0
#define PI 3.1415927
#define VIEWING_RAY      0
#define REFLECTION_RAY   2
#define TRANSPARENT_RAY 3
#define SPHERE_0 0 
#define SPHERE_1 1
#define CONE     2
#define PLANE    3 
#define EYE     -1


/***************************
 * App. Constants/Defaults *
 ***************************/

/* 
 * Default colors
 */

#define BACKGROUND_COLOR {0.5, 0.5, 0.5}
#define AMBIENT_COLOR    {1.0, 1.0, 1.0} 
#define LIGHT_COLOR      {1.0, 1.0, 1.0}
#define MAX_DEPTH 100 

/*
 * Default location of viewing plane, eye, and light
 */
 
#define THETA -7.0 
#define PHI 2.5 
#define TWIST 5.425 
#define VIEW_RADIUS 75 
#define EYE_RADIUS  85 
#define EYE_CENTER  {0.0, 0.0, EYE_RADIUS}
#define LIGHT_CENTER {2.0, -6.0, 5.0}


/* 
 * Other defaults/constants 
 */

#define AMBIENT_FACTOR     0.4
#define DIFFUSE_FACTOR     0.6
#define SPECULAR_FACTOR    0.5
#define TRANSPARENT_FACTOR 0.0
#define SHINY_FACTOR       100.0

#define WIDTH 100 
#define HEIGHT 100 
#define N_I 1.0
#define N_R 1.5


/*********
 * Types *
 *********/

typedef struct {
  double x; 
  double y;
  double z;
} VectorType;

typedef struct {
  VectorType point;
  VectorType direction;
  VectorType color;
  int        type;
} RayType;


/*********************
 * Globals Variables *
 *********************/

/* 
 * colors 
 */

VectorType backgroundColor = BACKGROUND_COLOR; 
VectorType ambientColor    = AMBIENT_COLOR;
VectorType lightColor      = LIGHT_COLOR;

/* 
 * lighting 
 */

double ambientFactor     = AMBIENT_FACTOR;
double diffuseFactor     = DIFFUSE_FACTOR;
double specularFactor    = SPECULAR_FACTOR;
double transparentFactor = TRANSPARENT_FACTOR;
double shinyFactor       = SHINY_FACTOR;

/* 
 * viewplane and eye
 */

VectorType viewRight;
VectorType viewUp;
VectorType viewCenter;
VectorType eyeCenter;
double theta = THETA;
double phi = PHI;
double eyeRadius = EYE_RADIUS;
double viewRadius = VIEW_RADIUS; 
double twist = TWIST; 

/*
 * other
 */

VectorType lightCenter = LIGHT_CENTER;  /* location of light source */
int windowWidth = WIDTH;                /* width of screen */         
int windowHeight = HEIGHT;              /* height of screen */ 
BOOLEAN clamp = FALSE;
VectorType RED        = {1.0, 0.0, 0.0}; 
VectorType GREEN      = {0.0, 1.0, 0.0}; 
VectorType BLUE       = {0.0, 0.0, 1.0}; 
VectorType YELLOW     = {1.0, 1.0, 0.0}; 
VectorType LIGHT_GREY = {0.75, 0.75, 0.75}; 
VectorType DARK_GREY  = {0.5, 0.5, 0.5}; 
BOOLEAN setAmbient = TRUE;
BOOLEAN setDiffuse = FALSE;
BOOLEAN setSpecular = FALSE;
BOOLEAN setTransparent = FALSE;
BOOLEAN setShiny = FALSE;

/************************
 * Forward Declarations *
 ************************/

void Display(void);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void SetCameraPosition(int width, int height);
void Render(void);
void RayTrace(RayType *ray, int depth, int which);
void Normalize(VectorType *vector);
void PrintVector(VectorType *vector);
void ScalarVectorMult(double scalarIn, VectorType *vectorIn, VectorType *vectorOut);
void ScalarVectorDiv(double scalarIn, VectorType *vectorIn, VectorType *vectorOut);
void ScalarVectorMultAdd(double scalarIn, VectorType *vectorIn, VectorType *vectorOut);
void VectorAdd(VectorType *vectorIn1, VectorType* vectorIn2, VectorType* vectorOut);
void VectorSub(VectorType *vectorIn1, VectorType *vectorIn2, VectorType* vectorOut);
void VectorCopy(VectorType *vectorIn, VectorType* vectorOut);
void VectorMult(VectorType *vectorIn1, VectorType *vectorIn2, VectorType* vectorOut);
double Norm(VectorType *vector);
void CalcRay(int xScreen, int yScreen, RayType *ray);
double DotProduct(VectorType *v1, VectorType *v2);
double CrossProduct(VectorType *v1, VectorType *v2, VectorType *vOut);
BOOLEAN HitSphere(VectorType *Pc, double r, RayType *ray, double *s);
BOOLEAN HitPlane(VectorType *N, double D, RayType *ray, double *s);
BOOLEAN HitCone(VectorType *center, VectorType *length, RayType *ray, double *s);
void ClampNumber(double *Num);
void CalcViewplane();
void RotatePointAboutVector(VectorType *v, VectorType *p, double theta);
void PrintMatrix(double A[3][3]);
void MatrixMultiply(double A[3][3], double B[3][3], double C[3][3]);
void MatrixVectorMultiply(double A[3][3], double B[3], double C[3]);
void usage();

/****************
 * Subroutines  *
 ****************/

/*
 * main() initializes GLUT, creates the main window, 
 * and loops for events. 
 */

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize (windowWidth, windowHeight); 
   glutCreateWindow ("mp6");
   glutDisplayFunc(Display); 
   glutKeyboardFunc(Keyboard);
   glutSpecialFunc(SpecialKeyboard);
   glutReshapeFunc(SetCameraPosition);
   SetCameraPosition(windowWidth, windowHeight);
   usage();
   glutMainLoop();
}

/*
 * Keyboard()/SpecialKeyboard() process input from the keyboard.
 * 
 * Side effects: Terminates application on 'Q' and 'q' 
 *               Rotates viewplane on "arrow" keys
 */

void SpecialKeyboard(int key, int x, int y) {
  BOOLEAN redisplay = FALSE;

  if (key == GLUT_KEY_UP) {
    phi += 0.5;
    redisplay = TRUE;
  } else if (key == GLUT_KEY_DOWN) {
    phi -= 0.5;
    redisplay = TRUE;
  } else if (key == GLUT_KEY_LEFT) {
    theta += 0.5;
    redisplay = TRUE;
  } else if (key == GLUT_KEY_RIGHT) {
    theta -= 0.5;
    redisplay = TRUE;
  }
  if (redisplay) {
    Display();
  }
}

void Keyboard(unsigned char key, int x, int y) {
  BOOLEAN redisplay = FALSE;

  key = toupper(key);
  if (key == 'Q') {
    exit(0);
  }
  if (key == '-') {
    viewRadius+=10;
    eyeRadius+=10;
    redisplay = TRUE;
  } else if (key == '+') {
    viewRadius-=10;
    eyeRadius-=10;
    redisplay = TRUE;
  } else if (key == 'J') {
    twist += 0.2;
    redisplay = TRUE;
  } else if (key == 'K') {
    twist -= 0.2;
    redisplay = TRUE;
  } else if (key == 'R') {
    twist =  TWIST; 
    phi   =  PHI; 
    theta =  THETA; 
    redisplay = TRUE;
  } else if (key == 'A') {
    printf("Set ambient lighting with digit keys.\n"); 
    setAmbient = TRUE;
    setDiffuse = FALSE;
    setSpecular = FALSE;
    setShiny = FALSE;
    setTransparent = FALSE;
  } else if (key == 'D') {
    printf("Set diffuse lighting with digit keys.\n"); 
    setAmbient = FALSE;
    setDiffuse = TRUE;
    setSpecular = FALSE;
    setShiny = FALSE;
    setTransparent = FALSE;
  } else if (key == 'T') {
    printf("Set transparent lighting with digit keys.\n"); 
    setAmbient = FALSE;
    setDiffuse = FALSE;
    setSpecular = FALSE;
    setShiny = FALSE;
    setTransparent = TRUE;
  } else if (key == 'S') {
    printf("Set specular lighting with digit keys.\n"); 
    setAmbient = FALSE;
    setDiffuse = FALSE;
    setSpecular = TRUE;
    setShiny = FALSE;
    setTransparent = FALSE;
  } else if (key == 'H') {
    printf("Set shiny lighting with digit keys.\n"); 
    setShiny = TRUE;
    setAmbient = FALSE;
    setDiffuse = FALSE;
    setSpecular = FALSE;
    setTransparent = FALSE;
  } else if (key == 'C') {
    clamp = TRUE;
    redisplay = TRUE;
  } else if (key == 'N') {
    clamp = FALSE;
    redisplay = TRUE;
  } else if (isdigit(key)) {
    if (setAmbient) {
      ambientFactor = (key - '0')/10.0;
    } else if (setDiffuse) {
      diffuseFactor = (key - '0')/10.0;
    } else if (setSpecular) {
      specularFactor = (key - '0')/10.0;
    } else if (setTransparent) {
      transparentFactor = (key - '0')/10.0;
    } else if (setShiny) {
      shinyFactor = (key - '0')*100.0;
    }
    redisplay = TRUE;
  } else {
  }
  if (redisplay) {
    Display();
  }
}

/*
 * SetCameraPosition() sets up the viewing area for 
 * the window. 
 */

void SetCameraPosition(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble) width, (GLdouble) height, 0.0);
  windowWidth  = width;
  windowHeight = height;
}

/*
 * Display() is responsible for redrawing the window.  
 */

void Display(void)
{
  glClear(GL_COLOR_BUFFER_BIT); /* clear window */ 
  CalcViewplane();              /* calculate location/direction of view plane */
  Render();                     /* render scene */
  glFlush();                    /* flush all commands */
}

/*
 * Render() calls RayTrace for each ray in the viewing plane.  Rays are
 * determined by the direction from the location of the eye to the location 
 * of each pixel in the view plane.  RayTrace() * returns the color of the
 * ray (i.e., color of pixel in the view plane) in the field ray.color which 
 * is a VectorType (3-tuple for RGB) field in the RayType structure. 
 */

void Render() {
  int xScreen, yScreen;
  RayType ray;

  for (yScreen=0;yScreen<windowHeight;yScreen++) {
    for (xScreen=0;xScreen<windowWidth;xScreen++) {
      CalcRay(xScreen, yScreen, &ray);
      ray.type = VIEWING_RAY;
      RayTrace(&ray, 0, EYE); 

      /* clamp stuff */

      if (ray.color.x > 1.0) {
        ray.color.x = 1.0;
      }
      if (ray.color.y > 1.0) {
        ray.color.y = 1.0;
      }
      if (ray.color.z > 1.0) {
        ray.color.z = 1.0;
      }
      if (clamp) {
        ClampNumber(&ray.color.x);
        ClampNumber(&ray.color.y);
        ClampNumber(&ray.color.z);
      }
      glColor3f(ray.color.x, ray.color.y, ray.color.z); 
      glBegin(GL_POINTS);
      glVertex2i(xScreen,yScreen);
      glEnd();
    }
  }
} 

void RayTrace(RayType *ray, int depth, int fromObject) {
  VectorType Pc0  = { 2.5, 0.0, 1.0};    /* center of sphere 0 */
  VectorType Pc1  = { 0.0, 0.0, 1.0};    /* center of sphere 1 */
  VectorType Pc2  = {-2.5, 0.0, 1.0};    /* center of sphere 2 */
  VectorType N    = { 0.0, 0.0, 1.0};    /* normal to bullseye */
  VectorType N2   = { 0.0, 0.0, -1.0};   /* normal to bullseye */
  VectorType L    = { 1.0, 1.0, 2.0};    /* axis lengths of cone */
  VectorType Cone = {-2.5, 0.0, 2.0};    /* center of cone */

  RayType R;                             /* reflection ray */ 
  RayType V;                             /* viewing ray */ 
  RayType T;                             /* transmission ray */ 
  RayType S;                             /* shadow ray */ 
  VectorType intersectionPoint;          /* object intersection */ 
  VectorType exitPoint;                  /* exit point on cone */ 
  VectorType entryDirection;             /* direction to cone entry */ 
  VectorType lightDirection;             /* direction to light */ 
  VectorType viewerDirection;            /* direction to viewer */ 

  double s[9];            
  BOOLEAN hitObject[9];
  BOOLEAN hitSomething = FALSE;
  BOOLEAN reflective = FALSE;
  BOOLEAN transparent = FALSE;
  BOOLEAN totalReflection = FALSE;
  int i, min;
  double radius = 1.0;
  VectorType objectColor, diffuseLight, ambientLight, specularLight;
  VectorType H;
  double theta_i, theta_r;
  BOOLEAN inShadow = FALSE;


  /* 
   * determine which objects we hit
   */

  hitObject[0] = HitSphere(&Pc0, 1.0, ray, &s[0]);
  hitObject[1] = HitSphere(&Pc1, 1.0, ray, &s[1]);
  hitObject[2] = HitCone(&Cone, &L, ray, &s[2]);
  hitObject[3] = HitPlane(&N, 0.0, ray, &s[3]);
  if (!hitObject[3]) {
    hitObject[3] = HitPlane(&N2, 0.0, ray, &s[3]);
  }

  /* 
   * select closest object
   */

  hitSomething = FALSE;
  for (i=0;i<4;i++) {
    if (hitObject[i]) {
      if (hitSomething) {
        if (s[i]<s[min]) {
          min = i;
        } 
      } else {
        min = i;
        hitSomething = TRUE;
      }
    }
  } 

  if (hitSomething) {

    /* point of intersection */ 

    VectorCopy(&ray->point, &intersectionPoint);
    ScalarVectorMultAdd(s[min], &ray->direction, &intersectionPoint);

    /* direction vector back to viewer */

    VectorSub(&ray->point, &intersectionPoint, &viewerDirection);
    Normalize(&viewerDirection);

    /* direction to light */

    VectorSub(&lightCenter, &intersectionPoint, &lightDirection);
    Normalize(&lightDirection);
  
    switch(min) {
      case SPHERE_0:
        VectorCopy(&LIGHT_GREY, &objectColor);
        objectColor.x += .10;
        VectorSub(&intersectionPoint, &Pc0, &N);
        ScalarVectorDiv(radius, &N, &N); 
        if (depth<MAX_DEPTH) {  
          VectorCopy(&intersectionPoint, &R.point);
          ScalarVectorMult(2.0*DotProduct(&N,&viewerDirection),&N, &R.direction);
          VectorSub(&R.direction, &viewerDirection, &R.direction);
          Normalize(&R.direction); 
          reflective = TRUE;
        }
        break;
      case SPHERE_1: 
        VectorCopy(&LIGHT_GREY, &objectColor);
        objectColor.y += .10;
        VectorSub(&intersectionPoint, &Pc1, &N);
        ScalarVectorDiv(radius, &N, &N); 
        if (depth<MAX_DEPTH) {  
          VectorCopy(&intersectionPoint, &R.point);
          ScalarVectorMult(2.0*DotProduct(&N,&viewerDirection),&N, &R.direction);
          VectorSub(&R.direction, &viewerDirection, &R.direction);
          Normalize(&R.direction); 
          reflective = TRUE;
        }
        break;
      case CONE:
        VectorCopy(&BLUE, &objectColor);
        VectorSub(&intersectionPoint, &Cone, &N);
        N.x *=  2.0*pow(L.y,2)*pow(L.z,2);
        N.y *=  2.0*pow(L.x,2)*pow(L.z,2);
        N.z *= -2.0*pow(L.x,2)*pow(L.y,2);
        Normalize(&N);
        if (depth<MAX_DEPTH) {  
          if (fromObject==CONE) {
            if ((1+pow(N_R/N_I,2)*(pow(DotProduct(&viewerDirection, &N),2)-1))<0) {
              totalReflection = TRUE; 
              /* TIR - so do nothing */
            }
          } else {
            if ((1+pow(N_I/N_R,2)*(pow(DotProduct(&viewerDirection, &N),2)-1))<0) {
              totalReflection = TRUE; 
              /* TIR - so do nothing */
            }
          }
          if (!totalReflection) {
            VectorCopy(&intersectionPoint, &T.point);
            theta_i = acos(DotProduct(&N, &viewerDirection));
            theta_r = asin(N_I/N_R*sin(theta_i));
            theta_r = asin(N_I/N_R*sin(theta_i));
            ScalarVectorMult((N_I/N_R)*cos(theta_i)-cos(theta_r),&N,&T.direction);
            ScalarVectorMult(N_I/N_R, &viewerDirection, &viewerDirection);
            Normalize(&viewerDirection);
            VectorSub(&T.direction, &viewerDirection, &T.direction);
            Normalize(&T.direction);
            transparent = TRUE;
          }
        }
        break;
      case PLANE:
        VectorCopy(&ray->color, &objectColor);
        break;
    }

    /* start with ambient light */

    VectorMult(&objectColor, &ambientColor, &ray->color);
    ScalarVectorMult(ambientFactor, &ray->color, &ray->color);

    /* shadow test */

    VectorCopy(&intersectionPoint, &S.point);
    VectorCopy(&lightDirection, &S.direction);
    for (i=1;i<4;i++) {
      switch ((i+min)%4) {
        case 0:
          inShadow = HitSphere(&Pc0, 1.0, &S, &s[0]);
          break;
        case 1:
          inShadow = HitSphere(&Pc1, 1.0, &S, &s[1]);
          break;
        case 2:
          inShadow = HitCone(&Cone, &L, &S, &s[2]);
          break;
      }
      if (inShadow) {
        break;
      } 
    }

    if (!inShadow) {

      /* add diffuse light */

      if (DotProduct(&N, &lightDirection)>0) { 
        VectorMult(&objectColor, &lightColor, &diffuseLight);
        ScalarVectorMult(DotProduct(&N, &lightDirection), &diffuseLight, &diffuseLight);
        ScalarVectorMult(diffuseFactor, &diffuseLight, &diffuseLight);
        VectorAdd(&ray->color, &diffuseLight, &ray->color); 
      }

      /* add specular light last for glassy look ;) */

      if (DotProduct(&N, &lightDirection)>0) { 
        VectorAdd(&viewerDirection, &lightDirection, &H); 
        ScalarVectorDiv(Norm(&H), &H, &H);
        ScalarVectorMult(pow(DotProduct(&N,&H),shinyFactor), &lightColor, &specularLight);
        ScalarVectorMult(specularFactor, &specularLight, &specularLight);
        VectorAdd(&ray->color, &specularLight, &ray->color); 
      }
    }

    /* recurse if reflective or transparent */

    if (reflective) {
      R.type = REFLECTION_RAY;
      RayTrace(&R,depth+1, min);
      ScalarVectorMult(specularFactor, &R.color, &R.color);
      ScalarVectorMult(1.0-specularFactor, &ray->color, &ray->color);
      VectorAdd(&R.color, &ray->color, &ray->color);
    } else if (transparent) {
      T.type = TRANSPARENT_RAY; 
      RayTrace(&T,depth+1, CONE);
      ScalarVectorMult(transparentFactor, &T.color, &T.color);
      ScalarVectorMult(1.0-transparentFactor, &ray->color, &ray->color);
      VectorAdd(&T.color, &ray->color, &ray->color);
    }

  } else {
    VectorCopy(&backgroundColor, &ray->color);
  }
}


/*******************************************
 * Intersection checks for various objects * 
 *******************************************/

BOOLEAN HitSphere(VectorType *Pc, double r, RayType *ray, double *s) {
  VectorType deltaP;
  double dotProduct;
  double discriminant;
  double s1, s2;

  VectorSub(Pc, &ray->point, &deltaP);
  dotProduct = DotProduct(&ray->direction, &deltaP);
  discriminant = pow(dotProduct,2) - pow(Norm(&deltaP),2) + r*r;

  if (discriminant < 0) {
    return FALSE;
  } else {
    s1 = dotProduct + sqrt(discriminant); 
    s2 = dotProduct - sqrt(discriminant); 
    if (s1 < s2) {
      *s = s1;
    } else {
      *s = s2;
    } 
    if (*s<=0.00001) {
      return FALSE;
    } else {
      return TRUE;
    }
  }
}

BOOLEAN HitPlane(VectorType *N, double D, RayType *ray, double *s) {
  double value;
  VectorType intersection; 
  int which;
  double radius;

  if (DotProduct(&ray->direction,N)>=0) {
    return FALSE;
  } 
  value = -(D + DotProduct(N,&ray->point))/DotProduct(N,&ray->direction);
  if (value > 0) {
    *s = value;
    ScalarVectorMult(*s, &ray->direction, &intersection);
    VectorAdd(&intersection, &ray->point, &intersection);
    radius = sqrt(pow(intersection.x,2)+pow(intersection.y,2));
    if (radius <= 4.0) {
      which = radius/0.5;
      if (which%2==0) {
        VectorCopy(&YELLOW, &ray->color);
      } else {
        VectorCopy(&BLUE, &ray->color);
      }
      return TRUE;
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
}

BOOLEAN HitCone(VectorType *center, VectorType *length, RayType *ray, double *s) {
  double h, j, k;
  double a, b, c;
  double x0, y0, z0;
  double ux, uy, uz;
  double discriminant;
  double A, B, C;
  double s1, s2;
  double z;
  BOOLEAN s1good, s2good; 

  a = center->x; b = center->y; c = center->z;
  h = length->x; j = length->y; k = length->z;
  x0 = ray->point.x; 
  y0 = ray->point.y;
  z0 = ray->point.z;
  ux = ray->direction.x; 
  uy = ray->direction.y;
  uz = ray->direction.z;

  A = j*j*k*k*ux*ux + h*h*k*k*uy*uy - h*h*j*j*uz*uz;
  B = 2.0*j*j*k*k*ux*(x0-a) + 2.0*h*h*k*k*uy*(y0-b) - 2.0*h*h*j*j*uz*(z0-c); 
  C = j*j*k*k*(x0-a)*(x0-a) + h*h*k*k*(y0-b)*(y0-b) - h*h*j*j*(z0-c)*(z0-c);

  discriminant = B*B - 4.0*A*C;
  if (discriminant < 0) {
    return FALSE;
  } else {
    s1 = (-B + sqrt(discriminant))/(2.0*A);
    z  = s1*uz + z0;
    if ((z>=0)&&(z<=c)) {
      s1good = TRUE;
    } else {
      s1good = FALSE;
    }
    s2 = (-B - sqrt(discriminant))/(2.0*A);
    z  = s2*uz + z0;
    if ((z>=0)&&(z<=c)) {
      s2good = TRUE;
    } else {
      s2good = FALSE;
    }
    if (s1good&&s2good) {
      if (ray->type == TRANSPARENT_RAY) {
        if (s1 > s2) {
          *s = s1;
        } else {
          *s = s2;
        }
      } else {
        if (s1 < s2) {
          *s = s1;
        } else {
          *s = s2;
        }
      }
    } else if (s1good) {
      *s = s1;
    } else if (s2good) {
      *s = s2;
    } else {
      return FALSE;
    }
    if (*s<=0.000001) {
      return FALSE; 
    } else {
      return TRUE; 
    }
  }
}

/*
 * CalcRay() calculates the ray from the eye to screen position
 * (xScreen,yScreen). 
 */

void CalcRay(int xScreen, int yScreen, RayType *ray) {
  double xUnit, yUnit;
  VectorType pixelCoord;
 
  /* transform screen coords to unit square */
 
  xUnit = (double) xScreen/windowWidth - 0.5;
  yUnit = 0.5 - (double) yScreen/windowHeight;
 
  /* transform unit coords 3D world coords  */
 
  ScalarVectorMult(xUnit, &viewRight, &pixelCoord);
  ScalarVectorMultAdd(yUnit, &viewUp, &pixelCoord);
  VectorAdd(&viewCenter, &pixelCoord, &pixelCoord);
 
  /* calculate ray passing through reference point and pixel */
 
  VectorSub(&pixelCoord, &eyeCenter, &ray->direction);
  Normalize(&ray->direction);
 
  /* set starting point to reference point */
 
  VectorCopy(&eyeCenter, &ray->point);
}

/*****************************
 * Various Vector Operations *
 *****************************/

void Normalize(VectorType *vector) {
  double magnitude;

  magnitude = Norm(vector);
  if (magnitude == 0.0) {
    printf("Fatal Error: magnitude = 0!!!\n");
    exit(1);
  }
  vector->x = vector->x/magnitude;
  vector->y = vector->y/magnitude;
  vector->z = vector->z/magnitude;
}
double Norm(VectorType *vector) {
  return sqrt(pow(vector->x,2)+pow(vector->y,2)+pow(vector->z,2));
}
void ScalarVectorMult(double scalarIn, VectorType *vectorIn, VectorType *vectorOut) {
  vectorOut->x = scalarIn*vectorIn->x;
  vectorOut->y = scalarIn*vectorIn->y;
  vectorOut->z = scalarIn*vectorIn->z;
}
void ScalarVectorDiv(double scalarIn, VectorType *vectorIn, VectorType *vectorOut) {
  if (scalarIn == 0.0) {
    printf("Fatal Error: division by 0 in ScalarVectorDiv!!!\n");
    exit(1);
  }
  vectorOut->x = vectorIn->x/scalarIn;
  vectorOut->y = vectorIn->y/scalarIn;
  vectorOut->z = vectorIn->z/scalarIn;
}
void ScalarVectorMultAdd(double scalarIn, VectorType *vectorIn, VectorType *vectorOut) {
  vectorOut->x += scalarIn*vectorIn->x;
  vectorOut->y += scalarIn*vectorIn->y;
  vectorOut->z += scalarIn*vectorIn->z;
}
void VectorAdd(VectorType *vectorIn1, VectorType *vectorIn2, VectorType* vectorOut) {
  vectorOut->x = vectorIn1->x + vectorIn2->x;
  vectorOut->y = vectorIn1->y + vectorIn2->y;
  vectorOut->z = vectorIn1->z + vectorIn2->z;
}
void VectorSub(VectorType *vectorIn1, VectorType *vectorIn2, VectorType* vectorOut) {
  vectorOut->x = vectorIn1->x - vectorIn2->x;
  vectorOut->y = vectorIn1->y - vectorIn2->y;
  vectorOut->z = vectorIn1->z - vectorIn2->z;
}
void VectorMult(VectorType *vectorIn1, VectorType *vectorIn2, VectorType* vectorOut) {
  vectorOut->x = vectorIn1->x * vectorIn2->x;
  vectorOut->y = vectorIn1->y * vectorIn2->y;
  vectorOut->z = vectorIn1->z * vectorIn2->z;
}
void VectorCopy(VectorType *vectorIn, VectorType* vectorOut) {
  vectorOut->x = vectorIn->x;
  vectorOut->y = vectorIn->y;
  vectorOut->z = vectorIn->z;
}
double DotProduct(VectorType *v1, VectorType *v2) {
  return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}
double CrossProduct(VectorType *v1, VectorType *v2, VectorType *vOut) {
  vOut->x = v1->y*v2->z - v1->z*v2->y;
  vOut->y = v1->z*v2->x - v1->x*v2->z;
  vOut->z = v1->x*v2->y - v1->y*v2->x;
}
void PrintVector(VectorType *vector) {
  printf("(%lf,%lf,%lf)\n", vector->x, vector->y, vector->z);
}

/*******************
 * Misc. Functions *
 *******************/

void ClampNumber(double *Num) {
  double TimesTen;
  
  TimesTen = *Num*10.0;
  *Num -= (TimesTen = (int)TimesTen)/10.0;
  if (*Num>1.0) {
    *Num = 1.0;
  }
}      

/*
 * The eye and the center of the viewplane are represented
 * with spherical coordinates.  CalcViewplane calculates the
 * (x,y,z) values of each based on the values of theta, phi
 * and the radius.  The viewUp and viewRight vectors are then
 * calculated to be orthogonal with the viewing vector.
 */

void CalcViewplane() { 
  VectorType rayToOrigin;

  /* calculate center of view plane via spherical coords. */
 
  viewCenter.x = viewRadius*sin(theta)*sin(phi);
  viewCenter.z = viewRadius*cos(theta)*sin(phi);
  viewCenter.y = viewRadius*cos(phi);
 
  /* calculate center of eye via spherical coords. */
 
  eyeCenter.x  = eyeRadius*sin(theta)*sin(phi);
  eyeCenter.z  = eyeRadius*cos(theta)*sin(phi);
  eyeCenter.y  = eyeRadius*cos(phi);
 
  /* make viewUp orthogonal to viewPlane */
 
  viewUp.x     = viewRadius*sin(theta)*sin(phi-PI/2.0);
  viewUp.z     = viewRadius*cos(theta)*sin(phi-PI/2.0);
  viewUp.y     = viewRadius*cos(phi-PI/2.0); Normalize(&viewUp);

  /* viewUp */
 
  RotatePointAboutVector(&viewCenter, &viewUp, twist);
 
  /* make viewRight orthogonal to both viewPlane & viewUp */

  CrossProduct(&viewUp, &viewCenter, &viewRight); Normalize(&viewRight);
}


/*
 * various matrix operations
 */  

void RotatePointAboutVector(VectorType *v, VectorType *p, double theta) {
  double Rx[3][3];
  double Rxi[3][3];
  double Ry[3][3];
  double Ryi[3][3];
  double Rz[3][3];
  double a,b,c,d;
  double point[3]; 
  double pointNew1[3]; 
  double pointNew2[3]; 
  VectorType u;

  VectorCopy(v, &u);
  Normalize(&u);
  a = u.x; 
  b = u.y;
  c = u.z;
  d = sqrt(b*b+c*c); 

  if (d!=0.0) {
    Rx[0][0] = 1.0; Rx[0][1] = 0.0; Rx[0][2] =  0.0;
    Rx[1][0] = 0.0; Rx[1][1] = c/d; Rx[1][2] = -b/d;
    Rx[2][0] = 0.0; Rx[2][1] = b/d; Rx[2][2] = c/d;

    Rxi[0][0] = 1.0; Rxi[0][1] =  0.0; Rxi[0][2] =  0.0;
    Rxi[1][0] = 0.0; Rxi[1][1] =  c/d; Rxi[1][2] =  b/d;
    Rxi[2][0] = 0.0; Rxi[2][1] = -b/d; Rxi[2][2] = c/d;
  } else {
    printf("already on zx plane!!!!\n");
    d = c;
  }

  Ry[0][0] = d;   Ry[0][1] = 0.0; Ry[0][2] = -a;
  Ry[1][0] = 0.0; Ry[1][1] = 1.0; Ry[1][2] =  0.0;
  Ry[2][0] = a;   Ry[2][1] = 0.0; Ry[2][2] =  d;

  Ryi[0][0] =  d;   Ryi[0][1] = 0.0; Ryi[0][2] = a;
  Ryi[1][0] =  0.0; Ryi[1][1] = 1.0; Ryi[1][2] = 0.0;
  Ryi[2][0] = -a;   Ryi[2][1] = 0.0; Ryi[2][2] = d;

  Rz[0][0] = cos(theta); Rz[0][1] = -sin(theta); Rz[0][2] = 0.0;
  Rz[1][0] = sin(theta); Rz[1][1] =  cos(theta); Rz[1][2] = 0.0;
  Rz[2][0] = 0.0;        Rz[2][1] =  0.0;        Rz[2][2] = 1.0;

  point[0] = p->x; point[1] = p->y; point[2] = p->z;
  MatrixVectorMultiply(Rx, point, pointNew1); 
  MatrixVectorMultiply(Ry, pointNew1, pointNew2); 
  MatrixVectorMultiply(Rz, pointNew2, pointNew1); 
  MatrixVectorMultiply(Ryi, pointNew1, pointNew2); 
  MatrixVectorMultiply(Rxi, pointNew2, pointNew1); 
  p->x = pointNew1[0]; p->y = pointNew1[1]; p->z = pointNew1[2];
}

void MatrixMultiply(double A[3][3], double B[3][3], double C[3][3]) {
  int i, j, k;
  double sum;

  for (i=0;i<3;i++) {
    for (j=0;j<3;j++) {
      sum = 0.0;
      for (k=0;k<3;k++) {
        sum += A[i][k]*B[k][j];
      }
      C[i][j] = sum;
    }
  }
}

void MatrixVectorMultiply(double A[3][3], double B[3], double C[3]) {
  int i,j,k;
  double sum;

  for (i=0;i<3;i++) {
    sum = 0.0;
    for (k=0;k<3;k++) {
      sum += A[i][k]*B[k];
    }
    C[i] = sum;
  }
}

void PrintMatrix(double A[3][3]) {
  int i, j;

  printf("\n");
  for (i=0;i<3;i++) {
    for (j=0;j<3;j++) {
      printf(" %1.5lf", A[i][j]);
    }
    printf("\n");
  }
}

void usage() {
  printf("\n\n");
  printf("***********************************************\n");
  printf("* Arrow keys rotate the viewplane.            *\n");
  printf("* Keys \"j\" and \"k\" twist the viewplane.       *\n");
  printf("* \"+\" to zoom in, \"-\" to zoom out.            *\n");
  printf("* \"q\" to quit.                                *\n");
  printf("* \"a\" set ambient lighting w/ digit keys      *\n");
  printf("* \"d\" set diffuse lighting w/ digit keys      *\n");
  printf("* \"s\" set specular lighting w/ digit keys.    *\n");
  printf("* \"t\" set transparent lighting w/ digit keys. *\n");
  printf("* \"h\" set shiny factor  w/ digit keys.        *\n");
  printf("* \"r\" reset viewplane to original position.   *\n");
  printf("***********************************************\n");
  printf("\n\n");
}
