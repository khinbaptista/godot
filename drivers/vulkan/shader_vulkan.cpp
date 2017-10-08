#include "shader_vulkan.h"

#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

using std::string;
using std::vector;

static vector<char> read_file(const string &filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	ERR_EXPLAIN("Failed to open file" + filename.c_str());
	ERR_FAIL_COND_V(!file.is_open(), {});

	size_t size = (size_t)file.tellg();
	vector<char> buffer(size);

	file.seekg(0);
	file.read(buffer.data(), size);

	file.close();
	return buffer;
}
