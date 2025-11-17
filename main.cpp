#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/glut.h>
#include <cmath>
#include <vector>

#define M_PI 3.14159265358979323846

struct Point3D { float x, y, z; };

// ---------------- GLOBAL ----------------
float angleX=0.0f, angleY=0.0f, angleZ=0.0f;
bool wireframeMode = true;
bool mushroom = true;

float camX = 0.0f;
float camY = 0.0f;
float camZ = 0.0f;

float zoom = 1.0f;

// ---------------- Cylinder Wireframe ----------------
Point3D P0(float u, float radius){
    return { radius*cosf(u), radius*sinf(u) , 0.0f};
}

Point3D P1(float u, float radius, float height){
    return { radius*cosf(u), radius*sinf(u), height};
}

void DrawCylinderWireframe(float radius, float height, int slices, int stacks){
    float du = 2*M_PI/slices, dv = 1.0f/stacks;

    for(int j=0;j<=stacks;j++){
        float v = j * dv;
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<slices;i++){
            float u = i*du;
            Point3D p0 = P0(u,radius);
            Point3D p1 = P1(u,radius,height);
            Point3D p { 
                p0.x*(1-v) + p1.x*v,
                p0.y*(1-v) + p1.y*v,
                p0.z*(1-v) + p1.z*v
            };
            glVertex3f(p.x,p.y,p.z);
        }
        glEnd();
    }

    for(int i=0;i<slices;i++){
        float u = i*du;
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float v = j * dv;
            Point3D p0 = P0(u,radius);
            Point3D p1 = P1(u,radius,height);
            Point3D p { 
                p0.x*(1-v) + p1.x*v,
                p0.y*(1-v) + p1.y*v,
                p0.z*(1-v) + p1.z*v
            };
            glVertex3f(p.x,p.y,p.z);
        }

        glEnd();
    }

    // 3. Vẽ đáy (bottom cap wireframe với tâm)
    glBegin(GL_LINES);
    Point3D center{0.0f, 0.0f, 0.0f}; // tâm đáy
    for(int i=0;i<slices;i++){
        float u = i * du;
        Point3D p = P0(u,radius); // điểm trên vòng tròn
        // vẽ đường từ tâm ra đỉnh
        glVertex3f(center.x, center.y, center.z);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    // 4. Vẽ nắp (top cap wireframe với tâm)
    glBegin(GL_LINES);
    Point3D topCenter{0.0f, 0.0f, height}; // tâm nắp
    for(int i=0;i<slices;i++){
        float u = i * du;
        Point3D p1 = P1(u,radius,height); // điểm trên vòng tròn
        // vẽ đường từ tâm ra đỉnh
        glVertex3f(topCenter.x, topCenter.y, topCenter.z);
        glVertex3f(p1.x, p1.y, p1.z);
    }
    glEnd();

}

void DrawCylinder(float radius, float height, int slices, int stacks){
    float du = 2*M_PI / slices;
    float dv = 1.0f / stacks;

    for(int i=0;i<slices;i++){
        float u0=i*du, u1=(i+1)*du;

        glBegin(GL_QUAD_STRIP);
        for(int j=0;j<=stacks;j++){
            float v=j*dv;

            Point3D b0=P0(u0,radius), t0=P1(u0,radius,height);
            glVertex3f(b0.x*(1-v)+t0.x*v, b0.y*(1-v)+t0.y*v, b0.z*(1-v)+t0.z*v);

            Point3D b1=P0(u1,radius), t1=P1(u1,radius,height);
            glVertex3f(b1.x*(1-v)+t1.x*v, b1.y*(1-v)+t1.y*v, b1.z*(1-v)+t1.z*v);
        }
        glEnd();
    }
}

// ---------------- Sphere Wireframe ----------------
void DrawSphereWireframe(float r, int slices, int stacks){
    for(int i=0;i<slices;i++){
        float t0 = i*M_PI/slices;
        float t1 = (i+1)*M_PI/slices;

        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float phi=j*2*M_PI/stacks;
            glVertex3f(r*sin(t0)*cos(phi), r*sin(t0)*sin(phi), r*cos(t0));
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float phi=j*2*M_PI/stacks;
            glVertex3f(r*sin(t1)*cos(phi), r*sin(t1)*sin(phi), r*cos(t1));
        }
        glEnd();
    }
}

// ---------------- Sphere Solid ----------------
void DrawSphere(float r, int slices, int stacks){
    for(int i=0;i<slices;i++){
        float t0=i*M_PI/slices;
        float t1=(i+1)*M_PI/slices;

        glBegin(GL_QUAD_STRIP);
        for(int j=0;j<=stacks;j++){
            float phi=j*2*M_PI/stacks;

            glVertex3f(r*sin(t0)*cos(phi), r*sin(t0)*sin(phi), r*cos(t0));
            glVertex3f(r*sin(t1)*cos(phi), r*sin(t1)*sin(phi), r*cos(t1));
        }
        glEnd();
    }
}

