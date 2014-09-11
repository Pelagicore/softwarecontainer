#include <string>
#include <sys/stat.h>

namespace pelagicontain {

	bool isDirectory(const std::string &path)
	{
		bool isDir = false;
		struct stat st;
		if (stat(path.c_str(), &st) == 0) {
			if ((st.st_mode & S_IFDIR) != 0) {
				isDir = true;
			}
		}
		return isDir;
	}

}
