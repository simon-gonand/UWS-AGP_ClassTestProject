//skyboxTexture.frag
//only texture no light
#version 330

precision highp float;

smooth in vec3 ex_TexCoord;
uniform samplerCube cubeMap;
out vec4 out_Color;

void main(void){
	out_Color = texture(cubeMap, ex_TexCoord);
}