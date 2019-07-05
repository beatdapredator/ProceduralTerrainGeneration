#version 420 core
in vec2 texCoordsExport;
in vec3 normalsExport;
flat in vec4 colorsExport;
uniform sampler2D grassTex;

out vec4 colorsOut;

struct Material 
{
	vec4 ambRefl;
	vec4 diffRefl;
	vec4 specRefl;
	vec4 emitCols;
	float shininess;
};

struct Light
{
vec4 ambCols;
vec4 diffCols;
vec4 specCols;
vec4 coords;
};
uniform Light light0;

uniform Material terrainFandB;

void main(void)
{
	vec3 normal = normalize(normalsExport);
	vec3 lightDirection = normalize(vec3(light0.coords));
	vec4 lightCalculation = max(dot(normal, lightDirection), 0.0f)* (light0.diffCols * terrainFandB.diffRefl);

	vec4 fieldTexColor = texture(grassTex, texCoordsExport);
   colorsOut = fieldTexColor * lightCalculation;

}