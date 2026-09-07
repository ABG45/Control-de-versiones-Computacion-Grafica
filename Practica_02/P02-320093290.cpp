//Práctica 2: índices, mesh, proyecciones, transformaciones geométricas
//
// PROGRAMA UNIFICADO (combina los dos mains anteriores en uno solo):
// 1) Las 3 figuras de clase se dibujan instanciando pirámide/cubo (meshList)
//    en vez de triángulos/cuadrados 2D, cada pieza con su propio shader de
//    color sólido (rojo, verde, azul, café, magenta).
// 2) Las iniciales "AGZ" se dibujan como título en la parte superior de la
//    escena, cada letra de un color distinto, reutilizando el MISMO
//    mecanismo de color por vértice (MeshColor + shaderColor) que ya se
//    usaba para el piso -- así no hace falta un shader ni una ventana aparte.
// 3) El fondo cambia de color aleatoriamente cada 2 segundos, evitando
//    blanco, negro y los colores usados en las piezas/letras (rojo, verde,
//    azul, café, magenta), tal como pide el título de la imagen de
//    referencia ("Color diferente a blanco, negro y los de las figuras").

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
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

const float toRadians = 3.14159265f / 180.0f; //grados a radianes

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshColorList;
std::vector<Shader> shaderList;

//Vertex shader genérico para los shaders de color sólido (pirámide/cubo)
static const char* vShader = "shaders/shader.vert";
//Fragment shaders de color sólido (uno por color)
static const char* fShaderRojo = "shaders/shaderrojo.frag";
static const char* fShaderVerde = "shaders/shaderverde.frag";
static const char* fShaderAzul = "shaders/shaderazul.frag";
static const char* fShaderCafe = "shaders/shadercafe.frag";
static const char* fShaderMagenta = "shaders/shadermagenta.frag";

//Shader de color por vértice (piso + letras)
static const char* vShaderColor = "shaders/shadercolor.vert";
static const char* fShaderColor = "shaders/shadercolor.frag";

// Índices dentro de shaderList
enum ShaderIndex
{
	SH_COLOR = 0,   // color por vértice: piso y letras
	SH_ROJO,
	SH_VERDE,
	SH_AZUL,
	SH_CAFE,
	SH_MAGENTA
};

// Índices dentro de meshColorList
enum MeshColorIndex
{
	MC_PISO = 0,
	MC_LETRA_A1,
	MC_LETRA_G,
	MC_LETRA_3
};

//------------------------------------------------------------------
// FONDO ALEATORIO (tomado del programa de iniciales)
//------------------------------------------------------------------
float colorTimer = 0.0f;
float bgR = 0.3f, bgG = 0.5f, bgB = 0.6f;

void GenerarColorAleatorio()
{
	// Restringimos cada canal a un rango medio [0.15, 0.75] para evitar
	// blanco (1,1,1) y negro (0,0,0); y evitamos que los 3 canales queden
	// casi iguales entre sí (grises), y que el color se acerque demasiado
	// a alguno de los colores sólidos usados en las piezas (rojo, verde,
	// azul, café, magenta), que siempre tienen algún canal en el extremo.
	float r, g, b;
	do
	{
		r = 0.15f + (rand() % 61) / 100.0f; // 0.15 - 0.75
		g = 0.15f + (rand() % 61) / 100.0f;
		b = 0.15f + (rand() % 61) / 100.0f;
	} while (fabs(r - g) < 0.05f && fabs(g - b) < 0.05f); // evita grises

	bgR = r; bgG = g; bgB = b;
}

void ActualizarColorFondo(float deltaTime)
{
	colorTimer += deltaTime;
	if (colorTimer >= 2.0f)
	{
		colorTimer = 0.0f;
		GenerarColorAleatorio();
	}
}

//------------------------------------------------------------------
// MALLAS 3D REUTILIZABLES: pirámide y cubo
//------------------------------------------------------------------
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
	meshList.push_back(obj1); // meshList[0] = piramide
}

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
	meshList.push_back(cubo); // meshList[1] = cubo
}

//------------------------------------------------------------------
// PISO (color por vértice)
//------------------------------------------------------------------
void CrearPiso()
{
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
	meshColorList.push_back(cuadradonegro); // meshColorList[MC_PISO]
}

//------------------------------------------------------------------
// INICIALES "AGZ" (bitmap 5x7, cada letra con color propio horneado
// en el vértice, igual mecanismo que las piezas de color del piso)
//------------------------------------------------------------------
const int FILAS = 7, COLUMNAS = 5;

int letraA[FILAS][COLUMNAS] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,1,1,1,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{1,0,0,0,1}
};

