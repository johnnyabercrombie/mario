/* Base code for texture mapping lab */
/* includes three images and three meshes - Z. Wood 2016 */
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include <BulletDynamics/btBulletDynamicsCommon.h>
#include "btBulletDynamicsCommon.h"

/* to use glee */
#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "glee.hpp"

using namespace std;
using namespace glm;

struct LightSource {
    vec3 intensity;
    vec3 pos;
};

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> marioProg;
shared_ptr<Program> worldProg;
vector<shared_ptr<Shape>> mario;
vector<shared_ptr<Shape>> thwomp;
vector<shared_ptr<Shape>> whomp;
vector<shared_ptr<Shape>> world;
shared_ptr<LightSource> light;

int g_GiboLen;
int g_width, g_height;
float g_rotate = 0; // was -1.5
float g_scale = 1;
vec3 marioPos;
float marioRot = 90;
vec3 g_trans;
vec3 eye, target, upV;
vector<vec3> thwompIdxs, whompIdxs;
vector<float> thwompYaws, whompYaws;
float cspeed = 0.2;
float phi, theta;
mat4 view;

//global data for ground plane
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    vec3 camView = target - eye;
    vec3 w = vec3(-1) * normalize(camView);
    vec3 u = normalize(cross(upV, w));
    
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
        light->pos.x -= 1;
    } else if(key == GLFW_KEY_E && action == GLFW_PRESS) {
        light->pos.x += 1;
    } else if (key == GLFW_KEY_A) {
        int state1 = glfwGetKey(window, GLFW_KEY_W);
        if (state1 == GLFW_PRESS) {
            cout << "go diag1" << endl;
            eye.x += cspeed * (-1 * w.x);
            target.x += cspeed * (-1 * w.x);
            eye.z += cspeed * (-1 * w.z);
            target.z += cspeed * (-1 * w.z);
        }
        int state2 = glfwGetKey(window, GLFW_KEY_S);
        if (state2 == GLFW_PRESS) {
            cout << "go diag1" << endl;
            eye.x += cspeed * w.x;
            target.x += cspeed * w.x;
            eye.z += cspeed * w.z;
            target.z += cspeed * w.z;
        }
        eye.x += cspeed * (-1 * u.x);
        target.x += cspeed * (-1 * u.x);
        eye.z += cspeed * (-1 * u.z);
        target.z += cspeed * (-1 * u.z);
    } else if (key == GLFW_KEY_D) {
        cout << "d pressed" << endl;
        int state1 = glfwGetKey(window, GLFW_KEY_W);
        if (state1 == GLFW_PRESS) {
            cout << "go diag12" << endl;
            eye.x += cspeed * (-1 * w.x);
            target.x += cspeed * (-1 * w.x);
            eye.z += cspeed * (-1 * w.z);
            target.z += cspeed * (-1 * w.z);
        }
        int state2 = glfwGetKey(window, GLFW_KEY_S);
        if (state2 == GLFW_PRESS) {
            cout << "go diag13" << endl;
            eye.x += cspeed * w.x;
            target.x += cspeed * w.x;
            eye.z += cspeed * w.z;
            target.z += cspeed * w.z;
        }
        eye.x += cspeed * u.x;
        target.x += cspeed * u.x;
        eye.z += cspeed * u.z;
        target.z += cspeed * u.z;
    } else if (key == GLFW_KEY_W) {
        int state1 = glfwGetKey(window, GLFW_KEY_D);
        if (state1 == GLFW_PRESS) {
            cout << "go diag2" << endl;
            eye.x += cspeed * u.x;
            target.x += cspeed * u.x;
            eye.z += cspeed * u.z;
            target.z += cspeed * u.z;
        }
        int state2 = glfwGetKey(window, GLFW_KEY_A);
        if (state2 == GLFW_PRESS) {
            cout << "go diag3" << endl;
            eye.x += cspeed * (-1 * u.x);
            target.x += cspeed * (-1 * u.x);
            eye.z += cspeed * (-1 * u.z);
            target.z += cspeed * (-1 * u.z);
        }
        marioPos.x += 400 * cspeed;
        
        eye.x += cspeed * (-1 * w.x);
        target.x += cspeed * (-1 * w.x);
        eye.z += cspeed * (-1 * w.z);
        target.z += cspeed * (-1 * w.z);
    } else if (key == GLFW_KEY_S) {
        int state1 = glfwGetKey(window, GLFW_KEY_D);
        if (state1 == GLFW_PRESS) {
            cout << "go diag2" << endl;
            eye.x += cspeed * u.x;
            target.x += cspeed * u.x;
            eye.z += cspeed * u.z;
            target.z += cspeed * u.z;
        }
        int state2 = glfwGetKey(window, GLFW_KEY_A);
        if (state2 == GLFW_PRESS) {
            cout << "go diag3" << endl;
            eye.x += cspeed * (-1 * u.x);
            target.x += cspeed * (-1 * u.x);
            eye.z += cspeed * (-1 * u.z);
            target.z += cspeed * (-1 * u.z);
        }
        eye.x += cspeed * w.x;
        target.x += cspeed * w.x;
        eye.z += cspeed * w.z;
        target.z += cspeed * w.z;
    }
    
    view = glm::lookAt(eye, target, upV);
}

