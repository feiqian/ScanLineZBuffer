#include "ScanLineZBuffer.h"

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
	if(width==this->width&&height==this->height) return;
	releaseMemory();
	this->width = width;
	this->height = height;

	zBuffer = new double[width];
	colorBuffer = new Color3*[height];
	for(int i=0;i<height;++i) colorBuffer[i] = new Color3[width];
}

bool ScanLineZBuffer::loadObj(const char* objFile)
{
	std::vector<ObjLoader::shape_t> shapes;
	std::vector<ObjLoader::material_t> materials;
	std::string err;
	bool ret = ObjLoader::LoadObj(shapes, materials, err, objFile);

	if (!err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	for (size_t i = 0; i < shapes.size(); i++) {
		size_t indexOffset = 0;

		for (size_t n = 0; n < shapes[i].mesh.num_vertices.size(); n++) {
			int ngon = shapes[i].mesh.num_vertices[n];
			ObjFace face;

			for (size_t f = 0; f < ngon; f++) {
				unsigned int v = shapes[i].mesh.indices[indexOffset + f];

				Vec3 vertex(shapes[i].mesh.positions[3*v+0],
					shapes[i].mesh.positions[3*v+1],
					shapes[i].mesh.positions[3*v+2]);
				
				face.vertexIndex.push_back(vertices.size());
				vertices.push_back(vertex);
			}
			indexOffset += ngon;

			face.color = Color3(1,1,1);
			faces.push_back(face);
		}
	}
	return true;
}

void ScanLineZBuffer::initScene()
{
	double scale;
	Point3 min_xyz(0xfffffff,0xfffffff,0xfffffff), max_xyz(-0xfffffff,-0xfffffff,-0xfffffff);
	for (vector<Vec3>::const_iterator vertex_it = vertices.begin(); vertex_it != vertices.end(); ++vertex_it)
	{
		const Vec3& vertex = (*vertex_it);
		min_xyz.x = min(min_xyz.x, vertex.x);
		min_xyz.y = min(min_xyz.y, vertex.y);
		min_xyz.z = min(min_xyz.z, vertex.z);
		max_xyz.x = max(max_xyz.x, vertex.x);
		max_xyz.y = max(max_xyz.y, vertex.y);
		max_xyz.z = max(max_xyz.z, vertex.z);
	}

	Vec3 range (max_xyz.x - min_xyz.x, max_xyz.y - min_xyz.y, 0); 
	scale = Length(range);

	//使所有顶点的x,y坐标都在屏幕窗口内
	double max_scalar;
	if (width < height) max_scalar = width / (2 * scale);
	else max_scalar = height / (2 * scale);

	newVertices.resize(vertices.size());
	for (int i=0,len=vertices.size();i<len;++i)
	{
		Vec3 vertex = vertices[i];
		vertex.x = vertex.x*max_scalar+width/2;
		vertex.y = vertex.y*max_scalar+height/2;
		vertex.z = vertex.z*max_scalar;
		newVertices[i] = vertex;
	}
}


void ScanLineZBuffer::buildPolygonAndEdgeTable()
{
	polygonTable.clear();
	edgeTable.clear();

	polygonTable.resize(height);
	edgeTable.resize(height);

	for (vector<ObjFace>::const_iterator face_it = faces.begin(); face_it != faces.end(); ++face_it)
	{
		double min_y=0xfffffff, max_y=-0xfffffff;
		PolygonTableEle pt;
		pt.id = face_it - faces.begin();
		pt.color = faces[pt.id].color;

		const vector<unsigned int>& vertexIndex = face_it->vertexIndex;
		for (int i=0,len = vertexIndex.size();i<len;++i)
		{
			Point3 pt1 = newVertices[vertexIndex[i]];
			Point3 pt2 = (i==len-1?newVertices[vertexIndex[0]]:newVertices[vertexIndex[i+1]]);

			if(pt1.y<pt2.y) swap(pt1,pt2);

			EdgeTableEle ete;
			ete.x = pt1.x;
			ete.dy = Round(pt1.y) - Round(pt2.y);

			if(ete.dy<=0) continue;

			ete.id = pt.id;
			ete.dx = -(pt1.x-pt2.x)/(pt1.y-pt2.y);
			edgeTable[Round(pt1.y)].push_back(ete);

			min_y = min(pt2.y,min_y);
			max_y = max(pt1.y,max_y);
		}


		pt.dy = Round(max_y) - Round(min_y);
		if(pt.dy<=0 || max_y<0 || min_y>=height) continue;

		Vec3 &a = newVertices[face_it->vertexIndex[0]],&b = newVertices[vertexIndex[1]],&c = newVertices[vertexIndex[2]];
		Vec3& normal = Normalize(Cross(b-a,c-b));

		pt.a = normal.x;
		pt.b = normal.y;
		pt.c = normal.z;
		pt.d = -(pt.a*newVertices[vertexIndex[0]].x+pt.b*newVertices[vertexIndex[0]].y+pt.c*newVertices[vertexIndex[0]].z);

		polygonTable[Round(max_y)].push_back(pt);
	}

}

void ScanLineZBuffer::addEdgeToActiveTable(int y,PolygonTableEle* pt_it)
{
	if(pt_it->dy<0) return;

	//把该多边形在oxy平面上的投影和扫描线相交的边加入到活化边表中
	for(list<EdgeTableEle>::const_iterator et_it = edgeTable[y].begin();et_it!=edgeTable[y].end();)
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
		et_it = edgeTable[y].erase(et_it);
	}

	// 对当前活化多边形的活化边表按照x排序
	pt_it->activeEdgeTable.sort(cmp);
}

