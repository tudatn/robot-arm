/*
* myrobot.cpp
*
* Class Description: simple robot arms.
* Ref: from example1.cpp (given by TA)
* Last modified on: 14 Mar 2018
* Author: Tu Dat Nguyen
* Comment:
    - Temporarily fixed the menu problem: still able use the mouse middle button
      to access menu
	- General: code needs improving: refactor
    - Refer to Readme for compiling
*/

#include "Angel.h"
#include <iostream>
#include <string.h>

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 0.5, 0.0, 0.5, 1.0 ),  // purple
    // color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;
const GLfloat SPHERE_RADIUS    = 0.25; // 0.25 * 2 = 0.5

// Shader transformation matrices
mat4  model_view;
GLuint ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, TopView = 4, SideView = 5, NumAngles = 3 };
// enum { TopView = 4, SideView = 5 };
int Axis = Base;
int View = SideView; // default view
GLfloat  Theta[NumAngles] = { 0.0 };

// setup for fetching}
// vec4 old_position, new_position;
point4 old_position, new_position, sphere_position;

// Menu option values
const int  Quit = 6;

double tolerance            = 0.0001;          // to check if the sphere is reached
double angle_base_old       = 0.0;
double angle_lower_arm_old  = 0.0;
double angle_upper_arm_old  = 0.0;
double angle_base_new       = 0.0;
double angle_lower_arm_new  = 0.0;
double angle_upper_arm_new  = 0.0;
double dif_base             = 0.0;
double dif_low              = 0.0;
double dif_upper            = 0.0;

int reach_base_old          = false;
int reach_lower_arm_old     = false;
int reach_upper_arm_old     = false;
int reach_base_new          = false;
int reach_lower_arm_new     = false;
int reach_upper_arm_new     = false;
int reach_base_initial      = false;
int reach_lower_arm_initial = false;
int reach_upper_arm_initial = false;

int fetchMode           = false;
int reach_sphere        = false;
int reach_new_position  = false;
//----------------------------------------------------------------------------

int Index = 0;

