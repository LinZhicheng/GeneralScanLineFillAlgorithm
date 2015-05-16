// GeneralScanLineFillAlgorithm.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdlib.h"
#include "GL/glut.h"
#include "GL/glaux.h"
#include "windows.h"

#define POINT_NUM 5
#define ROUND(a) (int)(a+0.5)

int w = 640, h = 480;

typedef struct Point{
	int x;
	int y;
}Point;

typedef struct Edge{
	Point start;
	Point end;
	Edge *next;
}Edge;

typedef struct ScanLine{
	int y;
	Edge *first;
	ScanLine *up;
}ScanLine;

typedef struct EDGE{
	int ymax;
	double x;
	double dx;
	EDGE *next;
}EDGE;

typedef struct SCANLINE{
	int y;
	EDGE *first;
	SCANLINE *up;
}SCANLINE;

int getYmin(Edge edge){
	if (edge.start.y < edge.end.y){
		return edge.start.y;
	}
	else{
		return edge.end.y;
	}
}

int getYmax(Edge edge){
	if (edge.start.y > edge.end.y){
		return edge.start.y;
	}
	else{
		return edge.end.y;
	}
}

void setYmax(Edge edge){
	if (edge.start.y > edge.end.y){
		edge.start.y -= 1;
	}
	else{
		edge.end.y -= 1;
	}
}

int getXmin(Edge edge){
	if (edge.start.x < edge.end.x){
		return edge.start.x;
	}
	else{
		return edge.end.x;
	}
}

Point getLowestPoint(Edge edge){
	if (edge.start.y < edge.end.y){
		return edge.start;
	}
	else{
		return edge.end;
	}
}

bool isUniqueMax(Edge edge, Edge edges[], int edgeNum){
	for (int i = 0; i < POINT_NUM; i++){
		if (edgeNum == i)
			continue;
		if (getYmax(edge) == getYmax(edges[i])){
			return false;
		}
	}
	return true;
}

void makeEdgesOrder(Edge edges[], ScanLine *sl, int lineNum){
	ScanLine *s = sl;
	Edge *p = NULL, *q = NULL;
	while (s != NULL){
		int y = s->y;
		for (int j = 0; j < POINT_NUM; j++){
			q = s->first;
			if (getYmin(edges[j]) == y){
				if (s->first == NULL){
					s->first = &edges[j];
					p = s->first;
				}
				else{
					p = &edges[j];
				}
				if (q == NULL){
					q = p;
				}
				else{
					Edge *r = NULL;
					while (q != NULL){
						if (getXmin(*p) >= getXmin(*q)){
							r = q;
							q = q->next;
						}
						else{
							if (r != NULL){
								r->next = p;
								p->next = q;
							}
							else{
								r = q;
								q = p;
								q->next = r;
								s->first = q;
							}
							break;
						}
					}
					if (q == NULL){
						q->next = p;
					}
				}
			}
		}
		s = s->up;
	}
}

void fill(int y, double x0, double xEnd){
	glVertex3d(x0, y, 0);
	glVertex3d(xEnd, y, 0);
}

void generalScanLineFillAlgorithm(int ymin, int ymax, ScanLine *sl, SCANLINE *SL, int lineNum){
	ScanLine *s = sl;
	SCANLINE *S = SL;
	Edge *p;
	EDGE *q, *r;
	while (s != NULL){
		p = s->first;
		q = S->first;
		while (p != NULL){
			r = new EDGE;
			r->dx = (double)(p->start.x - p->end.x) / (double)(p->start.y - p->end.y);
			r->x = getLowestPoint(*p).x;
			r->ymax = getYmax(*p);
			r->next = NULL;
			if (S->first == NULL){
				S->first = r;
				q = S->first;
			}
			else{
				q->next = r;
				q = q->next;
			}
			p = p->next;
		}
		s = s->up;
		S = S->up;
	}

	SCANLINE *AET = new SCANLINE;
	AET->first = NULL;
	AET->up = NULL;
	EDGE *l, *m, *n;
	S = SL;
	while (S != NULL){
		AET->y = S->y;
		if (AET->first == NULL)
			AET->first = S->first;
		l = AET->first;
		m = AET->first;
		while (m != NULL){
			if (m->ymax < S->y){
				if (m == l){
					l = m->next;
					m = l;
				}
				else{
					l->next = m->next;
					m = l->next;
				}
			}
			else{
				m->x += m->dx;
				//m->x = ROUND(m->x);
				m = m->next;
				l = l->next;
			}
		}
		n = S->first;
		while (n != NULL){
			l = n;
			l = l->next;
			n = n->next;
		}
		l = AET->first;
		m = l->next;
		n = AET->first;
		while (m != NULL){
			if (l->x > m->x){
				n->next = m;
				l->next = m->next;
				m->next = l;
				m = l->next;
				n = n->next;
			}
			else{
				n = n->next;
				l = l->next;
				m = m->next;
			}
		}
		l = AET->first;
		m = l->next;
		while (l != NULL&&m != NULL){
			fill(S->y, l->x, m->x);
			if (l->next == NULL || m->next == NULL){
				break;
			}
			l = m->next;
			m = l->next;
		}
		S = S->up;
	}
}

