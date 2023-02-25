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

void FilledTriangleLineSweeping(IntVector2 point0, IntVector2 point1, IntVector2 point2, TGAImage* image, TGAColor color) {
	if (point0.y > point1.y) std::swap(point0, point1);
	if (point0.y > point2.y) std::swap(point0, point2);
	if (point1.y > point2.y) std::swap(point1, point2);

	const int totalHeight = point2.y - point0.y + 1;
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

FloatVector3 ToBarycentricCoordinates(IntVector2 point, IntVector2* points) {
	FloatVector3 crossProduct =
		FloatVector3(points[1].x - points[0].x, points[2].x - points[0].x, points[0].x - point.x) ^
		FloatVector3(points[1].y - points[0].y, points[2].y - points[0].y, points[0].y - point.y);

	if (std::abs(crossProduct.z) < 1)
		return FloatVector3(-1, 1, 1);

	return FloatVector3(
		1. - (crossProduct.x + crossProduct.y) / crossProduct.z,
		crossProduct.x / crossProduct.z,
		crossProduct.y / crossProduct.z
	);
}

inline bool IsBarycentricValid(FloatVector3 barycentric) {
	return barycentric.x >= 0 && barycentric.y >= 0 && barycentric.z >= 0;
}

void FilledTriangleBarycentric(IntVector2* points, TGAImage* image, TGAColor color) {
	int imageWidth = image->GetWidth(),
		imageHeight = image->GetHeight();

	IntVector2 bboxMin(imageWidth - 1, imageHeight - 1);
	IntVector2 bboxMax(0, 0);

	for (int i = 0; i < 3; i++) {
		bboxMin.x = std::min(points[i].x, bboxMin.x);
		bboxMax.x = std::max(points[i].x, bboxMax.x);

		bboxMin.y = std::min(points[i].y, bboxMin.y);
		bboxMax.y = std::max(points[i].y, bboxMax.y);
	}

	IntVector2 iterationPoint;
	FloatVector3 barycentricScreenCoords;
	for (iterationPoint.x = bboxMin.x; iterationPoint.x < bboxMax.x; iterationPoint.x++) {
		for (iterationPoint.y = bboxMin.y; iterationPoint.y < bboxMax.y; iterationPoint.y++) {
			barycentricScreenCoords = ToBarycentricCoordinates(iterationPoint, points);
			if (IsBarycentricValid(barycentricScreenCoords))
				image->Set(iterationPoint.x, iterationPoint.y, color);
		}
	}
}

void FilledTriangleBarycentricWithBuffer(IntVector2* points, float* depths, float** zBuffer, TGAImage* image, TGAColor color) {
	int imageWidth = image->GetWidth(),
		imageHeight = image->GetHeight();

	IntVector2 bboxMin(imageWidth - 1, imageHeight - 1);
	IntVector2 bboxMax(0, 0);

	for (int i = 0; i < 3; i++) {
		bboxMin.x = std::min((int)points[i].x, bboxMin.x);
		bboxMax.x = std::max((int)points[i].x, bboxMax.x);

		bboxMin.y = std::min((int)points[i].y, bboxMin.y);
		bboxMax.y = std::max((int)points[i].y, bboxMax.y);
	}

	IntVector2 iterationPoint;
	float depth;
	FloatVector3 barycentricScreenCoords;
	for (iterationPoint.x = bboxMin.x; iterationPoint.x < bboxMax.x; iterationPoint.x++) {
		for (iterationPoint.y = bboxMin.y; iterationPoint.y < bboxMax.y; iterationPoint.y++) {
			barycentricScreenCoords = ToBarycentricCoordinates(iterationPoint, points);
			if (IsBarycentricValid(barycentricScreenCoords)) {
				depth = 0;
				depth += depths[0] * barycentricScreenCoords.x;
				depth += depths[1] * barycentricScreenCoords.y;
				depth += depths[2] * barycentricScreenCoords.z;

				if (zBuffer[iterationPoint.x][iterationPoint.y] < depth) {
					zBuffer[iterationPoint.x][iterationPoint.y] = depth;
					image->Set(iterationPoint.x, iterationPoint.y, color);

					//code to watch zBuffer image
					//int a = zBuffer[iterationPoint.x][iterationPoint.y] * 255;
					//image->Set(iterationPoint.x, iterationPoint.y, TGAColor(a, a, a, 255));
				}
			}
		}
	}
}

IntVector2 ToScreenCoordinates(FloatVector3 vector, int width, int height) {
	return IntVector2((vector.x + 1.) * width / 2, (vector.y + 1.) * height / 2);
}

int main(int argc, char** argv) {
	FloatVector3 lightDirection(0, 0, -1);
	int width = 1920,
		height = 1080;

	float** zBuffer = new float* [width];
	for (int i = 0; i < width; i++) {
		zBuffer[i] = new float[height];
		for (int j = 0; j < height; j++) {
			zBuffer[i][j] = std::numeric_limits<float>::min();
		}
	}

	TGAImage image(width, height, TGAImage::RGB);

	Model model("obj/african_head.obj");
	for (int i = 0; i < model.FaceAmount(); i++) {
		std::vector<int> face = model.Face(i);

		IntVector2 screenCoordinates[3];
		float depths[3];
		FloatVector3 worldCoordinates[3];
		for (int i = 0; i < 3; i++) {
			FloatVector3 vertice = model.Vertice(face[i]);
			screenCoordinates[i] = ToScreenCoordinates(vertice, width, height);
			depths[i] = (vertice.z + 1.)/2;
			worldCoordinates[i] = vertice;
		}

		FloatVector3 crossProduct =
			(worldCoordinates[2] - worldCoordinates[0]) ^ (worldCoordinates[1] - worldCoordinates[0]);
		crossProduct.Normalize();

		float intensity = lightDirection * crossProduct;

		if (intensity > 0)
			FilledTriangleBarycentricWithBuffer(screenCoordinates, depths, zBuffer, &image,
				//TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
				TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	}

	image.FlipVertically();

	const char* filename = "output.tga";
	image.WriteTGAFile(filename);
	system(filename);

	return 0;
}