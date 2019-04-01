#version 330 core

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec2 v_texCoords;

uniform mat4 modelmat;

out vec2 f_texCoords;

void main() {
	f_texCoords = v_texCoords;

	gl_Position = modelmat * vec4(v_pos, 0.f, 1.f);
}