// ---------------- Ellipsoid Wireframe ----------------
void DrawEllipsoidWire(float a, float b, float c, int slices, int stacks){
    for(int i=0;i<=slices;i++){
        float t = i*M_PI/slices;
        glBegin(GL_LINE_LOOP);
        for(int j=0;j<=stacks;j++){
            float p = j * 2*M_PI/stacks;
            glVertex3f(a*sin(t)*cos(p), c*sin(t)*sin(p), b*cos(t));
        }
        glEnd();
    }
}

// ---------------- Ellipsoid Solid ----------------
void DrawEllipsoid(float a, float b, float c, int slices, int stacks){
    for(int i=0;i<slices;i++){
        float t0=i*M_PI/slices;
        float t1=(i+1)*M_PI/slices;

        glBegin(GL_QUAD_STRIP);
        for(int j=0;j<=stacks;j++){
            float p=j*2*M_PI/stacks;
            glVertex3f(a*sin(t0)*cos(p), c*sin(t0)*sin(p), b*cos(t0));
            glVertex3f(a*sin(t1)*cos(p), c*sin(t1)*sin(p), b*cos(t1));
        }
        glEnd();
    }
}

// Tổ hợp
int C(int n,int i){
    if(i==0||i==n) return 1;
    int res=1;
    for(int j=1;j<=i;j++) res=res*(n-j+1)/j;
    return res;
}

// Bézier Curve
Point3D BezierCurvePoint(const std::vector<Point3D>& ctrl,float t){
    int n = ctrl.size()-1;
    Point3D p{0,0,0};
    for(int i=0;i<=n;i++){
        float b = C(n,i) * pow(1-t,n-i) * pow(t,i);
        p.x += ctrl[i].x * b;
        p.y += ctrl[i].y * b;
        p.z += ctrl[i].z * b;
    }
    return p;
}

void DrawBezierCurve(const std::vector<Point3D>& ctrl,int steps){
    glLineWidth(3);  // gân lá dày hơn
    glColor3f(0.55f,0.27f,0.07f); // nâu
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=steps;i++){
        float t=i/(float)steps;
        Point3D p = BezierCurvePoint(ctrl,t);
        glVertex3f(p.x,p.y,p.z);
    }
    glEnd();
    glLineWidth(1);
}

// Bézier Surface
Point3D BezierSurfacePoint(const std::vector<std::vector<Point3D>>& ctrl,float u,float v){
    int m = ctrl.size()-1;
    int n = ctrl[0].size()-1;
    Point3D p{0,0,0};
    for(int i=0;i<=m;i++){
        for(int j=0;j<=n;j++){
            float b = C(m,i)*C(n,j) * pow(1-u,m-i)*pow(u,i) * pow(1-v,n-j)*pow(v,j);
            p.x += ctrl[i][j].x * b;
            p.y += ctrl[i][j].y * b;
            p.z += ctrl[i][j].z * b;
        }
    }
    return p;
}

void DrawBezierSurfaceWireframe(const std::vector<std::vector<Point3D>>& ctrl,int slices,int stacks){
    glColor3f(0.2f,0.8f,0.2f); // xanh nhạt
    for(int i=0;i<slices;i++){
        float u0 = i/(float)slices;
        float u1 = (i+1)/(float)slices;
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=stacks;j++){
            float v = j/(float)stacks;
            Point3D p0 = BezierSurfacePoint(ctrl,u0,v);
            Point3D p1 = BezierSurfacePoint(ctrl,u1,v);
            glVertex3f(p0.x,p0.y,p0.z);
            glVertex3f(p1.x,p1.y,p1.z);
        }
        glEnd();
    }
}

// Scene: lá nghiêng ngang
float angle=0.0f;

void DrawLeafScene(float x, float y, float z){
    // 1. Bézier Surface – lá nghiêng nhiều
    std::vector<std::vector<Point3D>> leafCtrl = {
        { {-0.2f,0,0}, {-0.15f,0.05f,0.05f}, {0.15f,0.05f,-0.05f}, {0.2f,0,0} },
        { {-0.3f,0.3f,0.1f}, {-0.15f,0.4f,0.15f}, {0.15f,0.4f,-0.15f}, {0.3f,0.3f,-0.1f} },
        { {-0.25f,0.6f,0.15f}, {-0.1f,0.65f,0.05f}, {0.1f,0.65f,-0.05f}, {0.25f,0.6f,-0.15f} },
        { {-0.15f,1.0f,0.05f}, {0,1.0f,0.05f}, {0,1.0f,-0.05f}, {0.15f,1.0f,-0.05f} }
    };

    // Nghiêng lá ngang theo trục X
    glPushMatrix();
    glRotatef(-45,1,0,0);  // lá nghiêng -45° quanh X
    DrawBezierSurfaceWireframe(leafCtrl,25,25);

    // 2. Bézier Curve – gân lá
    std::vector<Point3D> midVein = {
        {0,0,0}, {0,0.3,0.05}, {0,0.6,-0.05}, {0,1.0,0}
    };
    DrawBezierCurve(midVein,50);
    glPopMatrix();
}


