#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <string>
#include <vector>
#include <memory>
#include "tiny_obj_loader.h"
#include "Texture.h"
#include <glm/gtc/type_ptr.hpp>

class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadMesh(const std::string &meshName);
    void createShape(tinyobj::shape_t & shape, tinyobj::material_t & objMaterial);
    void createShape(tinyobj::shape_t & shape);
	void init();
	void draw(const std::shared_ptr<Program> prog) const;
	void ComputeTex();
	void resize();
    void measure();
    glm::vec3 min;
    glm::vec3 max;
	
private:
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
    std::vector<int> matBuf;
    tinyobj::material_t material;
    std::shared_ptr<Texture> texture;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
    unsigned matBufID;
};

#endif
