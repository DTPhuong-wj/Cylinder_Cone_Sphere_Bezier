
#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <cstdio>

const float PI = 3.14159265358979323846f;

// --- State ---
enum Obj { OBJ_CYLINDER=1, OBJ_CONE, OBJ_SPHERE, OBJ_TORUS, OBJ_BEZIER_CURVE, OBJ_BEZIER_SURF };
int currentObj = OBJ_CYLINDER;
bool wireframe = false;
bool autoRotate = true;
bool rotX=false, rotY=true, rotZ=false;
float angleX=0.0f, angleY=0.0f, angleZ=0.0f;

// Mesh storage (positions, indices)
std::vector<float> g_vertices;            // x,y,z triplets
std::vector<unsigned int> g_indices;      // triangles
std::vector<float> g_curve;               // for bezier curve points (x,y,z)
int sphereStacks = 30, sphereSlices = 30;

// Bezier control points (curve)
float bezP[4][3] = {
    {-1.0f, 0.0f, 0.0f},
    {-0.5f, 1.0f, 0.0f},
    {0.5f, -1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f}
};

// Bezier surface control grid (4x4)
float surfP[4][4][3];

// --- Utility helpers ---
inline void clearMesh(){
    g_vertices.clear();
    g_indices.clear();
    g_curve.clear();
}
inline void addVertex(float x,float y,float z){
    g_vertices.push_back(x);
    g_vertices.push_back(y);
    g_vertices.push_back(z);
}
inline void addTri(unsigned int a,unsigned int b,unsigned int c){
    g_indices.push_back(a); g_indices.push_back(b); g_indices.push_back(c);
}

// --- Create objects (parametric) ---
void genCylinder(float radius, float height, int slices){
    clearMesh();
    float half = height*0.5f;
    // side rings (two vertices per slice)
    for(int i=0;i<=slices;i++){
        float a = 2.0f*PI*i / slices;
        float x = radius * cosf(a);
        float y = radius * sinf(a);
        addVertex(x,y,-half); // bottom ring
        addVertex(x,y, half); // top ring
    }
    // triangles for side
    for(int i=0;i<slices;i++){
        unsigned int p0 = i*2;
        unsigned int p1 = p0+1;
        unsigned int p2 = p0+2;
        unsigned int p3 = p0+3;
        addTri(p0,p1,p2);
        addTri(p1,p3,p2);
    }
    // bottom cap center
    unsigned int bottomCenter = g_vertices.size()/3;
    addVertex(0,0,-half);
    for(int i=0;i<slices;i++){
        unsigned int v = i*2;
        unsigned int vnext = ((i+1)%slices)*2;
        addTri(bottomCenter, vnext, v);
    }
    // top cap center
    unsigned int topCenter = g_vertices.size()/3;
    addVertex(0,0,half);
    for(int i=0;i<slices;i++){
        unsigned int v = i*2+1;
        unsigned int vnext = ((i+1)%slices)*2+1;
        addTri(topCenter, v, vnext);
    }
}

void genCone(float radius, float height, int slices){
    clearMesh();
    float half = height*0.5f;
    // apex
    addVertex(0,0,half); // index 0
    // base ring
    for(int i=0;i<=slices;i++){
        float a = 2.0f*PI*i/slices;
        addVertex(radius*cosf(a), radius*sinf(a), -half);
    }
    // sides
    for(int i=0;i<slices;i++){
        addTri(0, i+1, i+2);
    }
    // base center
    unsigned int baseCenter = g_vertices.size()/3;
    addVertex(0,0,-half);
    for(int i=0;i<slices;i++){
        unsigned int v = i+1;
        unsigned int vnext = ((i+1)%slices)+1;
        addTri(baseCenter, vnext, v);
    }
}

void genSphere(float R, int stacks, int slices){
    clearMesh();
    for(int i=0;i<=stacks;i++){
        float phi = PI * i / stacks; // 0..pi
        float z = R * cosf(phi);
        float r = R * sinf(phi);
        for(int j=0;j<=slices;j++){
            float theta = 2.0f*PI * j / slices; // 0..2pi
            float x = r * cosf(theta);
            float y = r * sinf(theta);
            addVertex(x,y,z);
        }
    }
    for(int i=0;i<stacks;i++){
        for(int j=0;j<slices;j++){
            unsigned int a = i*(slices+1) + j;
            unsigned int b = a + (slices+1);
            addTri(a,b,a+1);
            addTri(a+1,b,b+1);
        }
    }
}

