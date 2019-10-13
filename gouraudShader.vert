//texture.vert
//use texture no light
#version 330

struct lightStruct{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};

struct materialStruct{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform mat4 projection;
uniform mat4 modelView;
uniform lightStruct light;
uniform materialStruct material;

in vec3 in_Pos;
in vec3 in_Norm;

in vec2 in_TexCoord;
out vec2 ex_TexCoord;

out vec4 ex_Color;

void main (void){
	vec4 vertexPosition = modelView * vec4(in_Pos, 1.0);
	gl_Position = projection * vertexPosition;

	vec3 V = normalize(-vertexPosition.xyz);

	mat3 normalMatrix = transpose(inverse(mat3(modelView)));
	vec3 screenNormal = normalize(normalMatrix * in_Norm);

	vec4 ambientI = light.ambient * material.ambient;

	vec3 L = normalize(light.position.xyz - vertexPosition.xyz);

	vec4 diffuseI = light.diffuse * material.diffuse * max(dot(screenNormal, L),0);

	vec3 R = normalize(-reflect(L, screenNormal));
	vec4 specularI = light.specular * material.specular;

	specularI = specularI * pow(max(dot(R, V), 0), material.shininess);

	ex_Color = ambientI + diffuseI + specularI;

	ex_TexCoord = in_TexCoord;
}