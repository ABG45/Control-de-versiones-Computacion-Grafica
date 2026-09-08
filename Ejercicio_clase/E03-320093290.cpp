//pr�ctica 3: Modelado Geom�trico y C�mara Sint�tica.
//VERSI�N MODIFICADA: se agrega la construcci�n de la figura tipo "panel/marco" con
//rombos concentricos, arcos, circulo y piramides, reutilizando el cubo y la piramide
//triangular ya definidos en el proyecto (meshList[0] y meshList[1]).
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
#include <gtc\random.hpp>
//clases para dar orden y limpieza al c�digo
#include"Mesh.h"
#include"Shader.h"
#include"Sphere.h"
#include"Window.h"
#include"Camera.h"
//tecla E: Rotar sobre el eje X
//tecla R: Rotar sobre el eje Y
//tecla T: Rotar sobre el eje Z


using std::vector;

//Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0; //grados a radianes
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks

//�ndices de meshList, para no usar n�meros "m�gicos" (deben coincidir con el orden
//en que se llaman las funciones Crear* dentro de main())
const int MESH_CUBO = 0;
const int MESH_PIRAMIDE_TRI = 1;
const int MESH_CILINDRO = 2;
const int MESH_CONO = 3;
const int MESH_PIRAMIDE_CUAD = 4;
const int MESH_TRIANGULO_ESQUINA = 5;
const int MESH_TRIANGULO_DIAMANTE = 6;
const int MESH_ESQUINA_PENTAGONO = 7;

//Paleta de colores tomada de la imagen de referencia (cubo central)
const glm::vec3 COLOR_AZUL(0.0f, 0.0f, 1.0f);
const glm::vec3 COLOR_AMARILLO(1.0f, 1.0f, 0.0f);
const glm::vec3 COLOR_ROJO(1.0f, 0.0f, 0.0f);
const glm::vec3 COLOR_VERDE(0.0f, 1.0f, 0.0f);
const glm::vec3 COLOR_MORADO(0.50f, 0.0f, 1.0f);
const glm::vec3 COLOR_OLIVA(0.50f, 0.50f, 0.0f);
const glm::vec3 COLOR_MARCO(0.15f, 0.15f, 0.15f); //gris carb�n (marco/aro, evita negro puro)

//La base ahora es un CUBO real (mismo tama�o en X, Y y Z), no una caja achatada,
//para que el dise�o del panel se pueda repetir igual en las 6 caras.
const float CUBO_LADO = 3.6f;
const float MITAD_CUBO = CUBO_LADO / 2.0f; //distancia del centro del cubo a cualquiera de sus caras
const float RADIO_ROMBO_GRANDE = 1.25f; //distancia del centro de la cara a cada punta del rombo grande
const float MARCO_CENTRO = 1.55f;  //distancia del centro de la cara al centro de cada barra del marco
const float MARCO_GROSOR = 0.45f;  //ancho de cada barra del marco
const float MARCO_BORDE_INTERIOR = MARCO_CENTRO - MARCO_GROSOR / 2.0f; //d\u00f3nde empieza (hacia adentro) la franja gris del marco


void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};

	GLfloat cubo_vertices[] = {
		// front
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// back
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

// Pir�mide triangular regular
void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
			0,1,2,
			1,3,2,
			3,0,2,
			1,0,3

	};
	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,0.0f,	//0
		0.5f,-0.5f,0.0f,	//1
		0.0f,0.5f, -0.25f,	//2
		0.0f,-0.5f,-0.5f,	//3

	};
	Mesh* piramidet = new Mesh();
	piramidet->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(piramidet);

}


//funci�n para crear pir�mide cuadrangular unitaria
void CrearPiramideCuadrangular()
{
	unsigned int piramidecuadrangular_indices[] = {
		0,3,4,//frontal
		3,2,4,//izquierda
		2,1,4,//trasera
		1,0,4,//derecha
		0,1,2,//abajo1
		0,2,3//abajo2

	};
	GLfloat piramidecuadrangular_vertices[] = {
		0.5f,-0.5f,0.5f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,0.5f,
		0.0f,0.5f,0.0f,
	};
	Mesh* piramidec = new Mesh();
	piramidec->CreateMesh(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramidec);
}

