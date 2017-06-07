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
shared_ptr<Program> prog0;
shared_ptr<Program> prog1;
//shared_ptr<Program> prog2;
//shared_ptr<Shape> world;
//shared_ptr<Shape> shape;
vector<shared_ptr<Shape>> mario;
vector<shared_ptr<Shape>> thwomp;
vector<shared_ptr<Shape>> whomp;
vector<shared_ptr<Shape>> world;
shared_ptr<LightSource> light;

int g_GiboLen;
int g_width, g_height;
float g_rotate = 0; // was -1.5
float g_scale = 1;
vec3 g_trans;
vec3 eye, target, upV;
vector<vec3> thwompIdxs, whompIdxs;
vector<float> thwompYaws, whompYaws;
float cspeed = 0.05;
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
        eye.x += cspeed * (-1 * u.x);
        target.x += cspeed * (-1 * u.x);
        eye.z += cspeed * (-1 * u.z);
        target.z += cspeed * (-1 * u.z);
    } else if (key == GLFW_KEY_D) {
        eye.x += cspeed * u.x;
        target.x += cspeed * u.x;
        eye.z += cspeed * u.z;
        target.z += cspeed * u.z;
    } else if (key == GLFW_KEY_W) {
        eye.x += cspeed * (-1 * w.x);
        target.x += cspeed * (-1 * w.x);
        eye.z += cspeed * (-1 * w.z);
        target.z += cspeed * (-1 * w.z);
    } else if (key == GLFW_KEY_S) {
        eye.x += cspeed * w.x;
        target.x += cspeed * w.x;
        eye.z += cspeed * w.z;
        target.z += cspeed * w.z;
    }
    
    view = glm::lookAt(eye, target, upV);}

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
    
    if (phi > M_PI / 2) phi = M_PI / 2;
    if (phi < - M_PI / 2) phi = -M_PI / 2;
    
    target = {
        radius * cos(phi) * cos(theta),
        radius * sin(phi),
        radius * cos(phi) * cos(M_PI/2 - theta)
    };
    
    view = glm::lookAt(eye, target, upV);
}

float g_groundSize = 200;
float g_groundY = -1.5;

/* code to define the ground plane */
static void initGeom() {
  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };

  static GLfloat GrndTex[] = {
      0, 0, // back
      0, 1,
      1, 1,
      1, 0 };

   unsigned short idx[] = {0, 1, 2, 0, 2, 3};

   GLuint VertexArrayID;
	//generate the VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);
    
	 glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

}

