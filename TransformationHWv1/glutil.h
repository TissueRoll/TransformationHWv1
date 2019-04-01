#ifndef GLUTIL_H
#define GLUTIL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLfloat PI = (GLfloat) acos(-1);

inline void checkForErrors(unsigned int shader, std::string type);

inline GLuint loadProgram(const GLchar* vsh, const GLchar* fsh) {
	/*
		Loads a shader program. Takes 2 strings as arguments: file name of vertex shader, file name of fragment shader
	*/

	GLuint ID;

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		// open files
		vShaderFile.open(vsh);
		fShaderFile.open(fsh);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkForErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkForErrors(fragment, "FRAGMENT");
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkForErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return ID;
}

inline void checkForErrors(unsigned int shader, std::string type) {
	int success;
	char infoLog[1024];
	if (type != "PROGRAM") { // link errors
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else { // shader compile errors
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

inline void loadTexture(GLuint* tex, GLuint texUnit, const GLchar * fileName) {
	/*
		Loads 2D textures
		tex -> GLuint where to store the texture
		texUnit -> which texture unit to load the texture into
		fileName -> file name of image to load
	*/
	glGenTextures(1, tex);
	glActiveTexture(GL_TEXTURE0+texUnit);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//loading images
	int w, h, n;
	auto *data = stbi_load(fileName, &w, &h, &n, 0);
	if (data) {
		GLuint format = n == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture.\n";
	}
	stbi_image_free(data);
}

struct Matrix4 {
	GLfloat data[16];
	/*
		should be column-major, meaning the matrix looks like
		0  4  8 12
		1  5  9 13
		2  6 10 14
		3  7 11 15
	*/
	Matrix4() {
		// instantiate as an identity matrix
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				data[i + j * 4] = i == j ? 1.f : 0.f;
			}
		}
	}

	void set(GLuint row, GLuint col, GLfloat val) {
		// sets the value of the element in the given index
		assert(row > -1 || row < 4 || col > -1 || col < 4);
		data[col * 4 + row] = val;
	}

	GLfloat get(GLuint row, GLuint col) {
		// retrieves a value in the given index
		assert(row > -1 || row < 4 || col > -1 || col < 4);
		return data[col * 4 + row];
	}

	void print() {
		// for debugging
		std::cout << data[0] << " " << data[4] << " " << data[8] << " " << data[12] << "\n";
		std::cout << data[1] << " " << data[5] << " " << data[9] << " " << data[13] << "\n";
		std::cout << data[2] << " " << data[6] << " " << data[10] << " " << data[14] << "\n";
		std::cout << data[3] << " " << data[7] << " " << data[11] << " " << data[15] << "\n\n";
	}
};

Matrix4 multiply(Matrix4 a, Matrix4 b) {
	Matrix4 result;
	// returns AB
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.data[i + j * 4] = 0.f;
			for (int k = 0; k < 4; k++) {
				result.data[i + j * 4] += a.data[i + k * 4] * b.data[k + j * 4];
			}
		}
	}
	return result;
}

Matrix4 rotate(Matrix4 mat, GLfloat a, GLfloat x, GLfloat y, GLfloat z) {
	// creates a matrix B that rotaties by amount a in degrees in the given normalized arbitrary axis (x, y, z)
	// returns multiply(B, MAT)
	Matrix4 result;
	result.data[0] = cos(a) + (x*x)*(1-cos(a));
	result.data[1] = y*x*(1-cos(a))+z*sin(a);
	result.data[2] = z*x*(1-cos(a))-y*sin(a);
	result.data[4] = x*y*(1-cos(a))-z*sin(a);
	result.data[5] = cos(a) + (y*y)*(1-cos(a));
	result.data[6] = z*y*(1-cos(a))+x*sin(a);
	result.data[8] = x*z*(1-cos(a))+y*sin(a);
	result.data[9] = y*z*(1-cos(a))-x*sin(a);
	result.data[10] = cos(a) + (z*z)*(1-cos(a));
	return multiply(mat, result);
}

Matrix4 translate(Matrix4 mat, GLfloat x, GLfloat y, GLfloat z) {
	// creates a matrix B that translates in x-, y-, z-axis by amount denoted by inputs x, y, z
	// returns multiply(B, MAT)
	Matrix4 result;
	result.data[12] = x;
	result.data[13] = y;
	result.data[14] = z;
	return multiply(mat, result);
}

Matrix4 scale(Matrix4 mat, GLfloat x, GLfloat y, GLfloat z) {
	// creates a scaling matrix b that scales values in the respective axes by the respective inputs
	// returns multiply(B, MAT)
	Matrix4 result;
	result.data[0] = x;
	result.data[5] = y;
	result.data[10] = z;
	return multiply(mat, result);
}
#endif