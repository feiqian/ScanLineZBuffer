#include "ObjLoader.h"
#include <fstream>
#include <string>

bool ObjLoader::loadFromFile(const char* objPath)
{
	ifstream file(objPath);
	if(!file.is_open()) return false;

	string type;

	while (file>>type)
	{
		if(type=="v")
		{
			ObjVertex vt;
			file>>vt.point.x>>vt.point.y>>vt.point.z;
			vertices.push_back(vt);
		}
		else if(type=="f")
		{
			ObjFace face;
			int vIndex,tIndex,nIndex;
			int faceIndex = faces.size();

			while (true)
			{
				char ch = file.get();
				if(ch==' ') continue;
				else if(ch == '\n' || ch == EOF) break;
				else file.putback(ch);
				
				file>>vIndex;

				char splitter = file.get();
				nIndex = 0;

				if(splitter=='/')
				{
					splitter = file.get();
					if(splitter=='/')
					{
						file>>nIndex;
					}
					else
					{
						file.putback(splitter);
						file>>tIndex;
						splitter = file.get();
						if(splitter=='/')
						{
							file>>nIndex;
						}
						else file.putback(splitter);
					}
				}
				else file.putback(splitter);

				face.vertexIndex.push_back(vIndex-1);
				face.normalIndex.push_back(nIndex-1);
			}
			if(face.vertexIndex.size()>2){
				Point3 &a = vertices[face.vertexIndex[0]].point,&b = vertices[face.vertexIndex[1]].point,&c = vertices[face.vertexIndex[2]].point;
				Vec3& normal = Normalize(Cross(b-a,c-b));
				
				face.normal = normal;
				faces.push_back(face);
			}
		}
		else if(type=="vn")
		{
			Vec3 vn;
			file>>vn.x>>vn.y>>vn.z;
			normals.push_back(vn);
		}
	}

	file.close();
	return true;
}