//Tri�ngulo PLANO (sin altura, como una placa delgada) para llenar SOLO la
//esquina de la franja gris del marco de una cara, sin tocar el �rea azul
//central. Viene modelado ya para la esquina SUPERIOR-DERECHA de una cara
//unitaria (v�rtice exterior en (1,1)); para las otras 3 esquinas solo se rota
//90/180/270 grados sobre Z (ver ConstruirEsquinasCubo). Los otros dos v�rtices
//quedan en (b,1) y (1,b), con b = MARCO_BORDE_INTERIOR / MITAD_CUBO: as� los
//dos catetos corren pegados a los bordes de la cara y la hipotenusa queda
//dentro de la franja gris, sin llegar nunca al �rea azul del centro.
//IMPORTANTE: cada cara (tapa y lados) se agrega POR DUPLICADO, una vez con
//cada sentido de giro (0,1,2 y tambi�n 0,2,1). Esto hace que la pieza se vea
//s�lida sin importar si el proyecto tiene activado el "backface culling"; sin
//esto, la tapa frontal pod�a no dibujarse y solo se ve�an los lados delgados
//(por eso antes se ve�a como una raya en vez de un tri�ngulo relleno).
void CrearTrianguloEsquina()
{
	float b = MARCO_BORDE_INTERIOR / MITAD_CUBO;

	unsigned int trianguloesquina_indices[] = {
		0,1,2,  0,2,1,        //tapa frontal (los dos sentidos)
		3,5,4,  3,4,5,        //tapa trasera (los dos sentidos)
		0,1,4, 0,4,1, 0,4,3, 0,3,4, //lado pegado al borde "de arriba"
		1,2,5, 1,5,2, 1,5,4, 1,4,5, //hipotenusa (queda dentro de la franja gris)
		2,0,3, 2,3,0, 2,3,5, 2,5,3  //lado pegado al borde "derecho"
	};
	GLfloat trianguloesquina_vertices[] = {
		//tapa frontal (z=+0.5)
		1.0f, 1.0f, 0.5f,  //0: v�rtice real de la cara (esquina exterior)
		b,    1.0f, 0.5f,  //1: sobre el borde de arriba, en el l�mite interior del marco
		1.0f, b,    0.5f,  //2: sobre el borde derecho, en el l�mite interior del marco
		//tapa trasera (z=-0.5)
		1.0f, 1.0f, -0.5f, //3
		b,    1.0f, -0.5f, //4
		1.0f, b,    -0.5f  //5
	};
	Mesh* trianguloEsquina = new Mesh();
	trianguloEsquina->CreateMesh(trianguloesquina_vertices, trianguloesquina_indices, 18, 48);
	meshList.push_back(trianguloEsquina);
}

//Tri�ngulo PLANO "corta-esquina": se usa para tapar (en gris, color del marco)
//las 4 esquinas del cuadrado azul interior, dejando expuesto solo un ROMBO
//(diamante) en el centro de la cara, en vez de un cuadrado. Va modelado para
//la esquina SUPERIOR-DERECHA del cuadrado interior: v�rtice en (1,1) (la
//esquina de ese cuadrado) y los otros dos v�rtices EXACTAMENTE en (0,1) y
//(1,0) - es decir, en la mitad de cada lado - que es donde debe tocar la
//punta del rombo. Igual que el tri�ngulo de esquina, va por duplicado en
//ambos sentidos de giro para que se vea s�lido sin depender del culling.
void CrearTrianguloDiamante()
{
	unsigned int indices[] = {
		0,1,2,  0,2,1,
		3,5,4,  3,4,5,
		0,1,4, 0,4,1, 0,4,3, 0,3,4,
		1,2,5, 1,5,2, 1,5,4, 1,4,5,
		2,0,3, 2,3,0, 2,3,5, 2,5,3
	};
	GLfloat vertices[] = {
		//tapa frontal (z=+0.5)
		1.0f, 1.0f, 0.5f,  //0: esquina del cuadrado interior
		0.0f, 1.0f, 0.5f,  //1: mitad del lado de arriba (punta del rombo)
		1.0f, 0.0f, 0.5f,  //2: mitad del lado derecho (punta del rombo)
		//tapa trasera (z=-0.5)
		1.0f, 1.0f, -0.5f, //3
		0.0f, 1.0f, -0.5f, //4
		1.0f, 0.0f, -0.5f  //5
	};
	Mesh* trianguloDiamante = new Mesh();
	trianguloDiamante->CreateMesh(vertices, indices, 18, 48);
	meshList.push_back(trianguloDiamante);
}

