#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<FloatVector3> _verts;
	std::vector<std::vector<IntVector3>> _faces;
	std::vector<FloatVector2> _texVerts;
public:
	Model(const char* filename);
	~Model();
	int VerticeAmount();
	int FaceAmount();
	int TextureVerticeAmount();
	FloatVector3 Vertice(int i);
	FloatVector2 TextureVertice(int i);
	std::vector<IntVector3> Face(int idx);
};