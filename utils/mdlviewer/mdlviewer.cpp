/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
****/
// updates:
// 1-4-98	fixed initialization

#include <stdio.h>

#include <windows.h>

#include <gl\gl.h>
#include <gl\glu.h>
//#include <gl\glut.h>
#include <SDL2/SDL.h>

#include "mathlib.h"
#include "../../public/steam/steamtypes.h" // defines int32, required by studio.h
#include "..\..\engine\studio.h"
#include "mdlviewer.h"


#pragma warning( disable : 4244 ) // conversion from 'double ' to 'float ', possible loss of data
#pragma warning( disable : 4305 ) // truncation from 'const double ' to 'float '

SDL_Window*		sdlWin;
SDL_GLContext	sdlCtx;
bool			running;
bool			redraw;


vec3_t		g_vright;		// needs to be set to viewer's right in order for chrome to work
float		g_lambert = 1.5;

float		gldepthmin = 0;
float		gldepthmax = 10.0;


/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	glDepthFunc (GL_LEQUAL);
	glDepthRange (gldepthmin, gldepthmax);
	glDepthMask( 1 );
}

static StudioModel tempmodel;

void mdlviewer_display( )
{
	R_Clear( );

	tempmodel.SetBlending( 0, 0.0 );
	tempmodel.SetBlending( 1, 0.0 );

	static float prev;
	float curr = GetTickCount( ) / 1000.0;
	tempmodel.AdvanceFrame( curr - prev );
	prev = curr;

	tempmodel.DrawModel( );
}


int mdlviewer_init( char *modelname )
{
	// make a bogus texture
	// R_InitTexture( );

	if ( tempmodel.Init( modelname ) < 0 )
	{
		return -1;
	}

	tempmodel.SetSequence( 0 );

	tempmodel.SetController( 0, 0.0 );
	tempmodel.SetController( 1, 0.0 );
	tempmodel.SetController( 2, 0.0 );
	tempmodel.SetController( 3, 0.0 );
	tempmodel.SetMouth( 0 );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.,1.,.1,10.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor( 0, 0, 0.5, 0 );
}


void mdlviewer_nextsequence( void )
{
	int iSeq = tempmodel.GetSequence( );
	if (iSeq == tempmodel.SetSequence( iSeq + 1 ))
	{
		tempmodel.SetSequence( 0 );
	}
}


//////////////////////////////////////////////////////////////////


static int pstyle;
static int translate = 1;
static int mesh = 1;
static float transx = 0, transy = 0, transz = -2, rotx=235, roty=-90;
static float amplitude = 0.03;
static float freq = 5.0f;
static float phase = .00003;
static int ox = -1, oy = -1;
static int show_t = 1;
static int mot;
#define PAN	1
#define ROT	2
#define ZOOM 3

void pan(int x, int y) 
{
    transx +=  (x-ox)/500.;
    transy -= (y-oy)/500.;
    ox = x; oy = y;
    //glutPostRedisplay();
	redraw = true;

}

void zoom(int x, int y) 
{
    transz +=  (x-ox)/20.;
    ox = x;
    //glutPostRedisplay();
	redraw = true;
}

void rotate(int x, int y) 
{
    rotx += x-ox;
    if (rotx > 360.) rotx -= 360.;
    else if (rotx < -360.) rotx += 360.;
    roty += y-oy;
    if (roty > 360.) roty -= 360.;
    else if (roty < -360.) roty += 360.;
    ox = x; oy = y;
    //glutPostRedisplay();
	redraw = true;
}

void motion(int x, int y) 
{
    if (mot == PAN) 
		pan(x, y);
    else if (mot == ROT) 
		rotate(x,y);
	else if ( mot == ZOOM )
		zoom( x, y );
}

void mouse(int button, int state, int x, int y) 
{
    if(state == SDL_PRESSED) {
	switch(button) {
	case SDL_BUTTON_LEFT:
	    mot = PAN;
	    motion(ox = x, oy = y);
	    break;
	case SDL_BUTTON_RIGHT:
		mot = ROT;
	    motion(ox = x, oy = y);
	    break;
	case SDL_BUTTON_MIDDLE:
	    break;
	}
    } else if (state == SDL_RELEASED) {
	mot = 0;
    }
}

void help(void) 
{
    printf("left mouse     - pan\n");
    printf("right mouse    - rotate\n");
}

int init( char *arg ) 
{
	if ( mdlviewer_init( arg ) < 0 )
	{
		return -1;
	}

    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.,1.,.1,10.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // glTranslatef(0.,0.,-5.5);
    // glTranslatef(-.2.,1.0,-1.5);

    glClearColor( 0, 0, 0.5, 0 );

	return 0;
}

