#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <cstdio>

const float PI = 3.14159265358979323846f;

enum Obj { OBJ_CYLINDER=1, OBJ_CONE, OBJ_SPHERE, OBJ_TORUS, OBJ_BEZIER_CURVE, OBJ_BEZIER_SURF };
int currentObj = OBJ_CYLINDER;
bool wireframe = false;
float angleX=0.0f, angleY=0.0f, angleZ=0.0f;

bool mouseLeftDown = false;
int lastMouseX = 0, lastMouseY = 0;
float camDist = 5.0f;

std::vector<float> g_vertices;            
std::vector<unsigned int> g_indices;      
std::vector<float> g_curve;               
int sphereStacks = 30, sphereSlices = 30;

float bezP[4][3] = {
    {-1.0f, 0.0f, 0.0f},
    {-0.5f, 1.0f, 0.0f},
    {0.5f, -1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f}
};

float surfP[4][4][3];

inline void clearMesh(){ g_vertices.clear(); g_indices.clear(); g_curve.clear(); }
inline void addVertex(float x,float y,float z){ g_vertices.push_back(x); g_vertices.push_back(y); g_vertices.push_back(z); }
inline void addTri(unsigned int a,unsigned int b,unsigned int c){ g_indices.push_back(a); g_indices.push_back(b); g_indices.push_back(c); }

// --- Cylinder ---
void genCylinder(float radius, float height, int slices){
    clearMesh();
    float half = height*0.5f;
    for(int i=0;i<=slices;i++){
        float a = 2.0f*PI*i / slices;
        float x = radius * cosf(a);
        float y = radius * sinf(a);
        addVertex(x,y,-half); 
        addVertex(x,y, half); 
    }
    for(int i=0;i<slices;i++){
        unsigned int p0=i*2,p1=p0+1,p2=p0+2,p3=p0+3;
        addTri(p0,p1,p2); addTri(p1,p3,p2);
    }
    unsigned int bottomCenter = g_vertices.size()/3; addVertex(0,0,-half);
    for(int i=0;i<slices;i++){
        unsigned int v=i*2,vnext=((i+1)%slices)*2;
        addTri(bottomCenter,vnext,v);
    }
    unsigned int topCenter = g_vertices.size()/3; addVertex(0,0,half);
    for(int i=0;i<slices;i++){
        unsigned int v=i*2+1,vnext=((i+1)%slices)*2+1;
        addTri(topCenter,v,vnext);
    }
}

void genCone(float radius, float height, int slices){
    clearMesh();
    float half = height*0.5f;
    addVertex(0,0,half); 
    for(int i=0;i<=slices;i++){
        float a=2.0f*PI*i/slices;
        addVertex(radius*cosf(a), radius*sinf(a), -half);
    }
    for(int i=0;i<slices;i++) addTri(0,i+1,i+2);
    unsigned int baseCenter=g_vertices.size()/3; addVertex(0,0,-half);
    for(int i=0;i<slices;i++){
        unsigned int v=i+1,vnext=((i+1)%slices)+1;
        addTri(baseCenter,vnext,v);
    }
}

void genSphere(float R,int stacks,int slices){
    clearMesh();
    for(int i=0;i<=stacks;i++){
        float phi=PI*i/stacks;
        float z=R*cosf(phi);
        float r=R*sinf(phi);
        for(int j=0;j<=slices;j++){
            float theta=2.0f*PI*j/slices;
            float x=r*cosf(theta); float y=r*sinf(theta);
            addVertex(x,y,z);
        }
    }
    for(int i=0;i<stacks;i++)
        for(int j=0;j<slices;j++){
            unsigned int a=i*(slices+1)+j,b=a+(slices+1);
            addTri(a,b,a+1); addTri(a+1,b,b+1);
        }
}

void genTorus(float R,float r,int ns,int nt){
    clearMesh();
    for(int i=0;i<=ns;i++){
        float u=2.0f*PI*i/ns; float cu=cosf(u),su=sinf(u);
        for(int j=0;j<=nt;j++){
            float v=2.0f*PI*j/nt; float cv=cosf(v),sv=sinf(v);
            float x=(R+r*cv)*cu; float y=(R+r*cv)*su; float z=r*sv;
            addVertex(x,y,z);
        }
    }
    for(int i=0;i<ns;i++)
        for(int j=0;j<nt;j++){
            unsigned int a=i*(nt+1)+j,b=a+(nt+1);
            addTri(a,b,a+1); addTri(a+1,b,b+1);
        }
}

// Bezier
float cubicBernstein(int i,float t){
    float u=1.0f-t;
    if(i==0) return u*u*u;
    if(i==1) return 3.0f*u*u*t;
    if(i==2) return 3.0f*u*t*t;
    return t*t*t;
}

