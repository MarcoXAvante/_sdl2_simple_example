#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL_events.h>
#include "MyWindow.h"
#include <fstream>
#include <iostream>
#include "json/json.h"

using namespace std;

using hrclock = chrono::high_resolution_clock;
using u8vec4 = glm::u8vec4;
using ivec2 = glm::ivec2;
using vec3 = glm::dvec3;
using mat4 = glm::dmat4x4;

static const ivec2 WINDOW_SIZE(512, 512);

class GraphicObject {
	mat4 _mat = glm::identity<mat4>();

public:
	const mat4& mat() const { return _mat; }
	vec3& pos() { return *(vec3*)&_mat[3]; }
	const vec3& fwd() { return *(vec3*)&_mat[2]; }
	const vec3& up() { return *(vec3*)&_mat[1]; }
	const vec3& left() { return *(vec3*)&_mat[0]; }

	void translate(const vec3& v) { _mat = glm::translate(_mat, v); };
	void rotate(double angle, const vec3& axis) { _mat = glm::rotate(_mat, angle, axis); };

	virtual void paint() = 0;
};

struct Camera : public GraphicObject {
	vec3 target() { return pos() + fwd(); }

	double fov = glm::radians(60.0);
	double zNear = 0.1;
	double zFar = 100.0;

	double aspect() { return static_cast<double>(WINDOW_SIZE.x) / WINDOW_SIZE.y; }

	void paint() override{
		glMatrixMode(GL_PROJECTION);
		mat4 proj_mat = glm::perspective(fov, aspect(), zNear, zFar);
		glLoadMatrixd(&proj_mat[0][0]);

		glMatrixMode(GL_MODELVIEW);
		mat4 view_mat = glm::lookAt(pos(), target(), up());
		glLoadMatrixd(&view_mat[0][0]);
	}

};

struct Triangle : public GraphicObject {
	u8vec4 color = u8vec4(255, 0, 0, 255);
	double size = 0.5;
	vec3 center = vec3(0, 0, 1);

	void paint() override {
		glMultMatrixd(&mat()[0][0]);

		glColor4ub(color.r, color.g, color.b, color.a);
		glBegin(GL_TRIANGLES);
		glVertex3d(center.x, center.y + size, center.z);
		glVertex3d(center.x - size, center.y - size, center.z);
		glVertex3d(center.x + size, center.y - size, center.z);
		glEnd();
	};
};

static Camera camera;
static Triangle triangle;

static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

static void init_openGL() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

static void draw_triangle(const u8vec4& color, const vec3& center, double size) {
	triangle.color = color;
	triangle.center = center;
	triangle.size = size;
}

static void display_func(Json::Value data) {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//camera.mat = glm::translate(camera.mat, vec3(0, 0, -0.01));
	//camera.pos().z += 0.01;
	draw_triangle(u8vec4(data["color"][0].asInt(), data["color"][1].asInt(), data["color"][2].asInt(), data["color"][3].asInt()), vec3(data["center"][0].asDouble(), data["center"][1].asDouble(), data["center"][2].asDouble()), data["size"].asDouble());

	triangle.paint();

	camera.rotate(1, camera.up());

	camera.paint();
	
	
	triangle.rotate(0.01, vec3(0, 0, 1));
}

static void init_opengl() {
	glewInit();
	glClearColor(0.5, 0.5, 0.5, 1.0);

	//camera.mat = glm::translate( camera.mat, vec3(0, 0, 1));
	camera.pos() = vec3(0, 0, 1.5);
	camera.rotate(180, camera.up());
}

static bool processEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv) {
	MyWindow window("SDL2 Simple Example", WINDOW_SIZE.x, WINDOW_SIZE.y);
	ifstream file("data.json", ifstream::binary);
	if (!file.is_open()) {
		cerr << "Error opening file!" << endl;
		return 1;
	}

	Json::Value jsonData;
	Json::CharReaderBuilder readerBuilder;
	string errs;

	if (!Json::parseFromStream(readerBuilder, file, &jsonData, &errs)) {
		cerr << "Error parsing the JSON file: " << errs << endl;
		return 1;
	}



	init_opengl();

	while (processEvents()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		const auto t0 = hrclock::now();
		display_func(jsonData["triangle"]);
		display_func(jsonData["triangle2"]);
		display_func(jsonData["triangle3"]);
		window.swapBuffers();
		const auto t1 = hrclock::now();
		const auto dt = t1 - t0;
		if(dt<FRAME_DT) this_thread::sleep_for(FRAME_DT - dt);
	}

	file.close();

	return 0;
}