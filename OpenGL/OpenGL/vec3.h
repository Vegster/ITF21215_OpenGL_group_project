#pragma once

#include <GL/glew.h>
#include <iostream>

class vec3 {

public:
	GLfloat x;
	GLfloat y;
	GLfloat z;

	vec3();
	vec3(const GLfloat& a, const GLfloat& b, const GLfloat& c);

	static vec3 cross(const vec3& v1, const vec3& v2);

	static GLfloat dot(const vec3& v1, const vec3& v2);

	vec3& normalize();
	static vec3 normalize(const vec3& v1);

	static vec3 add(const vec3& v1, const vec3& v2);
	static vec3 subtract(const vec3& v1, const vec3& v2);
	static vec3 multiply(const vec3& v1, const vec3& v2);
	static vec3 divide(const vec3& v1, const vec3& v2);

	static vec3 scale(const vec3& v, const GLfloat& k);

	friend vec3 operator+(const vec3& left, const vec3& right);
	friend vec3 operator-(const vec3& left, const vec3& right);
	friend vec3 operator*(const vec3& left, const vec3& right);
	friend vec3 operator/(const vec3& left, const vec3& right);

	friend std::ostream& operator<<(std::ostream& stream, const vec3& v);

};