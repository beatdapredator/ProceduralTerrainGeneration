#include <iostream>
#include <fstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "getbmp.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#pragma comment(lib, "glew32.lib") 

using namespace std;
using namespace glm;

// Size of the terrain
const int mapSize = 33;
int stepSize = mapSize - 1;
float terrain[mapSize][mapSize] = {};
float randMax = 1;
int H = 1;


const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;


// these are my variables so that I can impliment a 3d camera in my scene.(the direction of my camera, its location and the angle and axis in which it turns)
vec3 cameraLocation(0, 0, -10);
vec3 cameraDirection = glm::normalize(vec3(0, 0, 1));
vec3 cameraAngle (0, 0, 0);

struct Vertex
{
	float coords[4];
	float normals[3];
	float texcoords[2];
};

struct Matrix4x4
{
	float entries[16];
};

static mat4 projMat = mat4(1.0);

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

static mat3 normalMat = mat3(1.0);

static BitMapFile *image[1];




static const Light light0 =
{
	vec4(0.0, 0.0, 0.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 0.0, 0.0)
};


static const Material terrainFandB = 
{
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 0.0, 0.0, 1.0),
	50.0f
};

static const vec4 globAmb = vec4(0.2, 0.2, 0.2, 1.0);

static const Matrix4x4 IDENTITY_MATRIX4x4 =
{
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	}
};

static enum buffer { TERRAIN_VERTICES };
static enum object { TERRAIN };

// Globals
static Vertex terrainVertices[mapSize*mapSize] = {};

const int numStripsRequired = mapSize - 1;
const int verticesPerStrip = 2 * mapSize;

unsigned int terrainIndexData[numStripsRequired][verticesPerStrip];

static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
projMatLoc,
buffer[1],
vao[1],
normalMatLoc,
texture[1],
grassTexLoc;



// Function to read text file, used to read shader files
char* readTextFile(char* aTextFile)
{
	FILE* filePointer = fopen(aTextFile, "rb");
	char* content = NULL;
	long numVal = 0;

	fseek(filePointer, 0L, SEEK_END);
	numVal = ftell(filePointer);
	fseek(filePointer, 0L, SEEK_SET);
	content = (char*)malloc((numVal + 1) * sizeof(char));
	fread(content, 1, numVal, filePointer);
	content[numVal] = '\0';
	fclose(filePointer);
	return content;
}