//PENT\u00c1GONO plano: pieza de esquina que cubre TODA la esquina de una cara,
//desde el v\u00e9rtice real de la cara hasta las DOS puntas del rombo/diamante -
//sin dejar ning\u00fan hueco con el marco ni con el diamante. Reemplaza a la vez
//al tri\u00e1ngulo peque\u00f1o (que solo llegaba al marco) y a la pieza gris
//"corta-esquina" (que solo llegaba del marco al diamante): ahora es UNA sola
//pieza de color que llena todo ese espacio de una vez.
//V�rtices (para la esquina SUPERIOR-DERECHA), en sentido de recorrido:
//  V0=(b,0) -> V1=(1,0) -> V2=(1,1) -> V3=(0,1) -> V4=(0,b) -> (vuelve a V0)
//donde b = MARCO_BORDE_INTERIOR / MITAD_CUBO (el mismo valor usado para que
//el diamante toque exactamente ah� - ver ConstruirEsquinasDiamante). V0 y V4
//son las puntas del rombo; V1,V2,V3 son el borde real de la cara (V2 = la
//esquina). Como con las otras piezas planas, cada cara (tapas y lados) se
//agrega dos veces (con ambos sentidos de giro) para que se vea s�lida sin
//depender del culling.
void CrearEsquinaPentagono()
{
	float b = MARCO_BORDE_INTERIOR / MITAD_CUBO;

	unsigned int indices[] = {
		//tapa frontal (abanico desde V2), ambos sentidos
		2,3,4, 2,4,3,
		2,4,0, 2,0,4,
		2,0,1, 2,1,0,
		//tapa trasera (abanico desde V7=V2 trasero), ambos sentidos
		7,8,9, 7,9,8,
		7,9,5, 7,5,9,
		7,5,6, 7,6,5,
		//lado V0-V1
		0,1,6, 0,6,1, 0,6,5, 0,5,6,
		//lado V1-V2
		1,2,7, 1,7,2, 1,7,6, 1,6,7,
		//lado V2-V3
		2,3,8, 2,8,3, 2,8,7, 2,7,8,
		//lado V3-V4
		3,4,9, 3,9,4, 3,9,8, 3,8,9,
		//lado V4-V0
		4,0,5, 4,5,0, 4,5,9, 4,9,5
	};
	GLfloat vertices[] = {
		//tapa frontal (z=+0.5): V0..V4
		b,    0.0f, 0.5f,  //0
		1.0f, 0.0f, 0.5f,  //1
		1.0f, 1.0f, 0.5f,  //2: esquina real de la cara
		0.0f, 1.0f, 0.5f,  //3
		0.0f, b,    0.5f,  //4
		//tapa trasera (z=-0.5): V0..V4
		b,    0.0f, -0.5f, //5
		1.0f, 0.0f, -0.5f, //6
		1.0f, 1.0f, -0.5f, //7
		0.0f, 1.0f, -0.5f, //8
		0.0f, b,    -0.5f  //9
	};
	Mesh* esquinaPentagono = new Mesh();
	esquinaPentagono->CreateMesh(vertices, indices, 30, 96);
	meshList.push_back(esquinaPentagono);
}



