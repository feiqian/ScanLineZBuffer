#ifndef __SCAN_LINE_Z_BUFFER_H
#define __SCAN_LINE_Z_BUFFER_H

#include <iostream>
#include <algorithm>
#include <list>
#include <cassert>
#include "VMath.h"
#include "ObjLoader.h"
#include "GlutDisplay.h"
#include "PolygonAndEdgeTableStrcture.h"
using namespace std;

struct  ObjFace
{
	vector<unsigned int> vertexIndex;
	Color3 color;
};

class ScanLineZBuffer
{
public:
	ScanLineZBuffer();
	~ScanLineZBuffer();
	void run(const char* obj_file);
	Color3** render();
	void getSize(int& width,int& height);
	void setSize(int width,int height);

private:
	bool loadObj(const char* objFile);
	void initScene();
	void buildPolygonAndEdgeTable();
	void addEdgeToActiveTable(int y,PolygonTableEle* pt_it);

	void releaseMemory();

	vector<Vec3> vertices;
	vector<Vec3> newVertices;
	vector<ObjFace> faces;

	vector< list<PolygonTableEle>> polygonTable;
	vector< list<EdgeTableEle>> edgeTable;
	list< PolygonTableEle> activePolygonTable;

	int width,height;
	double* zBuffer;
	Color3** colorBuffer;
};

#endif