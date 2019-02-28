#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

#include <iostream>
#include "math.h"

//Globals
int size_x;
int size_z;
int terrainType;
//terrain x and z and iteration
int terX = 50;
int terZ = 50;
int terIt = 50;
const int fault = 0;
const int circle = 1;
//maximum and minimum draw_height values
float minHeight, maxHeight;

float heightMap[300][300];
int drawArray[4][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
float normalVectors[1000][1000][3];

float camPos[] = {80, 100, 60}; //where the camera is
float camUp[] = {0, 1, 0};      //up vector of the camera
float camTarget[] = {0, 0, 0};  //where the camera is looking at
float camSpeed = 0.5f;

float m_amb[] = {0.16, 0.22, 0.16, 1.0};
float m_dif[] = {0.54, 0.89, 0.63, 1.0};
float m_spec[] = {0.32, 0.32, 0.32, 1.0};
float shiny = 12.8;

float pos[4] = {2, 2, 2, 1};
float amb[4] = {1, 0.5, 0.5, 1};
float dif[4] = {1, 0.5, 0.3, 1};
float spc[4] = {0.5, 0.5, 0.5, 1};

/*calculate normal vector 
http://www.lighthouse3d.com/opengl/terrain/index.php?normals*/
void calNormalVector()
{
    float a[3];
    float b[3];
    float c[3];
    float d;
    for (int x = 0; x < size_x; x++)
    {
        for (int z = 0; z < size_z; z++)
        {
            a[0] = x + 1;
            a[1] = heightMap[x + 1][z] - heightMap[x][z];
            a[2] = z;

            b[0] = x + 1;
            b[1] = heightMap[x + 1][z + 1] - heightMap[x][z];
            b[2] = z + 1;

            //calculate cross product
            c[0] = a[1] * b[2] - a[2] * b[1];
            c[1] = a[2] * b[0] - a[0] * b[2];
            c[2] = a[0] * b[1] - a[1] * b[0];
            d = sqrtf(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);

            //normal vectors
            normalVectors[x][z][0] = c[0] / d;
            normalVectors[x][z][1] = c[1] / d;
            normalVectors[x][z][2] = c[2] / d;
        }
    }
}

void display()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camPos[0], camPos[1], camPos[2],
              camTarget[0], camTarget[1], camTarget[2],
              camUp[0], camUp[1], camUp[2]);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

    for (int x = 1; x < size_x; x++)
    {
        for (int z = 1; z < size_z; z++)
        {
            glBegin(GL_QUADS);
            for (int i = 0; i < 4; i++)
            {
                //draw polygon for each point
                int draw_x = x - drawArray[i][0];
                int draw_z = z - drawArray[i][1];
                float draw_y = heightMap[draw_x][draw_z];
                float draw_height = (draw_y - minHeight) / (maxHeight - minHeight);
                //set different color with respect to height
                if (draw_height >= 0.4)
                {
                    glColor3f(1.0, 1.0, 1.0);
                }
                else if (draw_height >= 0.1)
                {
                    glColor3f(0.0, 0.5, 0.0);
                }
                else
                {
                    glColor3f(0.0, 0.0, 0.5);
                }

                glNormal3fv(normalVectors[draw_x][draw_z]);
                glVertex3d(draw_x, draw_y, draw_z);
            }
        }
        glEnd();
    }
    glutSwapBuffers();
}

/* fault algorithm method
 http://www.lighthouse3d.com/opengl/terrain/index.php?impdetails*/
void faultAlgorithm(int terIt)
{
    for (int i = 0; i < terIt; i++)
    {
        float v = static_cast<float>(rand());
        float a = sin(v);
        float b = cos(v);
        float d = sqrt(size_x * size_x + size_z * size_z);
        float c = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * d - d / 2;

        // for each x,z point on the map determine if increase or decrease draw_height
        for (int x = 0; x < size_x; x++)
        {
            for (int z = 0; z < size_z; z++)
            {
                if (a * x + b * z - c > 0)
                { //increase draw_height
                    heightMap[x][z] += 1;
                    if (heightMap[x][z] > maxHeight)
                    {
                        maxHeight = heightMap[x][z];
                    }
                }
                else
                {
                    //decrease draw_height
                    heightMap[x][z] -= 1;
                    if (heightMap[x][z] < minHeight)
                    {
                        minHeight = heightMap[x][z];
                    }
                }
            }
        }
    }
}
/* circles algorithm method
http://www.lighthouse3d.com/opengl/terrain/index.php?circles */
void circlesAlgorithm(int terIt)
{
    for (int i = 0; i < terIt; i++)
    {
        //create a circle with random x and y and radius
        int random_x = rand() % size_x;
        int random_z = rand() % size_z;
        //terrainCircleSize defines the circle size and disp defines the maximum draw_height variation
        int terrainCircleSize = (rand() % 24) + 1;
        float disp = (rand() % 12) + 1;
        for (int x = 0; x < size_x; x++)
        {
            for (int z = 0; z < size_z; z++)
            {
                int tx = x - random_x;
                int tz = z - random_z;
                //get the distance from circle center
                float dist = sqrtf((tx * tx) + (tz * tz));
                float pd = (dist * 2) / terrainCircleSize;
                //check if value is within the circle
                if (fabs(pd) <= 1.0)
                {
                    //new draw_height
                    heightMap[x][z] += (disp / 2) + (cos(pd * 3.14) * (disp / 2));
                }
            }
        }
    }
    //find new max and min draw_height values
    minHeight = 0;
    maxHeight = 1;

    for (int x = 0; x < size_x; x++)
    {
        for (int z = 0; z < size_z; z++)
        {
            if (heightMap[x][z] < minHeight)
            {
                minHeight = heightMap[x][z];
            }
            else if (heightMap[x][z] > maxHeight)
            {
                maxHeight = heightMap[x][z];
            }
        }
    }
}

