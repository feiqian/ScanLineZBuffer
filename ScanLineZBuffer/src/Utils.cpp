#include "Utils.h"

//²Î¿¼£ºhttps://github.com/liuruoze/EasyPR/blob/master/src/util/util.cpp

std::vector<std::string> Utils::getFiles(const std::string& folder, const std::string& fileExtension)
{
	std::string path = folder+"\\*."+fileExtension;
	std::vector<std::string> files;
	struct _finddata_t file_info;
	//https://social.msdn.microsoft.com/Forums/silverlight/en-US/2e7dc3cb-c448-43e7-9587-01abcc532d55/findnext-api-crashes-for-x64bit-application-in-windows-81-and-windows-server-2012?forum=vcgeneral
	intptr_t  file_handler = _findfirst(path.c_str(), &file_info);

	while (file_handler != -1)
	{
		if (!(file_info.attrib & _A_SUBDIR)) files.push_back(file_info.name);
		if (_findnext(file_handler, &file_info) != 0) break;
	}
	_findclose(file_handler);

	return files;
}