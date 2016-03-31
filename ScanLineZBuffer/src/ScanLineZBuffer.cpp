#include "ScanLineZBuffer.h"
#include "Timer.h"
#include <omp.h>
#include "windows.h"
#define NUM_THREADS 10

//对活化多边形表排序的比较函数
static bool cmp(const ActiveEdgeTableEle &lhs, const ActiveEdgeTableEle &rhs)
{
	if (Round(lhs.xl) < Round(rhs.xl)) return true;
	else if (Round(lhs.xl) == Round(rhs.xl)) {
		if (Round(lhs.dxl) < Round(rhs.dxl))
			return true;
	}
	return false;
}

//计算顶点颜色
static Color3 phoneRender(const Point3& position,const Vec3& normal)
{
	static Point3 lightPosition(0,0,0);
	static double kd=0.8;
	static Color3 ambientColor(0.3,0.3,0.3);
	Color3 triColor((double)rand()/RAND_MAX,(double)rand()/RAND_MAX,(double)rand()/RAND_MAX);
	Color3 rgb;

	//calculate the diffuse color
	Vec3& rayDirection = Normalize(lightPosition-position);
	double dot = Dot(rayDirection,normal);
	if(dot>0.0) rgb+=dot*kd*triColor;
	rgb+=ambientColor;

	return Limit(rgb,0,1);
}

static void printProgress(double& progress,double increment)
{
	static CONSOLE_SCREEN_BUFFER_INFO info;

	if(!progress) GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&info);
	double newProgress = progress+increment;

	if(Round(newProgress)!=Round(progress)) 
	{
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),info.dwCursorPosition);
		cout<<"progress:"<<Round(newProgress)<<"%"<<endl;
	}
	progress = newProgress;
}

ScanLineZBuffer::ScanLineZBuffer()
{
	setSize(500,500);
}

ScanLineZBuffer::~ScanLineZBuffer()
{
	releaseMemory();
}

void ScanLineZBuffer::releaseMemory()
{
	if(zBuffer!=NULL)
	{
		delete [] zBuffer;
		zBuffer = NULL;
	}

	if(colorBuffer!=NULL)
	{
		for(int i=0;i<height;++i) delete [] colorBuffer[i];
		delete [] colorBuffer;
		colorBuffer = NULL;
	}
}

void ScanLineZBuffer::getSize(int& width,int& height)
{
	width = this->width;
	height = this->height;
}

void ScanLineZBuffer::setSize(int width,int height)
{
	if(width<0||height<0) return;
	if(width==this->width&&height==this->height) return;

	releaseMemory();
	this->width = width;
	this->height = height;

	needReRender = true;
	zBuffer = new double[width];
	colorBuffer = new Color3*[height];
	for(int i=0;i<height;++i) colorBuffer[i] = new Color3[width];
}

bool ScanLineZBuffer::loadObj(const char* objFile)
{
	TotalTimer totalTimer("loading obj file");
	return objLoader.loadFromFile(objFile);
}

void ScanLineZBuffer::initScene()
{
	double scale;
	Point3 min_xyz(0xfffffff,0xfffffff,0xfffffff), max_xyz(-0xfffffff,-0xfffffff,-0xfffffff);
	for (vector<ObjVertex>::const_iterator vertex_it = objLoader.vertices.begin(),end=objLoader.vertices.end(); vertex_it !=end ; ++vertex_it)
	{
		const Point3& vertex = vertex_it->point;
		min_xyz.x = min(min_xyz.x, vertex.x);
		min_xyz.y = min(min_xyz.y, vertex.y);
		max_xyz.x = max(max_xyz.x, vertex.x);
		max_xyz.y = max(max_xyz.y, vertex.y);
	}

	Vec3 range (max_xyz.x - min_xyz.x, max_xyz.y - min_xyz.y, 0); 
	scale = Length(range);

	//使所有顶点的x,y坐标都在屏幕窗口内
	double max_scalar;
	if (width < height) max_scalar = width / (2 * scale);
	else max_scalar = height / (2 * scale);

	newVertices.resize(objLoader.vertices.size());

	int len= objLoader.vertices.size();
	#pragma omp parallel for num_threads(NUM_THREADS)
	for(int i=0;i<len;++i)
	{
		Point3 vertex = objLoader.vertices[i].point;
		vertex.x = vertex.x*max_scalar+width/2;
		vertex.y = vertex.y*max_scalar+height/2;
		vertex.z = vertex.z*max_scalar;
		newVertices[i].point = vertex;
	}
}