float p2wx(double in_x, float left) {
	float c = (-2*left)/(g_width-1.0);
	float d = left;
    return c*in_x+d;
}

float p2wy(double in_y, float bot) {
	//flip y
  	in_y = g_height -in_y;
	float e = (-2*bot)/(g_height-1.0);
	float f = bot;
    return e*in_y + f;
}

static void resize_callback(GLFWwindow *window, int width, int height) {
	g_width = width;
	g_height = height;
	glViewport(0, 0, width, height);
}

float oldx, oldy;
float radius = 200;
bool started = false;
static void cursor_callback(GLFWwindow *window, double xpos, double ypos) {
    if (!started) {
        oldx = xpos;
        oldy = ypos;
        started = true;
    }
    
    theta +=  M_PI / 2 * ((xpos - oldx) / g_width);
    oldx = xpos;
    
    phi += M_PI / 2 * ((oldy - ypos) / g_height);
    oldy = ypos;
    
    if (phi > M_PI / 2) phi = M_PI / 2 - 1;
    if (phi < - M_PI / 2) phi = -M_PI / 2 - 1;
    
    target = {
        radius * cos(phi) * cos(theta),
        radius * sin(phi),
        radius * cos(phi) * cos(M_PI/2 - theta)
    };
    
    view = glm::lookAt(eye, target, upV);
}

static void init()
{
	GLSL::checkVersion();

	// Set background color.
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

    GLuint VertexArrayID;
    //generate the VAO
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    vector<tinyobj::shape_t> TOshapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    
    vec3 Gmin, Gmax;
    Gmin = vec3(1.1754E+38F);
    Gmax = vec3(-1.1754E+38F);

    bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "mario/mario.obj").c_str(), (RESOURCE_DIR + "mario/").c_str());
    if (!rc) {
        cerr << errStr << endl;
    } else {
        for (int i=0; i < TOshapes.size(); i++) {
            shared_ptr<Shape> shape = make_shared<Shape>();
            int matId = TOshapes.at(i).mesh.material_ids.at(0);
            shape->createShape(TOshapes.at(i), objMaterials.at(matId), RESOURCE_DIR + "mario/");
            shape->measure();
            shape->init();
            
            if (shape->min.x < Gmin.x) Gmin.x = shape->min.x;
            if (shape->min.y < Gmin.y) Gmin.y = shape->min.y;
            if (shape->min.z < Gmin.z) Gmin.z = shape->min.z;
            
            if (shape->max.x > Gmax.x) Gmax.x = shape->max.x;
            if (shape->max.y > Gmax.y) Gmax.y = shape->max.y;
            if (shape->max.z > Gmax.z) Gmax.z = shape->max.z;
            
            mario.push_back(shape);
        }
    }
//    GLSL::checkError();
    
    rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "world/WF.obj").c_str(), (RESOURCE_DIR + "world/").c_str());
    if (!rc) {
        cerr << errStr << endl;
    } else {
        for (int i=0; i < TOshapes.size(); i++) {
            shared_ptr<Shape> shape = make_shared<Shape>();
            int matId = TOshapes.at(i).mesh.material_ids.at(0);
            shape->createShape(TOshapes.at(i), objMaterials.at(matId), RESOURCE_DIR + "world/");
            shape->measure();
            shape->init();
            
            if (shape->min.x < Gmin.x) Gmin.x = shape->min.x;
            if (shape->min.y < Gmin.y) Gmin.y = shape->min.y;
            if (shape->min.z < Gmin.z) Gmin.z = shape->min.z;
            
            if (shape->max.x > Gmax.x) Gmax.x = shape->max.x;
            if (shape->max.y > Gmax.y) Gmax.y = shape->max.y;
            if (shape->max.z > Gmax.z) Gmax.z = shape->max.z;
            
            world.push_back(shape);
        }
    }
    
