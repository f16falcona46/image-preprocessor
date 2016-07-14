#include <cstdint>
#include <vector>
#include <cstdio>
#include <stdexcept>
#include <Eigen/Dense>
#include "imagefuncs.hxx"

std::vector<Eigen::MatrixXd> ifx::readbin(const std::string& filename, int width, int height) {
	std::vector<uint16_t> buf(2*width*height);
	FILE* f;
	errno_t err = fopen_s(&f, filename.c_str(), "rb");
	if (err) throw std::runtime_error(std::string("Could not open BIN file ")+filename);
	fread(buf.data(), sizeof(uint16_t), 2*width*height, f);
	std::vector<Eigen::MatrixXd> layers;
	layers.emplace_back(width, height);
	layers.emplace_back(width, height);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			layers[0](x,y) = (double)(buf[y*width+x]);
			layers[1](x,y) = (double)(buf[y*width+x + width*height]);
		}
	}
	fclose(f);
	return layers;
}