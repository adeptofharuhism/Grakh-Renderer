#include "TGAImage/tgaimage.h"
#include "model.h"
#include <iostream>

const TGAColor White = TGAColor(255, 255, 255, 255);
const TGAColor Red = TGAColor(255, 0, 0, 255);
const TGAColor Green = TGAColor(0, 255, 0, 255);

void Line(int x0, int y0, int x1, int y1, TGAImage* image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	const int yStep = y1 > y0 ? 1 : -1;
	const int deltaX = x1 - x0;
	const int deltaY = y1 - y0;
	float deltaError = std::abs(deltaY * 2);
	float error = 0;
	int y = y0;

	if (steep) {
		for (int x = x0; x < x1; x++) {
			image->Set(y, x, color);

			error += deltaError;
			if (error > deltaX) {
				y += yStep;
				error -= deltaX * 2;
			}
		}
	}
	else {
		for (int x = x0; x < x1; x++) {
			image->Set(x, y, color);

			error += deltaError;
			if (error > deltaX) {
				y += yStep;
				error -= deltaX * 2;
			}
		}
	}
}

void Triangle(IntVector2 point0, IntVector2 point1, IntVector2 point2, TGAImage* image, TGAColor color) {
	Line(point0.x, point0.y, point1.x, point1.y, image, color);
	Line(point1.x, point1.y, point2.x, point2.y, image, color);
	Line(point2.x, point2.y, point0.x, point0.y, image, color);
}

void FilledTriangle(IntVector2 point0, IntVector2 point1, IntVector2 point2, TGAImage* image, TGAColor color) {
	if (point0.y > point1.y) std::swap(point0, point1);
	if (point0.y > point2.y) std::swap(point0, point2);
	if (point1.y > point2.y) std::swap(point1, point2);

	const int totalHeight = point2.y - point0.y+1;
	const int firstSegmentHeight = point1.y - point0.y + 1;
	float boundaryPartThigh;
	float boundaryPartBase;
	IntVector2 basePoint;
	IntVector2 thighPoint;
	for (int y = point0.y; y <= point1.y; y++) {
		boundaryPartThigh = (float)(y - point0.y) / firstSegmentHeight;
		boundaryPartBase = (float)(y - point0.y) / totalHeight;

		basePoint = point0 + (point2 - point0) * boundaryPartBase;
		thighPoint = point0 + (point1 - point0) * boundaryPartThigh;
		if (basePoint.x > thighPoint.x) std::swap(thighPoint, basePoint);

		for (int x = basePoint.x; x <= thighPoint.x; x++) {
			image->Set(x, y, color);
		}
	}

	const int secondSegmentHeight = point2.y - point1.y + 1;
	for (int y = point1.y; y <= point2.y; y++) {
		boundaryPartThigh = (float)(y - point1.y) / secondSegmentHeight;
		boundaryPartBase = (float)(y - point0.y) / totalHeight;

		basePoint = point0 + (point2 - point0) * boundaryPartBase;
		thighPoint = point1 + (point2 - point1) * boundaryPartThigh;
		if (basePoint.x > thighPoint.x) std::swap(thighPoint, basePoint);

		for (int x = basePoint.x; x <= thighPoint.x; x++) {
			image->Set(x, y, color);
		}
	}
}

IntVector2 ToScreenCoordinates(FloatVector3 vector, int width, int height) {
	return IntVector2((vector.x + 1.) * width / 2, (vector.y + 1.) * height / 2);
}

int main(int argc, char** argv) {
	int width = 1920,
		height = 1080;

	TGAImage image(width, height, TGAImage::RGB);

	FilledTriangle(IntVector2(1000, 687), IntVector2(50, 687), IntVector2(800, 687), &image, Green);

	Model model("obj/african_head.obj");
	for (int i = 0; i < model.FaceAmount(); i++) {

		std::vector<int> face = model.Face(i);

		IntVector2 vectors[3];
		for (int i = 0; i < 3; i++) {
			vectors[i] = ToScreenCoordinates(model.Vertice(face[i]), width, height);
		}

		FilledTriangle(vectors[0], vectors[1], vectors[2], &image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
	}

	image.FlipVertically();

	const char* filename = "output.tga";
	image.WriteTGAFile(filename);
	system(filename);

	return 0;
}