// Programming_assignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
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

vec3 pos, dir, up;
GLuint shaderProgram;
GLuint control_vao, control_ebo;
GLuint bezier_vao, bezier_ebo;
GLuint line_vao, line_ebo;
GLuint circle_vao, circle_ebo;
GLuint intersection_vao, intersection_ebo;

vector<vec2> p; 
vector<vec2> b;
vector<vec3> controlPosition, controlColor;
vector<vec3> bezierPosition, bezierColor;
vector<vec3> linePosition, lineColor;
vector<vec3> circlePosition, circleColor;
vector<vec3> intersectionPosition, intersectionColor;
vector<int> controlIndices, bezierIndices, lineIndices, circleIndices;

Camera camera;

GLuint loadShaders(const char*, const char*);
void defineBezierVAO();
void defineControlVAO();
void defineLineVAO();

void minimax(vector<vec2>);

bool doesIntersect(vec2, vec2, vec2, float);
void drawIntersection(vec2);
void defineIntersectionVAO(vec2);

vec2 getCenter(vector<vec2>);
float getRadius(vec2, vector<vec2>);
void drawCircle(vec2, float);
void defineCircleVAO(vec2, float);

bool rdrag = false, ldrag = false;
int selectedPoint = -1;
int selectedControl = -1;
void mouseCallback(GLFWwindow*, int, int, int);
void pointDrag();

int main()
{
	/*** Initialize OpenGL ***/
	if (!glfwInit()) {
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(1000, 1000, "Programming Assignment 01 - Subdivision", NULL, NULL);
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

//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetMouseButtonCallback(window, mouseCallback);

	/*** Draw ***/
	camera = Camera();
	
	b.push_back(vec2(-5, -5));
	b.push_back(vec2(-2, 5));
	b.push_back(vec2(2, 5));
	b.push_back(vec2(5, -5));

	p.push_back(vec2(-7, 0));
	p.push_back(vec2(7, 0));

	cout << "Init point 0 : " << p[0].x << ", " << p[0].y << endl;
	cout << "Init point 1 : " << p[0].x << ", " << p[0].y << endl;

	cout << "Init control 0 : " << b[0].x << ", " << b[0].y << endl;
	cout << "Init control 1 : " << b[1].x << ", " << b[1].y << endl;
	cout << "Init control 2 : " << b[2].x << ", " << b[2].y << endl;
	cout << "Init control 3 : " << b[3].x << ", " << b[3].y << endl;

	vec2 initCenter = getCenter(b);
	float initRadius = getRadius(initCenter, b);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	while ( !glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS ) {

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pos = camera.cameraPosition;
		dir = camera.cameraFront;
		up = camera.cameraUp;

		glfwGetCursorPos(window, &mouseX, &mouseY);

		projectionMatrix = ortho(-10.0f, 10.0f, -10.0f, 10.f, 0.0f, 100.0f);
		viewMatrix = lookAt(pos, dir, up);
		modelMatrix = mat4(1.0f);

		if (ldrag) {
			pointDrag();
		}

		defineControlVAO();
		glUseProgram(shaderProgram);
		GLuint projectionID = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projectionMatrix[0][0]);
		GLuint viewID = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewID, 1, GL_FALSE, &viewMatrix[0][0]);
		GLuint modelID = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &modelMatrix[0][0]);

		glPointSize(7);
		glBindVertexArray(control_vao);
		glDrawArrays(GL_POINTS, 0, controlPosition.size());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo);
		glDrawElements(GL_LINES, controlIndices.size(), GL_UNSIGNED_INT, 0);

		defineLineVAO();
		glBindVertexArray(line_vao);
		glDrawArrays(GL_POINTS, 0, linePosition.size());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
		glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, 0);

		defineBezierVAO();
		glBindVertexArray(bezier_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo);
		glDrawElements(GL_LINES, bezierIndices.size(), GL_UNSIGNED_INT, 0);

		minimax(b);

		glfwSwapBuffers(window);
		glfwPollEvents();
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
		for (int i = 0; i < p.size(); i++) {
			if (pow((p[i].x - newPos.x), 2) + pow((p[i].y - newPos.y), 2) <= pow(0.3, 2)) {
				selectedPoint = i;
				break;
			}
		}
	}

	if (selectedControl == -1) {
		for (int i = 0; i < b.size(); i++) {
			if (pow((b[i].x - newPos.x), 2) + pow((b[i].y - newPos.y), 2) <= pow(0.3, 2)) {
				selectedControl = i;
				break;
			}
		}
	}

	if (selectedPoint != -1) {
		p[selectedPoint].x = newPos.x;
		p[selectedPoint].y = newPos.y;
	}
	if (selectedControl != -1) {
		b[selectedControl].x = newPos.x;
		b[selectedControl].y = newPos.y;
	}
}

