#version 420 core

layout(location=0) in vec4 terrainCoords;
layout(location=1) in vec3 terrainNormals;
layout(location=2) in vec2 terrainTexCoords;

uniform mat4 projMat;
uniform mat4 modelViewMat;
uniform vec4 globAmb;
uniform mat3 normalMat;


out vec2 texCoordsExport;
out vec3 normalsExport;

void main(void)
{
	texCoordsExport = terrainTexCoords;
	normalsExport = (normalMat * terrainNormals);

   gl_Position = projMat * modelViewMat * terrainCoords;
   //colorsExport = globAmb * terrainFandB.ambRefl;
}