void genTorus(float R,float r,int ns,int nt){
    clearMesh();
    for(int i=0;i<=ns;i++){
        float u = 2.0f*PI*i/ns;
        float cu = cosf(u), su = sinf(u);
        for(int j=0;j<=nt;j++){
            float v = 2.0f*PI*j/nt;
            float cv = cosf(v), sv = sinf(v);
            float x = (R + r*cv) * cu;
            float y = (R + r*cv) * su;
            float z = r * sv;
            addVertex(x,y,z);
        }
    }
    for(int i=0;i<ns;i++){
        for(int j=0;j<nt;j++){
            unsigned int a = i*(nt+1) + j;
            unsigned int b = a + (nt+1);
            addTri(a,b,a+1);
            addTri(a+1,b,b+1);
        }
    }
}

// Bezier helpers
float cubicBernstein(int i, float t){
    // i = 0..3
    float u = 1.0f - t;
    if(i==0) return u*u*u;
    if(i==1) return 3.0f * u*u * t;
    if(i==2) return 3.0f * u * t*t;
    return t*t*t;
}
void genBezierCurve(int segments = 100){
    g_curve.clear();
    for(int i=0;i<=segments;i++){
        float t = i/(float)segments;
        float x=0,y=0,z=0;
        for(int k=0;k<4;k++){
            float b = cubicBernstein(k,t);
            x += b * bezP[k][0];
            y += b * bezP[k][1];
            z += b * bezP[k][2];
        }
        g_curve.push_back(x); g_curve.push_back(y); g_curve.push_back(z);
    }
}

// Prepare default surface control points
void prepareSurfControl(){
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            surfP[i][j][0] = (i-1.5f);            // x
            surfP[i][j][1] = 0.5f * sinf(i*j);   // y - small variation
            surfP[i][j][2] = (j-1.5f);            // z
        }
    }
}
void genBezierSurface(int res = 30){
    clearMesh();
    // sample surface
    for(int iu=0; iu<=res; ++iu){
        float u = iu/(float)res;
        for(int iv=0; iv<=res; ++iv){
            float v = iv/(float)res;
            float px=0, py=0, pz=0;
            for(int i=0;i<4;i++){
                float bu = cubicBernstein(i,u);
                for(int j=0;j<4;j++){
                    float bv = cubicBernstein(j,v);
                    px += surfP[i][j][0] * (bu*bv);
                    py += surfP[i][j][1] * (bu*bv);
                    pz += surfP[i][j][2] * (bu*bv);
                }
            }
            addVertex(px,py,pz);
        }
    }
    // indices
    for(int i=0;i<res;i++){
        for(int j=0;j<res;j++){
            unsigned int a = i*(res+1) + j;
            unsigned int b = a + (res+1);
            addTri(a,b,a+1);
            addTri(a+1,b,b+1);
        }
    }
}

// Generate mesh for currently selected object
void generateObject(){
    switch(currentObj){
        case OBJ_CYLINDER: genCylinder(1.0f, 2.0f, 48); break;
        case OBJ_CONE:     genCone(1.0f, 2.0f, 48);     break;
        case OBJ_SPHERE:   genSphere(1.0f, sphereStacks, sphereSlices); break;
        case OBJ_TORUS:    genTorus(1.5f, 0.4f, 48, 32); break;
        case OBJ_BEZIER_CURVE: genBezierCurve(200); break;
        case OBJ_BEZIER_SURF: prepareSurfControl(); genBezierSurface(36); break;
    }
}

// --- Drawing functions (fixed-function immediate-style for simplicity) ---
void drawAxis(){
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    // X - red
    glColor3f(1,0,0);
    glVertex3f(0,0,0); glVertex3f(1.5f,0,0);
    // Y - green
    glColor3f(0,1,0);
    glVertex3f(0,0,0); glVertex3f(0,1.5f,0);
    // Z - blue
    glColor3f(0,0,1);
    glVertex3f(0,0,0); glVertex3f(0,0,1.5f);
    glEnd();
}

