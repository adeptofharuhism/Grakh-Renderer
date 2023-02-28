#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* fileName) : _verts(), _faces(), _texVerts() {
	std::ifstream in;
	in.open(fileName, std::ifstream::in);

	if (in.fail())
		return;

	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			FloatVector3 newVector;
			for (int i = 0; i < 3; i++)
				iss >> newVector.raw[i];
			_verts.push_back(newVector);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<IntVector3> newFace;
			int vertexIndex, textureVertexIndex, normaleVertexIndex;
			iss >> trash;
			while (iss >> vertexIndex >> trash >> textureVertexIndex >> trash >> normaleVertexIndex) {
				vertexIndex--; //индексы не с нуля, а с 1
				textureVertexIndex--;
				normaleVertexIndex--;
				newFace.push_back(IntVector3(vertexIndex, textureVertexIndex, normaleVertexIndex));
			}
			_faces.push_back(newFace);
		}
		else if (!line.compare(0, 3, "vt ")) {
			int floatTrash;
			FloatVector2 newVector;
			iss >> trash >> trash >> newVector.raw[0] >> newVector.raw[1] >> floatTrash;
			_texVerts.push_back(newVector);
		}
	}
	std::cerr << "v# " << _verts.size() << " f# " << _faces.size() << " vt# " << _texVerts.size() << std::endl;
}

Model::~Model() {
}

int Model::VerticeAmount() {
	return (int)_verts.size();
}

int Model::FaceAmount() {
	return (int)_faces.size();
}

int Model::TextureVerticeAmount() {
	return (int)_texVerts.size();
}

std::vector<IntVector3> Model::Face(int idx) {
	return _faces[idx];
}

FloatVector3 Model::Vertice(int i) {
	return _verts[i];
}

FloatVector2 Model::TextureVertice(int i) {
	return _texVerts[i];
}