void ScanLineZBuffer::buildPolygonAndEdgeTable()
{
	polygonTable.clear();
	edgeTable.clear();

	polygonTable.resize(height);
	edgeTable.resize(height);

	int len=objLoader.faces.size();
	#pragma omp parallel for 
	for (int i=0;i<len;++i)
	{
		double min_y=0xfffffff, max_y=-0xfffffff;
		Color3 rgb;
		ObjFace& face = objLoader.faces[i];

		PolygonTableEle pt;
		pt.id = i;
		
		const vector<int>& vertexIndex = objLoader.faces[i].vertexIndex;
		for (int i=0,len = vertexIndex.size();i<len;++i)
		{
			Point3 pt1 = newVertices[vertexIndex[i]].point;
			Point3 pt2 = (i==len-1?newVertices[vertexIndex[0]].point:newVertices[vertexIndex[i+1]].point);

			if(pt1.y<pt2.y) swap(pt1,pt2);

			EdgeTableEle ete;
			ete.dy = Round(pt1.y) - Round(pt2.y);
			rgb+=phoneRender(pt1,face.normalIndex[i]>=0?objLoader.normals[face.normalIndex[i]]:face.normal);

			if(ete.dy<=0) continue;

			ete.x = pt1.x;
			ete.id = pt.id;
			ete.dx = -(pt1.x-pt2.x)/(pt1.y-pt2.y);

			#pragma omp critical
			edgeTable[Round(pt1.y)].push_back(ete);

			min_y = min(pt2.y,min_y);
			max_y = max(pt1.y,max_y);
		}


		pt.dy = Round(max_y) - Round(min_y);
		if(pt.dy>0&&max_y>0&&min_y<height)
		{
			Point3 &a = newVertices[vertexIndex[0]].point,&b = newVertices[vertexIndex[1]].point,&c = newVertices[vertexIndex[2]].point;

			pt.a = face.normal.x;
			pt.b = face.normal.y;
			pt.c = face.normal.z;
			pt.d = -(pt.a*a.x+pt.b*a.y+pt.c*a.z);
			pt.color = rgb/vertexIndex.size();

			#pragma omp critical
			polygonTable[Round(max_y)].push_back(pt);
		}
	}
}

void ScanLineZBuffer::addEdgeToActiveTable(int y,PolygonTableEle* pt_it)
{
	//把该多边形在oxy平面上的投影和扫描线相交的边加入到活化边表中
	for(list<EdgeTableEle>::iterator et_it = edgeTable[y].begin(),end=edgeTable[y].end();et_it!=end;)
	{
		if(et_it->id!=pt_it->id){
			++et_it;
			continue;
		}

		ActiveEdgeTableEle aete;
		aete.xl = et_it->x;
		aete.dxl = et_it->dx;
		aete.dyl = et_it->dy;

		if(DoubleEquals(pt_it->c,0))
		{
			aete.zl=0;
			aete.dzx=0;
			aete.dzy=0;
		}
		else
		{
			aete.zl = -(pt_it->d+pt_it->a*et_it->x + pt_it->b*y) / pt_it->c;
			aete.dzx = -(pt_it->a / pt_it->c);
			aete.dzy = pt_it->b / pt_it->c;
		}

		pt_it->activeEdgeTable.push_back(aete);
		et_it->id = -1;
	}

	// 对当前活化多边形的活化边表按照x排序
	sort(pt_it->activeEdgeTable.begin(),pt_it->activeEdgeTable.end(),cmp);
}

Color3** ScanLineZBuffer::render()
{
	if(!needReRender) return colorBuffer;
	TotalTimer totalTimer("rendering");

	initScene();
	buildPolygonAndEdgeTable();

	activePolygonTable.clear();
	double progress = 0;

	for(int y = height-1;y>=0;--y)
	{
		memset(colorBuffer[y],0,sizeof(Color3)*width);
		fill(zBuffer,zBuffer+width,-0xfffffff);

		//检查分类的多边形表，如果有新的多边形涉及该扫描线，则把它放入活化的多边形表中
		for(list<PolygonTableEle>::iterator pt_it = polygonTable[y].begin(),end = polygonTable[y].end();pt_it!=end;++pt_it)
			activePolygonTable.push_back(*pt_it);
	
		int len = activePolygonTable.size();
		#pragma omp parallel for num_threads(NUM_THREADS)
		for(int i=0;i<len;++i)
		{
			PolygonTableEle& pte = activePolygonTable[i];

			addEdgeToActiveTable(y,&pte);

			vector<ActiveEdgeTableEle>& aet = pte.activeEdgeTable;
			assert(aet.size()%2==0);

			for(vector<ActiveEdgeTableEle>::iterator aet_it = aet.begin(),end=aet.end();aet_it!=end;++aet_it)
			{
				ActiveEdgeTableEle& edge1 = *aet_it;
				ActiveEdgeTableEle& edge2 = *(++aet_it);

				double zx = edge1.zl;

				for(int x=Round(edge1.xl),end=Round(edge2.xl);x<end;++x)
				{
					if(zx>=zBuffer[x])
					{
						zBuffer[x] = zx;
						colorBuffer[y][x] = pte.color;
					}
					zx+=aet_it->zl;
				}

				--edge1.dyl;
				--edge2.dyl;
				edge1.xl+=edge1.dxl;
				edge2.xl+=edge2.dxl;
				edge1.zl+=edge1.dzx*edge1.dxl+edge1.dzy;
				edge2.zl+=edge2.dzx*edge2.dxl+edge2.dzy;
			}

			int last = 0;
			for(int i=0,len=aet.size(); i<len; ++i, ++last)
			{
				while(aet[i].dyl<=0)
				{
					++i;
					if(i>=len) break;
				}
				if(i >= len) break;

				aet[last] = aet[i];   
			}
			aet.resize(last);

			--pte.dy;
		}

		int last = 0;
		for(int i=0,len=activePolygonTable.size(); i<len; ++i, ++last)
		{
			while(activePolygonTable[i].dy<0)
			{
				++i;
				if(i>=len) break;
			}
			if(i >= len) break;

			activePolygonTable[last] = activePolygonTable[i];   
		}
		activePolygonTable.resize(last);

		printProgress(progress,100.0/height);
	}

	needReRender = false;

	cout<<"vertex num:"<<objLoader.vertices.size()<<",face num:"<<objLoader.faces.size()<<endl;
	return colorBuffer;
}

void ScanLineZBuffer::run(const char* obj_file)
{
	if(loadObj(obj_file))
	{
		GlutDisplay::render(this);
	}
}