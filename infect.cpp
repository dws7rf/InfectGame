
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <ctime>
#include <cstdlib>

void drawsky(void);
//angle of rotation
float xpos = 0, ypos = 0, zpos = 0, xrot = 10, yrot = 0, angle=0.0;
float playTime = 0;
int npcCount;

float cRadius = 20.0f; 

float lastx, lasty;

float green[3] = {0.0f, 1.0f, 0.0f};


int winConditon = 0;

struct npc {
    float xpos;
    float zpos;
    int numberOfMoves;
    int currentMoveState;
    int infected;
    int direction;
    float movementRate;
};

int directions[4] = {1,2,3,4};

GLfloat normals[][3] = {{-1.0,-1.0,-1.0},{1.0,-1.0,-1.0},
    {1.0,1.0,-1.0}, {-1.0,1.0,-1.0}, {-1.0,-1.0,1.0}, 
    {1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}};

GLfloat vertices[][3] = {{-1.0,0.0,-0.5},{1.0,0.0,-0.5},
    {1.0,3.0,-0.5}, {-1.0,3.0,-0.5}, {-1.0,0.0,0.5}, 
    {1.0,0.0,0.5}, {1.0,3.0,0.5}, {-1.0,3.0,0.5}};

GLfloat mousevert[][3] = {{-.25,0.0,-.5},{.25,0.0,-.5},
    {.25,.25,-.5}, {-.25,.25,-.5}, {-.25,0.0,.5}, 
    {.25,0.0,.5}, {.25,.25,.5}, {-.25,.25,.5}, {0, 0, -1.25}};


GLfloat skyvert[][4] = {{-75.0,-1.0,-75.0},{75.0,-1.0,-75.0},
    {75.0,20.0,-75.0}, {-75.0,20.0,-75.0}, {-75.0,-1.0,75.0}, 
    {75.0,-1.0,75.0}, {75.0,20.0,75.0}, {-75.0,20.0,75.0}};

GLuint fronttexture;
GLuint back;
GLuint sidetx;
GLuint toptexture;
GLuint skytexture;

GLuint LoadTexture( const char * filename, int width, int height , int clamp)
{
    GLuint texture;
    unsigned char * data;
    FILE * file;

    file = fopen( filename, "rb" );
    if ( file == NULL ) {
        std::cout<<"failed to open "<<filename<<"\n";
        return 0;
    }
    data = (unsigned char *)malloc( width * height * 3 );
    unsigned char header[54];
    fread(header, 1, 54,  file);

    fread( data, 1, width * height * 3, file );
    fclose( file );

    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_3D, texture );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );


    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    if(clamp == 1){
        glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }
    else{
        glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, width, height, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    free( data );
    return texture;
}

float collision (float p1x, float p1z, float p2x, float p2z ) {
    float dist;
    p2x = p2x;
    p2z = p2z;
    dist = sqrt(((p1x - p2x) * (p1x - p2x)) + ((p1z - p2z) * (p1z - p2z)));
    return dist;
}


static npc npcs[100000];

static void updateCubePos(void){
    for(int i=0; i<npcCount; i++){
        if(npcs[i].currentMoveState>=npcs[i].numberOfMoves){
            npcs[i].direction++;
            if(npcs[i].direction >4)
            npcs[i].direction = 1;
            npcs[i].currentMoveState = 0;
        }
        switch (npcs[i].direction){
            case 1:
                npcs[i].xpos += npcs[i].movementRate;
                break;
            case 2:
                npcs[i].zpos+= npcs[i].movementRate;
                break;
            case 3:
                npcs[i].xpos-= npcs[i].movementRate;
                break;
            case 4:
                npcs[i].zpos-= npcs[i].movementRate;
                break;
        }
        npcs[i].currentMoveState++;
    }
}


void timer_function( int value)
{
    int allInfeced = 1;
    for(int i=0; i<npcCount; i++){
        if(npcs[i].infected==0){
            allInfeced = 0;
        }
    }
    if(allInfeced == 1){
        std::cout<< "You Won in " << playTime * .03 << " seconds!\n";
        exit(0);
    }
    if(value != 1){
        playTime++;
        updateCubePos();
        glutPostRedisplay();
        glutTimerFunc( 30,timer_function , 0);
    }
}



void cubepositions (void) { 

    for (int i=0;i<npcCount;i++)
    {
        npcs[i].xpos = rand()%40-20;
        npcs[i].zpos = rand()%40-20;
        npcs[i].infected = 0;
        npcs[i].numberOfMoves = rand()%50+1;
        npcs[i].currentMoveState = 0;
        npcs[i].direction = directions[0];
        npcs[i].movementRate = rand()%1+.1;
    }
}