//    rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "thwomp/Thwomp.obj").c_str(), (RESOURCE_DIR + "thwomp/").c_str());
//    if (!rc) {
//        cerr << errStr << endl;
//    } else {
//        for (int i=0; i < TOshapes.size(); i++) {
//            shared_ptr<Shape> shape = make_shared<Shape>();
//            int matId = TOshapes.at(i).mesh.material_ids.at(0);
//            shape->createShape(TOshapes.at(i), objMaterials.at(matId), RESOURCE_DIR + "thwomp/");
//            shape->measure();
//            shape->init();
//            
//            if (shape->min.x < Gmin.x) Gmin.x = shape->min.x;
//            if (shape->min.y < Gmin.y) Gmin.y = shape->min.y;
//            if (shape->min.z < Gmin.z) Gmin.z = shape->min.z;
//            
//            if (shape->max.x > Gmax.x) Gmax.x = shape->max.x;
//            if (shape->max.y > Gmax.y) Gmax.y = shape->max.y;
//            if (shape->max.z > Gmax.z) Gmax.z = shape->max.z;
//            
//            thwomp.push_back(shape);
//        }
//    }
//    
//    rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "whomp/Whomp.obj").c_str(), (RESOURCE_DIR + "whomp/").c_str());
//    if (!rc) {
//        cerr << errStr << endl;
//    } else {
//        for (int i=0; i < TOshapes.size(); i++) {
//            shared_ptr<Shape> shape = make_shared<Shape>();
//            int matId = TOshapes.at(i).mesh.material_ids.at(0);
//            shape->createShape(TOshapes.at(i), objMaterials.at(matId), RESOURCE_DIR + "whomp/");
//            shape->measure();
//            shape->init();
//            
//            if (shape->min.x < Gmin.x) Gmin.x = shape->min.x;
//            if (shape->min.y < Gmin.y) Gmin.y = shape->min.y;
//            if (shape->min.z < Gmin.z) Gmin.z = shape->min.z;
//            
//            if (shape->max.x > Gmax.x) Gmax.x = shape->max.x;
//            if (shape->max.y > Gmax.y) Gmax.y = shape->max.y;
//            if (shape->max.z > Gmax.z) Gmax.z = shape->max.z;
//            
//            whomp.push_back(shape);
//        }
//    }
    
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = Gmax.x-Gmin.x;
    yExtent = Gmax.y-Gmin.y;
    zExtent = Gmax.z-Gmin.z;
    if (xExtent >= yExtent && xExtent >= zExtent) {
        maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) {
        maxExtent = zExtent;
    }
    float shiftX = Gmin.x + (xExtent/ 2.0);
    float shiftY = Gmin.y + (yExtent / 2.0);
    float shiftZ = Gmin.z + (zExtent)/2.0;
    
    g_trans = vec3(shiftX, shiftY, shiftZ);
    g_scale = 2.0 / maxExtent;
    
    cout << "global scale " << g_scale << endl;
    cout << "global trans " << g_trans.x << " " << g_trans.y << " " << g_trans.z << endl;
    
    light = make_shared<LightSource>();
    light->intensity = vec3(1, 1, 1);
    light->pos = vec3(0, 5, 0);
    
    marioPos = vec3(0,1000,-4000);
    eye = vec3(-4,5,-8.7);
    upV = vec3(0,1,0);
    
    phi = 0;
    theta = 0;
    
    target = {
        radius * cos(phi) * cos(theta),
        radius * sin(phi),
        radius * cos(phi) * cos(M_PI/2 - theta)
    };

    view = glm::lookAt(eye, target, upV);

	// Initialize the GLSL programs
	marioProg = make_shared<Program>();
	marioProg->setVerbose(true);
	marioProg->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
	marioProg->init();
	
	worldProg = make_shared<Program>();
	worldProg->setVerbose(true);
	worldProg->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
	worldProg->init();

	/// Add uniform and attributes to each of the programs
	marioProg->addUniform("P");
	marioProg->addUniform("M");
    marioProg->addUniform("V");
    marioProg->addUniform("lightPos");
    marioProg->addUniform("lightIntensity");
    marioProg->addUniform("MatAmb");
    marioProg->addUniform("MatDif");
    marioProg->addUniform("MatSpec");
    marioProg->addUniform("shine");
	marioProg->addAttribute("vertPos");
    marioProg->addAttribute("vertNor");
	marioProg->addAttribute("vertTex");
    marioProg->addUniform("Texture");
    marioProg->addUniform("hasTexture");
    marioProg->addUniform("eyePos");
	
	worldProg->addUniform("P");
	worldProg->addUniform("M");
    worldProg->addUniform("V");
    worldProg->addUniform("lightPos");
    worldProg->addUniform("lightIntensity");
    worldProg->addUniform("MatAmb");
    worldProg->addUniform("MatDif");
    worldProg->addUniform("MatSpec");
    worldProg->addUniform("shine");
	worldProg->addAttribute("vertPos");
    worldProg->addAttribute("vertNor");
	worldProg->addAttribute("vertTex");
    worldProg->addUniform("Texture");
    worldProg->addUniform("hasTexture");
    worldProg->addUniform("eyePos");
}


