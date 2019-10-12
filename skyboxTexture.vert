//skyboxTexture.vert
//only texture no light
#version 330

uniform mat4 projection;
uniform mat4 modelView;

in vec3 in_Pos;
smooth out vec3 ex_TexCoord;

void main(void){
	ex_TexCoord = normalize(in_Pos);

	vec4 vertexPos = modelView * vec4(in_Pos, 1.0);
	gl_Position = projection * vertexPos;
}