#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_() {
	std::ifstream in;
	in.open(filename, std::ifstream::in);

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
			verts_.push_back(newVector);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> newFace;
			int passIndex, vertexIndex;
			iss >> trash;
			while (iss >> vertexIndex >> trash >> passIndex >> trash >> passIndex) {
				vertexIndex--; //индексы не с нуля
				newFace.push_back(vertexIndex);
			}
			faces_.push_back(newFace);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::VerticeAmount() {
	return (int)verts_.size();
}

int Model::FaceAmount() {
	return (int)faces_.size();
}

std::vector<int> Model::Face(int idx) {
	return faces_[idx];
}

FloatVector3 Model::Vertice(int i) {
	return verts_[i];
}