/****DRAW
This is the most important function in your program - this is where you
will actually issue the commands to draw any geometry you have set up to
draw
********/
static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create the matrix stacks 
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	P->pushMatrix();
    P->perspective(45.0f, aspect, 0.01f, 100.0f);

    MV->pushMatrix();
        MV->loadIdentity();
        MV->rotate(g_rotate, vec3(1, 0, 0));
        MV->scale(g_scale);
        MV->translate(-1.0f * g_trans);

        // draw the mario mesh
        marioProg->bind();
        MV->pushMatrix();
            MV->translate(marioPos);
            MV->scale(vec3(3,3,3));
            MV->rotate(radians(marioRot), vec3(0,1,0));
//            eye = P->topMatrix() * view * MV->topMatrix() * vec4(1);
//            P->topMa * V * M * vec4(vertPos.xyz, 1.0);

            glUniformMatrix4fv(marioProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
            glUniformMatrix4fv(marioProg->getUniform("M"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
            glUniformMatrix4fv(marioProg->getUniform("V"), 1, GL_FALSE, value_ptr(view));
            glUniform3fv(marioProg->getUniform("eyePos"), 1, value_ptr(eye));
            glUniform3fv(marioProg->getUniform("lightPos"), 1, value_ptr(light->pos));
            glUniform3fv(marioProg->getUniform("lightIntensity"), 1, value_ptr(light->intensity));
            for (int i = 0; i < mario.size(); ++i) {
                mario[i]->draw(marioProg);
                GLSL::checkError();
            }
        MV->popMatrix();
        marioProg->unbind();

        // draw the world sphere
        worldProg->bind();
        MV->pushMatrix();
            MV->scale(vec3(50, 50, 50));
            glUniformMatrix4fv(worldProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
            glUniformMatrix4fv(worldProg->getUniform("M"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
            glUniformMatrix4fv(worldProg->getUniform("V"), 1, GL_FALSE, value_ptr(view));
            glUniform3fv(worldProg->getUniform("eyePos"), 1, value_ptr(eye));
            glUniform3fv(worldProg->getUniform("lightPos"), 1, value_ptr(light->pos));
            glUniform3fv(worldProg->getUniform("lightIntensity"), 1, value_ptr(light->intensity));
            for (int i = 0; i < world.size(); ++i) {
                world[i]->draw(worldProg);
            }
        MV->popMatrix();
        worldProg->unbind();
    
    MV->popMatrix();
	P->popMatrix();
}

int main(int argc, char **argv)
{
	g_width = 640 * 2;
	g_height = 480 * 2;
	/* we will always need to load external shaders to set up where */
	if(argc < 2) {
      cout << "Please specify the resource directory." << endl;
      return 0;
   }
   RESOURCE_DIR = argv[1] + string("/");

	/* your main will always include a similar set up to establish your window
      and GL context, etc. */

	// Set error callback as openGL will report errors but they need a call back
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	//request the highest possible version of OGL - important for mac
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(g_width, g_height, "textures", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}

	glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1); // newly added
    
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
    //set the cursor call back
    glfwSetCursorPosCallback(window, cursor_callback);
	//set the window resize call back
	glfwSetFramebufferSizeCallback(window, resize_callback);

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
    init();
    cout << "done initializing shaders" << endl;
    
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