/*
Crear cilindro, cono y esferas con arreglos din�micos vector creados en el Semestre 2023 - 1 : por S�nchez P�rez Omar Alejandro
*/
void CrearCilindro(int res, float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//c�lculo del paso interno en la circunferencia y variables que almacenar�n cada coordenada de cada v�rtice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//ciclo for para crear los v�rtices de las paredes del cilindro
	for (n = 0; n <= (res); n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		//caso para terminar el c�rculo
		else {
			x = R * cos((0) * dt);
			z = R * sin((0) * dt);
		}
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			case 3:
				vertices.push_back(x);
				break;
			case 4:
				vertices.push_back(0.5);
				break;
			case 5:
				vertices.push_back(z);
				break;
			}
		}
	}

	//ciclo for para crear la circunferencia inferior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(-0.5f);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}

	//ciclo for para crear la circunferencia superior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(0.5);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}

	//Se generan los indices de los v�rtices
	for (i = 0; i < vertices.size(); i++) indices.push_back(i);

	//se genera el mesh del cilindro
	Mesh* cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

//funci�n para crear un cono
void CrearCono(int res, float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//c�lculo del paso interno en la circunferencia y variables que almacenar�n cada coordenada de cada v�rtice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//caso inicial para crear el cono
	vertices.push_back(0.0);
	vertices.push_back(0.5);
	vertices.push_back(0.0);

	//ciclo for para crear los v�rtices de la circunferencia del cono
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}
	vertices.push_back(R * cos(0) * dt);
	vertices.push_back(-0.5);
	vertices.push_back(R * sin(0) * dt);


	for (i = 0; i < res + 2; i++) indices.push_back(i);

	//se genera el mesh del cono
	Mesh* cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono);
}


// ============================================================================
//  CONSTRUCCI�N DE LA FIGURA (marco + rombos + arcos + c�rculo + pir�mides)
// ============================================================================
//
// Estrategia: en vez de crear un Mesh nuevo por cada pieza (cientos de piezas),
// se reutiliza el MISMO mesh de cubo (y el de pir�mide cuadrangular) y se
// generan "instancias": cada instancia solo guarda posici�n, escala, dos
// rotaciones y color. En el render loop se recorre esta lista y, para cada
// instancia, se arma la matriz de modelo y se manda el color como uniform
// (igual que ya hac�a el c�digo original con glUniform3fv).
//
// NOTA IMPORTANTE: se asume que tu clase Mesh tiene un m�todo RenderMesh()
// (el que normalmente acompa�a a CreateMesh en este tipo de proyectos, junto
// a RenderMeshGeometry() que ya usabas para el cilindro/cono). Si tu clase
// se llama diferente, solo hay que cambiar esa l�nea en el render loop.

struct Instance
{
	int meshIndex;
	glm::mat4 transform; //transform final de la pieza (cara del cubo + posici�n + rotaci�n + escala)
	glm::vec3 color;
};

vector<Instance> figuraInstances;

void AddInstanceMatrix(int meshIndex, const glm::mat4& transform, glm::vec3 color)
{
	Instance inst;
	inst.meshIndex = meshIndex;
	inst.transform = transform;
	inst.color = color;
	figuraInstances.push_back(inst);
}

//Arma la matriz LOCAL de una pieza dentro del sistema de coordenadas de una cara:
//posPlano = (x, y, d), donde (x,y) es la posici�n dentro de la cara y d es qu� tanto
//sobresale la pieza hacia AFUERA del cubo (perpendicular a esa cara).
glm::mat4 PiezaLocal(glm::vec3 posPlano, glm::vec3 scale, float angleXDeg, float angleZDeg)
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, posPlano);
	m = glm::rotate(m, glm::radians(angleZDeg), glm::vec3(0.0f, 0.0f, 1.0f));
	m = glm::rotate(m, glm::radians(angleXDeg), glm::vec3(1.0f, 0.0f, 0.0f));
	m = glm::scale(m, scale);
	return m;
}

void AddInstanceFull(const glm::mat4& cara, int meshIndex, glm::vec3 posPlano, glm::vec3 scale,
	float angleXDeg, float angleZDeg, glm::vec3 color)
{
	AddInstanceMatrix(meshIndex, cara * PiezaLocal(posPlano, scale, angleXDeg, angleZDeg), color);
}

//versi�n corta para piezas que solo necesitan girar sobre Z (marco, rombos, arcos, c�rculo)
void AddInstance(const glm::mat4& cara, int meshIndex, glm::vec3 posPlano, glm::vec3 scale, float angleZDeg, glm::vec3 color)
{
	AddInstanceFull(cara, meshIndex, posPlano, scale, 0.0f, angleZDeg, color);
}

