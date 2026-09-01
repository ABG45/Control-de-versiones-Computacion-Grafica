//Práctica 2: índices, mesh, proyecciones, transformaciones geométricas
#include <stdio.h>
#include <string.h>
#include<cmath>
#include<vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include<glm.hpp>
#include<gtc\matrix_transform.hpp>
#include<gtc\type_ptr.hpp>
//clases para dar orden y limpieza al código
#include"Mesh.h"
#include"Shader.h"
#include"Window.h"
//Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0; //grados a radianes
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshColorList;
std::vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
static const char* fShaderColor = "shaders/shadercolor.frag";

float angulo = 0.0f;

//color café en RGB : 0.478, 0.255, 0.067

//Pirámide triangular regular
void CreaPiramide()
{
	unsigned int indices[] = {
		0,1,2,
		1,3,2,
		3,0,2,
		1,0,3
	};
	GLfloat vertices[] = {
		-0.5f, -0.5f,0.0f,
		0.5f,-0.5f,0.0f,
		0.0f,0.5f, -0.25f,
		0.0f,-0.5f,-0.5f,
	};
	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 12, 12);
	meshList.push_back(obj1);
}

//Vértices de un cubo
void CrearCubo()
{
	unsigned int cubo_indices[] = {
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		4, 5, 1, 1, 0, 4,
		3, 2, 6, 6, 7, 3
	};
	GLfloat cubo_vertices[] = {
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

// Índices en meshColorList al terminar esta función:
// 0 letras (sin usar)   1 triangulorojo    2 cuadradoverde
// 3 trianguloazul       4 trianguloverde   5 trianguloamarillo
// 6 trianguloPurpura    7 cuadradorojo     8 cuadradocafe
// 9 cuadradonegro (piso)
// OJO: triangulo* tiene medio-ancho = 1 (vertices en -1..1)
//      cuadrado*  tiene medio-ancho = 0.5 (vertices en -0.5..0.5)
void CrearLetrasyFiguras()
{
	GLfloat vertices_letras[] = {
		-1.0f,	-1.0f,		0.5f,			0.0f,	0.0f,	1.0f,
		1.0f,	-1.0f,		0.5f,			0.0f,	0.0f,	1.0f,
		1.0f,	1.0f,		0.5f,			0.0f,	0.0f,	1.0f,
	};
	MeshColor* letras = new MeshColor();
	letras->CreateMeshColor(vertices_letras, 18);
	meshColorList.push_back(letras); // 0

	GLfloat vertices_triangulorojo[] = {
		-1.0f,	-1.0f,		0.5f,			1.0f,	0.0f,	0.0f,
		1.0f,	-1.0f,		0.5f,			1.0f,	0.0f,	0.0f,
		0.0f,	1.0f,		0.5f,			1.0f,	0.0f,	0.0f,
	};
	MeshColor* triangulorojo = new MeshColor();
	triangulorojo->CreateMeshColor(vertices_triangulorojo, 18);
	meshColorList.push_back(triangulorojo); // 1

	GLfloat vertices_cuadradoverde[] = {
		-0.5f,	-0.5f,		0.5f,			0.0f,	1.0f,	0.0f,
		0.5f,	-0.5f,		0.5f,			0.0f,	1.0f,	0.0f,
		0.5f,	0.5f,		0.5f,			0.0f,	1.0f,	0.0f,
		-0.5f,	-0.5f,		0.5f,			0.0f,	1.0f,	0.0f,
		0.5f,	0.5f,		0.5f,			0.0f,	1.0f,	0.0f,
		-0.5f,	0.5f,		0.5f,			0.0f,	1.0f,	0.0f,
	};
	MeshColor* cuadradoverde = new MeshColor();
	cuadradoverde->CreateMeshColor(vertices_cuadradoverde, 36);
	meshColorList.push_back(cuadradoverde); // 2

	GLfloat vertices_trianguloazul[] = {
		-1.0f, -1.0f, 0.5f,   0.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 0.5f,   0.0f, 0.0f, 1.0f,
		 0.0f,  1.0f, 0.5f,   0.0f, 0.0f, 1.0f,
	};
	MeshColor* trianguloazul = new MeshColor();
	trianguloazul->CreateMeshColor(vertices_trianguloazul, 18);
	meshColorList.push_back(trianguloazul); // 3

	GLfloat vertices_trianguloverde[] = {
		-1.0f, -1.0f, 0.5f,   0.0f, 0.5f, 0.0f,
		 1.0f, -1.0f, 0.5f,   0.0f, 0.5f, 0.0f,
		 0.0f,  1.0f, 0.5f,   0.0f, 0.5f, 0.0f,
	};
	MeshColor* trianguloverde = new MeshColor();
	trianguloverde->CreateMeshColor(vertices_trianguloverde, 18);
	meshColorList.push_back(trianguloverde); // 4

	GLfloat vertices_trianguloamarillo[] = {
		-1.0f, -1.0f, 0.5f,   1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, 0.5f,   1.0f, 1.0f, 0.0f,
		 0.0f,  1.0f, 0.5f,   1.0f, 1.0f, 0.0f,
	};
	MeshColor* trianguloamarillo = new MeshColor();
	trianguloamarillo->CreateMeshColor(vertices_trianguloamarillo, 18);
	meshColorList.push_back(trianguloamarillo); // 5

	GLfloat vertices_trianguloPurpura[] = {
		-1.0f, -1.0f, 0.5f,   0.5f, 0.0f, 1.0f,
		 1.0f, -1.0f, 0.5f,   0.5f, 0.0f, 1.0f,
		 0.0f,  1.0f, 0.5f,   0.5f, 0.0f, 1.0f,
	};
	MeshColor* trianguloPurpura = new MeshColor();
	trianguloPurpura->CreateMeshColor(vertices_trianguloPurpura, 18);
	meshColorList.push_back(trianguloPurpura); // 6

	GLfloat vertices_cuadradorojo[] = {
		-0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
	};
	MeshColor* cuadradorojo = new MeshColor();
	cuadradorojo->CreateMeshColor(vertices_cuadradorojo, 36);
	meshColorList.push_back(cuadradorojo); // 7

	GLfloat vertices_cuadradocafe[] = {
		-0.5f, -0.5f, 0.5f,   0.478f, 0.255f, 0.067f,
		 0.5f, -0.5f, 0.5f,   0.478f, 0.255f, 0.067f,
		 0.5f,  0.5f, 0.5f,   0.478f, 0.255f, 0.067f,
		-0.5f, -0.5f, 0.5f,   0.478f, 0.255f, 0.067f,
		 0.5f,  0.5f, 0.5f,   0.478f, 0.255f, 0.067f,
		-0.5f,  0.5f, 0.5f,   0.478f, 0.255f, 0.067f,
	};
	MeshColor* cuadradocafe = new MeshColor();
	cuadradocafe->CreateMeshColor(vertices_cuadradocafe, 36);
	meshColorList.push_back(cuadradocafe); // 8

	GLfloat vertices_cuadradonegro[] = {
		-0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
	};
	MeshColor* cuadradonegro = new MeshColor();
	cuadradonegro->CreateMeshColor(vertices_cuadradonegro, 36);
	meshColorList.push_back(cuadradonegro); // 9
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	Shader* shader2 = new Shader();
	shader2->CreateFromFiles(vShaderColor, fShaderColor);
	shaderList.push_back(*shader2);
}

void DibujarPieza(MeshColor* mesh, GLuint uniformModel, GLuint uniformProjection,
	const glm::mat4& projection, glm::vec3 figuraOffset,
	float anguloGrados, glm::vec3 traslacionLocal, glm::vec3 escala)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, figuraOffset);
	if (anguloGrados != 0.0f)
		model = glm::rotate(model, glm::radians(anguloGrados), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, traslacionLocal);
	model = glm::scale(model, escala);

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	mesh->RenderMeshColor();
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	//Escena 2D "por capas": desactivamos el depth test para que el orden de
	//dibujo (pintor) decida qué queda encima, aunque todo esté al mismo Z.
	glDisable(GL_DEPTH_TEST);

	CreaPiramide();
	CrearCubo();
	CrearLetrasyFiguras();
	CreateShaders();
	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;

	// Ortho con la misma relación de aspecto que la ventana (800/600 = 4/3)
	glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 100.0f);

	glm::mat4 model(1.0);

	while (!mainWindow.getShouldClose())
	{
		glfwPollEvents();
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // fondo blanco
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[1].useShader();
		uniformModel = shaderList[1].getModelLocation();
		uniformProjection = shaderList[1].getProjectLocation();

		glm::vec3 offset1(-2.5f, 0.0f, -4.0f); // figura 1 (barras)
		glm::vec3 offset2(0.0f, 0.0f, -4.0f); // figura 2 (cuadrado + rombos)
		glm::vec3 offset3(2.5f, 0.0f, -4.0f); // figura 3 (Sierpinski)
		glm::vec3 offsetGlobal(0.0f, 0.0f, -4.0f); // para piezas que no pertenecen a una sola figura (piso)

		// ===================== PISO NEGRO: toca ambos extremos de la pantalla =====================
		// ortho va de -4 a 4 en X -> medio-ancho necesario = 4 -> escala.x = 4 / 0.5 = 8
		DibujarPieza(meshColorList[9], uniformModel, uniformProjection, projection,
			offsetGlobal, 0.0f, glm::vec3(0.0f, -1.36f, 0.0f), glm::vec3(8.0f, 0.30f, 1.0f));

		// ===================== FIGURA 1: barras + triángulos invertidos =====================
		// Triángulos: base=0.45 de medio-ancho -> sus vértices exteriores quedan en x=-0.45 y x=+0.45
		DibujarPieza(meshColorList[5], uniformModel, uniformProjection, projection,
			offset1, 0.0f, glm::vec3(0.0f, 0.30f, 0.0f), glm::vec3(0.45f, -0.30f, 1.0f)); // amarillo
		DibujarPieza(meshColorList[1], uniformModel, uniformProjection, projection,
			offset1, 0.0f, glm::vec3(0.0f, -0.32f, 0.0f), glm::vec3(0.45f, -0.30f, 1.0f)); // rojo
		DibujarPieza(meshColorList[4], uniformModel, uniformProjection, projection,
			offset1, 0.0f, glm::vec3(0.0f, -0.92f, 0.0f), glm::vec3(0.45f, -0.30f, 1.0f)); // verde

		// Barras café: su borde interior toca x=-0.45/+0.45 (vértice exterior de los triángulos),
		// suben más allá de la base del triángulo amarillo (y=0.92) y bajan hasta el piso (y=-1.0)
		DibujarPieza(meshColorList[8], uniformModel, uniformProjection, projection,
			offset1, 0.0f, glm::vec3(-0.50f, -0.16f, 0.0f), glm::vec3(0.10f, 2.10f, 1.0f)); // barra izq
		DibujarPieza(meshColorList[8], uniformModel, uniformProjection, projection,
			offset1, 0.0f, glm::vec3(0.50f, -0.16f, 0.0f), glm::vec3(0.10f, 2.10f, 1.0f)); // barra der

		// ============= FIGURA 2: cuadrado (4 triángulos) + rombo azul + rombo café =============
		DibujarPieza(meshColorList[5], uniformModel, uniformProjection, projection,
			offset2, -135.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(1.0f, -0.5f, 1.0f)); // amarillo (arriba)
		DibujarPieza(meshColorList[1], uniformModel, uniformProjection, projection,
			offset2, 135.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(1.0f, -0.5f, 1.0f)); // rojo (derecha)
		DibujarPieza(meshColorList[4], uniformModel, uniformProjection, projection,
			offset2, 45.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(1.0f, -0.5f, 1.0f)); // verde (abajo)
		DibujarPieza(meshColorList[6], uniformModel, uniformProjection, projection,
			offset2, -45.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(1.0f, -0.5f, 1.0f)); // morado (izquierda)

		// Rombo azul (grande, centro): 2 mitades de trianguloazul; sus puntas tocan
		// los puntos medios de los 4 lados del cuadrado exterior
		DibujarPieza(meshColorList[3], uniformModel, uniformProjection, projection,
			offset2, 0.0f, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 0.5f, 1.0f)); // mitad de arriba
		DibujarPieza(meshColorList[3], uniformModel, uniformProjection, projection,
			offset2, 0.0f, glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(1.0f, -0.5f, 1.0f)); // mitad de abajo

		// Rombo café (chico), centrado dentro del rombo azul (cuadrado rotado 45°)
		DibujarPieza(meshColorList[8], uniformModel, uniformProjection, projection,
			offset2, 45.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.45f, 0.45f, 1.0f));

		// ===================== FIGURA 3: triángulo grande dividido en 4 =====================
		DibujarPieza(meshColorList[6], uniformModel, uniformProjection, projection,
			offset3, 0.0f, glm::vec3(0.0f, 0.28f, 0.0f), glm::vec3(0.5f, 0.5f, 1.0f)); // morado
		DibujarPieza(meshColorList[4], uniformModel, uniformProjection, projection,
			offset3, 0.0f, glm::vec3(-0.5f, -0.72f, 0.0f), glm::vec3(0.5f, 0.5f, 1.0f)); // verde
		DibujarPieza(meshColorList[1], uniformModel, uniformProjection, projection,
			offset3, 0.0f, glm::vec3(0.5f, -0.72f, 0.0f), glm::vec3(0.5f, 0.5f, 1.0f)); // rojo
		DibujarPieza(meshColorList[5], uniformModel, uniformProjection, projection,
			offset3, 0.0f, glm::vec3(0.0f, -0.72f, 0.0f), glm::vec3(0.5f, -0.5f, 1.0f)); // amarillo invertido

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}