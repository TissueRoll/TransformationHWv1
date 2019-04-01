#version 330 core

uniform sampler2D f_tex;

in vec2 f_texCoords;

out vec4 color;

void main(){
	color = texture(f_tex, f_texCoords);
}
