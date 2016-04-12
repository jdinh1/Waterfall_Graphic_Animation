//Jonathan Dinh
//cs335 Spring 2016 Homework-1
//4-9-2016
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <stdio.h>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <stdlib.h>
#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 50000
#define GRAVITY 0.1
extern "C" {
	#include "fonts.h"
}

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;
GC gc;
//Structures

struct Global {
	int bubblemode;
	Global() {
		bubblemode = 0;	
	}
}g;
struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Circle {
	float radius;
	Vec center;
	int detail;
}circle;

struct Game {
	Shape box[5];
	Particle particle[MAX_PARTICLES];
	int n;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void showMenu(int x, int y, const char* msg);
void bubblemode(Game *game);

int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;
	srand(time(0));	

	//declare a cicle
	circle.radius = 150.0;
	circle.center.x = WINDOW_WIDTH - 90.0;
	circle.center.y = -50.0;
	circle.detail = 150;
	
	//declare box shapes
	game.box[0].width = 60;
	game.box[0].height = 10;
	game.box[0].center.x = 80;
	game.box[0].center.y = WINDOW_HEIGHT - 40;

	game.box[1].width = 60;
	game.box[1].height = 10;
	game.box[1].center.x = 130;
        game.box[1].center.y = WINDOW_HEIGHT - 80;
	
	game.box[2].width = 60;
	game.box[2].height = 10;
	game.box[2].center.x = 180;
	game.box[2].center.y = WINDOW_HEIGHT - 120;

	game.box[3].width = 60;
	game.box[3].height = 10;        
	game.box[3].center.x = 230;                              
	game.box[3].center.y = WINDOW_HEIGHT - 160; 

	
	game.box[4].width = 60;
	game.box[4].height = 10;        
	game.box[4].center.x = 280;                              
	game.box[4].center.y = WINDOW_HEIGHT - 200; 

	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}


void showMenu(int x, int y, const char *msg) 
{
	char str[64];
	XDrawString(dpy, win, gc, x, y, msg, strlen(msg));
	
	y += 16;
	sprintf(str,"(B) - Bubble: %s",
		(g.bubblemode==1) ? "ON" : "OFF");
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Homework 1 - Waterfall Model");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y, float xvel) {
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	int randomY = rand()% 30-40;
	float yvel = randomY/10.0;
	p->velocity.y = yvel;
	p->velocity.x =  xvel;
	game->n++;
	
}

void bubblemode(Game *game)
{
	if (g.bubblemode !=1)
		return;
	int random = 0;
	int y = WINDOW_HEIGHT - 5;
	int x = 70;
	for (int i = 0; i < 20; i++) {
		random = rand() % 60 - 30;
		float xvel = (float)random/10.0;
		makeParticle(game, x, y, xvel);
	}

}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			std::cout << e->xbutton.x << " " << e->xbutton.y << std::endl;
			int y = WINDOW_HEIGHT - 20;
			makeParticle(game, e->xbutton.x, y, 1.0);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			game->box[0].center.x = e->xbutton.x;
			game->box[0].center.y = WINDOW_HEIGHT - e->xbutton.y;
		    	return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
			return;
		int y = WINDOW_HEIGHT - e->xbutton.y;
		makeParticle(game, e->xbutton.x, y, 1.0);
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		switch (key) {
			case XK_b:
				g.bubblemode ^= 1;
				break;
			case XK_Escape:
				return 1;
			}
		//You may check other keys here.

	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;
	for (int i = 0; i < game->n; i++) {
		p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= 0.4;
	//check for collision with shapes...
	for (int j = 0; j < 5; j++) {
		Shape *s;
		s = &game->box[j];
		if (p->s.center.y  >= s->center.y - (s->height) &&  
	    	    p->s.center.y  <= s->center.y + (s->height) &&
	    	    p->s.center.x  >= s->center.x - (s->width) &&
	    	    p->s.center.x  <= s->center.x + (s->width)) {
			if (p->velocity.x >= 0 && p->velocity.x <= 0.5)
				p->velocity.x += 0.5;
			if (p->velocity.x < 0)
				p->velocity.x *= -1.0;
			p->velocity.y *= -0.3;
		}
	}
		float d1 = p->s.center.y - circle.center.y;
		float d2 = p->s.center.x - circle.center.x;
		float dist = sqrt(d1*d1 + d2*d2);
		      	
		if (dist <= circle.radius) {
			//reposition particles
			p->s.center.x = (d2/dist) * circle.radius + (float)circle.center.x + 1.0;
			p->s.center.y = (d1/dist) * circle.radius + (float)circle.center.y + 1.0;
			
			if (p->velocity.x > 2.5 ||
			    p->s.center.x > circle.center.x) 
				p->velocity.x += 0.05;
			else 
				p->velocity.x -= 0.2;
			p->velocity.y -= 0.1;	
		}	
		//check for off-screen
		if (p->s.center.y < 0.0 ||
		    p->s.center.y > WINDOW_HEIGHT ||
		    p->s.center.x < 0.0 ||
		    p->s.center.x > WINDOW_WIDTH) {
			game->particle[i] = game->particle[game->n-1];
			game->n--;
		}
	}
}

void drawCircle(float x, float y, float radius, int detail)
{
	float radian = 2.0 * 3.14;
	glPushMatrix();
	glColor3ub(90,140,90);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i <= detail; i++) {
		glVertex2f(
			x + (radius * cos(i * radian / detail)),
			y + (radius * sin(i * radian / detail))
		);
		
	}
	glEnd();
	glPopMatrix();
}

void render(Game *game)
{
	//showMenu(0,0,"testing");
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	bubblemode(game);
	Rect r[6];

	r[0].bot =  WINDOW_HEIGHT - 21;
	r[0].left = 440;
	r[0].center = 1;

	r[1].bot = WINDOW_HEIGHT - 51;
	r[1].left = 80;
	r[1].center = 1;

	r[2].bot = WINDOW_HEIGHT - 91;
	r[2].left = 130;
	r[2].center = 1;

	r[3].bot = WINDOW_HEIGHT - 131;
	r[3].left = 180;
	r[3].center = 1;

	r[4].bot = WINDOW_HEIGHT - 171;
	r[4].left = 230;
	r[4].center = 1;

	r[5].bot = WINDOW_HEIGHT - 211;
	r[5].left = 280;
	r[5].center = 1;

	
	//draw box
	for (int i = 0; i < 5; i++) {
		Shape *s;
		glColor3ub(90,140,90);
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
	}
	
	ggprint8b(&r[0], 16, 0x00ff0000, "Waterfall Model");
	ggprint8b(&r[0], 16, 0x00ff0000, "(B) - Bubble: %s",(g.bubblemode==1) ? "ON ":"OFF");
	ggprint16(&r[1], 0, 0x00ffa500, "Requirements");
	ggprint16(&r[2], 0, 0x00ffa500, "Design");
	ggprint16(&r[3], 0, 0x00ffa500, "Coding");
	ggprint16(&r[4], 0, 0x00ffa500, "Testing");
	ggprint16(&r[5], 0, 0x00ffa500, "Maintenance");
	//draw all particles here
	drawCircle(circle.center.x, 
		circle.center.y, 
		circle.radius,
	  	circle.detail	
	);

	glPushMatrix();
	for (int i =0; i < game->n; i++) {
		Vec *c = &game->particle[i].s.center;
		int randomBlue = rand()% 60 + 190;
		int randomGreen = rand()% 70 + 100;
		int randomRed = rand()% 80 + 100;
		glColor3ub(randomRed, randomGreen ,randomBlue);
		w = 1.5;
		h = 1.5;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}


