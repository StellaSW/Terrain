/**
  *author: Shengyu wu
  */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <time.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

using namespace std;

//Globals
float camPos[] = {0, 0, 200}; //where the camera is
int angleX = 44;
int angleY = -20;
int angleZ = 0;

//maximum and minimum draw_height values
float minHeight, maxHeight;
int mode = 1;               // 1 = solid polygons; 2 = wireframe; 3 = solid polygons & wireframe

const int terrainSize = 100; 
float heights[terrainSize][terrainSize];
float normals[terrainSize][terrainSize][3];
bool definedHeights = false;
bool isCircle = true;
bool isWireframe = false;
bool isQuad = true;
bool shadingFlat = true;
const int iterations = 50;
int circles[iterations][4];

float m_amb[] = {0.19125, 0.0735, 0.0225, 1};
float m_dif[] = {0.7038, 0.27048, 0.0828, 1};
float m_spec[] = {0.256777, 0.137622, 0.086014, 1};
float shiny = 0.3;

float pos[4] = {50, -500, 50, 1};
float amb[4] = {0.1, 0.1, 0.1, 1};
float dif[4] = {0.8, 0.8, 0.8, 1};
float spc[4] = {0.5, 0.5, 0.5, 1};

/*calculate normal vector 
http://www.lighthouse3d.com/opengl/terrain/index.php?normals*/
void calNormalVector()
{
    float x1, y1, z1, x2, y2, z2, xc, yc, zc;
    for (int x = 0; x < terrainSize - 1; x++)
    {
        for (int z = 0; z < terrainSize - 1; z++)
        {
            x1 = x + 1;
            y1 = heights[x + 1][z] - heights[x][z];
            z1 = z;

            x2 = x + 1;
            y2 = heights[x + 1][z + 1] - heights[x][z];
            z2 = z + 1;

            xc = y1 * z2 - z1 * y2;
            yc = z1 * x2 - x1 * z2;
            zc = x1 * y2 - y1 * x2;
            float n = sqrtf(xc * xc + yc * yc + zc * zc);

            normals[x][z][0] = xc / n;
            normals[x][z][1] = yc / n;
            normals[x][z][2] = zc / n;
        }
    }
}

/* fault algorithm method
 http://www.lighthouse3d.com/opengl/terrain/index.php?impdetails*/
void faultAlgorithm()
{
    for (int i = 0; i < 200; i++)
    {
        float r = static_cast<float>(rand());
        float d = sqrt(terrainSize * terrainSize + terrainSize * terrainSize);
        float c = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * d - d / 2;

        for (int x = 0; x < terrainSize; x++)
        {
            for (int z = 0; z < terrainSize; z++)
            {
                if (sin(r) * x + cos(r) * z - c > 0)
                    heights[x][z]++;
                else
                    heights[x][z]--;
            }
        }
    }
}
/* circles algorithm method
http://www.lighthouse3d.com/opengl/terrain/index.php?circles */
void randCircles()
{
    srand(time(NULL));
    for (int i = 0; i < iterations; i++) // 50 iterations
    {
        circles[i][0] = rand() % (terrainSize - 1); // circle vertex x
        circles[i][1] = rand() % (terrainSize - 1); // circle vertex y
        circles[i][2] = rand() % 30 - 15;    // disp: random -15 ~ 15
        circles[i][3] = rand() % 10 + 10;    // terrainCircleSize random 10 ~ 20
    }
}

void circleAlgorithm(int xc, int zc, int disp, int terrainCircleSize)
{
    for (int x = 1; x <= terrainSize - 1; x++)
    {
        for (int z = 1; z <= terrainSize - 1; z++)
        {
            float pd = sqrt(pow(xc - x, 2) + pow(zc - z, 2)) / terrainCircleSize;
            if (fabs(pd) <= 1.0)
                heights[x][z] += disp / 2 + cos(pd * 3.14) * disp / 2;
        }
    }
}

void resetHeightmap(void)
{
    for (int x = 0; x <= terrainSize - 1; x++)
        for (int z = 0; z <= terrainSize - 1; z++)
            heights[x][z] = 0;
}

//generalize terrain
void heightmap(void)
{
    resetHeightmap();
    if (isCircle)
    {
        randCircles();
        for (int i = 0; i < 100; i++)
        {
            circleAlgorithm(circles[i][0], circles[i][1], circles[i][2], circles[i][3]);
        }
    }
    else
    {
        faultAlgorithm();
    }
    /* define min height and max height*/
    minHeight = 0;
    maxHeight = 0;
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            if (heights[i][j] < minHeight)
                minHeight = heights[i][j];
            else if (heights[i][j] > maxHeight)
                maxHeight = heights[i][j];
        }
    }
    definedHeights = true; // to avoid the duplication of heightmap algorithm

    calNormalVector();
}

