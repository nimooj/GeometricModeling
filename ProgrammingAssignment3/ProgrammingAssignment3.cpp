// ProgrammingAssignment3.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

vector<vec2> bezierControl0, bezierControl1;
vector<vec3> bezier0, bezier1;

GLuint control_vao0, control_vao1, control_ebo0, control_ebo1;
GLuint control_position_vbo0, control_position_vbo1, control_color_vbo;
vector<vec3> controlPosition, controlColor;
vector<int> controlIndices;
void defineControlVAO(int);

GLuint bezier_vao0, bezier_vao1, bezier_ebo0, bezier_ebo1;
GLuint bezier_position_vbo0, bezier_position_vbo1, bezier_color_vbo;
vector<vec3> bezierPosition, bezierColor;
vector<int> bezierIndices;
void defineBezierVAO(int);

GLuint point_vao;
GLuint point_position_vbo, point_color_vbo;
vec3 ptPosition, ptColor;
void definePointVAO(vec2);

GLuint circle_vao, circle_ebo;
GLuint circle_position_vbo, circle_color_vbo;
vector<vec3> circlePosition, circleColor;
vector<int> circleIndices;
vec2 getCenter(vector<vec2>);
float getRadius(vec2, vector<vec2>);
void defineCircleVAO(vec2, float);
void drawCircle(vec2, float);

GLuint line_vao, line_ebo;
GLuint line_position_vbo, line_color_vbo;
vector<vec3> linePosition, lineColor;
vector<int> lineIndices;
void defineLineVAO(vec2, vec2);
void drawLine(vec2, vec2);

bool rdrag = false, ldrag = false;
int selectedPoint = -1;
int selectedControl0 = -1, selectedControl1 = -1;
void mouseCallback(GLFWwindow*, int, int, int);
void pointDrag();

GLuint intersection_vao;
GLuint intersection_position_vbo, intersection_color_vbo;
vector<vec3> intersectionPosition, intersectionColor;
void drawIntersection(vec2);
void defineIntersectionVAO(vec2);

float getDist(vec2, vec2);
void shortestDist(vector<vec2>, vector<vec2>);

GLuint shaderProgram, vertexShader, fragmentShader;
GLuint loadShaders(const char*, const char*);