void ground (void){
    glBegin(GL_QUADS);
    glVertex3f(-1000, 0, -1000);
    glVertex3f(-1000, 0, 1000);
    glVertex3f(1000, 0, 1000);
    glVertex3f(1000, 0, -1000);
    glEnd();
}

void sky (void){
    drawsky();
}

void polygon(int a, int b, int c, int d, int facingside){
    glEnable(GL_TEXTURE_3D);
    if(facingside == 1){
        glBindTexture(GL_TEXTURE_3D, fronttexture);
    }
    if(facingside == 2){
        glBindTexture(GL_TEXTURE_3D, back);
    }
    if(facingside == 3){
        glBindTexture(GL_TEXTURE_3D, sidetx);
    }
    if(facingside == 4){
        glBindTexture(GL_TEXTURE_3D, toptexture);
    }
    glBegin(GL_QUADS);
        glTexCoord3d(0.0, 0.0, 2.0);
        glNormal3fv(normals[a]);
        glVertex3fv(vertices[a]);
        glTexCoord3d(0.0,1.0,2.0);
        glNormal3fv(normals[b]);
        glVertex3fv(vertices[b]);
        glTexCoord3d(1.0,1.0,2.0);
        glNormal3fv(normals[c]);
        glVertex3fv(vertices[c]);
        glTexCoord3d(1.0,0.0,2.0);
        glNormal3fv(normals[d]);
        glVertex3fv(vertices[d]);
    glEnd();
    glDisable(GL_TEXTURE_3D);
}


void polycube(void){
    polygon(0,3,2,1,1);
    polygon(2,3,7,6,4);
    polygon(0,3,7,4,3);
    polygon(5,6,2,1,3);
    polygon(4,7,6,5,2);
    polygon(0,1,5,4,4);
}


//draw the cube
void cube (void) {
    for (int i=0;i<npcCount;i++)
    {
        if(npcs[i].infected == 1){
            glColor3f(0.0f, 1.0f, 0.0f);
        }
        else{
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        float rotation = 0;
        switch (npcs[i].direction){
            case 1:
                rotation = 90.0;
                break;
            case 3:
                rotation = -90.0;
                break;
            case 4:
                rotation = 180.0;
                break;
        }
        glTranslatef(-npcs[i].xpos, 0, -npcs[i].zpos); 
        glRotatef(rotation, 0.0, 1.0, 0.0);
        polycube(); 
        glRotatef(-rotation, 0.0, 1.0, 0.0);
        glTranslatef(npcs[i].xpos, 0, npcs[i].zpos);
    }
}

void polymouse(int a, int b, int c, int d){
    glBegin(GL_POLYGON);
        glNormal3fv(normals[a]);
        glVertex3fv(mousevert[a]);
        glNormal3fv(normals[b]);
        glVertex3fv(mousevert[b]);
        glNormal3fv(normals[c]);
        glVertex3fv(mousevert[c]);
        glNormal3fv(normals[d]);
        glVertex3fv(mousevert[d]);
    glEnd();
}

void mouse(void){
    polymouse(0,3,2,1);
    polymouse(2,3,7,6);
    polymouse(0,4,7,3);
    polymouse(1,2,6,5);
    polymouse(4,5,6,7);
    polymouse(0,1,5,4);
    polymouse(0,8,3,0);
    polymouse(0,8,1,0);
    polymouse(3,8,2,3);
    polymouse(2,8,1,2);
}

void polysky(int a, int b, int c, int d){
    glEnable(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, skytexture);
    glBegin(GL_POLYGON);
    glTexCoord3d(0.0, 0.0, 2.0);
        glNormal3fv(normals[a]);
        glVertex3fv(skyvert[a]);
        glTexCoord3d(0.0, 0.0, 2.0);
        glNormal3fv(normals[b]);
        glVertex3fv(skyvert[b]);
        glTexCoord3d(0.0, 0.0, 2.0);
        glNormal3fv(normals[c]);
        glVertex3fv(skyvert[c]);
        glTexCoord3d(0.0, 0.0, 2.0);
        glNormal3fv(normals[d]);
        glVertex3fv(skyvert[d]);
    glEnd();
    glDisable(GL_TEXTURE_3D);
}

void drawsky(void){
    polysky(0,3,2,1);
    polysky(2,3,7,6);
    polysky(0,4,7,3);
    polysky(1,2,6,5);
    polysky(4,5,6,7);
    polysky(0,1,5,4);
}

void drawmouse(void){
    glColor3f(.5f, .45f, .45f);
    mouse();
}

void init (void) {
    cubepositions();
}

void enable (void) {
    glEnable (GL_DEPTH_TEST); 
    glEnable (GL_LIGHTING); 
    glEnable (GL_LIGHT0); 
    glEnable (GL_LIGHT1);
    GLfloat lightpos[] = {0., 1., .2, 0.};
    GLfloat light2pos[] = {.2, 1., 0., 0.};
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightfv(GL_LIGHT1, GL_POSITION, light2pos);
    glEnable (GL_COLOR_MATERIAL);
    glShadeModel (GL_SMOOTH); 

}

void display (void) {
    glClearColor (0.0,0.0,0.0,1.0); 
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    enable();

    glLoadIdentity(); 
    
    glTranslatef(0.0f, 1.0f, -cRadius);
    glRotatef(xrot,1.0,0.0,0.0);
    glColor3f(0.5f, 0.35f, 0.05f);
    ground();
    glColor3f(.43f, .85f, .97f);
    sky();
    glColor3f(0.0f, 0.0f, 1.0f);
    drawmouse(); 
    
    glRotatef(yrot,0.0,1.0,0.0);  
    glTranslated(xpos,0.0f,zpos); 
    glColor3f(1.0f, 1.0f, 1.0f);
    for(int i=0; i<npcCount; i++);
    {
        cube();
    } 

    glutSwapBuffers(); 
    angle++; 
}

void reshape (int w, int h) {
    glViewport (0, 0, (GLsizei)w, (GLsizei)h); 
    glMatrixMode (GL_PROJECTION); 

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 0.1, 100.0); 
    glMatrixMode (GL_MODELVIEW); 

}

