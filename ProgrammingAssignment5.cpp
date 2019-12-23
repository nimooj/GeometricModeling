// ProgrammingAssignment5.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#define PI 3.1415926535897

#include "./include/GL/glew.h"
#include "./include/GL/wglew.h"

#include "./include/GLFW/glfw3.h"

#include "./include/GL/glut.h"
#include "./include/glm/glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "./include/glm/gtx/transform.hpp"
#include "./include/glm/gtc/type_ptr.hpp"
#include "./include/glm/gtc/matrix_transform.hpp"

#include "Camera.h"

using namespace glm;
using namespace std;

GLFWwindow* window;
mat4 projectionMatrix;
mat4 viewMatrix;
mat4 modelMatrix;

Camera camera;
vec3 pos, dir, up;

GLuint shaderProgram, vertexShader, fragmentShader;
GLuint loadShaders(const char*, const char*);

bool rdrag = false, ldrag = false;
int selectedPoint = -1;
int selectedControl = -1;
void mouseCallback(GLFWwindow*, int, int, int);
void pointDrag();

float getDist(vec2, vec2);

/*** POINT ***/
vec2 point;
GLuint point_vao;
GLuint point_position_vbo, point_color_vbo;
vec3 ptPosition, ptColor;
void definePointVAO(vec2);

/*** LINE ***/
GLuint line_vao, line_ebo;
GLuint line_position_vbo, line_color_vbo;
vector<vec3> linePosition, lineColor;
vector<int> lineIndices;
void drawLine(vec2, vec2);
void defineLineVAO(vec2, vec2);

/*** PROJECTION ***/
GLuint projection_vao;
GLuint projection_position_vbo, projection_color_vbo;
vector<vec3> projectionPosition, projectionColor;
void drawProjection(vec2);
void defineProjectionVAO(vec2);

/*** SEGMENTATION ***/
GLuint segment_vao;
GLuint segment_position_vbo, segment_color_vbo;
vector<vec3> segmentPosition, segmentColor;
void drawSegment(vec2);
void defineSegmentVAO(vec2);

/*** CONTROLS ***/
vector<vec2> control;
GLuint control_vao, control_ebo;
GLuint control_position_vbo, control_color_vbo;
vector<vec3> controlPosition, controlColor;
vector<int> controlIndices;
void defineControlVAO();

/*** BEZIER ***/
vector<vec2> bezier;
GLuint bezier_vao, bezier_ebo;
GLuint bezier_position_vbo, bezier_color_vbo;
vector<vec3> bezierPosition, bezierColor;
vector<int> bezierIndices;
void defineBezierVAO();

void subdivideCurve();
void projectToCurve();

int main()
{
	if (!glfwInit()) {
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(1000, 1000, "Programming Assignment 03 - Minimum Distance of curves", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	double mouseX = 0, mouseY = 0;
	glfwSetMouseButtonCallback(window, mouseCallback);

	camera = Camera();

	point = vec2(8, 6);

	control.push_back(vec2(-5, -5));
	control.push_back(vec2(-2, 5));
	control.push_back(vec2(2, 5));
	control.push_back(vec2(5, -5));

	for (float t = 0; t <= 1; t += 0.01) {
		float x = pow(1 - t, 3) * control[0].x + 3 * pow(1 - t, 2) * t * control[1].x + 3 * (1 - t) * pow(t, 2) * control[2].x + pow(t, 3) * control[3].x;
		float y = pow(1 - t, 3) * control[0].y + 3 * pow(1 - t, 2) * t * control[1].y + 3 * (1 - t) * pow(t, 2) * control[2].y + pow(t, 3) * control[3].y;
		bezier.push_back(vec3(x, y, 0));
	}

	definePointVAO(point);

	defineControlVAO();
	defineBezierVAO();

	while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (ldrag) {
			pointDrag();
		}

		pos = camera.cameraPosition;
		dir = camera.cameraFront;
		up = camera.cameraUp;

		projectionMatrix = ortho(-10.0f, 10.0f, -10.0f, 10.f, 0.0f, 100.0f);
		viewMatrix = lookAt(pos, dir, up);
		modelMatrix = mat4(1.0f);

		glUseProgram(shaderProgram);
		GLuint projectionID = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projectionMatrix[0][0]);
		GLuint viewID = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewID, 1, GL_FALSE, &viewMatrix[0][0]);
		GLuint modelID = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &modelMatrix[0][0]);

		glPointSize(7);
		glBindVertexArray(point_vao);
		glDrawArrays(GL_POINTS, 0, 1);

		glBindVertexArray(control_vao);
		glDrawArrays(GL_POINTS, 0, controlPosition.size());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo);
		glDrawElements(GL_LINES, controlIndices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(bezier_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo);
		glDrawElements(GL_LINES, bezierIndices.size(), GL_UNSIGNED_INT, 0);

 


		glfwSwapBuffers(window);
		glfwPollEvents();

		glBindVertexArray(0);
	}

	/*
	glDeleteVertexArrays(1, &point_vao);
	glDeleteVertexArrays(1, &control_vao0);
	glDeleteVertexArrays(1, &control_vao1);
	glDeleteVertexArrays(1, &projection_vao);
	glDeleteVertexArrays(1, &line_vao);
	glDeleteVertexArrays(1, &bezier_vao0);
	glDeleteVertexArrays(1, &bezier_vao1);
	glDeleteVertexArrays(1, &circle_vao);

	glDeleteBuffers(1, &point_position_vbo);
	glDeleteBuffers(1, &point_color_vbo);
	glDeleteBuffers(1, &control_position_vbo0);
	glDeleteBuffers(1, &control_position_vbo1);
	glDeleteBuffers(1, &control_color_vbo);
	glDeleteBuffers(1, &projection_position_vbo);
	glDeleteBuffers(1, &projection_color_vbo);
	glDeleteBuffers(1, &line_position_vbo);
	glDeleteBuffers(1, &line_color_vbo);
	glDeleteBuffers(1, &bezier_position_vbo0);
	glDeleteBuffers(1, &bezier_position_vbo1);
	glDeleteBuffers(1, &bezier_color_vbo);
	glDeleteBuffers(1, &circle_position_vbo);
	glDeleteBuffers(1, &circle_color_vbo);
	*/

	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void subdivideCurve() {

}

