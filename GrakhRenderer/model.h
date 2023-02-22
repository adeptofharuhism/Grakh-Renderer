#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<FloatVector3> verts_;
	std::vector<std::vector<int> > faces_;
public:
	Model(const char *filename);
	~Model();
	int VerticeAmount();
	int FaceAmount();
	FloatVector3 Vertice(int i);
	std::vector<int> Face(int idx);
};