void drawMesh(){
    if(currentObj == OBJ_BEZIER_CURVE){
        // draw curve as line strip (white)
        glColor3f(1,1,0.2f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for(size_t i=0;i<g_curve.size(); i+=3){
            glVertex3f(g_curve[i], g_curve[i+1], g_curve[i+2]);
        }
        glEnd();
        return;
    }

    // draw triangles
    glColor3f(0.85f,0.85f,0.85f);
    if(g_indices.empty()){
        // fallback: draw as points
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        for(size_t i=0;i<g_vertices.size(); i+=3) glVertex3f(g_vertices[i],g_vertices[i+1],g_vertices[i+2]);
        glEnd();
        return;
    }

    glBegin(GL_TRIANGLES);
    for(size_t i=0;i<g_indices.size(); i+=3){
        unsigned int a = g_indices[i];
        unsigned int b = g_indices[i+1];
        unsigned int c = g_indices[i+2];
        glVertex3f(g_vertices[a*3+0], g_vertices[a*3+1], g_vertices[a*3+2]);
        glVertex3f(g_vertices[b*3+0], g_vertices[b*3+1], g_vertices[b*3+2]);
        glVertex3f(g_vertices[c*3+0], g_vertices[c*3+1], g_vertices[c*3+2]);
    }
    glEnd();
}

// --- GLUT callbacks ---
void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(4.0, 3.0, 4.0,   0,0,0,   0,1,0);

    // Draw axis (not affected by polygon mode)
    glDisable(GL_LIGHTING);
    drawAxis();

    // object transform
    glPushMatrix();
    glRotatef(angleX,1,0,0);
    glRotatef(angleY,0,1,0);
    glRotatef(angleZ,0,0,1);

    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // optionally enable simple lighting for nicer appearance
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    float lightpos[4] = {3.0f,4.0f,3.0f,1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    float ambient[] = {0.2f,0.2f,0.2f,1.0f};
    float diffuse[] = {0.8f,0.8f,0.8f,1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    drawMesh();

    glDisable(GL_LIGHTING);
    glPopMatrix();

    glutSwapBuffers();
}

void idle(){
    // update rotation
    if(autoRotate){
        if(rotX) angleX += 0.3f;
        if(rotY) angleY += 0.5f;
        if(rotZ) angleZ += 0.2f;
        if(angleX>360) angleX -= 360;
        if(angleY>360) angleY -= 360;
        if(angleZ>360) angleZ -= 360;
    }
    glutPostRedisplay();
}

void reshape(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/(double)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key,int x,int y){
    switch(key){
        case '1': currentObj = OBJ_CYLINDER; generateObject(); break;
        case '2': currentObj = OBJ_CONE;     generateObject(); break;
        case '3': currentObj = OBJ_SPHERE;   generateObject(); break;
        case '4': currentObj = OBJ_TORUS;    generateObject(); break;
        case '5': currentObj = OBJ_BEZIER_CURVE; generateObject(); break;
        case '6': currentObj = OBJ_BEZIER_SURF;  generateObject(); break;
        case 'w': case 'W': wireframe = !wireframe; break;
        case ' ': autoRotate = !autoRotate; break;
        case 'x': case 'X': rotX = !rotX; break;
        case 'y': case 'Y': rotY = !rotY; break;
        case 'z': case 'Z': rotZ = !rotZ; break;
        case 27: exit(0); break; // ESC
    }
}

int main(int argc,char** argv){
    // Init GLUT
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280,720);
    glutCreateWindow("LAB05 - Curves & Surfaces (fixed-function)");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    // initial surface control
    prepareSurfControl();

    // generate first object
    generateObject();

    // callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    // background
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);

    printf("Controls:\n 1..6: select object\n W: wireframe toggle\n Space: toggle auto-rotate\n X/Y/Z: toggle rotation axes\n ESC: exit\n");

    glutMainLoop();
    return 0;
}