Color3** ScanLineZBuffer::render()
{
	initScene();
	buildPolygonAndEdgeTable();

	activePolygonTable.clear();
	double progress = 0;

	for(int y = height-1;y>=0;--y)
	{
		for(int i=0;i<width;++i)
		{
			colorBuffer[y][i] = Color3::BLACK;
			zBuffer[i] = -0xfffffff;
		}

		//检查分类的多边形表，如果有新的多边形涉及该扫描线，则把它放入活化的多边形表中
		for(list<PolygonTableEle>::iterator pt_it = polygonTable[y].begin();pt_it!=polygonTable[y].end();++pt_it)
			activePolygonTable.push_back(*pt_it);
	
		for(list<PolygonTableEle>::iterator apt_it = activePolygonTable.begin();apt_it!=activePolygonTable.end();)
		{
			addEdgeToActiveTable(y,&(*apt_it));

			list<ActiveEdgeTableEle>& aet = apt_it->activeEdgeTable;
			assert(aet.size()%2==0);

			for(list<ActiveEdgeTableEle>::iterator aet_it = aet.begin();aet_it!=aet.end();++aet_it)
			{
				ActiveEdgeTableEle& edge1 = *aet_it;
				ActiveEdgeTableEle& edge2 = *(++aet_it);

				double zx = edge1.zl;

				for(int x=Round(edge1.xl);x<Round(edge2.xl);++x)
				{
					if(zx>=zBuffer[x])
					{
						zBuffer[x] = zx;
						colorBuffer[y][x] = apt_it->color;
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

			for(list<ActiveEdgeTableEle>::iterator aet_it = aet.begin();aet_it!=aet.end();)
			{
				if(aet_it->dyl<=0) aet_it = aet.erase(aet_it);
				else ++aet_it;
			}

			--apt_it->dy;
			if(apt_it->dy<0) apt_it = activePolygonTable.erase(apt_it);
			else ++apt_it;
		}

		double newProgress = progress+(100.0/height);
		if((int)newProgress!=(int)progress) cout<<"progress:"<<(int)progress<<"%"<<endl;
		progress = newProgress;
	}

	cout<<"vertex num:"<<vertices.size()<<endl;
	cout<<"face num:"<<faces.size()<<endl;
	return colorBuffer;
}

void ScanLineZBuffer::run(const char* obj_file)
{
	if(loadObj(obj_file))
	{
		if(!vertices.size()||!faces.size()) return;
		GlutDisplay::render(this);
	}

}