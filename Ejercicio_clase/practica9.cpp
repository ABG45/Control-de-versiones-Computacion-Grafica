/*
Práctica 9: Animación básica y texturas animadas (sobre la escena de las prácticas 7 y 8)
Modelos en escena: sólo el rover (con jerarquía, Models/<pieza>.obj) y el avión de Rochelle (Models/avion_rochelle.obj).

Animación (práctica 9):
- Rover (animación básica, por bandera y condicionales): ciclo infinito de orilla a orilla del piso.
  Avanza sobre su eje X (su frente) hasta la orilla +X del piso, la bandera "avanza" cambia y retrocede
  hasta la orilla -X, y así sucesivamente. Cambian la traslación del cuerpo y la rotación de las 6 llantas.
- Avión (animación compleja, por funciones): vuela un circuito cerrado tipo pista de carreras
  (2 rectas + 2 curvas). Siempre va hacia adelante: su orientación sale de la tangente del camino.
  * Recta 1: se eleva (de alturaBaja a alturaAlta).
  * Curva 1: vuelta curva arriba, inclinándose (alabeo) hacia el centro de la curva.
  * Recta 2: baja su altura y luego hace 2 barrel rolls (giro de 720° sobre su eje, como los Arwing de Star Fox).
  * Curva 2: vuelta curva abajo y vuelve a empezar.
- Q pausa/reanuda las dos animaciones.
- Textura animada: el vertex shader suma "toffset" a las coordenadas de textura (TexCoord = tex + toffset).
  * Flecha: su textura se desplaza en U cada frame.
  * Números: un solo quad con 1/4 x 1/3 de la textura "numerosbase.tga"; el offset elige qué número se ve.
  * Número cambiante: avanza al siguiente número cada 0.5 s (velocidad visible, se mide con el tiempo y no por frame).
  * Cartel que alterna automáticamente entre Textures/numero1.tga y Textures/numero2.tga cada segundo.

Iluminación (prácticas 7 y 8) que se conserva:
- spotLights[2]: faro frontal del rover (va con él). spotLights[3] y [4]: faros del avión (abajo y al frente).
- F: el faro del avión hacia abajo cambia de color. P: prende/apaga la luz verde de la pinza del rover (pointLights[1]).
Teclas: Z/X, C/V, B/N, M/, mueven el brazo del rover y R/T sus ruedas.
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
//#include "Sphere.h"  // no se usa en esta práctica y el archivo no existe en el proyecto
#include"Model.h"
#include "Skybox.h"
#include "jerarquia.h" //piezas del rover: archivo, padre, pivote y eje de giro

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<MeshModel*> meshListModel; // recibe xyz uv nx ny nz
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;   //práctica 9: textura animada de flechas
Texture NumerosTexture;  //práctica 9: todos los números en una textura (4 columnas x 3 filas)
Texture Numero1Texture;  //práctica 9: cartel que alterna entre número 1...
Texture Numero2Texture;  //...y número 2

//variables para animación (práctica 9)
bool avanza = true;                 //bandera: true = el rover avanza (+X), false = retrocede
float movCoche = 1.0f;              //posición X animada del rover (empieza en x = 1)
const float movOffset = 0.2f;       //velocidad del rover (unidades por frame a 60 fps): ~12 unidades/s
//Orillas del piso: el plano mide 20 x 20 y se escala 30 veces -> va de -300 a 300 en X y Z.
//El rover (escala 0.4) mide de -6.61 a +6.96 en su X local, así que se le resta su medio largo
//para que su frente (o su parte trasera) se detenga justo en la orilla.
const float mitadPiso = 10.0f * 30.0f;
const float limiteFrenteRover = mitadPiso - 6.96f * 0.4f; //orilla +X
const float limiteAtrasRover = -mitadPiso + 6.61f * 0.4f; //orilla -X
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0f;
float tiempoNumero = 0.0f;          //tiempo del último cambio del número cambiante


//Rover con jerarquía: un Model por pieza, en el mismo orden que kPartesRover (jerarquia.h)
Model RoverPartes[kNumPartesRover];
float anguloRover[kNumPartesRover] = { 0.0f }; //ángulo actual de cada articulación (grados)
glm::vec3 posRover(0.0f, -1.0f, 6.0f); //posición base del rover; la animación le suma movCoche en X (su frente)
float giroLlantas = 0.0f;              //giro de las llantas al avanzar (grados)

//=========== Avión (práctica 9): circuito cerrado con subida, bajada, vueltas curvas y barrel rolls ===========
//Pista vista desde arriba (centro en el origen): dos rectas de largo pistaL unidas por dos semicírculos de radio pistaR.
//  recta 1: z = -pistaR, de x = -L/2 a +L/2 (hacia +X)  -> se eleva
//  curva 1: centro (+L/2, 0)                             -> vuelta curva arriba
//  recta 2: z = +pistaR, de x = +L/2 a -L/2 (hacia -X)  -> baja y hace 2 barrel rolls
//  curva 2: centro (-L/2, 0)                             -> vuelta curva abajo
const float pistaL = 40.0f;
const float pistaR = 15.0f;
const float alturaBaja = 4.0f;          //el piso está en y = -1
const float alturaAlta = 12.0f;
const float velAvion = 0.15f;           //unidades por frame a 60 fps (~9 unidades/s, ~19 s por vuelta)
const float escalaAvion = 0.6f;
float distAvion = 0.0f;                 //distancia recorrida sobre la pista (el avión sólo avanza: siempre crece)
float alabeoAvion = 0.0f;               //inclinación lateral actual en las curvas (grados)
glm::mat4 matAvion(1.0f);               //matriz del avión de este frame (para dibujarlo y para sus faros)

float PerimetroPista() { return 2.0f * pistaL + 2.0f * 3.14159265f * pistaR; }

//Posición del avión a una distancia d sobre la pista (la altura depende del tramo)
glm::vec3 PuntoPista(float d)
{
	const float P = PerimetroPista();
	const float largoCurva = 3.14159265f * pistaR;
	d = fmodf(d, P);
	if (d < 0.0f) d += P;
	float x, y, z;
	if (d < pistaL) //recta 1: se eleva (smoothstep: empieza y termina suave, sin picos)
	{
		float u = d / pistaL;
		x = -pistaL / 2.0f + d;
		z = -pistaR;
		y = glm::mix(alturaBaja, alturaAlta, glm::smoothstep(0.1f, 0.9f, u));
	}
	else if (d < pistaL + largoCurva) //curva 1: arriba
	{
		float phi = (d - pistaL) / pistaR;
		x = pistaL / 2.0f + pistaR * sinf(phi);
		z = -pistaR * cosf(phi);
		y = alturaAlta;
	}
	else if (d < 2.0f * pistaL + largoCurva) //recta 2: baja en la primera mitad
	{
		float u = (d - pistaL - largoCurva) / pistaL;
		x = pistaL / 2.0f - u * pistaL;
		z = pistaR;
		y = glm::mix(alturaAlta, alturaBaja, glm::smoothstep(0.0f, 0.45f, u));
	}
	else //curva 2: abajo
	{
		float phi = (d - 2.0f * pistaL - largoCurva) / pistaR;
		x = -pistaL / 2.0f - pistaR * sinf(phi);
		z = pistaR * cosf(phi);
		y = alturaBaja;
	}
	return glm::vec3(x, y, z);
}

//Barrel roll: en la segunda mitad de la recta 2 el avión gira 720° (2 vueltas) sobre su eje de vuelo.
//Termina en 720° = 0°, así que al salir del giro queda derecho sin saltos.
float GiroBarril(float d)
{
	const float P = PerimetroPista();
	const float largoCurva = 3.14159265f * pistaR;
	d = fmodf(d, P);
	if (d < 0.0f) d += P;
	float inicio = pistaL + largoCurva;
	if (d < inicio || d > inicio + pistaL) return 0.0f;
	float u = (d - inicio) / pistaL;
	return 720.0f * glm::smoothstep(0.5f, 0.95f, u);
}

//Calcula la matriz del avión: posición sobre la pista, nariz (+Z del modelo) hacia donde avanza,
//alabeo hacia el centro de las curvas y el barrel roll.
void ActualizarAvion(float dt)
{
	distAvion += velAvion * dt;
	distAvion = fmodf(distAvion, PerimetroPista()); //para que la variable no crezca sin límite

	glm::vec3 pos = PuntoPista(distAvion);
	//dirección de avance = tangente del camino (diferencia entre un punto adelante y uno atrás).
	//Incluye la componente vertical, así que el avión se inclina solo hacia arriba al subir y hacia abajo al bajar.
	glm::vec3 adelante = glm::normalize(PuntoPista(distAvion + 0.5f) - PuntoPista(distAvion - 0.5f));

	//¿cuánto está girando? cambio de rumbo entre un poco atrás y un poco adelante (producto cruz, componente Y)
	glm::vec3 f1 = glm::normalize(PuntoPista(distAvion - 0.5f) - PuntoPista(distAvion - 1.5f));
	glm::vec3 f2 = glm::normalize(PuntoPista(distAvion + 1.5f) - PuntoPista(distAvion + 0.5f));
	float giro = glm::cross(f1, f2).y;
	float alabeoObjetivo = glm::clamp(-giro * 600.0f, -45.0f, 45.0f);
	//se acerca poco a poco al objetivo: el avión entra y sale de las curvas inclinándose suave
	alabeoAvion += (alabeoObjetivo - alabeoAvion) * glm::min(1.0f, 0.05f * dt);

	//base del avión: Z = adelante, X = ala, Y = arriba
	glm::vec3 ejeX = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), adelante));
	glm::vec3 ejeY = glm::cross(adelante, ejeX);
	glm::mat4 m(1.0f);
	m[0] = glm::vec4(ejeX, 0.0f);
	m[1] = glm::vec4(ejeY, 0.0f);
	m[2] = glm::vec4(adelante, 0.0f);
	m[3] = glm::vec4(pos, 1.0f);
	//alabeo de la curva + barrel roll: los dos son giros sobre el eje de vuelo (Z del modelo)
	m = glm::rotate(m, (alabeoAvion + GiroBarril(distAvion)) * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	m = glm::scale(m, glm::vec3(escalaAvion, escalaAvion, escalaAvion));
	matAvion = m;
}

//colores del faro del avión hacia abajo; la tecla F avanza al siguiente (ver Window.cpp)
const glm::vec3 coloresFaroAvion[6] = {
	glm::vec3(1.0f, 0.0f, 0.0f), //rojo
	glm::vec3(1.0f, 1.0f, 0.0f), //amarillo
	glm::vec3(0.0f, 1.0f, 0.0f), //verde
	glm::vec3(0.0f, 1.0f, 1.0f), //cian
	glm::vec3(0.0f, 0.0f, 1.0f), //azul
	glm::vec3(1.0f, 0.0f, 1.0f), //magenta
};
const int kPinza = 4; //índice de "Brazo_Pinza" en kPartesRover (jerarquia.h)
//punta de la pinza en coordenadas locales de Brazo_Pinza.obj (su vértice más alto está en y = 1.86);
//se deja un poco afuera de la malla para que la luz no quede "enterrada" en la pieza
const glm::vec3 puntaPinzaLocal(0.26f, 1.95f, -0.07f);

//Matrices de los modelos que llevan faro: se usan para dibujarlos Y para colocar sus luces,
//por eso la luz siempre queda pegada al modelo.
glm::mat4 MatrizRover()
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, posRover + glm::vec3(movCoche, 0.0f, 0.0f)); //posición base + animación (práctica 9)
	m = glm::scale(m, glm::vec3(0.4f, 0.4f, 0.4f));
	return m;
}
glm::mat4 MatrizAvion() //se calcula una vez por frame en ActualizarAvion()
{
	return matAvion;
}
//punto y dirección en coordenadas del modelo -> mundo
glm::vec3 PuntoMundo(const glm::mat4& m, glm::vec3 p) { return glm::vec3(m * glm::vec4(p, 1.0f)); }
glm::vec3 DirMundo(const glm::mat4& m, glm::vec3 d) { return glm::normalize(glm::vec3(m * glm::vec4(d, 0.0f))); }
//Modelo de las prácticas anteriores
Model Avion_M;    //avión de Rochelle (práctica 6)


Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


//función de calculo de normales por promedio de vértices 
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};
	
	//Práctica 9: quads para las texturas animadas (normal hacia -Y, convención del curso)
	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,
	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,
	};

	//un solo número: 1/4 del ancho (0.25) y 1/3 del alto (0.67 a 1.0) de numerosbase.tga
	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,
	};

	// las normales se calculan ANTES de mandar los vértices a la GPU
	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);
	//calcAverageNormals da la normal hacia afuera; el shader del curso la espera invertida
	for (int i = 5; i < 64; i += 8) { vegetacionVertices[i] *= -1.0f; vegetacionVertices[i + 1] *= -1.0f; vegetacionVertices[i + 2] *= -1.0f; }
	for (int i = 5; i < 32; i += 8) { vertices[i] *= -1.0f; vertices[i + 1] *= -1.0f; vertices[i + 2] *= -1.0f; }

	MeshModel *obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel *obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel *obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);

	MeshModel *obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(obj4);

	MeshModel* obj5 = new MeshModel();
	obj5->CreateMeshModel(flechaVertices, flechaIndices, 32, 6);
	meshListModel.push_back(obj5); //meshListModel[4]: flecha

	MeshModel* obj6 = new MeshModel();
	obj6->CreateMeshModel(scoreVertices, scoreIndices, 32, 6);
	meshListModel.push_back(obj6); //meshListModel[5]: todos los números

	MeshModel* obj7 = new MeshModel();
	obj7->CreateMeshModel(numeroVertices, numeroIndices, 32, 6);
	meshListModel.push_back(obj7); //meshListModel[6]: solo un número


}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();

	
	//Rover: se carga cada pieza por separado (Models/<pieza>.obj)
	for (int i = 0; i < kNumPartesRover; i++)
	{
		RoverPartes[i].LoadModel(std::string("Models/") + kPartesRover[i].archivo);
	}
	
	//Avión de la práctica 6
	Avion_M.LoadModel("Models/avion_rochelle.obj");
	ActualizarAvion(0.0f); //matriz inicial del avión (inicio de la recta 1)

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		-6.0f, 1.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	//luz verde en la punta de la pinza del rover. Su posición se calcula cada frame con la matriz de la pinza.
	pointLights[1] = PointLight(0.0f, 1.0f, 0.0f,
		0.3f, 1.5f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.1f, 0.05f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	//luz fija
	spotLights[1] = SpotLight(0.0f, 1.0f, 0.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;
	
	//faro frontal del rover (blanco cálido). Posición y dirección se actualizan cada frame.
	spotLights[2] = SpotLight(1.0f, 1.0f, 0.8f,
		0.5f, 2.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.05f, 0.01f,
		20.0f);
	spotLightCount++;

	//faro del avión hacia abajo: empieza en rojo y la tecla F cambia su color (ver el ciclo principal)
	spotLights[3] = SpotLight(1.0f, 0.0f, 0.0f,
		0.5f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.05f, 0.01f,
		25.0f);
	spotLightCount++;

	//faro del avión hacia adelante (amarillo)
	spotLights[4] = SpotLight(1.0f, 0.9f, 0.4f,
		0.5f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.05f, 0.01f,
		15.0f);
	spotLightCount++;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f); //offset de textura que se manda al shader (práctica 9)
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Animación básica del rover (práctica 9): por bandera y condicionales, en ciclo infinito.
		//Avanza hasta la orilla +X del piso, cambia la bandera y retrocede hasta la orilla -X; luego vuelve a avanzar.
		//Las llantas giran con el desplazamiento (más abajo, con dxAnim).
		float dxAnim = 0.0f;
		if (mainWindow.getAnimRover())
		{
			if (avanza)
			{
				dxAnim = movOffset * deltaTime;
				if (movCoche + dxAnim >= limiteFrenteRover) //llegó a la orilla: se queda en ella y cambia de sentido
				{
					dxAnim = limiteFrenteRover - movCoche;
					avanza = false;
				}
			}
			else
			{
				dxAnim = -movOffset * deltaTime;
				if (movCoche + dxAnim <= limiteAtrasRover)
				{
					dxAnim = limiteAtrasRover - movCoche;
					avanza = true;
				}
			}
			movCoche += dxAnim;

			//Animación compleja del avión: recorre su circuito (sube, vuelta, baja + barrel rolls, vuelta)
			ActualizarAvion(deltaTime);
		}

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation(); //para la textura con movimiento (práctica 9)

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		//sirve para que en tiempo de ejecución (dentro del while) se cambien propiedades de la luz
		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());


		//las llantas ruedan con el desplazamiento del rover: ángulo = distancia / radio (radio 1.24 * escala 0.4).
		//Al retroceder dxAnim es negativo y las llantas giran al revés.
		giroLlantas -= (dxAnim / (1.24f * 0.4f)) / toRadians;

		//Rover con jerarquía: aquí sólo se CALCULAN las matrices de las piezas (se dibujan más abajo).
		//Se hace antes de mandar las luces al shader porque la luz de la pinza necesita matRover[kPinza].
		//Teclas: Z/X base del brazo, C/V segmento 1, B/N segmento 2, M/, pinza, R/T ruedas
		glm::mat4 matRover[kNumPartesRover];
		{
			bool* teclas = mainWindow.getsKeys();
			const float vel = 1.0f * deltaTime;
			struct { int parte; int mas; int menos; } controles[] = {
				{ 1, GLFW_KEY_Z, GLFW_KEY_X },
				{ 2, GLFW_KEY_C, GLFW_KEY_V },
				{ 3, GLFW_KEY_B, GLFW_KEY_N },
				{ 4, GLFW_KEY_M, GLFW_KEY_COMMA },
			};
			for (auto& ctl : controles)
			{
				if (teclas[ctl.mas])   anguloRover[ctl.parte] += vel;
				if (teclas[ctl.menos]) anguloRover[ctl.parte] -= vel;
			}
			for (int i = 0; i < kNumPartesRover; i++)
			{
				//las ruedas (nombre "Rueda_...") giran todas juntas con R/T
				if (kPartesRover[i].nombre[0] == 'R')
				{
					if (teclas[GLFW_KEY_R]) anguloRover[i] += vel;
					if (teclas[GLFW_KEY_T]) anguloRover[i] -= vel;
				}
				else //las ruedas dan vueltas completas; el resto respeta su límite de jerarquia.h
					anguloRover[i] = glm::clamp(anguloRover[i], kPartesRover[i].minGrados, kPartesRover[i].maxGrados);
			}

			glm::mat4 roverBase = MatrizRover(); //la misma matriz que usa el faro

			//matriz de cada pieza = matriz del padre * traslación al pivote * rotación en su eje
			//(el padre siempre aparece antes que el hijo en kPartesRover)
			for (int i = 0; i < kNumPartesRover; i++)
			{
				const ParteRover& p = kPartesRover[i];
				glm::mat4 m = (p.padre < 0) ? roverBase : matRover[p.padre];
				m = glm::translate(m, glm::vec3(p.offset[0], p.offset[1], p.offset[2]));
				float ang = anguloRover[i];
				if (p.nombre[0] == 'R') ang += giroLlantas; //las llantas también giran al avanzar
				if (p.eje == 'X') m = glm::rotate(m, ang * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
				if (p.eje == 'Y') m = glm::rotate(m, ang * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
				if (p.eje == 'Z') m = glm::rotate(m, ang * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
				matRover[i] = m;
			}
		}

		//faros: se colocan con la matriz del modelo en cada frame, por eso siguen al rover y al avión
		{
			glm::mat4 mRover = MatrizRover();
			//frente del cuerpo del rover (pivote del cuerpo + medio largo), apuntando al frente y un poco al piso
			spotLights[2].SetFlash(PuntoMundo(mRover, glm::vec3(5.6f, 5.4f, 0.0f)),
				DirMundo(mRover, glm::vec3(1.0f, -0.3f, 0.0f)));

			glm::mat4 mAvion = MatrizAvion();
			spotLights[3].SetFlash(PuntoMundo(mAvion, glm::vec3(0.0f, -0.9f, 0.5f)), //panza del avión
				DirMundo(mAvion, glm::vec3(0.0f, -1.0f, 0.0f)));
			spotLights[4].SetFlash(PuntoMundo(mAvion, glm::vec3(0.0f, 0.0f, 3.7f)),  //nariz del avión
				DirMundo(mAvion, glm::vec3(0.0f, -0.1f, 1.0f)));

			//color del faro hacia abajo según las veces que se ha presionado F
			spotLights[3].SetColor(coloresFaroAvion[mainWindow.getColorFaroAvion()]);

			//luz de la pinza: punta local de la pinza llevada al mundo con la matriz jerárquica de la pinza,
			//así sigue al rover y a cada giro del brazo (Z/X, C/V, B/N, M/,)
			pointLights[1].SetPos(PuntoMundo(matRover[kPinza], puntaPinzaLocal));
		}

		//luces puntuales que se mandan al shader: la [0] siempre y la de la pinza sólo si está encendida (P).
		//Apagar = no mandarla (el shader sólo recorre las primeras "count" luces del arreglo).
		PointLight lucesActivas[MAX_POINT_LIGHTS];
		unsigned int lucesActivasCount = 0;
		lucesActivas[lucesActivasCount++] = pointLights[0];
		if (mainWindow.getLuzPinza())    lucesActivas[lucesActivasCount++] = pointLights[1];

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(lucesActivas, lucesActivasCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);



		glm::mat4 model(1.0);
		glm::mat4 modelaux(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		//el offset de textura se reinicia cada frame: el piso, el rover y el avión NO llevan textura animada
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[2]->RenderMeshModel();
	
		//Rover con jerarquía: se dibuja con las matrices matRover calculadas al inicio del frame
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		plainTexture.UseTexture();
		for (int i = 0; i < kNumPartesRover; i++)
		{
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matRover[i]));
			RoverPartes[i].RenderModel(uniformColor); //usa el color Kd de rover.mtl
		}
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);


		//Avión de Rochelle (práctica 6)
		model = MatrizAvion(); //la misma matriz que usan sus dos faros
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Avion_M.RenderModel(uniformColor);

		//Agave ¿qué sucede si lo renderizan antes que los demás modelos?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[3]->RenderMeshModel();
		glDisable(GL_BLEND);

		//================= Práctica 9: texturas animadas =================
		//(sigue activo el blending del Agave para respetar la transparencia de las texturas .tga)
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//textura con movimiento: la uniform no se puede modificar directamente, se actualiza la variable y se manda
		toffsetflechau += 0.001f * deltaTime;
		toffsetflechav = 0.0f;
		if (toffsetflechau > 1.0f) //para que no se desborde la variable
			toffsetflechau = 0.0f;
		toffset = glm::vec2(toffsetflechau, toffsetflechav);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		FlechaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[4]->RenderMeshModel();

		//plano con todos los números
		toffsetnumerou = 0.0f;
		toffsetnumerov = 0.0f;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();

		//número 1 (offset 0,0: el quad ya tiene las coordenadas del 1)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshListModel[6]->RenderMeshModel();

		for (int i = 1; i < 4; i++)
		{
			//números 2-4: misma fila, se recorre 0.25 en U por cada número
			toffsetnumerou += 0.25f;
			toffsetnumerov = 0.0f;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f - (i * 3.0f), 2.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshListModel[6]->RenderMeshModel();
		}

		for (int j = 1; j < 5; j++)
		{
			//números 5-8: fila de abajo (-0.33 en V)
			toffsetnumerou += 0.25f;
			toffsetnumerov = -0.33f;
			toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-7.0f - (j * 3.0f), 5.0f, -6.0f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshListModel[6]->RenderMeshModel();
		}

		//número cambiante
		//¿Cómo hacer para que sea a una velocidad visible? Se cambia por TIEMPO y no en cada frame:
		//sólo cuando ya pasaron 0.5 s desde el último cambio se avanza al siguiente número (1, 2, 3, 4, 1, ...)
		if (now - tiempoNumero >= 0.5f)
		{
			tiempoNumero = now;
			toffsetnumerocambiau += 0.25f;
			if (toffsetnumerocambiau > 0.8f) //después del 4 (offset 0.75) regresa al 1
				toffsetnumerocambiau = 0.0f;
		}
		toffsetnumerov = 0.0f;
		toffset = glm::vec2(toffsetnumerocambiau, toffsetnumerov);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshListModel[6]->RenderMeshModel();

		//cambiar automáticamente entre textura número 1 y número 2 (cada segundo)
		toffset = glm::vec2(0.0f, 0.0f);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-13.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		if (((int)now) % 2 == 0)
			Numero1Texture.UseTexture();
		else
			Numero2Texture.UseTexture();
		meshListModel[5]->RenderMeshModel();

		glDisable(GL_BLEND);
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
