#include <iostream>
#include "Utils.h"
#include "ScanLineZBuffer.h"
using namespace std;

int main(int argc,char** argv)
{
	string objDirectory="../data";	
	vector<string> files = Utils::getFiles(objDirectory,"obj");

	if(files.size())
	{
		for(int i=0,len=files.size();i<len;++i)
		{
			cout<<i<<":"<<files[i]<<endl;
		}

		int id;
		while(true)
		{
			cout<<"please choose one obj file to render: ";
			cin>>id;
			if(id>=0&&id<files.size()) break;
			else cout<<"valid input"<<endl;
		}

		string objFile =  objDirectory+"/"+files[id];

		ScanLineZBuffer scanLineZBuffer;
		scanLineZBuffer.run(objFile.c_str());
	}

	return 0;
}