void projectToCurve() {

}

void definePointVAO(vec2 pt) {
	glGenVertexArrays(1, &point_vao);
	glBindVertexArray(point_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	ptPosition = vec3(pt, 0);
	ptColor = vec3(0, 0, 0);

	glGenBuffers(1, &point_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, point_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3), &ptPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &point_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, point_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3), &ptColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glBindVertexArray(0);
}

void drawLine(vec2 n, vec2 m) {
	defineLineVAO(n, m);
	glBindVertexArray(line_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
	glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, 0);
}

void defineLineVAO(vec2 n, vec2 m) {
	linePosition.clear();
	lineColor.clear();
	lineIndices.clear();

	glGenVertexArrays(1, &line_vao);
	glBindVertexArray(line_vao);
	shaderProgram = loadShaders("shaders/vertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	linePosition.push_back(vec3(n, 0));
	linePosition.push_back(vec3(m, 0));
	for (int i = 0; i < 2; i++) {
		lineColor.push_back(vec3(0, 1, 0));
	}

	lineIndices.push_back(0);
	lineIndices.push_back(1);

	glGenBuffers(1, &line_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, line_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, linePosition.size() * sizeof(vec3), &linePosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &line_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, line_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, lineColor.size() * sizeof(vec3), &lineColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &line_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineIndices.size() * sizeof(int), &lineIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void drawProjection(vec2 pt) {
	glPointSize(8);
	defineProjectionVAO(pt);
	glBindVertexArray(projection_vao);
	glDrawArrays(GL_POINTS, 0, projectionPosition.size());
}

void defineProjectionVAO(vec2 pt) {
	projectionPosition.clear();
	projectionColor.clear();

	glGenVertexArrays(1, &projection_vao);
	glBindVertexArray(projection_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	projectionPosition.push_back(vec3(pt, 0));
	projectionColor.push_back(vec3(1, 0, 0));

	glGenBuffers(1, &projection_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, projection_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, projectionPosition.size() * sizeof(vec3), &projectionPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &projection_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, projection_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, projectionColor.size() * sizeof(vec3), &projectionColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glBindVertexArray(0);
}

void drawSegment(vec2 pt) {
	glPointSize(8);
	defineSegmentVAO(pt);
	glBindVertexArray(segment_vao);
	glDrawArrays(GL_POINTS, 0, segmentPosition.size());
}

void defineSegmentVAO(vec2 pt) {
	segmentPosition.clear();
	segmentColor.clear();

	glGenVertexArrays(1, &segment_vao);
	glBindVertexArray(segment_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	segmentPosition.push_back(vec3(pt, 0));
	segmentColor.push_back(vec3(1, 1, 0));

	glGenBuffers(1, &segment_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, segment_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, segmentPosition.size() * sizeof(vec3), &segmentPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &projection_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, projection_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, projectionColor.size() * sizeof(vec3), &projectionColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glBindVertexArray(0);
}

void defineControlVAO() {
	controlPosition.clear();
	controlColor.clear();
	controlIndices.clear();

	glGenVertexArrays(1, &control_vao);
	glBindVertexArray(control_vao);

	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	for (int i = 0; i < 4; i++) {
		controlPosition.push_back(vec3(control[i], 0));
		controlColor.push_back(vec3(0, 0, 1));

		if (i < 3) {
			controlIndices.push_back(i);
			controlIndices.push_back(i + 1);
		}
	}

	glGenBuffers(1, &control_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, control_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, controlPosition.size() * sizeof(vec3), &controlPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &control_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, control_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, controlColor.size() * sizeof(vec3), &controlColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &control_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, controlIndices.size() * sizeof(int), &controlIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void defineBezierVAO() {
	bezierPosition.clear();
	bezierColor.clear();
	bezierIndices.clear();

	glGenVertexArrays(1, &bezier_vao);
	glBindVertexArray(bezier_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	//bezierPosition.insert(bezierPosition.end(), bezier.begin(), bezier.end());

	for (int i = 0; i < bezier.size(); i++) {
		bezierPosition.push_back(vec3(bezier[i], 0));
	}

	bezierColor.assign(bezierPosition.size(), vec3(0, 0, 0));

	for (int i = 0; i < bezierPosition.size() - 1; i++) {
		bezierIndices.push_back(i);
		bezierIndices.push_back(i + 1);
	}

	glGenBuffers(1, &bezier_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bezier_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, bezierPosition.size() * sizeof(vec3), &bezierPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &bezier_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bezier_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, bezierColor.size() * sizeof(vec3), &bezierColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &bezier_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bezierIndices.size() * sizeof(int), &bezierIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

float getDist(vec2 a, vec2 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

void pointDrag() {
	GLint viewport[4];
	GLdouble modelview[16] = { 0 };
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)xpos;
	winY = (float)viewport[3] - (float)ypos;

	glReadPixels(xpos, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	// Conversion to Object coordinate
	vec4 npos = vec4(posX, posY, posZ, 1);
	vec4 newPos = inverse(modelMatrix) * inverse(viewMatrix) * inverse(projectionMatrix) * npos;

	if (selectedPoint == -1) {
		if (pow((point.x - newPos.x), 2) + pow((point.y - newPos.y), 2) <= pow(0.3, 2)) {
			selectedPoint = 1;
		}
	}

	if (selectedControl == -1) {
		for (int i = 0; i < control.size(); i++) {
			if (pow((control[i].x - newPos.x), 2) + pow((control[i].y - newPos.y), 2) <= pow(0.3, 2)) {
				selectedControl = i;
				break;
			}
		}
	}

	if (selectedPoint != -1) {
		point.x = newPos.x;
		point.y = newPos.y;
		vec2 p3 = vec2(point);

		glBindBuffer(GL_ARRAY_BUFFER, point_position_vbo);
		void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &p3, sizeof(vec2));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	if (selectedControl != -1) {
		control[selectedControl].x = newPos.x;
		control[selectedControl].y = newPos.y;
		vector<vec3> c3;
		for (int i = 0; i < control.size(); i++) {
			c3.push_back(vec3(control[i], 0));
		}

		glBindBuffer(GL_ARRAY_BUFFER, control_position_vbo);
		void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &c3[0], sizeof(vec3) * c3.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);

		vector<vec3> bez3;
		for (float t = 0; t < 1; t += 0.01) {
			float x = pow(1 - t, 3) * control[0].x + 3 * pow(1 - t, 2) * t * control[1].x + 3 * (1 - t) * pow(t, 2) * control[2].x + pow(t, 3) * control[3].x;
			float y = pow(1 - t, 3) * control[0].y + 3 * pow(1 - t, 2) * t * control[1].y + 3 * (1 - t) * pow(t, 2) * control[2].y + pow(t, 3) * control[3].y;
			bez3.push_back(vec3(vec2(x, y), 0));
		}

		glBindBuffer(GL_ARRAY_BUFFER, bezier_position_vbo);
		ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &bez3[0], sizeof(vec3) * bez3.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			ldrag = true;
		}
		else if (action == GLFW_RELEASE) {
			ldrag = false;
			selectedPoint = -1;
			selectedControl = -1;
		}
	}
}

GLuint loadShaders(const char* vertexFilePath, const char* fragmentFilePath) {
	// Create shaders
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read vertex shader from file
	string vertexShaderSource;
	ifstream vertexShaderStream(vertexFilePath, ios::in);
	if (vertexShaderStream.is_open()) {
		stringstream sstr;
		sstr << vertexShaderStream.rdbuf();
		vertexShaderSource = sstr.str();
		vertexShaderStream.close();
	}
	else {
		return -1;
	}

	string fragmentShaderSource;
	ifstream fragmentShaderStream(fragmentFilePath, ios::in);
	if (fragmentShaderStream.is_open()) {
		stringstream sstr;
		sstr << fragmentShaderStream.rdbuf();
		fragmentShaderSource = sstr.str();
		fragmentShaderStream.close();
	}
	else {
		return -1;
	}

	GLint success;
	GLchar infoLog[512];

	// Compile Vertex shader
	char const * vertexShaderPointer = vertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &vertexShaderPointer, NULL);
	glCompileShader(vertexShader);

	// Check vertex Shader
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	}

	// Compile Fragment shader
	const char * fragmentShaderPointer = fragmentShaderSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentShaderPointer, NULL);
	glCompileShader(fragmentShader);

	// Check fragment Shader
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	}

	// Linking to program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check program
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);

	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}