void quad( int a, int b, int c, int d ) {
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void colorcube() {
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void base() {
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
			BASE_HEIGHT,
			BASE_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void upper_arm() {
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     UPPER_ARM_HEIGHT,
			     UPPER_ARM_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void lower_arm() {
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     LOWER_ARM_HEIGHT,
			     LOWER_ARM_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void sphere() {
    mat4 instance;
    // case: move sphere together with arms
    if (reach_sphere && !reach_new_position)
        instance = Translate( 0.0, UPPER_ARM_HEIGHT + SPHERE_RADIUS, 0.0 );
    else
        instance = Translate( sphere_position ); // case: sphere is released

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );

    glutSolidSphere( SPHERE_RADIUS, 50, 50 );   // how to colorize/ shade sphere -> ask TA for help
}

//----------------------------------------------------------------------------

void display( void ) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // avoid LookAt and glOrtho
    if (View == TopView) {
        // model_view = ( Translate(0, BASE_WIDTH, 0) * RotateX(90.0) );
        
    }
    else
        model_view = ( RotateX(0) );

    // draw sphere first when sphere is not reached or is released
    if ((fetchMode && !reach_sphere) || reach_new_position)
        sphere();
    // Accumulate ModelView Matrix as we traverse the tree
    model_view *= RotateY(Theta[Base] );
    base();

    model_view *= ( Translate(0.0, BASE_HEIGHT, 0.0) *
    RotateZ(Theta[LowerArm]) );
    lower_arm();

    model_view *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
    RotateZ(Theta[UpperArm]) );
    upper_arm();

    // draw sphere after all motions of robot amrs and base
    if (fetchMode && reach_sphere && !reach_new_position)
        sphere();

    glutSwapBuffers();
}

//----------------------------------------------------------------------------
// MOTION
double base_angle      = 0.0;
double lower_arm_angle = 0.0;
double upper_arm_angle = 0.0;
void motion(int) {
    // FETCH SPHERE
    if (!reach_base_old) {
        base_angle  += angle_base_old / 100;
        Theta[Base] += angle_base_old / 100;
    }
    if (!reach_base_old && base_angle > angle_base_old - tolerance && base_angle < angle_base_old + tolerance) {
        reach_base_old = true;
        base_angle     = 0.0;
    }

    if (!reach_lower_arm_old) {
        lower_arm_angle += angle_lower_arm_old / 100;
        Theta[LowerArm]  = lower_arm_angle;
    }
    if (lower_arm_angle > angle_lower_arm_old - tolerance && lower_arm_angle < angle_lower_arm_old + tolerance) {
        reach_lower_arm_old = true;
        lower_arm_angle     = 0.0;
    }

    if (!reach_upper_arm_old) {
        upper_arm_angle += angle_upper_arm_old / 100;
        Theta[UpperArm]  = upper_arm_angle;
    }
    if (upper_arm_angle > angle_upper_arm_old - tolerance && upper_arm_angle < angle_upper_arm_old + tolerance) {
        reach_upper_arm_old = true;
        upper_arm_angle     = 0.0;
    }
    if (reach_base_old && reach_lower_arm_old && reach_upper_arm_old) reach_sphere = true;

    // MOVE TO NEW LOCATION
    if (reach_sphere && !reach_base_new) {
        base_angle  += dif_base / 100;
        Theta[Base] += dif_base / 100;
    }
    //
    if (reach_sphere && base_angle > dif_base - tolerance && base_angle < dif_base + tolerance) {
        reach_base_new = true;
        base_angle     = angle_base_new;
    }
    //
    if (reach_sphere && !reach_lower_arm_new) {
        lower_arm_angle += dif_low / 100;
        Theta[LowerArm] += dif_low / 100;
    }
    //
    if (reach_sphere && lower_arm_angle > dif_low - tolerance && lower_arm_angle < dif_low + tolerance) {
        reach_lower_arm_new = true;
        lower_arm_angle     = angle_lower_arm_new;
    }
    //
    if (reach_sphere && !reach_upper_arm_new) {
        upper_arm_angle += dif_upper / 100;
        Theta[UpperArm] += dif_upper / 100;
    }

    if (reach_sphere && upper_arm_angle > dif_upper - tolerance && upper_arm_angle < dif_upper + tolerance) {
        reach_upper_arm_new = true;
        upper_arm_angle     = angle_upper_arm_new;
    }

    if (reach_base_new && reach_lower_arm_new && reach_upper_arm_new) {
        reach_new_position = true;
        sphere_position    = new_position; // to redraw
    }
    // RETURN TO INITIAL POSITION (ORIGIN)
    if (reach_new_position && !reach_base_initial) {
        base_angle  -= (angle_base_new) / 100;
        Theta[Base] -= (angle_base_new) / 100;
    }
    if (reach_new_position && base_angle > -tolerance && base_angle < tolerance) {
        reach_base_initial = true;
        base_angle         = 0.0;
    }

    if (reach_new_position && !reach_lower_arm_initial) {
        lower_arm_angle -= (angle_lower_arm_new) / 100;
        Theta[LowerArm] -= (angle_lower_arm_new) / 100;
    }
    if (reach_new_position && lower_arm_angle > -tolerance && lower_arm_angle < tolerance) {
        reach_lower_arm_initial = true;
        lower_arm_angle         = 0.0;
    }

    if (reach_new_position && !reach_upper_arm_initial) {
        upper_arm_angle -= (angle_upper_arm_new) / 100;
        Theta[UpperArm] -= (angle_upper_arm_new) / 100;
    }
    if (reach_new_position && upper_arm_angle > -tolerance && upper_arm_angle < tolerance) {
        reach_upper_arm_initial = true;
        upper_arm_angle         = 0.0;
    }
    glutPostRedisplay();
    glutTimerFunc(10.0, motion, 0);
}

//----------------------------------------------------------------------------

void init( void ) {
    colorcube();
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		  NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader81.glsl", "fshader81.glsl" );
    glUseProgram( program );

    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );

    glEnable( GL_DEPTH );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y ) {

    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
    	// Incrase the joint angle
            Theta[Axis] += 5.0;
    	if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
    }

    if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
    	// Decrase the joint angle
            Theta[Axis] -= 5.0;
    	if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void menu( int option ) {
    if ( option == Quit ) {
    	exit( EXIT_SUCCESS );
    }
    else {
        printf("%i\n",option);
        if (option == TopView) {
            View = TopView;
        }
        else if (option == SideView) View = SideView;
        else Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape( int width, int height ) {
    glViewport( 0, 0, width, height );

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5.0, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width)/height;

    if ( aspect > 1.0 ) {
    	left *= aspect;
    	right *= aspect;
    }
    else {
    	bottom /= aspect;
    	top /= aspect;
    }

    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

    model_view = mat4( 1.0 );  // An Identity matrix
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

// calculate triangle angles
double triangle_angle (double a, double b, double c) {
    return acos((pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b)) * 180 / M_PI;
}

// calculate desired angle of the base to reach sphere
double get_angle_base (point4 p) {
    double angle = -atan(p.z / p.x) * 180 / M_PI;
    if (p.x < 0)
        angle -= 180;
    else if (p.z < 0)
        angle -= 360;   // trick to simplify: always rotate in negative degree
    return angle;
}

// calculate desired angle of lower_arm to reach sphere
double get_angle_lower_arm (point4 p) {
    double xz_projection = sqrt(pow(p.x, 2) + pow(p.z, 2)); // distance from origin to projection of sphere on xz
    double xy_projection = sqrt(pow(p.y - BASE_HEIGHT, 2) + pow(xz_projection, 2)); // distance from origin to sphere
    double temp1 = atan(fabs(p.y - BASE_HEIGHT) / xz_projection) * 180 / M_PI;
    double temp2 = triangle_angle(LOWER_ARM_HEIGHT, xy_projection, UPPER_ARM_HEIGHT + SPHERE_RADIUS); // ensure the tip of upper_arm touch the sphere
    if (p.y >= 2)
        return -(90 - temp2 - temp1);
    else
        return -(90 - (temp2 - temp1));
}

// calculate desired angle of upper_arm to reach sphere
double get_angle_upper_arm (point4 p) {
    double xz_projection = sqrt(pow(p.x, 2) + pow(p.z, 2)); // distance from origin to projection of sphere on xz
    double xy_projection = sqrt(pow(p.y - BASE_HEIGHT, 2) + pow(xz_projection, 2)); // distance from origin to sphere
    double temp4 = triangle_angle(UPPER_ARM_HEIGHT + SPHERE_RADIUS, LOWER_ARM_HEIGHT, xy_projection); // ensure the tip of upper_arm touch the sphere
    return -(180 - temp4);
}

// get different rotation angle between old_position and new_position
double get_dif(double angle_old, double angle_new) {
    double dif = fabs(angle_old - angle_new);
    if (fabs(angle_new) > fabs(angle_old)) return -dif;
    else return dif;
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit( &argc, argv );

    // check no. of arguments
    if (argc == 8) {
        fetchMode = true;
        old_position = point4(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1.0);
        new_position = point4(atof(argv[4]), atof(argv[5]), atof(argv[6]), 1.0);
        sphere_position = old_position;
        View = (strcmp(argv[7], "-tv") == 0) ? TopView : SideView;
        // fetch sphere
        angle_base_old      = get_angle_base(old_position);
        angle_lower_arm_old = get_angle_lower_arm(old_position);
        angle_upper_arm_old = get_angle_upper_arm(old_position);

        // move to new position
        angle_base_new      = get_angle_base(new_position);
        angle_lower_arm_new = get_angle_lower_arm(new_position);
        angle_upper_arm_new = get_angle_upper_arm(new_position);
        // move base, lower_arm, and upper_arm
        dif_base  = get_dif(angle_base_old, angle_base_new);
        // move lower_arm
        dif_low   = get_dif(angle_lower_arm_old, angle_lower_arm_new);
        // move upper_arm
        dif_upper = get_dif(angle_upper_arm_old, angle_upper_arm_new);
    }

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    // glutInitContextVersion( 3, 2 );
    // glutInitContextProfile( GLUT_CORE_PROFILE ); --> just to enable the menu: tested at CSIL comp
    glutCreateWindow( "robot" );

    // Iff you get a segmentation error at line 34, please uncomment the line below
    glewExperimental = GL_TRUE;
    glewInit();

    init();
    glutDisplayFunc( display );
    if (fetchMode)
        glutTimerFunc(10.0, motion, 0);

    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );

    glutCreateMenu( menu );
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry( "base", Base );
    glutAddMenuEntry( "lower arm", LowerArm );
    glutAddMenuEntry( "upper arm", UpperArm );
    glutAddMenuEntry( "top view", TopView );
    glutAddMenuEntry( "side view", SideView );
    glutAddMenuEntry( "quit", Quit );
    glutAttachMenu( GLUT_MIDDLE_BUTTON );

    glutMainLoop();
    return 0;
}