void display(void) 
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix();

    glTranslatef(transx, transy, transz);
    
	glRotatef(rotx, 0., 1., 0.);
    glRotatef(roty, 1., 0., 0.);

    glScalef( 0.01, 0.01, 0.01 );
	glCullFace( GL_FRONT );
	glEnable( GL_DEPTH_TEST );

	mdlviewer_display( );

    glPopMatrix();
    //glutSwapBuffers();
	SDL_GL_SwapWindow ( sdlWin );

    //glutPostRedisplay();
}

void reshape(int w, int h) 
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (double)w / (double)h, 0.1, 10.0);
	glMatrixMode(GL_MODELVIEW);

	redraw = true;
}

/*ARGSUSED1*/
void key(unsigned char key) 
{
    switch(key) 
	{
		case 'h': 
			help(); 
		break;

		case 'p':
			printf("Translation: %f, %f %f\n", transx, transy, transz );
		break;

		case '\033':	// Escape
			running = false;
		break;

		case ' ':
			mdlviewer_nextsequence( );
		break;

		default: 
		break;
    }
    //glutPostRedisplay();
	redraw = true;
}

int main(int argc, char** argv) 
{
	if (argc != 2)
	{
		printf("usage : %s <filename>\n", argv[0] );
		return 1;
	}

    //glutInitWindowSize(512, 512);
    //glutInit(&argc, argv);
    //glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
    //(void)glutCreateWindow(argv[0]);

	if ( SDL_Init ( SDL_INIT_VIDEO ) < 0 )
	{
		printf ( "error: SDL_Init failed: %s\n", SDL_GetError () );
		return 1;
	}

	sdlWin = SDL_CreateWindow ( "mdlviewer",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
	if ( !sdlWin )
	{
		printf ( "error: SDL_CreateWindow failed: %s\n", SDL_GetError () );

		SDL_Quit ();
		return 1;
	}

	sdlCtx = SDL_GL_CreateContext ( sdlWin );
	if ( !sdlCtx )
	{
		printf ( "error: SDL_GL_CreateContext failed: %s\n", SDL_GetError () );

		SDL_DestroyWindow ( sdlWin );
		SDL_Quit ();
		return 1;
	}

	if ( SDL_GL_MakeCurrent ( sdlWin, sdlCtx ) < 0 )
	{
		printf ( "error: SDL_GL_MakeCurrent failed: %s\n", SDL_GetError () );

		SDL_GL_DeleteContext ( sdlCtx );
		SDL_DestroyWindow ( sdlWin );
		SDL_Quit ();
		return 1;
	}

	SDL_GL_SetSwapInterval ( -1 );

    if ( init( argv[1] ) != 0 )
	{
		SDL_GL_MakeCurrent ( sdlWin, NULL );
		SDL_GL_DeleteContext ( sdlCtx );
		SDL_DestroyWindow ( sdlWin );
		SDL_Quit ();
		return 1;
	}

    //glutDisplayFunc(display);
    //glutKeyboardFunc(key);
    //glutReshapeFunc(reshape);
    //glutMouseFunc(mouse);
    //glutMotionFunc(motion);
    //glutMainLoop();

	running = true;
	redraw = true;
	SDL_Event event;
	SDL_PumpEvents ();
	while ( running )
	{
		while ( SDL_PollEvent ( &event ) )
		{
			switch ( event.type )
			{
			case ( SDL_QUIT ):
				running = false;
				break;

			case ( SDL_KEYDOWN ):
				key ( event.key.keysym.sym );
				break;

			case ( SDL_WINDOWEVENT ):
				if ( event.window.event == SDL_WINDOWEVENT_RESIZED )
				{
					reshape ( event.window.data1, event.window.data2 );
				}
				break;

			case ( SDL_MOUSEBUTTONDOWN ):
			case ( SDL_MOUSEBUTTONUP ):
				mouse (
					event.button.button, event.button.state,
					event.motion.x, event.motion.y );
				break;

			case ( SDL_MOUSEMOTION ):
				motion ( event.motion.x, event.motion.y );
				break;

			default:
				break;
			}
		}

		display ();

		SDL_PumpEvents ();
	}

	SDL_GL_MakeCurrent ( sdlWin, NULL );
	SDL_GL_DeleteContext ( sdlCtx );
	SDL_DestroyWindow ( sdlWin );
	SDL_Quit ();

    return 0;
}

