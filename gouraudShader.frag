//gouraudShader.frag
#version 330

precision highp float;

uniform sampler2D textureUnit0;
in vec2 ex_TexCoord;

in vec4 ex_Color;
layout(location = 0) out vec4 out_Color;

void main(void) {
	out_Color = texture(textureUnit0, ex_TexCoord) * ex_Color;
}