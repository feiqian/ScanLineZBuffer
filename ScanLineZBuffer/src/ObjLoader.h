#ifndef __OBJ_LOADER_H__
#define __OBJ_LOADER_H__
#include <iostream>
#include <vector>
#include "VMath.h"

using namespace std;

struct ObjFace
{
	vector<int> vertexIndex;
	vector<int> normalIndex;
	Vec3 normal;
};

struct ObjVertex
{
	Point3 point;
	Color3 color;
	int faceIndex;
};

struct ObjLoader
{
	bool loadFromFile(const char* objPath);
	vector<ObjVertex> vertices;
	vector<ObjFace> faces;
	vector<Vec3> normals;
};


#endif