int letraG[FILAS][COLUMNAS] = {
	{0,1,1,1,0},
	{1,0,0,0,1},
	{1,0,0,0,0},
	{1,0,1,1,1},
	{1,0,0,0,1},
	{1,0,0,0,1},
	{0,1,1,1,0}
};

int letraZ[FILAS][COLUMNAS] = {
	{1,1,1,1,1},
	{0,0,0,1,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,1,0,0,0},
	{1,0,0,0,0},
	{1,1,1,1,1}
};

// Genera los vértices (pos + color) de una letra, centrada en el origen
// local (para poder posicionarla luego con una simple traslación, igual
// que se hace con las piezas de las figuras).
std::vector<GLfloat> GenerarVerticesLetraColor(int bitmap[FILAS][COLUMNAS],
	float cellW, float cellH, float r, float g, float b)
{
	std::vector<GLfloat> verts;
	float anchoLetra = COLUMNAS * cellW;
	float altoLetra = FILAS * cellH;
	float gx0base = -anchoLetra / 2.0f;
	float gy0base = -altoLetra / 2.0f;

	for (int fila = 0; fila < FILAS; fila++)
	{
		for (int col = 0; col < COLUMNAS; col++)
		{
			if (bitmap[fila][col] == 1)
			{
				float x0 = gx0base + col * cellW;
				float x1 = x0 + cellW;

				// La fila 0 del bitmap es la de ARRIBA
				float yTop = gy0base + (FILAS - fila) * cellH;
				float yBot = yTop - cellH;

				// Triangulo 1 (inferior izquierdo)
				verts.push_back(x0); verts.push_back(yBot); verts.push_back(0.0f);
				verts.push_back(r); verts.push_back(g); verts.push_back(b);

				verts.push_back(x1); verts.push_back(yBot); verts.push_back(0.0f);
				verts.push_back(r); verts.push_back(g); verts.push_back(b);

				verts.push_back(x0); verts.push_back(yTop); verts.push_back(0.0f);
				verts.push_back(r); verts.push_back(g); verts.push_back(b);

				// Triangulo 2 (superior derecho)
				verts.push_back(x1); verts.push_back(yBot); verts.push_back(0.0f);
				verts.push_back(r); verts.push_back(g); verts.push_back(b);

				verts.push_back(x1); verts.push_back(yTop); verts.push_back(0.0f);
				verts.push_back(r); verts.push_back(g); verts.push_back(b);

				verts.push_back(x0); verts.push_back(yTop); verts.push_back(0.0f);
				verts.push_back(r); verts.push_back(g); verts.push_back(b);
			}
		}
	}
	return verts;
}

void CrearLetras()
{
	float cellW = 0.15f, cellH = 0.18f;

	// Letra 1 (A) -> rojo, Letra 2 (G) -> verde, Letra 3 (Z) -> azul
	auto vertsA1 = GenerarVerticesLetraColor(letraA, cellW, cellH, 1.0f, 0.0f, 0.0f);
	auto vertsG = GenerarVerticesLetraColor(letraG, cellW, cellH, 0.0f, 0.6f, 0.0f);
	auto vertsLetra3 = GenerarVerticesLetraColor(letraZ, cellW, cellH, 0.0f, 0.4f, 1.0f);

	MeshColor* letraA1Mesh = new MeshColor();
	letraA1Mesh->CreateMeshColor(vertsA1.data(), (unsigned int)vertsA1.size());
	meshColorList.push_back(letraA1Mesh); // MC_LETRA_A1

	MeshColor* letraGMesh = new MeshColor();
	letraGMesh->CreateMeshColor(vertsG.data(), (unsigned int)vertsG.size());
	meshColorList.push_back(letraGMesh); // MC_LETRA_G

	MeshColor* letra3Mesh = new MeshColor();
	letra3Mesh->CreateMeshColor(vertsLetra3.data(), (unsigned int)vertsLetra3.size());
	meshColorList.push_back(letra3Mesh); // MC_LETRA_3
}

//------------------------------------------------------------------
// SHADERS
//------------------------------------------------------------------
void CreateShaders()
{
	Shader* shaderColor = new Shader();
	shaderColor->CreateFromFiles(vShaderColor, fShaderColor);
	shaderList.push_back(*shaderColor); // SH_COLOR

	Shader* shaderRojo = new Shader();
	shaderRojo->CreateFromFiles(vShader, fShaderRojo);
	shaderList.push_back(*shaderRojo); // SH_ROJO

	Shader* shaderVerde = new Shader();
	shaderVerde->CreateFromFiles(vShader, fShaderVerde);
	shaderList.push_back(*shaderVerde); // SH_VERDE

	Shader* shaderAzul = new Shader();
	shaderAzul->CreateFromFiles(vShader, fShaderAzul);
	shaderList.push_back(*shaderAzul); // SH_AZUL

	Shader* shaderCafe = new Shader();
	shaderCafe->CreateFromFiles(vShader, fShaderCafe);
	shaderList.push_back(*shaderCafe); // SH_CAFE

	Shader* shaderMagenta = new Shader();
	shaderMagenta->CreateFromFiles(vShader, fShaderMagenta);
	shaderList.push_back(*shaderMagenta); // SH_MAGENTA
}