// ---- 0. Transformaciones de cara --------------------------------------------
// Cada cara del cubo se arma con el MISMO dise�o (marco, esquinas, rombos, etc.),
// dibujado en un plano local; esta funci�n calcula la matriz que "pega" ese plano
// a la cara correspondiente del cubo: primero lo gira para que su normal quede
// apuntando hacia la direcci�n de esa cara, y luego lo aleja MITAD_CUBO unidades
// del centro en esa misma direcci�n.
glm::mat4 TransformCara(glm::vec3 normal, float anguloYDeg, float anguloXDeg)
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, normal * MITAD_CUBO);
	m = glm::rotate(m, glm::radians(anguloYDeg), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::rotate(m, glm::radians(anguloXDeg), glm::vec3(1.0f, 0.0f, 0.0f));
	return m;
}

const glm::mat4 CARA_FRENTE = TransformCara(glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f);
const glm::mat4 CARA_ATRAS = TransformCara(glm::vec3(0.0f, 0.0f, -1.0f), 180.0f, 0.0f);
const glm::mat4 CARA_DERECHA = TransformCara(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.0f);
const glm::mat4 CARA_IZQUIERDA = TransformCara(glm::vec3(-1.0f, 0.0f, 0.0f), -90.0f, 0.0f);
const glm::mat4 CARA_ARRIBA = TransformCara(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, -90.0f);
const glm::mat4 CARA_ABAJO = TransformCara(glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, 90.0f);

// ---- 1. Marco cuadrado exterior (4 barras, gris carb�n) --------------------
void ConstruirMarcoExterior(const glm::mat4& cara)
{
	float d = 0.05f;
	AddInstance(cara, MESH_CUBO, glm::vec3(0.0f, MARCO_CENTRO, d), glm::vec3(3.60f, MARCO_GROSOR, 0.30f), 0.0f, COLOR_MARCO); //superior
	AddInstance(cara, MESH_CUBO, glm::vec3(0.0f, -MARCO_CENTRO, d), glm::vec3(3.60f, MARCO_GROSOR, 0.30f), 0.0f, COLOR_MARCO); //inferior
	AddInstance(cara, MESH_CUBO, glm::vec3(-MARCO_CENTRO, 0.0f, d), glm::vec3(MARCO_GROSOR, 3.60f, 0.30f), 0.0f, COLOR_MARCO); //izquierda
	AddInstance(cara, MESH_CUBO, glm::vec3(MARCO_CENTRO, 0.0f, d), glm::vec3(MARCO_GROSOR, 3.60f, 0.30f), 0.0f, COLOR_MARCO); //derecha
}

