// phongShader.vert
#version 330

uniform mat4 modelView;
uniform mat4 projection;
uniform vec4 lightPosition;

in vec3 in_Pos;
in vec3 in_Norm;

out vec3 ex_N;
out vec3 ex_V;
out vec3 ex_L;

in vec2 in_TexCoord;
out vec2 ex_TexCoord;

void main(void){
	vec4 vertexPosition = modelView * vec4(in_Pos,1.0);

	ex_V = normalize(-vertexPosition).xyz;

	mat3 normalmatrix = transpose(inverse(mat3(modelView)));
	ex_N = normalize(normalmatrix * in_Norm);
	ex_L = normalize(lightPosition.xyz - vertexPosition.xyz);

	gl_Position = projection * vertexPosition;

	ex_TexCoord = in_TexCoord;
}