void genBezierCurve(int segments=100){
    g_curve.clear();
    for(int i=0;i<=segments;i++){
        float t=i/(float)segments,x=0,y=0,z=0;
        for(int k=0;k<4;k++){
            float b=cubicBernstein(k,t);
            x+=b*bezP[k][0]; y+=b*bezP[k][1]; z+=b*bezP[k][2];
        }
        g_curve.push_back(x); g_curve.push_back(y); g_curve.push_back(z);
    }
}

void prepareSurfControl(){
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++){
            surfP[i][j][0]=(i-1.5f);
            surfP[i][j][1]=0.5f*sinf(i*j);
            surfP[i][j][2]=(j-1.5f);
        }
}

void genBezierSurface(int res=30){
    clearMesh();
    for(int iu=0;iu<=res;iu++){
        float u=iu/(float)res;
        for(int iv=0;iv<=res;iv++){
            float v=iv/(float)res;
            float px=0,py=0,pz=0;
            for(int i=0;i<4;i++){
                float bu=cubicBernstein(i,u);
                for(int j=0;j<4;j++){
                    float bv=cubicBernstein(j,v);
                    px+=surfP[i][j][0]*(bu*bv);
                    py+=surfP[i][j][1]*(bu*bv);
                    pz+=surfP[i][j][2]*(bu*bv);
                }
            }
            addVertex(px,py,pz);
        }
    }
    for(int i=0;i<res;i++)
        for(int j=0;j<res;j++){
            unsigned int a=i*(res+1)+j,b=a+(res+1);
            addTri(a,b,a+1); addTri(a+1,b,b+1);
        }
}

void generateObject(){
    switch(currentObj){
        case OBJ_CYLINDER: genCylinder(1.0f,2.0f,48); break;
        case OBJ_CONE: genCone(1.0f,2.0f,48); break;
        case OBJ_SPHERE: genSphere(1.0f,sphereStacks,sphereSlices); break;
        case OBJ_TORUS: genTorus(1.5f,0.4f,48,32); break;
        case OBJ_BEZIER_CURVE: genBezierCurve(200); break;
        case OBJ_BEZIER_SURF: prepareSurfControl(); genBezierSurface(50); break;
    }
}

// --- Drawing ---
void drawAxis(){
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1.5f,0,0);
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1.5f,0);
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1.5f);
    glEnd();
}

void drawMesh(){
    // --- BEZIER CURVE ---
    if(currentObj==OBJ_BEZIER_CURVE){
        glColor3f(1,1,0.2f); glLineWidth(2.0f); glBegin(GL_LINE_STRIP);
        for(size_t i=0;i<g_curve.size();i+=3) glVertex3f(g_curve[i],g_curve[i+1],g_curve[i+2]);
        glEnd();
        glColor3f(1,0,0); glPointSize(8.0f); glBegin(GL_POINTS);
        for(int i=0;i<4;i++) glVertex3f(bezP[i][0],bezP[i][1],bezP[i][2]);
        glEnd();
        glColor3f(0.5f,0.5f,0.5f); glLineWidth(1.0f); glBegin(GL_LINE_STRIP);
        for(int i=0;i<4;i++) glVertex3f(bezP[i][0],bezP[i][1],bezP[i][2]);
        glEnd();
        return;
    }

    // --- BEZIER SURFACE ---
    if(currentObj==OBJ_BEZIER_SURF){
        glColor3f(0.85f,0.85f,0.85f);
        if(!g_indices.empty()){
            glBegin(GL_TRIANGLES);
            for(size_t i=0;i<g_indices.size();i+=3){
                unsigned int a=g_indices[i],b=g_indices[i+1],c=g_indices[i+2];
                glVertex3f(g_vertices[a*3+0],g_vertices[a*3+1],g_vertices[a*3+2]);
                glVertex3f(g_vertices[b*3+0],g_vertices[b*3+1],g_vertices[b*3+2]);
                glVertex3f(g_vertices[c*3+0],g_vertices[c*3+1],g_vertices[c*3+2]);
            }
            glEnd();
        }
        glColor3f(1,0,0); glPointSize(6.0f); glBegin(GL_POINTS);
        for(int i=0;i<4;i++) for(int j=0;j<4;j++) glVertex3f(surfP[i][j][0],surfP[i][j][1],surfP[i][j][2]);
        glEnd();
        glColor3f(0.5f,0.5f,0.5f); glLineWidth(1.0f);
        for(int i=0;i<4;i++){ glBegin(GL_LINE_STRIP); for(int j=0;j<4;j++) glVertex3f(surfP[i][j][0],surfP[i][j][1],surfP[i][j][2]); glEnd(); }
        for(int j=0;j<4;j++){ glBegin(GL_LINE_STRIP); for(int i=0;i<4;i++) glVertex3f(surfP[i][j][0],surfP[i][j][1],surfP[i][j][2]); glEnd(); }
        return;
    }

    // --- Normal mesh ---
    switch(currentObj){
        case OBJ_CYLINDER: glColor3f(1.0f,0.5f,0.0f); break;
        case OBJ_CONE: glColor3f(0.8f,0.0f,0.0f); break;
        case OBJ_SPHERE: glColor3f(0.0f,0.7f,0.2f); break;
        case OBJ_TORUS: glColor3f(0.0f,0.5f,1.0f); break;
        default: glColor3f(0.85f,0.85f,0.85f); break;
    }

    if(g_indices.empty()){
        glPointSize(3.0f); glBegin(GL_POINTS);
        for(size_t i=0;i<g_vertices.size();i+=3) glVertex3f(g_vertices[i],g_vertices[i+1],g_vertices[i+2]);
        glEnd(); return;
    }

    glBegin(GL_TRIANGLES);
    for(size_t i=0;i<g_indices.size();i+=3){
        unsigned int a=g_indices[i],b=g_indices[i+1],c=g_indices[i+2];
        glVertex3f(g_vertices[a*3+0],g_vertices[a*3+1],g_vertices[a*3+2]);
        glVertex3f(g_vertices[b*3+0],g_vertices[b*3+1],g_vertices[b*3+2]);
        glVertex3f(g_vertices[c*3+0],g_vertices[c*3+1],g_vertices[c*3+2]);
    }
    glEnd();
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camDist,camDist*0.75f,camDist, 0,0,0, 0,1,0);

    glDisable(GL_LIGHTING); drawAxis();

    glPushMatrix();
    glRotatef(angleX,1,0,0); glRotatef(angleY,0,1,0); glRotatef(angleZ,0,0,1);

    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    float lightpos[4]={3.0f,4.0f,3.0f,1.0f};
    glLightfv(GL_LIGHT0,GL_POSITION,lightpos);
    float ambient[]={0.2f,0.2f,0.2f,1.0f}, diffuse[]={0.8f,0.8f,0.8f,1.0f};
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambient); glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);

    drawMesh();

    glDisable(GL_LIGHTING);
    glPopMatrix();

    glutSwapBuffers();
}

