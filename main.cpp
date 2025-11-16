#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/glut.h>
#include <cmath>
#include <vector>

#define M_PI 3.14159265358979323846

struct Point3D { float x, y, z; };

// Góc xoay
float angleX=0.0f, angleY=0.0f, angleZ=0.0f;

// ---------------- Cylinder ----------------
Point3D P0(float u, float radius) { Point3D p; p.x=radius*cos(u); p.y=0; p.z=radius*sin(u); return p; }
Point3D P1(float u, float radius, float height) { Point3D p; p.x=radius*cos(u); p.y=height; p.z=radius*sin(u); return p; }

void DrawCylinderWireframe(float radius, float height, int slices, int stacks){
    float Delta_U = 2*M_PI/slices, Delta_V = 1.0f/stacks;
    
    // Vòng tròn ngang
    for(int j=0;j<=stacks;j++){
        float v=j*Delta_V;
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<slices;i++){
            float u = i*Delta_U;
            Point3D p_bottom = P0(u,radius);
            Point3D p_top = P1(u,radius,height);
            Point3D p;
            p.x = p_bottom.x*(1-v)+p_top.x*v;
            p.y = p_bottom.y*(1-v)+p_top.y*v;
            p.z = p_bottom.z*(1-v)+p_top.z*v;
            glVertex3f(p.x,p.y,p.z);
        }
        glEnd();
    }

    // Đường dọc
    for(int i=0;i<slices;i++){
        float u = i*Delta_U;
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float v = j*Delta_V;
            Point3D p_bottom = P0(u,radius);
            Point3D p_top = P1(u,radius,height);
            Point3D p;
            p.x = p_bottom.x*(1-v)+p_top.x*v;
            p.y = p_bottom.y*(1-v)+p_top.y*v;
            p.z = p_bottom.z*(1-v)+p_top.z*v;
            glVertex3f(p.x,p.y,p.z);
        }
        glEnd();
    }
}

// ---------------- Sphere wireframe ----------------
void DrawSphereWireframe(float radius, int slices, int stacks){
    for(int i=0;i<slices;i++){
        float theta0 = i*M_PI/slices, theta1=(i+1)*M_PI/slices;
        // Vòng ngang
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float phi = j*2*M_PI/stacks;
            float x = radius*sin(theta0)*cos(phi), y = radius*cos(theta0), z = radius*sin(theta0)*sin(phi);
            glVertex3f(x,y,z);
        }
        glEnd();

        // Vòng dọc
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float phi = j*2*M_PI/stacks;
            float x = radius*sin(theta1)*cos(phi), y = radius*cos(theta1), z = radius*sin(theta1)*sin(phi);
            glVertex3f(x,y,z);
        }
        glEnd();
    }
}

// ---------------- Mushroom wireframe ----------------
void DrawMushroomWireframe(float x, float y, float z){
    glPushMatrix();
     glTranslatef(x, y, z);
    glColor3f(1.0f,1.0f,1.0f);

    // Thân nấm
    DrawCylinderWireframe(0.2f,1.0f,18,5);

    // Mũ nấm
    glTranslatef(0,1.0f,0);
    glPushMatrix();
    glScalef(1.0f,0.5f,1.0f);
    DrawSphereWireframe(0.6f,18,9);
    glPopMatrix();

    // Chấm trắng trên mũ
    std::vector<Point3D> dots = {
        {0.2f,0.25f,0.0f}, {-0.2f,0.22f,0.1f}, {0.0f,0.23f,-0.2f}, {0.1f,0.21f,0.3f}, {-0.3f,0.24f,-0.1f}
    };
    for(auto &d: dots){
        glPushMatrix();
        glTranslatef(d.x,d.y,d.z);
        DrawSphereWireframe(0.05f,8,8);
        glPopMatrix();
    }

    glPopMatrix();
}

// ---------------- Axes ----------------
void DrawAxes(float length=2.0f){
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // X đỏ
    glColor3f(1,0,0);
    glVertex3f(0,0,0); glVertex3f(length,0,0);
    // Y xanh lá
    glColor3f(0,1,0);
    glVertex3f(0,0,0); glVertex3f(0,length,0);
    // Z xanh dương
    glColor3f(0,0,1);
    glVertex3f(0,0,0); glVertex3f(0,0,length);
    glEnd();
    glLineWidth(1.0f);
}

// ---------------- Display ----------------
void display(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(4,2,4,0,0.5,0,0,1,0);

    // Xoay theo bàn phím
    glRotatef(angleX,1,0,0);
    glRotatef(angleY,0,1,0);
    glRotatef(angleZ,0,0,1);

    DrawAxes(2.0f);
    DrawMushroomWireframe(1,0,2);


    glutSwapBuffers();
}

// ---------------- Keyboard ----------------
void keyboard(unsigned char key,int,int){
    switch(key){
        case 'x': angleX+=5; break;
        case 'X': angleX-=5; break;
        case 'y': angleY+=5; break;
        case 'Y': angleY-=5; break;
        case 'z': angleZ+=5; break;
        case 'Z': angleZ-=5; break;
    }
    glutPostRedisplay();
}

// ---------------- Reshape ----------------
void reshape(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,(float)w/h,0.1,100.0);
    glMatrixMode(GL_MODELVIEW);
}

// ---------------- Main ----------------
int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Mushroom Wireframe with Axes & Grid");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
