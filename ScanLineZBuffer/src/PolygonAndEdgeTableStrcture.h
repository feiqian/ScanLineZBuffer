#ifndef __POLYGON_AND_EDGE_TABLE_STRUCTURE_H__
#define __POLYGON_AND_EDGE_TABLE_STRUCTURE_H__
#include <iostream>
#include <list>
#include "VMath.h"

struct EdgeTableEle
{
	double x;//�ߵ��϶˵��x����
	double dx;//��������ɨ���߽����x�����
	int dy;//�߿�Խ��ɨ������Ŀ
	int id;//����������εı��
};

struct ActiveEdgeTableEle
{
	double xl; // �󽻵��x����
	double dxl; // (�󽻵����)������ɨ���߽����x����֮��
	int dyl; // �Ժ��󽻵����ڱ��ཻ��ɨ������Ϊ��ֵ���Ժ�����ÿ����һ��ɨ���߼�1
	double zl; // �󽻵㴦���������ƽ������ֵ
	double dzx; // ��ɨ���������߹�һ������ʱ�����������ƽ����������������ƽ�淽�̣�dzx = -a/c(c!=0)
	double dzy; // ��y���������ƹ�һ��ɨ����ʱ�����������ƽ����������������ƽ�淽�̣�dzy = b/c(c!=0)
	int id; // ��������ڵĶ���εı��
};

struct PolygonTableEle
{
	double a,b,c,d;//���������ƽ��ķ���ϵ��
	int id;//����α��
	int dy;//����ο�Ծ��ɨ������Ŀ
	Color3 color;
	std::vector< ActiveEdgeTableEle> activeEdgeTable;
};

#endif