void init(GLvoid)     // Create Some Everyday Functions
{

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void display(void)   // Create The Display Function
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix

	Point point[POINT_NUM];
	Edge edges[POINT_NUM];
	int ymin, ymax, lineNum;
	point[0].x = 300; point[0].y = 290;
	point[1].x = 390; point[1].y = 230;
	point[2].x = 350; point[2].y = 130;
	point[3].x = 250; point[3].y = 140;
	point[4].x = 210; point[4].y = 230;

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	for (int i = 0; i < POINT_NUM; i++){
		glVertex2d(point[i].x, point[i].y);
	}
	glEnd();

	for (int i = 0; i < POINT_NUM; i++){
		edges[i].start = point[i];
		edges[i].end = point[(i + 1) % POINT_NUM];
		edges[i].next = NULL;
	}

	ymin = point[0].y;
	ymax = point[0].y;
	for (int i = 1; i < POINT_NUM; i++){
		if (point[i].y < ymin)
			ymin = point[i].y;
		else if (point[i].y > ymax)
			ymax = point[i].y;
	}
	lineNum = ymax - ymin + 1;

	ScanLine *sl = new ScanLine, *s = sl;
	SCANLINE *SL = new SCANLINE, *S = SL;

	for (int i = 0; i < lineNum; i++){
		s->y = ymin + i;
		s->first = NULL;
		S->y = ymin + i;
		S->first = NULL;
		if (i == lineNum - 1){
			s->up = NULL;
			S->up = NULL;
		}
		else{
			s->up = new ScanLine;
			s = s->up;
			S->up = new SCANLINE;
			S = S->up;
		}
	}

	for (int i = 0; i < POINT_NUM; i++){
		if (isUniqueMax(edges[i], edges, i)){
			setYmax(edges[i]);
		}
	}

	makeEdgesOrder(edges, sl, lineNum);
	glColor3f(1.0f, 0.0f, 0.0f);
	glLineWidth(5);
	glBegin(GL_LINES);
	generalScanLineFillAlgorithm(ymin, ymax, sl, SL, lineNum);
	glEnd();

	glutSwapBuffers();
	// Swap The Buffers To Not Be Left With A Clear Screen
}

void reshape(int w, int h)   // Create The Reshape Function (the viewport)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);  // Select The Projection Matrix
	glLoadIdentity();                // Reset The Projection Matrix
	gluOrtho2D(0.0, w, 0.0, h);
	glMatrixMode(GL_MODELVIEW);  // Select The Model View Matrix
	glLoadIdentity();    // Reset The Model View Matrix
}

void keyboard(unsigned char key, int x, int y)  // Create Keyboard Function
{
	switch (key) {
	case 27:        // When Escape Is Pressed...
		exit(0);   // Exit The Program
		break;        // Ready For Next Case
	default:        // Now Wrap It Up
		break;
	}
}

void arrow_keys(int a_keys, int x, int y)  // Create Special Function (required for arrow keys)
{
	switch (a_keys) {
	case GLUT_KEY_UP:     // When Up Arrow Is Pressed...
		glutFullScreen(); // Go Into Full Screen Mode
		break;
	case GLUT_KEY_DOWN:               // When Down Arrow Is Pressed...
		glutReshapeWindow(w, h); // Go Into A 640 By 480 Window
		break;
	default:
		break;
	}
}

void main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	glutInit(&argc, argv); // Erm Just Write It =)
	init();
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // Display Mode
	glutInitWindowSize(w, h); // If glutFullScreen wasn't called this is the window size
	glutCreateWindow("扫描线填充"); // Window Title (argv[0] for current directory as title)
	glutDisplayFunc(display);  // Matching Earlier Functions To Their Counterparts
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrow_keys);
	glutMainLoop();          // Initialize The Main Loop
}