#ifndef __POLYGON_AND_EDGE_TABLE_STRUCTURE_H__
#define __POLYGON_AND_EDGE_TABLE_STRUCTURE_H__
#include <iostream>
#include <list>
#include "VMath.h"

struct EdgeTableEle
{
	double x;//边的上端点的x坐标
	double dx;//相邻两条扫描线交点的x坐标差
	int dy;//边跨越的扫描线数目
	int id;//边所属多边形的编号
};

struct ActiveEdgeTableEle
{
	double xl; // 左交点的x坐标
	double dxl; // (左交点边上)两相邻扫描线交点的x坐标之差
	int dyl; // 以和左交点所在边相交的扫描线数为初值，以后向下每处理一条扫描线减1
	double zl; // 左交点处多边形所在平面的深度值
	double dzx; // 沿扫描线向右走过一个像素时，多边形所在平面的深度增量。对于平面方程，dzx = -a/c(c!=0)
	double dzy; // 沿y方向向下移过一根扫描线时，多边形所在平面的深度增量。对于平面方程，dzy = b/c(c!=0)
	int id; // 交点对所在的多边形的编号
};

struct PolygonTableEle
{
	double a,b,c,d;//多边形所在平面的方程系数
	int id;//多边形编号
	int dy;//多边形跨跃的扫描线数目
	Color3 color;
	std::vector< ActiveEdgeTableEle> activeEdgeTable;
};

#endif