// ---------------- Mushroom ----------------
void DrawMushroom(float x,float y,float z){
    glPushMatrix();
    glTranslatef(0.5f, 0.5f, 0.0f);

    // Thân
    glColor3f(0.8f,0.5f,0.3f);
    if(wireframeMode) DrawCylinderWireframe(0.2f,1.0f,18,5);
    else              DrawCylinder(0.2f,1.0f,36,10);

    // Mũ
    glTranslatef(0, 0, 1.0f);
    glPushMatrix();
    glScalef(1,1, 0.5f); 
    glColor3f(1,0,0);

    if(wireframeMode) DrawEllipsoidWire(0.6f,0.3f,0.6f,18,9);
    else              DrawEllipsoid(0.6f,0.3f,0.6f,36,18);

    glPopMatrix();

    // Chấm trắng
    glColor3f(1,1,1);
    std::vector<Point3D> dots = {
        {0.2f,0.15f,0.0f}, {-0.2f,0.15f,0.1f},
        {0.0f,0.15f,-0.2f}, {0.1f,0.15f,0.3f},
        {-0.3f,0.15f,-0.1f}
    };

    for(auto& d : dots){
        glPushMatrix();
        glTranslatef(d.x,d.y,d.z);
        if(wireframeMode) DrawSphereWireframe(0.06f,8,8);
        else              DrawSphere(0.06f,12,12);
        glPopMatrix();
    }

    glPopMatrix();
}

// ---------------- Axes ----------------
void DrawAxes(float len=2){
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(len,0,0);
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,len,0);
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,len);

    glEnd();
}

// ---------------- DISPLAY ----------------
void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(4*zoom,2*zoom,4*zoom, 0,0.5,0, 0,0,1);

    glTranslatef(camX, camY, camZ);

    glRotatef(angleX,1,0,0);
    glRotatef(angleY,0,1,0);
    glRotatef(angleZ,0,0,1);

    DrawAxes();

    if(mushroom) DrawMushroom(1,0,1);
    else         DrawLeafScene(1,0,1);

    glutSwapBuffers();
}

// ---------------- KEYBOARD ----------------
void keyboard(unsigned char key,int,int){
    switch(key){
        case 't': case 'T': wireframeMode = !wireframeMode; break;
        case 'm': case 'M': mushroom = !mushroom; break;

        case 'x': angleX+=5; break;
        case 'X': angleX-=5; break;

        case 'y': angleY+=5; break;
        case 'Y': angleY-=5; break;

        case 'z': angleZ+=5; break;
        case 'Z': angleZ-=5; break;

        case '+': zoom -= 0.1f; if(zoom<0.2f) zoom=0.2f; break;
        case '-': zoom += 0.1f; break;

        case 'a': camX -= 0.1f; break;
        case 'd': camX += 0.1f; break;
        case 'w': camZ -= 0.1f; break;
        case 's': camZ += 0.1f; break;
    }
    glutPostRedisplay();
}

// ---------------- SPECIAL KEYS (Arrow keys) ----------------
void specialKeys(int key, int, int){
    switch(key){
        case GLUT_KEY_LEFT:  angleY -= 5; break;
        case GLUT_KEY_RIGHT: angleY += 5; break;
        case GLUT_KEY_UP:    angleX -= 5; break;
        case GLUT_KEY_DOWN:  angleX += 5; break;
    }
    glutPostRedisplay();
}

// ---------------- MOUSE ZOOM (FreeGLUT) ----------------
void mouse(int button, int state, int x, int y){
    if(state != GLUT_DOWN) return;

    if(button == 3) { // scroll up
        zoom -= 0.1f;
        if(zoom < 0.2f) zoom = 0.2f;
    } 
    else if(button == 4){ // scroll down
        zoom += 0.1f;
    }

    glutPostRedisplay();
}


// ---------------- MAIN ----------------
void reshape(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,(float)w/h,0.1,100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Mushroom - Move + Rotate + Zoom");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);    

    glutMainLoop();
    return 0;
}
