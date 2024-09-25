#include <GL/glew.h>
#include <chrono>
#include <thread>
#include <exception>
#include <glm/glm.hpp>
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

static const ivec2 WINDOW_SIZE(512, 512);
static const unsigned int FPS = 60;
static const auto FRAME_DT = 1.0s / FPS;

static void init_openGL() {
	glewInit();
	if (!GLEW_VERSION_3_0) throw exception("OpenGL 3.0 API is not available.");
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

static void draw_triangle(const u8vec4& color, const vec3& center, double size) {
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_TRIANGLES);
	glVertex3d(center.x, center.y + size, center.z);
	glVertex3d(center.x - size, center.y - size, center.z);
	glVertex3d(center.x + size, center.y - size, center.z);
	glEnd();
}

static void display_func(Json::Value data) {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_triangle(u8vec4(data["color"][0].asInt(), data["color"][1].asInt(), data["color"][2].asInt(), data["color"][3].asInt()), vec3(data["center"][0].asDouble(), data["center"][1].asDouble(), data["center"][2].asDouble()), data["size"].asDouble());
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



	init_openGL();

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