#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
public:
	Model(const char *filename);
	~Model();
	int VerticeAmount();
	int FaceAmount();
	Vec3f Vertice(int i);
	std::vector<int> Face(int idx);
};

#endif //__MODEL_H__