bool doesIntersect(vec2 p0, vec2 p1, vec2 c, float r) {
	float determinant = pow((p1.x - p0.x)*(p0.x - c.x) + (p1.y - p0.y)*(p0.y - c.y), 2) - (pow((p1.x - p0.x), 2) + pow((p1.y - p0.y), 2)) * (pow((p0.x - c.x), 2) + pow((p0.y - c.y), 2) - pow(r, 2));

	if (determinant >= 0)
		return true;
	else
		return false;
}

vec2 getCenter(vector<vec2> controls) {
	vec2 center = vec2(0, 0);

	for (int i = 0; i < controls.size(); i++) {
		center.x += controls[i].x;
		center.y += controls[i].y;
	}
	center.x /= controls.size();
	center.y /= controls.size();

	return center;
}

float getRadius(vec2 center, vector<vec2> controls) {
	float radius = 0, distance = 0;

	radius = sqrt(pow(controls[0].x - center.x, 2) + pow(controls[0].y - center.y, 2));

	distance = radius;
	for (int i = 1; i < controls.size(); i++) {
		distance = sqrt(pow(controls[i].x - center.x, 2) + pow(controls[i].y - center.y, 2));
		if (distance > radius) {
			radius = distance;
		}
	}
	
	return radius;
}

void minimax(vector<vec2> controls) {
	vec2 center = getCenter(controls);
	float radius = getRadius(center, controls);

	if ( doesIntersect(p[0], p[1], center, radius)  ) {     /*** If intersection, draw circle and proceed ***/
		drawCircle(center, radius);

		if (radius <= pow(10, -3)) {
			drawIntersection(center);
			return;
		}
		else {     /*** else exit ***/
			vector<vec2> leftControl, rightControl;
			vec2 b10, b11, b12, b20, b21, b30;
			b10 = vec2(0.5 * controls[0].x + 0.5 * controls[1].x, 0.5 * controls[0].y + 0.5 * controls[1].y);
			b11 = vec2(0.5 * controls[1].x + 0.5 * controls[2].x, 0.5 * controls[1].y + 0.5 * controls[2].y);
			b12 = vec2(0.5 * controls[2].x + 0.5 * controls[3].x, 0.5 * controls[2].y + 0.5 * controls[3].y);

			b20 = vec2(0.5 * b10.x + 0.5 * b11.x, 0.5 * b10.y + 0.5 * b11.y);
			b21 = vec2(0.5 * b11.x + 0.5 * b12.x, 0.5 * b11.y + 0.5 * b12.y);

			b30 = vec2(0.5 * b20.x + 0.5 * b21.x, 0.5 * b20.y + 0.5 * b21.y);

			/*** Left segment ***/
			leftControl.push_back(controls[0]);
			leftControl.push_back(b10);
			leftControl.push_back(b20);
			leftControl.push_back(b30);
			minimax(leftControl);

			/*** Right segment ***/
			rightControl.push_back(controls[3]);
			rightControl.push_back(b12);
			rightControl.push_back(b21);
			rightControl.push_back(b30);
			minimax(rightControl);
		}
	}
	else {
		return;
	}
		
}

void drawIntersection(vec2 pt) {
	glPointSize(8);
	defineIntersectionVAO(pt);
	glBindVertexArray(intersection_vao);
	glDrawArrays(GL_POINTS, 0, intersectionPosition.size());
}