static void init()
{
	GLSL::checkVersion();

	// Set background color.
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

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
            shape->createShape(TOshapes.at(i), objMaterials.at(matId));
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

    rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "thwomp/Thwomp.obj").c_str(), (RESOURCE_DIR + "thwomp/").c_str());
    if (!rc) {
        cerr << errStr << endl;
    } else {
        for (int i=0; i < TOshapes.size(); i++) {
            shared_ptr<Shape> shape = make_shared<Shape>();
            int matId = TOshapes.at(i).mesh.material_ids.at(0);
            shape->createShape(TOshapes.at(i), objMaterials.at(matId));
            shape->measure();
            shape->init();
            
            if (shape->min.x < Gmin.x) Gmin.x = shape->min.x;
            if (shape->min.y < Gmin.y) Gmin.y = shape->min.y;
            if (shape->min.z < Gmin.z) Gmin.z = shape->min.z;
            
            if (shape->max.x > Gmax.x) Gmax.x = shape->max.x;
            if (shape->max.y > Gmax.y) Gmax.y = shape->max.y;
            if (shape->max.z > Gmax.z) Gmax.z = shape->max.z;
            
            thwomp.push_back(shape);
        }
    }
    
    rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "whomp/Whomp.obj").c_str(), (RESOURCE_DIR + "whomp/").c_str());
    if (!rc) {
        cerr << errStr << endl;
    } else {
        for (int i=0; i < TOshapes.size(); i++) {
            shared_ptr<Shape> shape = make_shared<Shape>();
            int matId = TOshapes.at(i).mesh.material_ids.at(0);
            shape->createShape(TOshapes.at(i), objMaterials.at(matId));
            shape->measure();
            shape->init();
            
            if (shape->min.x < Gmin.x) Gmin.x = shape->min.x;
            if (shape->min.y < Gmin.y) Gmin.y = shape->min.y;
            if (shape->min.z < Gmin.z) Gmin.z = shape->min.z;
            
            if (shape->max.x > Gmax.x) Gmax.x = shape->max.x;
            if (shape->max.y > Gmax.y) Gmax.y = shape->max.y;
            if (shape->max.z > Gmax.z) Gmax.z = shape->max.z;
            
            whomp.push_back(shape);
        }
    }
    
    rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + "world/WF.obj").c_str(), (RESOURCE_DIR + "world/").c_str());
    if (!rc) {
        cerr << errStr << endl;
    } else {
        for (int i=0; i < TOshapes.size(); i++) {
            shared_ptr<Shape> shape = make_shared<Shape>();
            int matId = TOshapes.at(i).mesh.material_ids.at(0);
            shape->createShape(TOshapes.at(i), objMaterials.at(matId));
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
    
    eye = vec3(0,1,0);
    target = vec3(0,0,0);
    upV = vec3(0,1,0);
    
    phi = 0;
    theta = M_PI / 2;
    
    target = {
        radius * cos(phi) * cos(theta),
        radius * sin(phi),
        radius * cos(phi) * cos(M_PI/2 - theta)
    };
    
    view = glm::lookAt(eye, target, upV);
    
    for (int i = 0; i < 15; ++i) {
        thwompIdxs.push_back(vec3(rand() % int(2 * g_groundSize + 1) - g_groundSize, 25, rand() % int(2 * g_groundSize + 1) - g_groundSize));
        whompIdxs.push_back(vec3(rand() % int(2 * g_groundSize + 1) - g_groundSize, 5, rand() % int(2 * g_groundSize + 1) - g_groundSize));
        thwompYaws.push_back(radians(float(rand() % 360)));
        whompYaws.push_back(radians(float(rand() % 360)));
    }

	// Initialize the GLSL programs
	prog0 = make_shared<Program>();
	prog0->setVerbose(true);
	prog0->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag0.glsl");
	prog0->init();
	
	prog1 = make_shared<Program>();
	prog1->setVerbose(true);
	prog1->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag0.glsl");
	prog1->init();
  
//	prog2 = make_shared<Program>();
//	prog2->setVerbose(true);
//	prog2->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag2.glsl");
//	prog2->init();
	
	//////////////////////////////////////////////////////
   // Intialize textures
   //////////////////////////////////////////////////////
//   texture0 = make_shared<Texture>();
//   texture0->setFilename(RESOURCE_DIR + "mario/Logo.png");
//   texture0->init();
//   texture0->setUnit(0);
//   texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

//   texture1 = make_shared<Texture>();
//   texture1->setFilename(RESOURCE_DIR + "world/2C725067_c.png");
//   texture1->init();
//   texture1->setUnit(1);
//   texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
   
//	texture2 = make_shared<Texture>();
//   texture2->setFilename(RESOURCE_DIR + "grass.jpg");
//   texture2->init();
//   texture2->setUnit(2);
//   texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	/// Add uniform and attributes to each of the programs
	prog0->addUniform("P");
	prog0->addUniform("M");
    prog0->addUniform("V");
    prog0->addUniform("lightPos");
    prog0->addUniform("lightIntensity");
    prog0->addUniform("MatAmb");
    prog0->addUniform("MatDif");
    prog0->addUniform("MatSpec");
    prog0->addUniform("shine");
	prog0->addAttribute("vertPos");
    prog0->addAttribute("vertNor");
	prog0->addAttribute("vertTex");
    prog0->addUniform("Texture");
	
	prog1->addUniform("P");
	prog1->addUniform("M");
    prog1->addUniform("V");
    prog1->addUniform("lightPos");
    prog1->addUniform("lightIntensity");
    prog1->addUniform("MatAmb");
    prog1->addUniform("MatDif");
    prog1->addUniform("MatSpec");
    prog1->addUniform("shine");
	prog1->addAttribute("vertPos");
    prog1->addAttribute("vertNor");
	prog1->addAttribute("vertTex");
    prog1->addUniform("Texture");
	
//	prog2->addUniform("P");
//	prog2->addUniform("M");
//    prog2->addUniform("V");
//    prog2->addUniform("lightPos");
//    prog2->addUniform("lightIntensity");
//    prog2->addUniform("MatAmb");
//    prog2->addUniform("MatDif");
//    prog2->addUniform("MatSpec");
//    prog2->addUniform("shine");
//	prog2->addAttribute("vertPos");
//    prog2->addAttribute("vertNor");
// 	prog2->addAttribute("vertTex");
//    prog2->addUniform("Texture2");
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

        //draw the mario mesh
        prog0->bind();
        MV->pushMatrix();
//            MV->translate(vec3(-1, 0, 0));
//           MV->rotate(cTheta, vec3(0, 1, 0));
//            MV->scale(vec3(0.2,0.2,0.2));
//            texture0->bind(prog0->getUniform("Texture0"));
            glUniformMatrix4fv(prog0->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
            glUniformMatrix4fv(prog0->getUniform("M"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
            glUniformMatrix4fv(prog0->getUniform("V"), 1, GL_FALSE, value_ptr(view));
            glUniform3fv(prog0->getUniform("lightPos"), 1, value_ptr(light->pos));
            glUniform3fv(prog0->getUniform("lightIntensity"), 1, value_ptr(light->intensity));
            for (int i = 0; i < mario.size(); ++i) {
                mario[i]->draw(prog0);
            }
        MV->popMatrix();
        prog0->unbind();

//        draw the world sphere
//        prog1->bind();
//        MV->pushMatrix();
////            MV->translate(vec3(1, 0, 0));
//    //            MV->rotate(cTheta, vec3(0, 1, 0));
//            MV->scale(vec3(10,10,10));
////            texture1->bind(prog1->getUniform("Texture1"));
////            glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
//            glUniformMatrix4fv(prog1->getUniform("M"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
//            glUniformMatrix4fv(prog1->getUniform("V"), 1, GL_FALSE, value_ptr(view));
//            glUniform3fv(prog1->getUniform("lightPos"), 1, value_ptr(light->pos));
//            glUniform3fv(prog1->getUniform("lightIntensity"), 1, value_ptr(light->intensity));
//            for (int i = 0; i < world.size(); ++i) {
//                world[i]->draw(prog1);
//            }
//            glDisableVertexAttribArray(0);
//            glDisableVertexAttribArray(1);
//            glDisableVertexAttribArray(2);
//        MV->popMatrix();
//        prog1->unbind();

        //draw the ground plane	
//        prog2->bind();
//        MV->pushMatrix();
//            texture2->bind(prog2->getUniform("Texture2"));
//            glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
//            glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
//            glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, value_ptr(view));
//
//            glEnableVertexAttribArray(0);
//            glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
//            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//
//            glEnableVertexAttribArray(1);
//            glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
//            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
//             
//            glEnableVertexAttribArray(2);
//            glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
//            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
//
//            // draw!
//            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
//            glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);
//
//            glDisableVertexAttribArray(0);
//            glDisableVertexAttribArray(1);
//            glDisableVertexAttribArray(2);
//        MV->popMatrix();
//        prog2->unbind();
    
    MV->popMatrix();
	P->popMatrix();
}

int main(int argc, char **argv)
{

	g_width = 640;
	g_height = 480;
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
//	initGeom();
//	cout << "done initializing geometry" << endl;

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