// ---- 2. Esquinas de CADA CARA: un PENT\u00c1GONO por esquina -------------------
// Cubre TODA la esquina de la cara: desde el v\u00e9rtice real de la cara hasta
// las dos puntas del rombo/diamante central, sin dejar hueco con el marco ni
// con el diamante (ver CrearEsquinaPentagono). El marco gris solo se sigue
// viendo en la parte recta de cada borde (donde ning\u00fan pent\u00e1gono lo tapa).
// Al construirse POR CARA (y ya no compartida entre las 3 caras de un mismo
// v�rtice del cubo), cada cara puede mostrar sus 4 colores exactos en el
// orden correcto, sin el conflicto de colores que ten�amos con el dise�o
// anterior (pir�mides compartidas entre caras vecinas).
void ConstruirEsquinasCubo(const glm::mat4& cara)
{
	//IMPORTANTE: esta pieza comparte espacio (x,y) con el marco gris
	//(ConstruirMarcoExterior, que usa d=0.05f). Si ambos quedan a la MISMA
	//profundidad, se produce "z-fighting" (la tarjeta gr\u00e1fica no puede decidir
	//cu\u00e1l dibujar encima). Por eso aqu\u00ed se usa una "d" claramente MAYOR (m\u00e1s
	//hacia afuera) que la del marco, para que esta pieza gane siempre y tape
	//por completo al marco en la esquina.
	float d = 0.12f;
	float grosor = 0.20f;    //espesor de la pieza (casi plana)

	//La malla MESH_ESQUINA_PENTAGONO ya viene modelada para la esquina
	//SUPERIOR-DERECHA; para las otras 3 esquinas solo se rota sobre Z.
	AddInstance(cara, MESH_ESQUINA_PENTAGONO, glm::vec3(0.0f, 0.0f, d), glm::vec3(MITAD_CUBO, MITAD_CUBO, grosor), 0.0f, COLOR_ROJO);      //arriba-derecha
	AddInstance(cara, MESH_ESQUINA_PENTAGONO, glm::vec3(0.0f, 0.0f, d), glm::vec3(MITAD_CUBO, MITAD_CUBO, grosor), 90.0f, COLOR_AMARILLO);  //arriba-izquierda
	AddInstance(cara, MESH_ESQUINA_PENTAGONO, glm::vec3(0.0f, 0.0f, d), glm::vec3(MITAD_CUBO, MITAD_CUBO, grosor), 180.0f, COLOR_MORADO);   //abajo-izquierda
	AddInstance(cara, MESH_ESQUINA_PENTAGONO, glm::vec3(0.0f, 0.0f, d), glm::vec3(MITAD_CUBO, MITAD_CUBO, grosor), 270.0f, COLOR_VERDE);    //abajo-derecha
}



// ---- 2b. "Corta-esquinas" del cuadrado azul interior: 4 piezas GRISES que
// tapan las esquinas del cuadrado que forma la cara s�lida del cubo, dejando
// expuesto solo un ROMBO (diamante) en vez de un cuadrado - las puntas del
// rombo quedan justo en la mitad de cada lado del cuadrado interior (que es
// el borde interior del marco), igual que en la imagen de referencia. Esto no
// cambia los tri�ngulos de colores (siguen solo en la franja gris del marco).
void ConstruirEsquinasDiamante(const glm::mat4& cara)
{
	float d = 0.03f;      //apenas separado de la superficie del cubo
	float grosor = 0.10f; //espesor de la pieza (casi plana)
	float lado = MARCO_BORDE_INTERIOR; //tama�o del cuadrado interior a "recortar"

	AddInstance(cara, MESH_TRIANGULO_DIAMANTE, glm::vec3(0.0f, 0.0f, d), glm::vec3(lado, lado, grosor), 0.0f, COLOR_MARCO);
	AddInstance(cara, MESH_TRIANGULO_DIAMANTE, glm::vec3(0.0f, 0.0f, d), glm::vec3(lado, lado, grosor), 90.0f, COLOR_MARCO);
	AddInstance(cara, MESH_TRIANGULO_DIAMANTE, glm::vec3(0.0f, 0.0f, d), glm::vec3(lado, lado, grosor), 180.0f, COLOR_MARCO);
	AddInstance(cara, MESH_TRIANGULO_DIAMANTE, glm::vec3(0.0f, 0.0f, d), glm::vec3(lado, lado, grosor), 270.0f, COLOR_MARCO);
}

// ---- 3. Rombo (cuadrado girado), formado por 4 barras diagonales -----------
// r: distancia del centro a cada v�rtice (arriba/derecha/abajo/izquierda)
void ConstruirRombo(const glm::mat4& cara, float r, float grosor, float profundidad, float d, glm::vec3 color)
{
	float longitud = r * sqrt(2.0f) * 1.08f; //peque�o solape en las esquinas

	AddInstance(cara, MESH_CUBO, glm::vec3(r / 2.0f, r / 2.0f, d), glm::vec3(grosor, longitud, profundidad), -45.0f, color);   //lado sup-der "\"
	AddInstance(cara, MESH_CUBO, glm::vec3(r / 2.0f, -r / 2.0f, d), glm::vec3(grosor, longitud, profundidad), 45.0f, color);   //lado inf-der "/"
	AddInstance(cara, MESH_CUBO, glm::vec3(-r / 2.0f, -r / 2.0f, d), glm::vec3(grosor, longitud, profundidad), -45.0f, color); //lado inf-izq "\"
	AddInstance(cara, MESH_CUBO, glm::vec3(-r / 2.0f, r / 2.0f, d), glm::vec3(grosor, longitud, profundidad), 45.0f, color);   //lado sup-izq "/"
}