void idle(){ glutPostRedisplay(); }
void reshape(int w,int h){ glViewport(0,0,w,h); glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(45.0,(double)w/(double)h,0.1,100.0); glMatrixMode(GL_MODELVIEW); }

void keyboard(unsigned char key,int x,int y){
    switch(key){
        case '1': currentObj=OBJ_CYLINDER; generateObject(); break;
        case '2': currentObj=OBJ_CONE; generateObject(); break;
        case '3': currentObj=OBJ_SPHERE; generateObject(); break;
        case '4': currentObj=OBJ_TORUS; generateObject(); break;
        case '5': currentObj=OBJ_BEZIER_CURVE; generateObject(); break;
        case '6': currentObj=OBJ_BEZIER_SURF; generateObject(); break;
        case 'w': case 'W': wireframe=!wireframe; break;
        case 'x': angleX+=5.0f; break;
        case 'X': angleX-=5.0f; break;
        case 'y': angleY+=5.0f; break;
        case 'Y': angleY-=5.0f; break;
        case 'z': angleZ+=5.0f; break;
        case 'Z': angleZ-=5.0f; break;
        case 27: exit(0); break;
    }
}

void mouseButton(int button,int state,int x,int y){
    if(button==GLUT_LEFT_BUTTON){ mouseLeftDown=(state==GLUT_DOWN); lastMouseX=x; lastMouseY=y; }
    if(button==3){ camDist-=0.5f; if(camDist<2.0f) camDist=2.0f; }
    if(button==4){ camDist+=0.5f; if(camDist>20.0f) camDist=20.0f; }
}

void mouseMotion(int x,int y){
    if(mouseLeftDown){
        int dx=x-lastMouseX, dy=y-lastMouseY;
        angleY+=dx*0.5f; angleX+=dy*0.5f;
        lastMouseX=x; lastMouseY=y;
        glutPostRedisplay();
    }
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(600,600);
    glutCreateWindow("LAB05 - Curves & Surfaces (Keyboard + Mouse)");

    glEnable(GL_DEPTH_TEST); glEnable(GL_NORMALIZE); glShadeModel(GL_SMOOTH);
    prepareSurfControl(); generateObject();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glClearColor(0.12f,0.12f,0.12f,1.0f);

    printf("Controls:\n 1..6: select object\n W: wireframe toggle\n X/x,Y/y,Z/z: rotate\nMouse drag: rotate\nScroll: zoom\nESC: exit\n");

    glutMainLoop();
    return 0;
}