void keyboard (unsigned char key, int x, int y) {
    if (key=='d')
    {
    yrot += 5;
    if (yrot >360) yrot -= 360;
    }

    if (key=='a')
    {
    yrot -= 5;
    if (yrot >360) yrot -= 360;
    }

    if (key=='z')
    {
        if(cRadius + 5 <25)
        cRadius += 5;
    }

    if (key=='x')
    {
        if(cRadius - 5 > 5)
            cRadius -= 5;
    }

    if (key=='w')
    {
        int cancel = 0;
        float xrotrad, yrotrad, xtemp, ztemp;
        yrotrad = (yrot / 180 * 3.141592654f);
        xrotrad = (xrot / 180 * 3.141592654f);
        xtemp = xpos - float(sin(yrotrad));
        ztemp = zpos + float(sin(yrotrad));
        for(int i=0; i<npcCount; i++){
            float dist = collision(xtemp, ztemp, npcs[i].xpos, npcs[i].zpos);
            float oldDist = collision(xpos, zpos, npcs[i].xpos, npcs[i].zpos);
            if(dist < 2 && (oldDist >= dist)){
                cancel = 1;
                npcs[i].infected = 1;
            }
        }
        if(cancel == 0){
            xpos -= 1.5*float(sin(yrotrad));
            zpos += 1.5*float(cos(yrotrad));
            ypos -= 1.5*float(sin(xrotrad));
        }
        
    }

    if (key=='s')
    {
    float xrotrad, yrotrad;
    yrotrad = (yrot / 180 * 3.141592654f);
    xrotrad = (xrot / 180 * 3.141592654f); 
    xpos += float(sin(yrotrad));
    zpos -= float(cos(yrotrad));
    ypos += float(sin(xrotrad));
    }

    if (key==27)
    {
    exit(0);
    }
}

void mouseMovement(int x, int y) {
    int diffx=x-lastx; 
    int diffy=y-lasty; 
    lastx=x; 
    lasty=y; 
    
    if(xrot+diffy <=90 && xrot+diffy >= 20){
        xrot += (float) diffy;
    }
    yrot += (float) diffx;
}

int main (int argc, char **argv) {
    glutInit (&argc, argv);
    npcCount = atoi(argv[1]);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH); 
    glutInitWindowSize (500, 500); 
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Plague Game"); 
    init (); 
    glutDisplayFunc (display); 
    glutIdleFunc (display); 
    glutReshapeFunc (reshape);
    fronttexture = LoadTexture( "front.bmp", 128, 256, 1 );
    back = LoadTexture("back.bmp", 128, 256, 1);
    sidetx = LoadTexture("side.bmp", 128,256,1 );
    toptexture = LoadTexture("top.bmp", 256, 256, 1);
    skytexture = LoadTexture("sky.bmp", 256, 256, 0);
    glutPassiveMotionFunc(mouseMovement); 
    glutKeyboardFunc (keyboard); 
    glutTimerFunc (1, timer_function,0);
    glutMainLoop (); 
    return 0;
}