void shaderCompileTest(GLuint shader)
{
	GLint result = GL_FALSE;
	int logLength;
	glGetShaderiv(shader,
		GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<GLchar> vertShaderError((logLength > 1) ? logLength : 1);
	glGetShaderInfoLog(shader, logLength, NULL, &vertShaderError[0]);
	std::cout << &vertShaderError[0] <<
		std::endl;
}
// Initialization routine.
void setup(void)
{
	int count;
	float p1, p2, p3, p4, average, pfinal;
	srand(3);
	p1 = (rand() % 10) / 5.0 - 1.0;
	p2 = (rand() % 10) / 5.0 - 1.0;
	p3 = (rand() % 10) / 5.0 - 1.0;
	p4 = (rand() % 10) / 5.0 - 1.0;
	// Initialise terrain - set values in the height map to 0
	float terrain[mapSize][mapSize] = {};
	for (int x = 0; x < mapSize; x++)
	{
		for (int z = 0; z < mapSize; z++)
		{
			terrain[x][z] = 0;
		}
	}

	terrain[0][0] = p1*10.0;
	terrain[mapSize - 1][0] = p2*10.0;
	terrain[mapSize - 1][mapSize - 1] = p3*10.0;
	terrain[0][mapSize - 1] = p4*10.0;
	while (stepSize > 1)
	{
		for (int x = 0; x < mapSize - 1; x += stepSize)
			for (int y = 0; y < mapSize - 1; y += stepSize)
			{
				p1 = terrain[x][y];
				p2 = terrain[x][y + stepSize];
				p3 = terrain[x+ stepSize][y];
				p4 = terrain[x + stepSize][y - stepSize];
				average = (p1 + p2 + p3 + p4) / 4;
				pfinal = (rand() % 10 / 5.0 - 1.0);
				average = average + pfinal*10.0*randMax;
				terrain[x + stepSize / 2][y + stepSize / 2] = average;
			}
		
		for (int x = 0; x < mapSize - 1; x += stepSize)
			for (int y = 0; y < mapSize - 1; y += stepSize)
			{
				//first run
				count = 0;
				p1 = terrain[x][y]; count++;
				p2 = terrain[x][y + stepSize];
				if((x-stepSize / 2) >= 0){p3 = terrain[x - stepSize / 2][y - stepSize / 2]; count++;}
				else {p3 = 0.0;}
				if ((x + stepSize / 2) < mapSize) { p4 = terrain[x + stepSize / 2][y + stepSize / 2]; count++; }
				else { p4 = 0.0; }
				average = (p1 + p2 + p3 + p4) / (float)count;
				pfinal = (rand() % 10 / 5.0 - 1.0);
				average = average + pfinal*10.0*randMax;
				terrain[x][y + stepSize / 2] = average;

				//second run
				count = 0;
				p1 = terrain[x][y]; count++;
				p2 = terrain[x + stepSize][y];
				if ((y - stepSize / 2) >= 0) { p3 = terrain[x + stepSize / 2][y - stepSize / 2]; count++; }
				else { p3 = 0.0; }
				if ((y + stepSize / 2) < mapSize) { p4 = terrain[x + stepSize / 2][y + stepSize / 2]; count++; }
				else { p4 = 0.0; }
				average = (p1 + p2 + p3 + p4) / (float)count;
				pfinal = (rand() % 10 / 5.0 - 1.0);
				average = average + pfinal*10.0*randMax;
				terrain[x][y + stepSize / 2] = average;
				
				//third run
				count = 0;
				p1 = terrain[x + stepSize][y]; count++;
				p2 = terrain[x + stepSize][y + stepSize]; count++;
				p3 = terrain[x + stepSize / 2][y + stepSize / 2]; count++; 
				if ((x + stepSize + stepSize / 2) < mapSize) { p4 = terrain[x + stepSize + stepSize / 2][y + stepSize / 2]; count++; }
				else { p4 = 0.0; }
				average = (p1 + p2 + p3 + p4) / (float)count;
				pfinal = (rand() % 10 / 5.0 - 1.0);
				average = average + pfinal*10.0*randMax;
				terrain[x + stepSize][y + stepSize / 2] = average;
				
				//forth run
				count = 0;
				p1 = terrain[x][y + stepSize]; count++;
				p2 = terrain[x + stepSize][y + stepSize]; count++;
				p3 = terrain[x + stepSize / 2][y + stepSize / 2]; count++;
				if ((y +  stepSize + stepSize / 2) < mapSize) { p4 = terrain[x + stepSize / 2][y + stepSize + stepSize / 2]; count++; }
				else { p4 = 0.0; }
				average = (p1 + p2 + p3 + p4) / (float)count;
				pfinal = (rand() % 10 / 5.0 - 1.0);
				average = average + pfinal*10.0*randMax;
				terrain[x + stepSize / 2][y + stepSize] = average;
			}
		randMax = randMax * pow(2, -H);
		stepSize = stepSize / 2;
	}

	// TODO: Add your code here to calculate the height values of the terrain using the Diamond square algorithm
	

	// Intialise vertex array
	int i = 0;
	float fTextureS = float(mapSize)*0.1f;
	float fTextureT = float(mapSize)*0.1f;


	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			// Set the coords (1st 4 elements) and a default colour of black (2nd 4 elements) 
			terrainVertices[i] = { { (float)x, terrain[x][y], (float)y, 1.0 }, { 0.0f, 0.0f, 0.0f } };

			float fScaleC = float(x) / float(mapSize - 1);
			float fScaleR = float(y) / float(mapSize - 1);
			terrainVertices[i].texcoords[0] = (fTextureS*fScaleC);
			terrainVertices[i].texcoords[1] = (fTextureT*fScaleR);
			i++;

		}
	}

	// Now build the index data 
	i = 0;
	for (int z = 0; z < mapSize - 1; z++)
	{
		i = z * mapSize;
		for (int x = 0; x < mapSize * 2; x += 2)
		{
			terrainIndexData[z][x] = i;
			i++;
		}
		for (int x = 1; x < mapSize * 2 + 1; x += 2)
		{
			terrainIndexData[z][x] = i;
			i++;
		}
	}