struct Bound {
	vec2 point0, point1;
	float upperBound;
	float lowerBound;
};
auto cmp = [](Bound a, Bound b) { return a.lowerBound > b.lowerBound; };
priority_queue<Bound, vector<Bound>, decltype(cmp)> bounds(cmp);

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

	bezierControl0.push_back(vec2(-5, -5));
	bezierControl0.push_back(vec2(-2, -2));
	bezierControl0.push_back(vec2(2, -2));
	bezierControl0.push_back(vec2(5, -5));

	bezierControl1.push_back(vec2(-5, 6));
	bezierControl1.push_back(vec2(-2, 2));
	bezierControl1.push_back(vec2(2, 2));
	bezierControl1.push_back(vec2(5, 6));

	for (float t = 0; t < 1; t += 0.01) {
		float x = pow(1 - t, 3) * bezierControl0[0].x + 3 * pow(1 - t, 2) * t * bezierControl0[1].x + 3 * (1 - t) * pow(t, 2) * bezierControl0[2].x + pow(t, 3) * bezierControl0[3].x;
		float y = pow(1 - t, 3) * bezierControl0[0].y + 3 * pow(1 - t, 2) * t * bezierControl0[1].y + 3 * (1 - t) * pow(t, 2) * bezierControl0[2].y + pow(t, 3) * bezierControl0[3].y;
		bezier0.push_back(vec3(x, y, 0));

		x = pow(1 - t, 3) * bezierControl1[0].x + 3 * pow(1 - t, 2) * t * bezierControl1[1].x + 3 * (1 - t) * pow(t, 2) * bezierControl1[2].x + pow(t, 3) * bezierControl1[3].x;
		y = pow(1 - t, 3) * bezierControl1[0].y + 3 * pow(1 - t, 2) * t * bezierControl1[1].y + 3 * (1 - t) * pow(t, 2) * bezierControl1[2].y + pow(t, 3) * bezierControl1[3].y;
		bezier1.push_back(vec3(x, y, 0));
	}

	defineControlVAO(0);
	defineControlVAO(1);
	defineBezierVAO(0);
	defineBezierVAO(1);

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

		glBindVertexArray(control_vao0);
		glDrawArrays(GL_POINTS, 0, controlPosition.size());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo0);
		glDrawElements(GL_LINES, controlIndices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(control_vao1);
		glDrawArrays(GL_POINTS, 0, controlPosition.size());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo1);
		glDrawElements(GL_LINES, controlIndices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(bezier_vao0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo0);
		glDrawElements(GL_LINES, bezierIndices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(bezier_vao1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo1);
		glDrawElements(GL_LINES, bezierIndices.size(), GL_UNSIGNED_INT, 0);

		shortestDist(bezierControl0, bezierControl1);

		if (!bounds.empty()) {
			Bound p = bounds.top();
			drawIntersection(p.point0);
			drawIntersection(p.point1);
			drawLine(p.point0, p.point1);
		}

		while (!bounds.empty()) {
			bounds.pop();
		}

		/*
		findBound(bezierControl0, bezierControl1, 0);
		if (!pq0.empty()) {
			drawIntersection(pq0.top().point);
		}
		if (!pq1.empty()) {
			drawIntersection(pq1.top().point);
		}
		if (!pq0.empty() && !pq1.empty()) {
			drawLine(pq0.top().point, pq1.top().point);
			drawIntersection(pq0.top().point);
			drawIntersection(pq1.top().point);
		}

		while (!pq0.empty()) {
			pq0.pop();
		}
		while (!pq1.empty()) {
			pq1.pop();
		}
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();

		glBindVertexArray(0);
	}

	glDeleteVertexArrays(1, &point_vao);
	glDeleteVertexArrays(1, &control_vao0);
	glDeleteVertexArrays(1, &control_vao1);
	glDeleteVertexArrays(1, &intersection_vao);
	glDeleteVertexArrays(1, &line_vao);
	glDeleteVertexArrays(1, &bezier_vao0);
	glDeleteVertexArrays(1, &bezier_vao1);
	glDeleteVertexArrays(1, &circle_vao);

	glDeleteBuffers(1, &point_position_vbo);
	glDeleteBuffers(1, &point_color_vbo);
	glDeleteBuffers(1, &control_position_vbo0);
	glDeleteBuffers(1, &control_position_vbo1);
	glDeleteBuffers(1, &control_color_vbo);
	glDeleteBuffers(1, &intersection_position_vbo);
	glDeleteBuffers(1, &intersection_color_vbo);
	glDeleteBuffers(1, &line_position_vbo);
	glDeleteBuffers(1, &line_color_vbo);
	glDeleteBuffers(1, &bezier_position_vbo0);
	glDeleteBuffers(1, &bezier_position_vbo1);
	glDeleteBuffers(1, &bezier_color_vbo);
	glDeleteBuffers(1, &circle_position_vbo);
	glDeleteBuffers(1, &circle_color_vbo);

	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void shortestDist(vector<vec2> controls0, vector<vec2> controls1) { 
	vec2 centerB = getCenter(controls0);
	vec2 centerC = getCenter(controls1);
	float radiusB = getRadius(centerB, controls0);
	float radiusC = getRadius(centerC, controls1);

	drawCircle(centerB, radiusB);
	drawCircle(centerC, radiusC);

	vector<float> upperDists;
	upperDists.push_back(getDist(controls0[0], controls1[0]));
	upperDists.push_back(getDist(controls0[0], controls1[3]));
	upperDists.push_back(getDist(controls0[3], controls1[0]));
	upperDists.push_back(getDist(controls0[3], controls1[3]));
	sort(upperDists.begin(), upperDists.end());

	float upperBound = upperDists[0];
	float lowerBound = (getDist(centerB, centerC) <= (radiusB + radiusC))? 0 : (getDist(centerB, centerC) - (radiusB + radiusC));

	vec2 b10 = vec2(0.5 * controls0[0].x + 0.5 * controls0[1].x, 0.5 * controls0[0].y + 0.5 * controls0[1].y);
	vec2 b11 = vec2(0.5 * controls0[1].x + 0.5 * controls0[2].x, 0.5 * controls0[1].y + 0.5 * controls0[2].y);
	vec2 b12 = vec2(0.5 * controls0[2].x + 0.5 * controls0[3].x, 0.5 * controls0[2].y + 0.5 * controls0[3].y);
	vec2 b20 = vec2(0.5 * b10.x + 0.5 * b11.x, 0.5 * b10.y + 0.5 * b11.y);
	vec2 b21 = vec2(0.5 * b11.x + 0.5 * b12.x, 0.5 * b11.y + 0.5 * b12.y);
	vec2 b30 = vec2(0.5 * b20.x + 0.5 * b21.x, 0.5 * b20.y + 0.5 * b21.y);

	vec2 c10 = vec2(0.5 * controls1[0].x + 0.5 * controls1[1].x, 0.5 * controls1[0].y + 0.5 * controls1[1].y);
	vec2 c11 = vec2(0.5 * controls1[1].x + 0.5 * controls1[2].x, 0.5 * controls1[1].y + 0.5 * controls1[2].y);
	vec2 c12 = vec2(0.5 * controls1[2].x + 0.5 * controls1[3].x, 0.5 * controls1[2].y + 0.5 * controls1[3].y);
	vec2 c20 = vec2(0.5 * c10.x + 0.5 * c11.x, 0.5 * c10.y + 0.5 * c11.y);
	vec2 c21 = vec2(0.5 * c11.x + 0.5 * c12.x, 0.5 * c11.y + 0.5 * c12.y);
	vec2 c30 = vec2(0.5 * c20.x + 0.5 * c21.x, 0.5 * c20.y + 0.5 * c21.y);

	if (abs(upperBound - lowerBound) < pow(10, -3)) {
		Bound tmp;
		tmp.point0 = b30;
		tmp.point1 = c30;
		tmp.upperBound = upperBound;
		tmp.lowerBound = lowerBound;
		bounds.push(tmp);
		return;
	}
	else {
		vector<vec2> leftB, rightB;
		leftB.push_back(controls0[0]);
		leftB.push_back(b10);
		leftB.push_back(b20);
		leftB.push_back(b30);
		vec2 centerLeft = getCenter(leftB);
		float radiusLeft = getRadius(centerLeft, leftB);
		rightB.push_back(b30);
		rightB.push_back(b21);
		rightB.push_back(b12);
		rightB.push_back(controls0[3]);
		vec2 centerRight = getCenter(rightB);
		float radiusRight = getRadius(centerRight, rightB);

		vector<vec2> leftC, rightC;
		leftC.push_back(controls1[0]);
		leftC.push_back(c10);
		leftC.push_back(c20);
		leftC.push_back(c30);
		rightC.push_back(c30);
		rightC.push_back(c21);
		rightC.push_back(c12);
		rightC.push_back(controls1[3]);

		float lowerBoundLeft = (getDist(centerLeft, centerC) <= (radiusLeft + radiusC))? 0 : (getDist(centerLeft, centerC) - (radiusLeft + radiusC));
		float lowerBoundRight = (getDist(centerRight, centerC) <= (radiusRight + radiusC))? 0 : (getDist(centerRight, centerC) - (radiusRight + radiusC));
		
		float upDistLeft = getDist(b30, controls1[0]); 
		float upDistRight = getDist(b30, controls1[3]); 

		float upperBoundLeft = upperBound, upperBoundRight = upperBound;
		if (upDistLeft < upperBoundLeft) {
			upperBoundLeft = upDistLeft;
		}
		if (upDistRight < upperBoundRight) {
			upperBoundRight = upDistRight;
		}

		if (abs(upperBoundLeft - lowerBoundLeft) < pow(10, -3)) {
			Bound tmp;
			tmp.point0 = centerLeft;
			tmp.point1 = centerC;
			tmp.upperBound = upperBoundLeft;
			tmp.lowerBound = lowerBoundLeft;

			bounds.push(tmp);
			return;
		}
		if (abs(upperBoundRight - lowerBoundRight) < pow(10, -3)) {
			Bound tmp;
			tmp.point0 = centerRight;
			tmp.point1 = centerC;
			tmp.upperBound = upperBoundRight;
			tmp.lowerBound = lowerBoundRight;

			bounds.push(tmp);
			return;
		}

		if (lowerBoundLeft < lowerBoundRight) {
			shortestDist(controls1, leftB);
		}
		else if (lowerBoundRight < lowerBoundLeft) {
			shortestDist(controls1, rightB);
		}
		else if (lowerBoundLeft == lowerBoundRight) {
			shortestDist(controls1, leftB);
			shortestDist(controls1, rightB);
		}
	}
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

void defineControlVAO(int type) {
	controlPosition.clear();
	controlColor.clear();
	controlIndices.clear();

	if (type == 0) {
		glGenVertexArrays(1, &control_vao0);
		glBindVertexArray(control_vao0);
	}
	else if (type == 1) {
		glGenVertexArrays(1, &control_vao1);
		glBindVertexArray(control_vao1);
	}
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	for (int i = 0; i < 4; i++) {
		if (type == 0) 
			controlPosition.push_back(vec3(bezierControl0[i], 0));
		else if (type == 1)
			controlPosition.push_back(vec3(bezierControl1[i], 0));
		controlColor.push_back(vec3(0, 0, 1));

		if (i < 3) {
			controlIndices.push_back(i);
			controlIndices.push_back(i + 1);
		}
	}

	if (type == 0) {
		glGenBuffers(1, &control_position_vbo0);
		glBindBuffer(GL_ARRAY_BUFFER, control_position_vbo0);
		glBufferData(GL_ARRAY_BUFFER, controlPosition.size() * sizeof(vec3), &controlPosition[0], GL_STATIC_DRAW);
	}
	else if (type == 1) {
		glGenBuffers(1, &control_position_vbo1);
		glBindBuffer(GL_ARRAY_BUFFER, control_position_vbo1);
		glBufferData(GL_ARRAY_BUFFER, controlPosition.size() * sizeof(vec3), &controlPosition[0], GL_STATIC_DRAW);
	}

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &control_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, control_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, controlColor.size() * sizeof(vec3), &controlColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	if (type == 0) {
		glGenBuffers(1, &control_ebo0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo0);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, controlIndices.size() * sizeof(int), &controlIndices[0], GL_STATIC_DRAW);
	}
	else if (type == 1) {
		glGenBuffers(1, &control_ebo1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, control_ebo1);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, controlIndices.size() * sizeof(int), &controlIndices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
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

void defineCircleVAO(vec2 center, float r) {
	circlePosition.clear();
	circleColor.clear();
	circleIndices.clear();

	glGenVertexArrays(1, &circle_vao);
	glBindVertexArray(circle_vao);
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	for (int i = 0; i < 360; i += 1) {
		circlePosition.push_back(vec3(vec2(r * cos(i * PI / 180) + center.x, r * sin(i * PI / 180) + center.y), 0));
		circleColor.push_back(vec3(0, 0.5, 0.5));
	}

	for (int i = 0; i < circlePosition.size() - 1; i++) {
		circleIndices.push_back(i);
		circleIndices.push_back(i + 1);
	}
	circleIndices.push_back(circlePosition.size() - 1);
	circleIndices.push_back(0);

	glGenBuffers(1, &circle_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, circle_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, circlePosition.size() * sizeof(vec3), &circlePosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &circle_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, circle_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, circleColor.size() * sizeof(vec3), &circleColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glGenBuffers(1, &circle_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, circleIndices.size() * sizeof(int), &circleIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void defineBezierVAO(int type) {
	bezierPosition.clear();
	bezierColor.clear();
	bezierIndices.clear();

	if (type == 0) {
		glGenVertexArrays(1, &bezier_vao0);
		glBindVertexArray(bezier_vao0);

	}
	else if (type == 1) {
		glGenVertexArrays(1, &bezier_vao1);
		glBindVertexArray(bezier_vao1);

	}
	shaderProgram = loadShaders("shaders/VertexShader.vertexshader", "shaders/fragmentShader.fragmentShader");

	if (type == 0) 
		bezierPosition.insert(bezierPosition.end(), bezier0.begin(), bezier0.end());
	else if (type == 1)
		bezierPosition.insert(bezierPosition.end(), bezier1.begin(), bezier1.end());

	bezierColor.assign(bezierPosition.size(), vec3(0, 0, 0));

	for (int i = 0; i < bezierPosition.size() - 1; i++) {
		bezierIndices.push_back(i);
		bezierIndices.push_back(i + 1);
	}

	if (type == 0) {
		glGenBuffers(1, &bezier_position_vbo0);
		glBindBuffer(GL_ARRAY_BUFFER, bezier_position_vbo0);
		glBufferData(GL_ARRAY_BUFFER, bezierPosition.size() * sizeof(vec3), &bezierPosition[0], GL_STATIC_DRAW);
	}
	else if (type == 1) {
		glGenBuffers(1, &bezier_position_vbo1);
		glBindBuffer(GL_ARRAY_BUFFER, bezier_position_vbo1);
		glBufferData(GL_ARRAY_BUFFER, bezierPosition.size() * sizeof(vec3), &bezierPosition[0], GL_STATIC_DRAW);
	}

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &bezier_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bezier_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, bezierColor.size() * sizeof(vec3), &bezierColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	if (type == 0) {
		glGenBuffers(1, &bezier_ebo0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo0);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bezierIndices.size() * sizeof(int), &bezierIndices[0], GL_STATIC_DRAW);
	}
	else if (type == 1) {
		glGenBuffers(1, &bezier_ebo1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bezier_ebo1);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bezierIndices.size() * sizeof(int), &bezierIndices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);
}

void drawLine(vec2 n, vec2 m) {
	defineLineVAO(n, m);
	glBindVertexArray(line_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
	glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, 0);
}

void drawCircle(vec2 center, float r) {
	defineCircleVAO(center, r);
	glBindVertexArray(circle_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circle_ebo);
	glDrawElements(GL_LINES, circleIndices.size(), GL_UNSIGNED_INT, 0);
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

	glGenBuffers(1, &intersection_position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, intersection_position_vbo);
	glBufferData(GL_ARRAY_BUFFER, intersectionPosition.size() * sizeof(vec3), &intersectionPosition[0], GL_STATIC_DRAW);

	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);

	glGenBuffers(1, &intersection_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, intersection_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, intersectionColor.size() * sizeof(vec3), &intersectionColor[0], GL_STATIC_DRAW);
	GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
	glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(color_attribute);

	glBindVertexArray(0);
}

float getDist(vec2 a, vec2 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
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

	if (selectedControl0 == -1) {
		for (int i = 0; i < bezierControl0.size(); i++) {
			vec2 control = bezierControl0[i];
			if (pow((control.x - newPos.x), 2) + pow((control.y - newPos.y), 2) <= pow(0.3, 2)) {
				selectedControl0 = i;
				break;
			}
		}
	}
	if (selectedControl1 == -1) {
		for (int i = 0; i < bezierControl1.size(); i++) {
			vec2 control = bezierControl1[i];
			if (pow((control.x - newPos.x), 2) + pow((control.y - newPos.y), 2) <= pow(0.3, 2)) {
				selectedControl1 = i;
				break;
			}
		}
	}

	if (selectedControl0 != -1) {
		bezierControl0[selectedControl0].x = newPos.x;
		bezierControl0[selectedControl0].y = newPos.y;
		vector<vec3> b3;
		for (int i = 0; i < bezierControl0.size(); i++) {
			b3.push_back(vec3(bezierControl0[i], 0));
		}

		glBindBuffer(GL_ARRAY_BUFFER, control_position_vbo0);
		void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &b3[0], sizeof(vec3) * b3.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);

		vector<vec3> bez3;
		for (float t = 0; t < 1; t += 0.01) {
			float x = pow(1 - t, 3) * bezierControl0[0].x + 3 * pow(1 - t, 2) * t * bezierControl0[1].x + 3 * (1 - t) * pow(t, 2) * bezierControl0[2].x + pow(t, 3) * bezierControl0[3].x;
			float y = pow(1 - t, 3) * bezierControl0[0].y + 3 * pow(1 - t, 2) * t * bezierControl0[1].y + 3 * (1 - t) * pow(t, 2) * bezierControl0[2].y + pow(t, 3) * bezierControl0[3].y;
			bez3.push_back(vec3(vec2(x, y), 0));
		}

		glBindBuffer(GL_ARRAY_BUFFER, bezier_position_vbo0);
		ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &bez3[0], sizeof(vec3) * bez3.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	if (selectedControl1 != -1) {
		bezierControl1[selectedControl1].x = newPos.x;
		bezierControl1[selectedControl1].y = newPos.y;
		vector<vec3> b3;
		for (int i = 0; i < bezierControl1.size(); i++) {
			b3.push_back(vec3(bezierControl1[i], 0));
		}

		glBindBuffer(GL_ARRAY_BUFFER, control_position_vbo1);
		void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, &b3[0], sizeof(vec3) * b3.size());
		glUnmapBuffer(GL_ARRAY_BUFFER);

		vector<vec3> bez3;
		for (float t = 0; t < 1; t += 0.01) {
			float x = pow(1 - t, 3) * bezierControl1[0].x + 3 * pow(1 - t, 2) * t * bezierControl1[1].x + 3 * (1 - t) * pow(t, 2) * bezierControl1[2].x + pow(t, 3) * bezierControl1[3].x;
			float y = pow(1 - t, 3) * bezierControl1[0].y + 3 * pow(1 - t, 2) * t * bezierControl1[1].y + 3 * (1 - t) * pow(t, 2) * bezierControl1[2].y + pow(t, 3) * bezierControl1[3].y;
			bez3.push_back(vec3(vec2(x, y), 0));
		}

		glBindBuffer(GL_ARRAY_BUFFER, bezier_position_vbo1);
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
			selectedControl0 = -1;
			selectedControl1 = -1;
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
