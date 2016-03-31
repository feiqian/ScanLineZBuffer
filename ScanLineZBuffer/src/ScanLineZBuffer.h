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


class ScanLineZBuffer
{
public:
	ScanLineZBuffer();
	~ScanLineZBuffer();
	void run(const char* obj_file);
	void* render();
	void getSize(int& width,int& height);
	void setSize(int width,int height);

private:
	bool loadObj(const char* objFile);
	void initScene();
	void buildPolygonAndEdgeTable();
	void addEdgeToActiveTable(int y,PolygonTableEle* pt_it);

	void releaseMemory();

	ObjLoader objLoader;
	vector<ObjVertex> newVertices;

	vector< list<PolygonTableEle>> polygonTable;
	vector< list<EdgeTableEle>> edgeTable;
	vector< PolygonTableEle> activePolygonTable;

	bool needReRender;
	int width,height;
	double* zBuffer;
	GLubyte* colorBuffer;
};

#endif