//initialize terrain
void initTerrain()
{
    size_x = 50;
    size_z = 50;
    terrainType = circle;
    circlesAlgorithm(50);
}

//generalize terrain
void terrain(int terX, int terZ, int type_op, int terIt)
{
    size_x = terX;
    size_z = terZ;
    terrainType = type_op;
    if (terrainType == circle)
    {
        circlesAlgorithm(terIt);
    }
    else if (terrainType == fault)
    {
        faultAlgorithm(terIt);
    }
    calNormalVector();
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 300);

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

    //enalbe for wireframe representation
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        exit(0);
        break;
    case 'r':
    case 'R':
        terrain(terX, terZ, terrainType, terIt);
        break;
    //wireframe representation
    case 'w':
    case 'W':
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case 'e':
    case 'E':
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case 'f':
    case 'F':
        glShadeModel(GL_FLAT);
        break;
    case 's':
    case 'S':
        glShadeModel(GL_SMOOTH);
        break;
    case 'k':
    case 'K':
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        break;
    case 'l':
    case 'L':
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        break;
    }
    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        camPos[1] += camSpeed;
        break;
    case GLUT_KEY_DOWN:
        camPos[1] -= camSpeed;
        break;
    case GLUT_KEY_LEFT:
        camPos[2] += camSpeed;
        break;
    case GLUT_KEY_RIGHT:
        camPos[2] -= camSpeed;
        break;
    }
}

void FPS(int val)
{
    glutPostRedisplay();
    glutTimerFunc(17, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

void menuProc1(int value)
{
    if (value = 5){
        terrainType == circle;
        terrain(terX, terZ, circle, terIt);
    }
    if (value = 6){
        terrainType == fault;
        terrain(terX, terZ, fault, terIt);
    }
    glutPostRedisplay();
}

void menuProc2(int value)
{
    switch (value)
    {
    case 1:
        terX = 50;
        terZ = 50;
        break;
    case 2:
        terX = 100;
        terZ = 100;
        break;
    case 3:
        terX = 150;
        terZ = 150;
        break;
    case 4:
        terX = 300;
        terZ = 300;
        break;
    }
    terrain(terX, terZ, terrainType, terIt);
    glutPostRedisplay();
}

void menuProc(int value)
{
}

void createOurMenu()
{
    int submenu_1 = glutCreateMenu(menuProc1);
    glutAddMenuEntry("circle", 5);
    glutAddMenuEntry("default", 6);

    int submenu_2 = glutCreateMenu(menuProc2);
    glutAddMenuEntry("50 x 50", 1);
    glutAddMenuEntry("100 x 100", 2);
    glutAddMenuEntry("150 x 150", 3);
    glutAddMenuEntry("300 x 300", 4);

    int main_id = glutCreateMenu(menuProc);
    glutAddSubMenu("terrain type", submenu_1);
    glutAddSubMenu("terrain size", submenu_2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void callbackinit()
{
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(0, FPS, 0);
}

void readme(void)
{
    printf("\nw : EnableWireframe\n");
    printf("\ne : DisableWireframe\n");
    printf("\nl : Enable Lighting \n");
    printf("\nk : Disable Lighting\n");
    printf("\nf : Flat shading\n");
    printf("\ns : Smooth shading\n");
    printf("\nr : Random Terrain\n");
    printf("\nq : Quit");
    printf("\nRight Click: menu \n");
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(800, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Terrain");

    readme();
    initTerrain();
    callbackinit();
    init();
    createOurMenu();
    glutMainLoop();
}