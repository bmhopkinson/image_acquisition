#include "system/LogUtils.h"

#include <sys/stat.h>

int mkdir_recursive(const char* pathname, mode_t mode) {
	std::string pathstring(pathname);
	std::string pathtemp;

	int ret = 0;
	int searched = 0;
	do {
		searched = pathstring.find('/', searched+1);
		pathtemp.assign(pathstring, 0, searched);
		ret = mkdir(pathtemp.c_str(), mode);
	} while (searched != std::string::npos);

	return ret;
}
