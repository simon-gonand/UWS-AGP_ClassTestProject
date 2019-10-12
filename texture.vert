//texture.vert
//use texture no light
#version 330

uniform mat4 projection;
uniform mat4 modelView;

in vec3 in_Pos;

in vec2 in_TexCoord;
out vec2 ex_TexCoord;

void main (void){
	vec4 vertexPosition = modelView * vec4(in_Pos, 1.0);
	gl_Position = projection * vertexPosition;

	ex_TexCoord = in_TexCoord;
}