void setVertex(int x, int z)
{
    float y = heights[x + terrainSize / 2][z + terrainSize / 2]; // y is height
    float r, g, b;
    if (!isWireframe)
    {
        float percent = (y - minHeight) / (maxHeight - minHeight);
        if (percent > 0.5) // from yellow (low) to red (hight)
        {
            r = 1;
            g = 1 - (percent - 0.5) * 2;
            b = 0;
        }
        else // from green (low) to yellow (high)
        {
            r = percent * 2;
            g = 1;
            b = 0;
        }
        glColor3f(r, g, b);
    }
    else
    {
        glColor3f(1, 1, 1); // white colour for wireframe in mode 3
    }
    glNormal3fv(normals[x + terrainSize / 2][z + terrainSize / 2]); // Normals
    glVertex3f(x, y, z);
}

void drawTerrain(void)
{
    if (isQuad)
        glBegin(GL_QUAD_STRIP);
    else
        glBegin(GL_TRIANGLE_STRIP);
    for (int z = -terrainSize / 2; z <= terrainSize / 2 - 2; z++)
    {
        if (z % 2 == 0)
        {
            for (int x = -terrainSize / 2; x <= terrainSize / 2 - 1; x++)
            {
                setVertex(x, z);
                setVertex(x, z + 1);
            }
        }
        else
        {
            for (int x = terrainSize / 2 - 1; x >= -terrainSize / 2; x--)
            {
                setVertex(x, z + 1);
                setVertex(x, z);
            }
        }
    }
    glEnd();
}

void display(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camPos[0], camPos[1], camPos[2], 0, 0, 0, 0, 1, 0);
    
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    glRotatef(angleZ, 0, 0, 1);

    if (!definedHeights)
    {
        heightmap();
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

    /* 3 modes */
    if (mode == 1)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawTerrain();
    }
    else if (mode == 2)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawTerrain();
    }
    else
    {
        isWireframe = true;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawTerrain();
        isWireframe = false;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawTerrain();
    }
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluOrtho2D(0, w, 0, h);
    gluPerspective(45, (float)((w + 0.0f) / h), 1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 500);

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

    //enable for wireframe representation
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
        definedHeights = false;
        break;
    //wireframe representation
    case 'w':
    case 'W':
        mode++;
        mode = mode % 3;
        break;
    case 'f':
    case 'F':
        isCircle = !isCircle;
        heightmap();
        break;
    case 's':
    case 'S':
        if (shadingFlat)
        {
            shadingFlat = false;
            glShadeModel(GL_SMOOTH);
        }
        else
        {
            shadingFlat = true;
            glShadeModel(GL_FLAT);
        }
        break;
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
    case 'a':
    case 'A':
        if (angleZ > -90)
            angleZ -= 2;
        break;
    case 'd':
    case 'D':
        if (angleZ < 90)
            angleZ += 2;
        break;
    case 't':
    case 'T':
        isQuad = false;
        break;
    case 'y':
    case 'Y':
        isQuad = true;
        break;
    }
}

void special(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        if (angleX > -90)
        angleX -= 2;
        break;
    case GLUT_KEY_DOWN:
        if (angleX < 90)
        angleX += 2;
        break;
    case GLUT_KEY_LEFT:
        if (angleY > -90)
        angleY -= 2;
        break;
    case GLUT_KEY_RIGHT:
        if (angleY < 90)
        angleY += 2;
        break;
    }
    glutPostRedisplay();
}

void FPS(int val)
{
    glutPostRedisplay();
    glutTimerFunc(0, FPS, 0); // 1sec = 1000, 60fps = 1000/60 = ~17
}

void callbackinit()
{
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, FPS, 0);
}

void readme(void)
{
    printf("\nw : Switch mode(1. solid polygons; 2. wireframe; 3. both of them)\n");
    printf("\nl : Enable Lighting \n");
    printf("\nk : Disable Lighting\n");
    printf("\nf : Switch between circle algorithm and fault algorithm\n");
    printf("\ns : Switch between flat shading and Gouraud shading\n");
    printf("\nr : Random Terrain\n");
    printf("\nq : Quit");
    printf("\narrow keys: Rotate model");
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(800, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Terrain");
    callbackinit();

    init();

    readme();

    glutMainLoop();
}