//calculate the normals
	for (int z = 0; z < mapSize - 1; z++)
	{
		int triPoint1;
		int triPoint2 = terrainIndexData[z][0];
		int triPoint3 = terrainIndexData[z][1];
		for (int x = 2; x < mapSize * 2; x++)
		{
			triPoint1 = triPoint2;
			triPoint2 = triPoint3;
			triPoint3 = terrainIndexData[z][x];

			Vertex triPoint1Info = terrainVertices[triPoint1];
			Vertex triPoint2Info = terrainVertices[triPoint2];
			Vertex triPoint3Info = terrainVertices[triPoint3];

			vec3 point1Position = vec3(triPoint1Info.coords[0], triPoint1Info.coords[1], triPoint1Info.coords[2]);
			vec3 point2Position = vec3(triPoint2Info.coords[0], triPoint2Info.coords[1], triPoint2Info.coords[2]);
			vec3 point3Position = vec3(triPoint3Info.coords[0], triPoint3Info.coords[1], triPoint3Info.coords[2]);

			vec3 dir1 = point2Position - point1Position;
			vec3 dir2 = point3Position - point1Position;

			vec3 normOfPlane = normalize(cross(dir2, dir1));
			
			if (normOfPlane.y < 0)
			{
				normOfPlane.y = normOfPlane.y * -1;
			}

			triPoint1Info.normals[0] += normOfPlane.x;
			triPoint1Info.normals[1] += normOfPlane.y;
			triPoint1Info.normals[2] += normOfPlane.z;

			if (normOfPlane.y < 0)
			{
				normOfPlane.y = normOfPlane.y * -1;
			}
			
			triPoint2Info.normals[0] += normOfPlane.x;
			triPoint2Info.normals[1] += normOfPlane.y;
			triPoint2Info.normals[2] += normOfPlane.z;

			if (normOfPlane.y < 0)
			{
				normOfPlane.y = normOfPlane.y * -1;
			}
			
			triPoint3Info.normals[0] += normOfPlane.x;
			triPoint3Info.normals[1] += normOfPlane.y;
			triPoint3Info.normals[2] += normOfPlane.z;

			

			terrainVertices[triPoint1] = triPoint1Info;
			terrainVertices[triPoint2] = triPoint2Info;
			terrainVertices[triPoint3] = triPoint3Info;
		}
		
	}

	glClearColor(1.0, 1.0, 1.0, 0.0);

	// Create shader program executable - read, compile and link shaders
	char* vertexShader = readTextFile("vertexShader.glsl");
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, (const char**)&vertexShader, NULL);
	glCompileShader(vertexShaderId);
	cout << "Vertex::" << endl;
	shaderCompileTest(vertexShaderId);
	

	char* fragmentShader = readTextFile("fragmentShader.glsl");
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, (const char**)&fragmentShader, NULL);
	glCompileShader(fragmentShaderId);
	cout << "Vertex::" << endl;
	shaderCompileTest(fragmentShaderId);

	programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);
	glUseProgram(programId);
	///////////////////////////////////////

	// Create vertex array object (VAO) and vertex buffer object (VBO) and associate data with vertex shader.
	glGenVertexArrays(1, vao);
	glGenBuffers(1, buffer);
	glBindVertexArray(vao[TERRAIN]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[TERRAIN_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(terrainVertices[0]), (GLvoid*)sizeof(terrainVertices[0].coords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2,GL_FLOAT,GL_FALSE,sizeof(terrainVertices[0]),(GLvoid*)(sizeof(terrainVertices[0].coords) +sizeof(terrainVertices[0].normals)));
	glEnableVertexAttribArray(2);
	///////////////////////////////////////

	// Obtain projection matrix uniform location and set value.
	projMatLoc = glGetUniformLocation(programId, "projMat");
	projMat = perspective(radians(60.0), (double) SCREEN_WIDTH / (double)SCREEN_HEIGHT, 0.1, 100.0);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

	///////////////////////////////////////

	// Obtain modelview matrix uniform location and set value.
	mat4 modelViewMat = mat4(1.0);
	// Move terrain into view - glm::translate replaces glTranslatef
	modelViewMat = glm::lookAt(cameraLocation, (cameraDirection + cameraLocation), vec3(0, 1, 0));
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
	normalMatLoc = glGetUniformLocation(programId,"normalMat");
	normalMat = transpose(inverse(mat3(modelViewMat)));
	glUniformMatrix3fv(normalMatLoc, 1,GL_FALSE, value_ptr(normalMat));
	
	//Set up materials and how they are effected 
	///////////////////////////////////////
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.ambRefl"), 1,&terrainFandB.ambRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.diffRefl"), 1, &terrainFandB.diffRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.specRefl"), 1, &terrainFandB.specRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "terrainFandB.emitCols"), 1, &terrainFandB.emitCols[0]);
	glUniform1f(glGetUniformLocation(programId, "terrainFandB.shininess"), terrainFandB.shininess);
	glUniform4fv(glGetUniformLocation(programId, "globAmb"), 1, &globAmb[0]);
	///////////////////////////////////////

	glUniform4fv(glGetUniformLocation(programId,"light0.ambCols"), 1,&light0.ambCols[0]);
	glUniform4fv(glGetUniformLocation(programId,"light0.diffCols"), 1,&light0.diffCols[0]);
	glUniform4fv(glGetUniformLocation(programId,"light0.specCols"), 1,&light0.specCols[0]);
	glUniform4fv(glGetUniformLocation(programId,"light0.coords"), 1,&light0.coords[0]);

	image[0] = getbmp("grass.bmp");
	glGenTextures(1, texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,GL_RGBA,GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	grassTexLoc =glGetUniformLocation(programId,"grassTex");
	glUniform1i(grassTexLoc, 0);

}

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// For each row - draw the triangle strip
	for (int i = 0; i < mapSize - 1; i++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, verticesPerStrip, GL_UNSIGNED_INT, terrainIndexData[i]);
	}

	glFlush();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, (float)w, (float)h);
}


