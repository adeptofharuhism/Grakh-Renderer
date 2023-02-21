#include "TGAImage/tgaimage.h"
#include "model.h"

const TGAColor White = TGAColor(255, 255, 255, 255);
const TGAColor Red = TGAColor(255, 0, 0, 255);

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
			image->set(y, x, color);

			error += deltaError;
			if (error > deltaX) {
				y += yStep;
				error -= deltaX * 2;
			}
		}
	}
	else {
		for (int x = x0; x < x1; x++) {
			image->set(x, y, color);

			error += deltaError;
			if (error > deltaX) {
				y += yStep;
				error -= deltaX * 2;
			}
		}
	}
}

int main(int argc, char** argv) {
	int width = 640,
		height = 480;
	TGAImage image(width, height, TGAImage::RGB);

	Model model("obj/african_head.obj");
	for (int i = 0; i < model.FaceAmount(); i++) {
		std::vector<int> face = model.Face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model.Vertice(face[j]);
			Vec3f v1 = model.Vertice(face[(j + 1) % 3]);

			int x0 = (v0.x + 1.) * width / 2;
			int y0 = (v0.y + 1.) * height / 2;

			int x1 = (v1.x + 1.) * width / 2;
			int y1 = (v1.y + 1.) * height / 2;

			Line(x0, y0, x1, y1, &image, Red);
		}
	}

	image.flip_vertically();

	const char* filename = "output.tga";
	image.write_tga_file(filename);
	system(filename);

	return 0;
}