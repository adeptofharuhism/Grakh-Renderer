#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tgaimage.h"

TGAImage::TGAImage() : _data(NULL), _width(0), _height(0), _bytespp(0) {
}

TGAImage::TGAImage(int w, int h, int bpp) : _data(NULL), _width(w), _height(h), _bytespp(bpp) {
	unsigned long nbytes = _width*_height*_bytespp;
	_data = new unsigned char[nbytes];
	memset(_data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage &img) {
	_width = img._width;
	_height = img._height;
	_bytespp = img._bytespp;
	unsigned long nbytes = _width*_height*_bytespp;
	_data = new unsigned char[nbytes];
	memcpy(_data, img._data, nbytes);
}

TGAImage::~TGAImage() {
	if (_data) delete [] _data;
}

TGAImage & TGAImage::operator =(const TGAImage &img) {
	if (this != &img) {
		if (_data) delete [] _data;
		_width  = img._width;
		_height = img._height;
		_bytespp = img._bytespp;
		unsigned long nbytes = _width*_height*_bytespp;
		_data = new unsigned char[nbytes];
		memcpy(_data, img._data, nbytes);
	}
	return *this;
}

bool TGAImage::ReadTGAFile(const char *filename) {
	if (_data) delete [] _data;
	_data = NULL;
	std::ifstream in;
	in.open (filename, std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		in.close();
		return false;
	}
	TGA_Header header;
	in.read((char *)&header, sizeof(header));
	if (!in.good()) {
		in.close();
		std::cerr << "an error occured while reading the header\n";
		return false;
	}
	_width   = header.width;
	_height  = header.height;
	_bytespp = header.bitsperpixel>>3;
	if (_width<=0 || _height<=0 || (_bytespp!=GRAYSCALE && _bytespp!=RGB && _bytespp!=RGBA)) {
		in.close();
		std::cerr << "bad bpp (or width/height) value\n";
		return false;
	}
	unsigned long nbytes = _bytespp*_width*_height;
	_data = new unsigned char[nbytes];
	if (3==header.datatypecode || 2==header.datatypecode) {
		in.read((char *)_data, nbytes);
		if (!in.good()) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	} else if (10==header.datatypecode||11==header.datatypecode) {
		if (!LoadRLEData(in)) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	} else {
		in.close();
		std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
		return false;
	}
	if (!(header.imagedescriptor & 0x20)) {
		FlipVertically();
	}
	if (header.imagedescriptor & 0x10) {
		FlipHorizontally();
	}
	std::cerr << _width << "x" << _height << "/" << _bytespp*8 << "\n";
	in.close();
	return true;
}

bool TGAImage::LoadRLEData(std::ifstream &in) {
	unsigned long pixelcount = _width*_height;
	unsigned long currentpixel = 0;
	unsigned long currentbyte  = 0;
	TGAColor colorbuffer;
	do {
		unsigned char chunkheader = 0;
		chunkheader = in.get();
		if (!in.good()) {
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
		if (chunkheader<128) {
			chunkheader++;
			for (int i=0; i<chunkheader; i++) {
				in.read((char *)colorbuffer.raw, _bytespp);
				if (!in.good()) {
					std::cerr << "an error occured while reading the header\n";
					return false;
				}
				for (int t=0; t<_bytespp; t++)
					_data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel>pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		} else {
			chunkheader -= 127;
			in.read((char *)colorbuffer.raw, _bytespp);
			if (!in.good()) {
				std::cerr << "an error occured while reading the header\n";
				return false;
			}
			for (int i=0; i<chunkheader; i++) {
				for (int t=0; t<_bytespp; t++)
					_data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel>pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

bool TGAImage::WriteTGAFile(const char *filename, bool rle) {
	unsigned char developer_area_ref[4] = {0, 0, 0, 0};
	unsigned char extension_area_ref[4] = {0, 0, 0, 0};
	unsigned char footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
	std::ofstream out;
	out.open (filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		out.close();
		return false;
	}
	TGA_Header header;
	memset((void *)&header, 0, sizeof(header));
	header.bitsperpixel = _bytespp<<3;
	header.width  = _width;
	header.height = _height;
	header.datatypecode = (_bytespp==GRAYSCALE?(rle?11:3):(rle?10:2));
	header.imagedescriptor = 0x20; // top-left origin
	out.write((char *)&header, sizeof(header));
	if (!out.good()) {
		out.close();
		std::cerr << "can't dump the tga file\n";
		return false;
	}
	if (!rle) {
		out.write((char *)_data, _width*_height*_bytespp);
		if (!out.good()) {
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
	} else {
		if (!UnloadRLEData(out)) {
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

bool TGAImage::UnloadRLEData(std::ofstream &out) {
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = _width*_height;
	unsigned long curpix = 0;
	while (curpix<npixels) {
		unsigned long chunkstart = curpix*_bytespp;
		unsigned long curbyte = curpix*_bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix+run_length<npixels && run_length<max_chunk_length) {
			bool succ_eq = true;
			for (int t=0; succ_eq && t<_bytespp; t++) {
				succ_eq = (_data[curbyte+t]==_data[curbyte+t+_bytespp]);
			}
			curbyte += _bytespp;
			if (1==run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw?run_length-1:run_length+127);
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
		out.write((char *)(_data+chunkstart), (raw?run_length*_bytespp:_bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
	return true;
}

TGAColor TGAImage::Get(int x, int y) {
	if (!_data || x<0 || y<0 || x>=_width || y>=_height) {
		return TGAColor();
	}
	return TGAColor(_data+(x+y*_width)*_bytespp, _bytespp);
}

bool TGAImage::Set(int x, int y, TGAColor c) {
	if (!_data || x<0 || y<0 || x>=_width || y>=_height) {
		return false;
	}
	memcpy(_data+(x+y*_width)*_bytespp, c.raw, _bytespp);
	return true;
}

int TGAImage::GetBytespp() {
	return _bytespp;
}

int TGAImage::GetWidth() {
	return _width;
}

int TGAImage::GetHeight() {
	return _height;
}

bool TGAImage::FlipHorizontally() {
	if (!_data) return false;
	int half = _width>>1;
	for (int i=0; i<half; i++) {
		for (int j=0; j<_height; j++) {
			TGAColor c1 = Get(i, j);
			TGAColor c2 = Get(_width-1-i, j);
			Set(i, j, c2);
			Set(_width-1-i, j, c1);
		}
	}
	return true;
}

bool TGAImage::FlipVertically() {
	if (!_data) return false;
	unsigned long bytes_per_line = _width*_bytespp;
	unsigned char *line = new unsigned char[bytes_per_line];
	int half = _height>>1;
	for (int j=0; j<half; j++) {
		unsigned long l1 = j*bytes_per_line;
		unsigned long l2 = (_height-1-j)*bytes_per_line;
		memmove((void *)line,      (void *)(_data+l1), bytes_per_line);
		memmove((void *)(_data+l1), (void *)(_data+l2), bytes_per_line);
		memmove((void *)(_data+l2), (void *)line,      bytes_per_line);
	}
	delete [] line;
	return true;
}

unsigned char *TGAImage::Buffer() {
	return _data;
}

void TGAImage::Clear() {
	memset((void *)_data, 0, _width*_height*_bytespp);
}

bool TGAImage::Scale(int w, int h) {
	if (w<=0 || h<=0 || !_data) return false;
	unsigned char *tdata = new unsigned char[w*h*_bytespp];
	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;
	unsigned long nlinebytes = w*_bytespp;
	unsigned long olinebytes = _width*_bytespp;
	for (int j=0; j<_height; j++) {
		int errx = _width-w;
		int nx   = -_bytespp;
		int ox   = -_bytespp;
		for (int i=0; i<_width; i++) {
			ox += _bytespp;
			errx += w;
			while (errx>=(int)_width) {
				errx -= _width;
				nx += _bytespp;
				memcpy(tdata+nscanline+nx, _data+oscanline+ox, _bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry>=(int)_height) {
			if (erry>=(int)_height<<1) // it means we jump over a scanline
				memcpy(tdata+nscanline+nlinebytes, tdata+nscanline, nlinebytes);
			erry -= _height;
			nscanline += nlinebytes;
		}
	}
	delete [] _data;
	_data = tdata;
	_width = w;
	_height = h;
	return true;
}