// this function holds my transfromation matrix for my scene (including changeing the look at so that it moves based off my camera location and direction)
void cameraDisplayUpdate()
{
	mat4 modelViewMat = mat4(1.0);
	modelViewMat = glm::lookAt(cameraLocation, (cameraDirection + cameraLocation), vec3(0, 1, 0));
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
	glutPostRedisplay();
}
// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	// this section is where I set up my key inputs as to make sure that the user can move the camera
	// using "W,A,S,D" will move the camera by increaseing/decrese depending on if its moving forwards or backwards the location in the direction the camera is facing by 0.4 speed
	//using "I,J,K,L" will rotate the camera by increasing/ decreasing the angle value in my angle and increasing that by one and using "SIN,COS,TAN" to work out the angles I need for my X,Y,Z axis on my camera
	case 'w':
	{
		cameraLocation = cameraLocation + cameraDirection * 0.4f; 
		cameraDisplayUpdate();
		break;
	}

	case's':
	{
		cameraLocation = cameraLocation - cameraDirection * 0.4f;
		cameraDisplayUpdate();
		break;
	}



	case 'j':
	{
		cameraAngle.y = cameraAngle.y + 1.0f;
		cameraDirection.x = sin(glm::radians(cameraAngle.y));
		cameraDirection.y = tan(glm::radians(cameraAngle.x));
		cameraDirection.z = cos(glm::radians(cameraAngle.y));
		cameraDirection = normalize(cameraDirection);
		cameraDisplayUpdate();
		break;
	}
	case 'l':
	{
		cameraAngle.y = cameraAngle.y - 1.0f;
		cameraDirection.x = sin(glm::radians(cameraAngle.y));
		cameraDirection.y = tan(glm::radians(cameraAngle.x));
		cameraDirection.z = cos(glm::radians(cameraAngle.y));
		cameraDirection = normalize(cameraDirection);
		cameraDisplayUpdate();
		break;
	}
	case 'i':
	{
		cameraAngle.x = cameraAngle.x + 1.0f;
		cameraDirection.x = sin(glm::radians(cameraAngle.y));
		cameraDirection.y = tan(glm::radians(cameraAngle.x));
		cameraDirection.z = cos(glm::radians(cameraAngle.y));
		cameraDirection = normalize(cameraDirection);
		cameraDisplayUpdate();
		break;
	}
	case 'k':
	{
		cameraAngle.x = cameraAngle.x - 1.0f;
		cameraDirection.x = sin(glm::radians(cameraAngle.y));
		cameraDirection.y = tan(glm::radians(cameraAngle.x));
		cameraDirection.z = cos(glm::radians(cameraAngle.y));
		cameraDirection = normalize(cameraDirection);
		cameraDisplayUpdate();
		break;
	}
	
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}


// Main routine.
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glEnable(GL_DEPTH_TEST);
	// Set the version of OpenGL (4.2)
	glutInitContextVersion(4, 2);
	// The core profile excludes all discarded features
	glutInitContextProfile(GLUT_CORE_PROFILE);
	// Forward compatibility excludes features marked for deprecation ensuring compatability with future versions
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("TerrainGeneration");
	cout << "to move backward and forward you use W(forward) and S(backwards)" << endl;
	cout << "to rotate the camera you use the I,K,J and L keys. To rotate up and down you use I(UP) and K(Down)" <<
		"to Rotate left and right you use J(Left) and L(Right)" << endl;

	// Set OpenGL to render in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
