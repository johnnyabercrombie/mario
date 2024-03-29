#include "Shape.h"
#include <iostream>

#include "GLSL.h"
#include "Program.h"
#include <assert.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "glee.hpp"

using namespace std;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0)
{
    min = glm::vec3(0);
    max = glm::vec3(0);
}

Shape::~Shape()
{
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape, tinyobj::material_t &objMaterial, const string resourceDir)
{
    posBuf = shape.mesh.positions;
    norBuf = shape.mesh.normals;
    texBuf = shape.mesh.texcoords;
    eleBuf = shape.mesh.indices;
    
    matBuf = shape.mesh.material_ids; // newly added
    material = objMaterial; // newly added
    
    if (material.ambient_texname.length() > 0) {
        texture = make_shared<Texture>();
        texture->setFilename(resourceDir + material.ambient_texname);
        texture->init();
        texture->setUnit(0);
        texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
    posBuf = shape.mesh.positions;
    norBuf = shape.mesh.normals;
    texBuf = shape.mesh.texcoords;
    eleBuf = shape.mesh.indices;
    matBuf = shape.mesh.material_ids; // newly added
}

void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		posBuf = shapes[0].mesh.positions;
		norBuf = shapes[0].mesh.normals;
		texBuf = shapes[0].mesh.texcoords;
		eleBuf = shapes[0].mesh.indices;
	}
}

/* Note this is fairly dorky - */
void Shape::ComputeTex() {
	float u, v;

		for (size_t n = 0; n < norBuf.size()/3; n++) {
			u = norBuf[n*3+0]/2.0 + 0.5;
			v = norBuf[n*3+1]/2.0 + 0.5;
         texBuf[n*3+0] = u;
         texBuf[n*3+1] = v;
      }
}


void Shape::resize() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
      if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
      if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

      if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
      if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

      if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
      if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
   }

   //From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
      posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
      assert(posBuf[3*v+0] >= -1.0 - epsilon);
      assert(posBuf[3*v+0] <= 1.0 + epsilon);
      posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
      assert(posBuf[3*v+1] >= -1.0 - epsilon);
      assert(posBuf[3*v+1] <= 1.0 + epsilon);
      posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
      assert(posBuf[3*v+2] >= -1.0 - epsilon);
      assert(posBuf[3*v+2] <= 1.0 + epsilon);
   }
}

void Shape::measure() {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    
    minX = minY = minZ = 1.1754E+38F;
    maxX = maxY = maxZ = -1.1754E+38F;
    
    //Go through all vertices to determine min and max of each dimension
    for (size_t v = 0; v < posBuf.size() / 3; v++) {
        if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
        if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];
        
        if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
        if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];
        
        if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
        if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
    }
    
    min.x = minX;
    min.y = minY;
    min.z = minZ;
    max.x = maxX;
    max.y = maxY;
    max.z = maxZ;
}

void Shape::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		norBufID = 0;
	} else {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		//texBufID = 0;
		//send in spherical constructed
		for (size_t v = 0; v < posBuf.size(); v++) {
         texBuf.push_back(0);
      }
      ComputeTex();

		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
    
    // newly added
//    glGenBuffers(1, &matBufID);
//    glBindBuffer(GL_ARRAY_BUFFER, matBufID);
//    glBufferData(GL_ARRAY_BUFFER, matBuf.size()*sizeof(int), &matBuf[0], GL_STATIC_DRAW);
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	// Bind position buffer
	int h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	int h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}
	
	// Bind texcoords buffer
	int h_tex = prog->getAttribute("vertTex");
	if(h_tex != -1 && texBufID != 0) {
		GLSL::enableVertexAttribArray(h_tex);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

    glUniform3f(prog->getUniform("MatAmb"), material.ambient[0], material.ambient[1], material.ambient[2]);
    glUniform3f(prog->getUniform("MatDif"), material.diffuse[0], material.diffuse[1], material.diffuse[2]);
    glUniform3f(prog->getUniform("MatSpec"), material.specular[0], material.specular[1], material.specular[2]);
    glUniform1f(prog->getUniform("shine"), material.shininess);
    
//    glUniform3fv(prog->getUniform("MatAmb"), 1, material.ambient);
//    glUniform3fv(prog->getUniform("MatDif"), 1, material.diffuse);
//    glUniform3fv(prog->getUniform("MatSpec"), 1, material.specular);
//    glUniform1f(prog->getUniform("shine"), material.shininess);
    
    if (texture != nullptr) {
        texture->bind(prog->getUniform("Texture"));
        glUniform1f(prog->getUniform("hasTexture"), 1.0);
    } else {
        glUniform1f(prog->getUniform("hasTexture"), 0.0);
    }
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
    if (texture != nullptr) {
        texture->unbind();
    }
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