//------------------------------------------------------------------
// FUNCIONES DE DIBUJO
//------------------------------------------------------------------
void DibujarPiezaMesh(Mesh* mesh, Shader& shaderColorSolido, const glm::mat4& projection,
	glm::vec3 figuraOffset, float anguloGrados, glm::vec3 traslacionLocal, glm::vec3 escala)
{
	shaderColorSolido.useShader();
	GLuint uniformModel = shaderColorSolido.getModelLocation();
	GLuint uniformProjection = shaderColorSolido.getProjectLocation();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, figuraOffset);
	if (anguloGrados != 0.0f)
		model = glm::rotate(model, glm::radians(anguloGrados), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, traslacionLocal);
	model = glm::scale(model, escala);

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	mesh->RenderMesh();
}

void DibujarPiezaColor(MeshColor* mesh, Shader& shaderColorVertice, const glm::mat4& projection,
	glm::vec3 figuraOffset, float anguloGrados, glm::vec3 traslacionLocal, glm::vec3 escala)
{
	shaderColorVertice.useShader();
	GLuint uniformModel = shaderColorVertice.getModelLocation();
	GLuint uniformProjection = shaderColorVertice.getProjectLocation();

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
	srand((unsigned int)time(NULL));
	GenerarColorAleatorio();

	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	glDisable(GL_DEPTH_TEST);

	CreaPiramide();   // meshList[0]
	CrearCubo();      // meshList[1]
	CrearPiso();      // meshColorList[MC_PISO]
	CrearLetras();    // meshColorList[MC_LETRA_A1 / MC_LETRA_G / MC_LETRA_3]
	CreateShaders();

	Mesh* piramide = meshList[0];
	Mesh* cubo = meshList[1];

	// Ortho con la misma relación de aspecto que la ventana (800/600 = 4/3)
	glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 100.0f);

	double lastTime = glfwGetTime();

	while (!mainWindow.getShouldClose())
	{
		double currentTime = glfwGetTime();
		float deltaTime = (float)(currentTime - lastTime);
		lastTime = currentTime;

		ActualizarColorFondo(deltaTime);
		glfwPollEvents();

		glClearColor(bgR, bgG, bgB, 1.0f); // fondo aleatorio (nunca blanco/negro/color de figura)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 offset1(-2.5f, 0.0f, -4.0f); // figura 1 (barras)
		glm::vec3 offset2(0.0f, 0.0f, -4.0f);  // figura 2 (cuadrado + rombos)
		glm::vec3 offset3(2.5f, 0.0f, -4.0f);  // figura 3 (Sierpinski)
		glm::vec3 offsetGlobal(0.0f, 0.0f, -4.0f); // piso y letras

		// ===================== TÍTULO: iniciales "AGZ" =====================
		float cellW = 0.15f, cellH = 0.18f, gapLetras = 0.20f;
		float anchoLetra = COLUMNAS * cellW;
		float totalWidth = 3 * anchoLetra + 2 * gapLetras;
		float startX = -totalWidth / 2.0f;
		float yTitulo = 2.3f;

		DibujarPiezaColor(meshColorList[MC_LETRA_A1], shaderList[SH_COLOR], projection,
			offsetGlobal, 0.0f, glm::vec3(startX + anchoLetra / 2.0f, yTitulo, 0.0f), glm::vec3(1.0f));
		DibujarPiezaColor(meshColorList[MC_LETRA_G], shaderList[SH_COLOR], projection,
			offsetGlobal, 0.0f, glm::vec3(startX + anchoLetra + gapLetras + anchoLetra / 2.0f, yTitulo, 0.0f), glm::vec3(1.0f));
		DibujarPiezaColor(meshColorList[MC_LETRA_3], shaderList[SH_COLOR], projection,
			offsetGlobal, 0.0f, glm::vec3(startX + 2 * (anchoLetra + gapLetras) + anchoLetra / 2.0f, yTitulo, 0.0f), glm::vec3(1.0f));

		// ===================== PISO NEGRO =====================
		DibujarPiezaColor(meshColorList[MC_PISO], shaderList[SH_COLOR], projection,
			offsetGlobal, 0.0f, glm::vec3(0.0f, -1.36f, 0.0f), glm::vec3(8.0f, 0.30f, 1.0f));

		// ===================== FIGURA 1: barras + triángulos invertidos =====================
		// OJO: la pirámide (meshList[0]) tiene medio-ancho/alto = 0.5, mientras que el
		// triángulo de color original (trianguloamarillo, etc.) tenía medio-ancho/alto = 1.
		// Por eso aquí la escala se DUPLICA respecto a la práctica original, para que el
		// tamaño final de la pieza en pantalla sea el mismo.
		DibujarPiezaMesh(piramide, shaderList[SH_MAGENTA], projection,
			offset1, 0.0f, glm::vec3(0.0f, 0.30f, 0.0f), glm::vec3(0.90f, -0.60f, 1.0f)); // (antes: amarillo)
		DibujarPiezaMesh(piramide, shaderList[SH_ROJO], projection,
			offset1, 0.0f, glm::vec3(0.0f, -0.32f, 0.0f), glm::vec3(0.90f, -0.60f, 1.0f)); // (antes: rojo)
		DibujarPiezaMesh(piramide, shaderList[SH_VERDE], projection,
			offset1, 0.0f, glm::vec3(0.0f, -0.92f, 0.0f), glm::vec3(0.90f, -0.60f, 1.0f)); // (antes: verde)

		DibujarPiezaMesh(cubo, shaderList[SH_CAFE], projection,
			offset1, 0.0f, glm::vec3(-0.50f, -0.16f, 0.0f), glm::vec3(0.10f, 2.10f, 1.0f)); // barra izq
		DibujarPiezaMesh(cubo, shaderList[SH_CAFE], projection,
			offset1, 0.0f, glm::vec3(0.50f, -0.16f, 0.0f), glm::vec3(0.10f, 2.10f, 1.0f)); // barra der

		// ============= FIGURA 2: cuadrado (4 "triángulos") + rombo grande + rombo chico =============
		// (misma corrección de escala x2 que en la figura 1, por ser piezas de pirámide)
		DibujarPiezaMesh(piramide, shaderList[SH_MAGENTA], projection,
			offset2, -135.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(2.0f, -1.0f, 1.0f)); // (antes: amarillo, arriba)
		DibujarPiezaMesh(piramide, shaderList[SH_ROJO], projection,
			offset2, 135.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(2.0f, -1.0f, 1.0f)); // (antes: rojo, derecha)
		DibujarPiezaMesh(piramide, shaderList[SH_VERDE], projection,
			offset2, 45.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(2.0f, -1.0f, 1.0f)); // (antes: verde, abajo)
		DibujarPiezaMesh(piramide, shaderList[SH_AZUL], projection,
			offset2, -45.0f, glm::vec3(0.0f, -1.21f, 0.0f), glm::vec3(2.0f, -1.0f, 1.0f)); // (antes: morado, izquierda)

		DibujarPiezaMesh(piramide, shaderList[SH_CAFE], projection,
			offset2, 0.0f, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(2.0f, 1.0f, 1.0f)); // rombo grande, mitad arriba (antes: azul)
		DibujarPiezaMesh(piramide, shaderList[SH_CAFE], projection,
			offset2, 0.0f, glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(2.0f, -1.0f, 1.0f)); // rombo grande, mitad abajo (antes: azul)

		DibujarPiezaMesh(cubo, shaderList[SH_MAGENTA], projection,
			offset2, 45.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.45f, 0.45f, 1.0f)); // rombo chico (antes: café)

		// ===================== FIGURA 3: triángulo grande dividido en 4 =====================
		// (misma corrección de escala x2 que en las figuras 1 y 2)
		DibujarPiezaMesh(piramide, shaderList[SH_MAGENTA], projection,
			offset3, 0.0f, glm::vec3(0.0f, 0.28f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // (antes: morado)
		DibujarPiezaMesh(piramide, shaderList[SH_VERDE], projection,
			offset3, 0.0f, glm::vec3(-0.5f, -0.72f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // (antes: verde)
		DibujarPiezaMesh(piramide, shaderList[SH_ROJO], projection,
			offset3, 0.0f, glm::vec3(0.5f, -0.72f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)); // (antes: rojo)
		DibujarPiezaMesh(piramide, shaderList[SH_CAFE], projection,
			offset3, 0.0f, glm::vec3(0.0f, -0.72f, 0.0f), glm::vec3(1.0f, -1.0f, 1.0f)); // (antes: amarillo invertido)

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}