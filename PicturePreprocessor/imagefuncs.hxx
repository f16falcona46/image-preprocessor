#ifndef IMAGEPROCESSING_FUNCTIONS_H
#define IMAGEPROCESSING_FUNCTIONS_H

#include <stdexcept>
#include <cstdio>
#include <csetjmp>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <Eigen/Dense>
#include <png.h>
namespace ifx {
std::vector<Eigen::MatrixXd> readbin(const std::string& filename, int width, int height);

template <typename MatrixType>
void write_image(const std::vector<MatrixType>& layers, const std::string& filename) {
	std::unique_ptr<std::FILE, int(*)(std::FILE*)> file (std::fopen(filename.c_str(), "wb"), std::fclose);
	std::unique_ptr<png_struct, std::function<void(png_struct*)>> png_ptr (png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr), [](png_structp p) {png_destroy_write_struct(&p, nullptr);});
	std::unique_ptr<png_info, std::function<void(png_info*)>> info_ptr (png_create_info_struct(png_ptr.get()), [&png_ptr](png_infop p){png_free_data(png_ptr.get(), p, PNG_FREE_ALL, -1);});
	std::vector<png_byte> row (layers[0].rows()*3);
	
	if (file == nullptr) {
		throw std::runtime_error(std::string("Could not open PNG file ") + filename);
	}
	if (png_ptr == nullptr) {
		throw std::runtime_error("Could not allocate PNG write struct");
	}
	if (info_ptr == nullptr) {
		throw std::runtime_error("Could not allocate PNG info struct");
	}
	if (setjmp(png_jmpbuf(png_ptr.get()))) {
		throw std::runtime_error("PNG writing failed");
	}
	png_init_io(png_ptr.get(), file.get());
	png_set_IHDR(png_ptr.get(), info_ptr.get(), layers[0].rows(), layers[0].cols(), 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr.get(), info_ptr.get());
	
	for (int y = 0; y < layers[0].cols(); ++y) {
		std::fill(row.begin(), row.end(), 0);
		for (int x = 0; x < layers[0].rows(); ++x) {
			for (typename std::vector<MatrixType>::size_type i = 0; i < layers.size(); ++i) {
				row[x*3+i] = (png_byte)(((double)(layers[i](x,y)))*255);
			}
		}
		png_write_row(png_ptr.get(), row.data());
	}
	png_write_end(png_ptr.get(), nullptr);
}
}

#endif