void defineIntersectionVAO(vec2 pt) {
	intersectionPosition.clear();
	intersectionColor.clear();

	glGenVertexArrays(1, &intersection_vao);
	glBindVertexArray(intersection_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	intersectionPosition.push_back(vec3(pt, 0));
	intersectionColor.push_back(vec3(1, 0, 0));

	GLuint position_vbo;
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, intersectionPosition.size() * sizeof(vec3), &intersectionPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	GLuint color_vbo;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, intersectionColor.size() * sizeof(vec3), &intersectionColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glBindVertexArray(0);
}

void drawCircle(vec2 center, float r) {
	defineCircleVAO(center, r);
	glBindVertexArray(circle_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle_ebo);
	glDrawElements(GL_LINES, circleIndices.size(), GL_UNSIGNED_INT, 0);
}

void defineCircleVAO(vec2 center, float r) {
	circlePosition.clear();
	circleColor.clear();
	circleIndices.clear();

	glGenVertexArrays(1, &circle_vao);
	glBindVertexArray(circle_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	for (int i = 0; i < 360; i += 1) {
		circlePosition.push_back(vec3(vec2(r * cos(i * PI/180) + center.x, r * sin(i * PI/180) + center.y), 0));
		circleColor.push_back(vec3(0, 0.5, 0.5));
	}

	for (int i = 0; i < circlePosition.size() - 1; i++) {
		circleIndices.push_back(i);
		circleIndices.push_back(i + 1);
	}
	circleIndices.push_back(circlePosition.size() - 1);
	circleIndices.push_back(0);

	GLuint position_vbo;
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, circlePosition.size() * sizeof(vec3), &circlePosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	GLuint color_vbo;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, circleColor.size() * sizeof(vec3), &circleColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &circle_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, circleIndices.size() * sizeof(int), &circleIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void defineBezierVAO() {
	bezierPosition.clear();
	bezierColor.clear();
	bezierIndices.clear();

	glGenVertexArrays(1, &bezier_vao);
	glBindVertexArray(bezier_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	for (float t = 0; t < 1; t += 0.01) {
		float x = pow(1 - t, 3) * b[0].x + 3 * pow(1 - t, 2) * t * b[1].x + 3 * (1 - t) * pow(t, 2) * b[2].x + pow(t, 3) * b[3].x;
		float y = pow(1 - t, 3) * b[0].y + 3 * pow(1 - t, 2) * t * b[1].y + 3 * (1 - t) * pow(t, 2) * b[2].y + pow(t, 3) * b[3].y;
		bezierPosition.push_back(vec3(vec2(x, y), 0));
		bezierColor.push_back(vec3(0, 0, 0));
	}
	for (int i = 0; i < bezierPosition.size() - 1; i++) {
		bezierIndices.push_back(i);
		bezierIndices.push_back(i + 1);
	}

	GLuint position_vbo;
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, bezierPosition.size() * sizeof(vec3), &bezierPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	GLuint color_vbo;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, bezierColor.size() * sizeof(vec3), &bezierColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &bezier_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bezierIndices.size() * sizeof(int), &bezierIndices[0], GL_STATIC_DRAW);

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
		controlPosition.push_back(vec3(b[i], 0));
		controlColor.push_back(vec3(0, 0, 1));

		if (i < 3) {
			controlIndices.push_back(i);
			controlIndices.push_back(i + 1);
		}
	}

	GLuint position_vbo;
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, controlPosition.size() * sizeof(vec3), &controlPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	GLuint color_vbo;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, controlColor.size() * sizeof(vec3), &controlColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &control_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, controlIndices.size() * sizeof(int), &controlIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void defineLineVAO() {
	linePosition.clear();
	lineColor.clear();
	lineIndices.clear();

	glGenVertexArrays(1, &line_vao);
	glBindVertexArray(line_vao);
	shaderProgram = loadShaders("shaders/vertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	for (int i = 0; i < 2; i++) {
		linePosition.push_back(vec3(p[i], 0));
		lineColor.push_back(vec3(0, 0, 0));
	}

	lineIndices.push_back(0);
	lineIndices.push_back(1);

	GLuint position_vbo;
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, linePosition.size() * sizeof(vec3), &linePosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	GLuint color_vbo;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, lineColor.size() * sizeof(vec3), &lineColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &line_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineIndices.size() * sizeof(int), &lineIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

GLuint loadShaders(const char* vertexFilePath, const char* fragmentFilePath) {
	// Create shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

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