void ConstruirRomboGrande(const glm::mat4& cara)
{
	ConstruirRombo(cara, RADIO_ROMBO_GRANDE, 0.22f, 0.26f, 0.14f, COLOR_AZUL);
}

void ConstruirRomboInterior(const glm::mat4& cara)
{
	ConstruirRombo(cara, 0.75f, 0.18f, 0.22f, 0.20f, COLOR_AZUL); //60% del rombo grande
}

// ---- 4. C�rculo central (aro), aproximado con un pol�gono de N barras ------
void ConstruirCirculoCentral(const glm::mat4& cara, float radio, float d, int segmentos, glm::vec3 color)
{
	float paso = 360.0f / segmentos;
	for (int i = 0; i < segmentos; i++)
	{
		float aMedioDeg = i * paso + paso / 2.0f;
		float aMedioRad = glm::radians(aMedioDeg);

		float x = radio * cos(aMedioRad);
		float y = radio * sin(aMedioRad);

		float longitud = 2.0f * radio * sin(glm::radians(paso / 2.0f)) * 1.25f;
		AddInstance(cara, MESH_CUBO, glm::vec3(x, y, d), glm::vec3(0.10f, longitud, 0.16f), aMedioDeg, color);
	}
}

// ---- 5. Elemento central: peque�o rombo s�lido verde oliva -----------------
void ConstruirElementoCentral(const glm::mat4& cara)
{
	AddInstance(cara, MESH_CUBO, glm::vec3(0.0f, 0.0f, 0.34f), glm::vec3(0.42f, 0.42f, 0.22f), 45.0f, COLOR_OLIVA);
}

// ---- 6. Arma el panel completo (esquinas + marco + corta-esquinas + rombos +
// c�rculo + centro) sobre UNA cara; se llama una vez por cada una de las 6
// caras del cubo. --------------------------------------------------------
void ConstruirPanelEnCara(const glm::mat4& cara)
{
	ConstruirEsquinasCubo(cara);
	ConstruirMarcoExterior(cara);
	//ConstruirEsquinasDiamante YA NO se llama: el pent\u00e1gono de
	//ConstruirEsquinasCubo ahora cubre toda la esquina (desde el v\u00e9rtice real
	//de la cara hasta las puntas del rombo), as\u00ed que esta pieza gris quedar\u00eda
	//completamente tapada de todas formas.
	//ConstruirRomboGrande/ConstruirRomboInterior YA NO se llaman: sus barras
	//estaban mal orientadas (apuntaban hacia las esquinas en vez de seguir el
	//borde real del diamante) y, ahora que las esquinas del cuadrado interior
	//son grises, esas barras azules de m�s se ve�an como l�neas saliendo del
	//rombo. La forma del diamante ya la da ConstruirEsquinasDiamante, as� que
	//no hac�an falta.
	ConstruirCirculoCentral(cara, 0.42f, 0.28f, 16, COLOR_MARCO);
	ConstruirElementoCentral(cara);
}

// (Antes hab�a aqu� una funci�n ConstruirBaseInferior() que agregaba una
// "estructura de soporte" extra debajo del cubo. Se elimin� porque haca que la
// cara de abajo se viera distinta a las otras 5: ahora las 6 caras usan
// exactamente el mismo panel, sin nada adicional en ninguna de ellas.)

// ---- Funci�n maestra: arma toda la figura ----------------------------------
void ConstruirFiguraCompleta()
{
	//Cubo base s�lido (mismo tama�o en las 3 dimensiones)
	AddInstance(glm::mat4(1.0f), MESH_CUBO, glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(CUBO_LADO, CUBO_LADO, CUBO_LADO), 0.0f, COLOR_AZUL);

	//El mismo panel decorado (esquinas + marco + rombos + c�rculo + centro) se
	//repite en las 6 caras del cubo
	ConstruirPanelEnCara(CARA_FRENTE);
	ConstruirPanelEnCara(CARA_ATRAS);
	ConstruirPanelEnCara(CARA_DERECHA);
	ConstruirPanelEnCara(CARA_IZQUIERDA);
	ConstruirPanelEnCara(CARA_ARRIBA);
	ConstruirPanelEnCara(CARA_ABAJO);
}
// ============================================================================


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	Shader* shader2 = new Shader();
	shader2->CreateFromFiles(vShaderColor, fShader);
	shaderList.push_back(*shader2);
}


int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();
	//Cilindro y cono reciben resoluci�n (slices, rebanadas) y Radio de circunferencia de la base y tapa

	CrearCubo();//�ndice 0 en MeshList (MESH_CUBO)
	CrearPiramideTriangular();//�ndice 1 en MeshList
	CrearCilindro(10, 1.0f);//�ndice 2 en MeshList
	CrearCono(25, 2.0f);//�ndice 3 en MeshList
	CrearPiramideCuadrangular();//�ndice 4 en MeshList (MESH_PIRAMIDE_CUAD)
	CrearTrianguloEsquina();//�ndice 5 en MeshList (MESH_TRIANGULO_ESQUINA)
	CrearTrianguloDiamante();//�ndice 6 en MeshList (MESH_TRIANGULO_DIAMANTE)
	CrearEsquinaPentagono();//�ndice 7 en MeshList (MESH_ESQUINA_PENTAGONO)
	CreateShaders();

	//Se arma la lista de instancias de la figura (reutiliza el cubo y la pir�mide ya creados)
	ConstruirFiguraCompleta();

	/*C�mara se usa el comando: glm::lookAt(vector de posici�n, vector de orientaci�n, vector up));
	En la clase Camera se reciben 5 datos:
	glm::vec3 vector de posici�n,
	glm::vec3 vector up,
	GlFloat yaw rotaci�n para girar hacia la derecha e izquierda
	GlFloat pitch rotaci�n para inclinar hacia arriba y abajo
	GlFloat velocidad de desplazamiento,
	GlFloat velocidad de vuelta o de giro
	Se usa el Mouse y las teclas WASD y su posici�n inicial est� en 0,0,1 y ve hacia 0,0,-1.
	*/

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.3f);


	GLuint uniformProjection = 0;
	GLuint uniformModel = 0;
	GLuint uniformView = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
	//glm::mat4 projection = glm::ortho(-1, 1, -1, 1, 1, 10);

	//Loop mientras no se cierra la ventana
	sp.init(); //inicializar esfera
	sp.load();//enviar la esfera al shader

	glm::mat4 model(1.0);//Inicializar matriz de Modelo 4x4

	while (!mainWindow.getShouldClose())
	{

		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;
		//Recibir eventos del usuario
		glfwPollEvents();
		//C�mara
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//Limpiar la ventana
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Se agrega limpiar el buffer de profundidad
		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		//Proyecci�n y vista son iguales para todas las piezas, se mandan una sola vez por frame
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		//Se dibuja cada instancia de la figura completa
		for (size_t idx = 0; idx < figuraInstances.size(); idx++)
		{
			Instance& inst = figuraInstances[idx];

			model = glm::mat4(1.0f);
			//Traslaci�n inicial para alejar toda la figura de la c�mara (el panel es m�s grande que un cubo unitario)
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -9.5f));
			//rotaciones globales controladas con teclado (E, R, T -> ejes X, Y, Z)
			model = glm::rotate(model, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f));

			//transformaci�n propia de cada pieza: ya incluye la cara del cubo a la que
			//pertenece (frente/atr�s/arriba/abajo/izq/der), su posici�n dentro de esa
			//cara, su rotaci�n y su escala (ver AddInstance/AddInstanceFull/TransformCara)
			model = model * inst.transform;

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniform3fv(uniformColor, 1, glm::value_ptr(inst.color));

			//NOTA: se asume que Mesh tiene un m�todo RenderMesh() (el que corresponde a
			//CreateMesh, tal como RenderMeshGeometry() corresponde a CreateMeshGeometry).
			meshList[inst.meshIndex]->RenderMesh();
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}