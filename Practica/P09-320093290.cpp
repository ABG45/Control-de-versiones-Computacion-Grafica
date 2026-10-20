/*
Proyecto Final - Computacion Grafica
Base del lab: ventana, camara, iluminacion, texturas, modelos (Assimp) y skybox
Controles: WASD y mouse mueven la camara, Y/U mueven el cubo, ESC cierra

Como esta acomodado este archivo (busquen los bloques ===):
 1. variables globales (texturas, modelos, materiales, luces)
 2. geometria hecha a mano (piso, plano de vegetacion, cubo)
 3. main: carga de texturas, modelos, skybox, materiales y luces
 4. while: aqui se dibuja todo cada frame
Dentro de cada bloque hay una seccion //--- ... --- por actividad (personajes, planetas, pista, etc.)
y cada quien escribe solo en las de su actividad. La lista de teclas por actividad esta en README.md
*/

//esto solo va aqui, en ningun otro archivo (si no da error de simbolos repetidos)
#define STB_IMAGE_IMPLEMENTATION

#include <vector>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

//iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// ================== 1. VARIABLES GLOBALES ==================

Window mainWindow;
std::vector<MeshModel*> meshListModel; //mallas hechas a mano, cada vertice es x y z, u v, nx ny nz
std::vector<Shader> shaderList;

Camera camera;

//texturas (los archivos van en la carpeta Textures)
Texture pruebaTexture;
Texture vegetacionTexture;

//modelos (los archivos van en Models y sus texturas en Textures)
Model Prueba_M;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

//luz direccional (el "sol", solo hay una)
DirectionalLight mainLight;
//arreglos de luces, el maximo de cada una esta en CommonValues.h
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

//shaders que usan todos los objetos (los del skybox los carga Skybox.cpp)
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

//cada actividad escribe solo en su seccion //--- ... ---, en los 4 bloques de este archivo y en Window.h/.cpp
//asi cada quien cambia lineas distintas y al juntar las ramas casi no hay conflictos

//--- personajes ---
Model capitanFalcon;

//modelos con jerarquia: cada uno esta separado en piezas (.obj) que comparten el mismo origen,
//todas con Y arriba, los pies en y = 0 y mirando hacia +Z. "der" e "izq" son la derecha e izquierda
//del personaje (la derecha queda en X negativa)
//por cada pieza hay: nombre del archivo, padre (indice de la pieza de la que cuelga, -1 = raiz),
//pivote (punto de la articulacion donde gira) y rotacion en grados (x, y, z) que se puede cambiar en el while
//el padre siempre va antes que sus hijos en los arreglos

//planta carnivora: mide ~1.4 de alto
const int NUM_PIEZAS_CARNIVORA = 15;
Model Carnivora_M[NUM_PIEZAS_CARNIVORA];
const char* piezasCarnivora[NUM_PIEZAS_CARNIVORA] = {
	"base", "tallo1", "tallo2", "tallo3", "cabeza", "mandibula", "lengua",
	"hoja_del_der", "hoja_del_izq", "hoja_tras_der", "hoja_tras_izq",
	"liana_atras", "liana_der", "liana_frente", "liana_izq" };
int padresCarnivora[NUM_PIEZAS_CARNIVORA] = {
	-1, 0, 1, 2, 3, 4, 5,
	0, 0, 0, 0,
	0, 0, 0, 0 };
glm::vec3 pivotesCarnivora[NUM_PIEZAS_CARNIVORA] = {
	glm::vec3(0.0f, 0.0f, 0.0f),            //base
	glm::vec3(0.0113f, 0.2274f, 0.0774f),   //tallo1
	glm::vec3(0.0197f, 0.4567f, -0.1489f),  //tallo2
	glm::vec3(0.0265f, 0.6000f, -0.3316f),  //tallo3
	glm::vec3(0.0137f, 0.7786f, -0.2390f),  //cabeza
	glm::vec3(0.0f, 0.8332f, 0.0054f),      //mandibula
	glm::vec3(0.0125f, 0.7558f, -0.1502f),  //lengua
	glm::vec3(-0.0691f, 0.1764f, 0.1596f),  //hoja_del_der
	glm::vec3(0.0921f, 0.1770f, 0.1610f),   //hoja_del_izq
	glm::vec3(-0.0642f, 0.1621f, -0.0007f), //hoja_tras_der
	glm::vec3(0.0865f, 0.1632f, -0.0009f),  //hoja_tras_izq
	glm::vec3(0.0110f, 0.1359f, -0.0315f),  //liana_atras
	glm::vec3(-0.0573f, 0.2234f, 0.0081f),  //liana_der
	glm::vec3(0.0437f, 0.2970f, 0.1427f),   //liana_frente
	glm::vec3(0.0550f, 0.2254f, -0.0065f)   //liana_izq
};
glm::vec3 rotCarnivora[NUM_PIEZAS_CARNIVORA]; //las globales empiezan en 0

//Gordon: mide ~1.4 de alto
const int NUM_PIEZAS_GORDON = 14;
Model Gordon_M[NUM_PIEZAS_GORDON];
const char* piezasGordon[NUM_PIEZAS_GORDON] = {
	"torso", "cabeza",
	"brazo_der", "antebrazo_der", "mano_der",
	"brazo_izq", "antebrazo_izq", "mano_izq",
	"muslo_der", "pantorrilla_der", "pie_der",
	"muslo_izq", "pantorrilla_izq", "pie_izq" };
int padresGordon[NUM_PIEZAS_GORDON] = {
	-1, 0,
	0, 2, 3,
	0, 5, 6,
	0, 8, 9,
	0, 11, 12 };
glm::vec3 pivotesGordon[NUM_PIEZAS_GORDON] = {
	glm::vec3(0.0f, 0.73277f, 0.01014f),       //torso (cadera)
	glm::vec3(0.0f, 1.22766f, -0.01482f),      //cabeza (cuello)
	glm::vec3(-0.14675f, 1.11121f, -0.03717f), //brazo_der (hombro)
	glm::vec3(-0.29002f, 0.94174f, -0.04760f), //antebrazo_der (codo)
	glm::vec3(-0.43085f, 0.77516f, -0.04468f), //mano_der (muneca)
	glm::vec3(0.14675f, 1.11121f, -0.03717f),  //brazo_izq
	glm::vec3(0.29002f, 0.94174f, -0.04760f),  //antebrazo_izq
	glm::vec3(0.43085f, 0.77516f, -0.04469f),  //mano_izq
	glm::vec3(-0.07392f, 0.73277f, 0.01014f),  //muslo_der (cadera)
	glm::vec3(-0.07569f, 0.39412f, -0.00743f), //pantorrilla_der (rodilla)
	glm::vec3(-0.07732f, 0.08130f, -0.03441f), //pie_der (tobillo)
	glm::vec3(0.07392f, 0.73277f, 0.01014f),   //muslo_izq
	glm::vec3(0.07569f, 0.39412f, -0.00743f),  //pantorrilla_izq
	glm::vec3(0.07732f, 0.08130f, -0.03441f)   //pie_izq
};
glm::vec3 rotGordon[NUM_PIEZAS_GORDON];

//Kirby: mide ~0.73 de alto
const int NUM_PIEZAS_KIRBY = 5;
Model Kirby_M[NUM_PIEZAS_KIRBY];
const char* piezasKirby[NUM_PIEZAS_KIRBY] = {
	"cuerpo", "mano_der", "mano_izq", "pie_der", "pie_izq" };
int padresKirby[NUM_PIEZAS_KIRBY] = { -1, 0, 0, 0, 0 };
glm::vec3 pivotesKirby[NUM_PIEZAS_KIRBY] = {
	glm::vec3(0.0f, 0.4158f, 0.0f),         //cuerpo (centro)
	glm::vec3(-0.2024f, 0.3899f, 0.0164f),  //mano_der
	glm::vec3(0.2014f, 0.3897f, 0.0170f),   //mano_izq
	glm::vec3(-0.1323f, 0.1498f, -0.0135f), //pie_der
	glm::vec3(0.1317f, 0.1505f, -0.0139f)   //pie_izq
};
glm::vec3 rotKirby[NUM_PIEZAS_KIRBY];

//--- planetas: Xen y planeta 2 ---

//--- pista de carreras y naves ---
Model pistaM;
Model nave1Race;

//Practica 9-1: la nave recorre el circuito de la pista (rectas, curvas, rampas de subida y bajada).
//El camino es una lista de puntos de control sobre el centro del camino, en coordenadas del MODELO de la pista
//(se tomaron de los vertices de pista.obj, siguiendo el centro entre las dos barandillas).
//La nave avanza a velocidad constante de un punto al siguiente; al llegar al ultimo regresa al primero (ciclo).
//peraltePista: inclinacion lateral del camino en cada punto (grados); el lado derecho de la pista esta muy peraltado.
const int NUM_PUNTOS_PISTA = 64;
glm::vec3 puntosPista[NUM_PUNTOS_PISTA] = {
	glm::vec3(-3.7f, 0.0f, 991.8f), //0
	glm::vec3(-1.1f, 0.0f, 888.6f), //1
	glm::vec3(0.3f, 0.0f, 785.3f), //2
	glm::vec3(-2.1f, 0.1f, 682.2f), //3
	glm::vec3(-9.4f, 0.1f, 579.2f), //4
	glm::vec3(-9.3f, 0.1f, 475.9f), //5
	glm::vec3(-4.0f, 0.1f, 372.9f), //6
	glm::vec3(-2.5f, 0.0f, 269.8f), //7
	glm::vec3(-6.9f, 0.0f, 166.6f), //8
	glm::vec3(-6.8f, 0.0f, 63.4f), //9
	glm::vec3(-2.6f, 0.0f, -39.8f), //10
	glm::vec3(-1.1f, 0.1f, -143.0f), //11
	glm::vec3(-3.9f, 0.1f, -246.2f), //12
	glm::vec3(-9.7f, 0.1f, -349.3f), //13
	glm::vec3(-13.5f, -0.2f, -452.5f), //14
	glm::vec3(-18.4f, -2.2f, -555.7f), //15
	glm::vec3(-25.7f, -7.2f, -658.6f), //16
	glm::vec3(-36.1f, -13.7f, -761.0f), //17
	glm::vec3(-40.9f, -17.8f, -864.3f), //18
	glm::vec3(-31.5f, -15.9f, -967.4f), //19
	glm::vec3(-4.0f, -5.8f, -1066.2f), //20
	glm::vec3(44.8f, 12.8f, -1156.1f), //21
	glm::vec3(118.6f, 39.0f, -1225.2f), //22
	glm::vec3(203.4f, 71.4f, -1273.2f), //23
	glm::vec3(295.4f, 107.7f, -1297.8f), //24
	glm::vec3(390.7f, 144.2f, -1292.3f), //25
	glm::vec3(483.2f, 177.3f, -1261.3f), //26
	glm::vec3(567.5f, 206.1f, -1211.1f), //27
	glm::vec3(639.8f, 229.7f, -1141.4f), //28
	glm::vec3(693.5f, 246.3f, -1053.2f), //29
	glm::vec3(721.7f, 254.9f, -953.7f), //30
	glm::vec3(725.7f, 256.3f, -851.3f), //31
	glm::vec3(721.3f, 252.5f, -748.7f), //32
	glm::vec3(715.3f, 246.4f, -645.7f), //33
	glm::vec3(727.4f, 241.8f, -543.4f), //34
	glm::vec3(733.5f, 241.6f, -440.4f), //35
	glm::vec3(746.9f, 244.2f, -338.4f), //36
	glm::vec3(759.7f, 245.9f, -236.2f), //37
	glm::vec3(769.2f, 246.2f, -134.0f), //38
	glm::vec3(778.5f, 248.8f, -31.5f), //39
	glm::vec3(788.5f, 257.2f, 71.0f), //40
	glm::vec3(798.6f, 272.2f, 172.8f), //41
	glm::vec3(808.3f, 292.0f, 273.3f), //42
	glm::vec3(815.4f, 312.7f, 374.6f), //43
	glm::vec3(818.3f, 330.3f, 476.4f), //44
	glm::vec3(817.1f, 343.1f, 579.4f), //45
	glm::vec3(813.7f, 351.8f, 682.0f), //46
	glm::vec3(809.8f, 357.6f, 784.3f), //47
	glm::vec3(791.7f, 360.2f, 885.4f), //48
	glm::vec3(786.5f, 358.1f, 988.7f), //49
	glm::vec3(780.4f, 350.6f, 1091.4f), //50
	glm::vec3(765.8f, 338.8f, 1192.9f), //51
	glm::vec3(732.5f, 323.1f, 1289.2f), //52
	glm::vec3(680.1f, 302.1f, 1376.4f), //53
	glm::vec3(610.8f, 274.5f, 1446.4f), //54
	glm::vec3(529.1f, 240.8f, 1496.6f), //55
	glm::vec3(437.7f, 202.6f, 1525.4f), //56
	glm::vec3(343.0f, 161.3f, 1527.7f), //57
	glm::vec3(252.9f, 119.6f, 1501.5f), //58
	glm::vec3(170.2f, 81.0f, 1452.7f), //59
	glm::vec3(98.6f, 48.1f, 1383.3f), //60
	glm::vec3(48.9f, 23.0f, 1294.5f), //61
	glm::vec3(15.5f, 7.6f, 1197.2f), //62
	glm::vec3(-1.4f, 1.3f, 1095.1f), //63
};
float peraltePista[NUM_PUNTOS_PISTA] = {
	-7.8f, -8.2f, -8.6f, -9.1f, -9.6f, -8.3f, -5.8f, -3.4f,
	-1.8f, -1.2f, -0.6f, -0.2f, -0.1f, -0.4f, -0.9f, -1.3f,
	-1.3f, -0.3f, 1.1f, 0.8f, -1.6f, -6.6f, -15.0f, -24.3f,
	-34.1f, -44.3f, -53.5f, -60.7f, -64.2f, -55.5f, -47.2f, -43.1f,
	-44.9f, -55.9f, -61.0f, -62.8f, -62.2f, -60.1f, -61.1f, -64.0f,
	-67.9f, -71.6f, -74.3f, -76.1f, -76.9f, -75.8f, -73.4f, -70.6f,
	-67.6f, -64.9f, -62.1f, -59.7f, -57.1f, -53.6f, -49.4f, -44.5f,
	-38.9f, -32.4f, -24.2f, -16.2f, -9.5f, -6.5f, -5.8f, -6.6f
};
float largoTramo[NUM_PUNTOS_PISTA]; //largo de cada tramo (del punto i al i+1), se calcula al cargar
float largoCircuito = 0.0f;         //suma de todos los tramos
float distNave = 0.0f;              //distancia recorrida en la vuelta actual
const float velNave = 3.0f;         //unidades de la pista por frame (a 60 fps ~36 s por vuelta)
const float alturaNave = 25.0f;     //la nave flota sobre el camino (su parte de abajo queda ~13 bajo su centro)
const float escalaNave = 0.4f;      //escala de la nave respecto a la pista (el camino mide ~80-180 de ancho)


//--- satelites y cuerpos espaciales ---
Model sateliteRace;

//Practica 9-1: la sonda espacial de la practica 4 (modelo jerarquico hecho con figuras) vuela en orbita
//alrededor de la pista y sus piezas se animan solas. En la practica 4 los paneles solares, el giro de la
//antena y la inclinacion del plato se movian con las teclas O, P e I; aqui sus angulos salen del tiempo.
Texture plainTexture; //blanca: las piezas de la sonda solo llevan color, como en la practica 4
int idxCilindroSonda; //indices en meshListModel de las figuras de la sonda (el cubo ya existe: meshListModel[2])
int idxConoSonda;
int idxEsferaSonda;
float tiempoSonda = 0.0f;         //avanza cada frame mientras la sonda esta activa (tecla O la pausa)
const glm::vec3 centroOrbita = glm::vec3(310.0f, 150.0f, -110.0f); //centro de la pista, arriba de su punto mas alto
const float radioOrbitaX = 220.0f; //la orbita es una elipse que rodea toda la pista
const float radioOrbitaZ = 380.0f;
const float alturaOrbita = 25.0f;  //ademas sube y baja 25 unidades, dos veces por vuelta
const float velOrbita = 0.15f;     //grados por frame (a 60 fps ~40 s por vuelta)
const float escalaSonda = 2.0f;
//colores de la practica 4
const glm::vec3 COLOR_EJE = glm::vec3(0.10f, 0.30f, 0.95f);
const glm::vec3 COLOR_METAL = glm::vec3(0.75f, 0.75f, 0.78f);
const glm::vec3 COLOR_OSCURO = glm::vec3(0.15f, 0.15f, 0.17f);
const glm::vec3 COLOR_ROJO = glm::vec3(0.90f, 0.25f, 0.15f);
const glm::vec3 COLOR_AMARILLO = glm::vec3(0.90f, 0.70f, 0.10f);
const glm::vec3 COLOR_PANEL = glm::vec3(0.10f, 0.15f, 0.55f);

//--- plataforma de lanzamiento y cohete ---

//--- estacion espacial y naves ---

//--- avatar ---

//--- camaras y recorridos ---

//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---


// ================== 2. GEOMETRIA HECHA A MANO ==================

//calcula las normales promediando las de los triangulos que comparten cada vertice
//se llama antes de CreateMeshModel porque ahi es donde se copian los vertices a la GPU
//ojo: en este proyecto las normales apuntan hacia ADENTRO del objeto, asi que los triangulos
//tienen que ir en sentido horario vistos desde afuera
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


//cada malla que se crea aqui se mete a meshListModel y luego se dibuja con su indice
void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	//normales hacia abajo (hacia adentro del piso) por como el shader calcula la luz
	//las UV llegan a 10 para que la textura se repita 10 veces
	GLfloat floorVertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	//dos planos cruzados, sirve para plantas o cosas con transparencia
	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4, 5, 6,
	   4, 6, 7
	};

	//mismas normales que el piso para que se ilumine igual de los dos lados
	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,
	};

	//CreateMeshModel(vertices, indices, cantidad de floats del arreglo, cantidad de indices)
	MeshModel* piso = new MeshModel();
	piso->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(piso); //meshListModel[0]

	MeshModel* vegetacion = new MeshModel();
	vegetacion->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(vegetacion); //meshListModel[1]
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1); //shaderList[0]
}


//cubo de 1x1x1 centrado en el origen, con la imagen completa en cada cara
void CrearCubo()
{
	//cada normal apunta hacia ADENTRO del cubo (al reves de su cara): el shader calcula la luz con
	//(FragPos - posicion de la luz), entonces si apuntaran hacia afuera esa cara se veria oscura
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		4, 5, 6,
		6, 7, 4,
		// back
		8, 9, 10,
		10, 11, 8,
		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,
	};

	GLfloat cubo_vertices[] = {
		//  x      y      z      S      T      NX     NY     NZ
		// front
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,

		// right
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

		 // back (la S va al reves para que la imagen no salga en espejo)
		 -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		  0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
		 -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,

		 // left
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		 -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		 -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,

		 // bottom
		 -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		  0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		  0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,

		 // top
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
	};

	MeshModel* cubo = new MeshModel();
	cubo->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(cubo); //meshListModel[2]
}


//dibuja un modelo con jerarquia: la matriz de cada pieza es la de su padre mas su propia rotacion
//para girar en la articulacion se lleva el pivote al origen, se rota y se regresa
//raiz es la matriz que coloca todo el modelo en la escena
void DibujaJerarquia(Model piezas[], int padres[], glm::vec3 pivotes[], glm::vec3 rotaciones[], int numPiezas,
	glm::mat4 raiz, GLuint uniformModel)
{
	glm::mat4 matrices[20]; //una por pieza, para que los hijos usen la de su padre
	for (int i = 0; i < numPiezas; i++)
	{
		glm::mat4 model;
		if (padres[i] == -1)
			model = raiz;
		else
			model = matrices[padres[i]];

		model = glm::translate(model, pivotes[i]);
		model = glm::rotate(model, rotaciones[i].x * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotaciones[i].y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotaciones[i].z * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, -pivotes[i]);
		matrices[i] = model;

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		piezas[i].RenderModel();
	}
}


//funciones con geometria de cada actividad; se llaman en la carga de recursos, dentro de su seccion
//el indice en meshListModel depende del orden en que se agregan, por eso cada quien lo guarda en una
//variable global antes del push_back (ej. idxCohete = meshListModel.size();) y dibuja con meshListModel[idxCohete]

//--- personajes ---

//--- planetas: Xen y planeta 2 ---

//--- pista de carreras y naves ---

//--- satelites y cuerpos espaciales ---
//figuras de la sonda (practica 9-1). Son las mismas de la practica 4 (cilindro, cono y esfera), pero aqui
//cada vertice lleva x y z, s t, nx ny nz como pide MeshModel, y las normales van hacia ADENTRO como en todo el proyecto.
//Las tres miden 1 de alto (y de -0.5 a 0.5) y tienen radio 1; en el while se escalan como en la practica 4.

//cilindro: res lados. Las paredes llevan sus propios vertices y cada tapa tiene un centro y su orilla,
//asi cada parte tiene su propia normal (paredes hacia el eje, tapas hacia el centro del cilindro)
void CrearCilindroSonda(int res)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	float dt = 2.0f * 3.14159265f / res;
	//paredes: dos vertices por lado (abajo y arriba)
	for (int n = 0; n <= res; n++)
	{
		float x = cos(n * dt), z = sin(n * dt);
		float s = (float)n / res;
		GLfloat abajo[] = { x, -0.5f, z,  s, 0.0f,  -x, 0.0f, -z };
		GLfloat arriba[] = { x, 0.5f, z,  s, 1.0f,  -x, 0.0f, -z };
		vertices.insert(vertices.end(), abajo, abajo + 8);
		vertices.insert(vertices.end(), arriba, arriba + 8);
	}
	for (int n = 0; n < res; n++)
	{
		unsigned int a = 2 * n, b = 2 * n + 1, c = 2 * n + 2, d = 2 * n + 3;
		unsigned int cara[] = { a, c, b,  b, c, d };
		indices.insert(indices.end(), cara, cara + 6);
	}
	//tapas: abajo (normal hacia arriba = hacia adentro) y arriba (normal hacia abajo)
	for (int tapa = 0; tapa < 2; tapa++)
	{
		float y = (tapa == 0) ? -0.5f : 0.5f;
		float ny = (tapa == 0) ? 1.0f : -1.0f;
		unsigned int centro = vertices.size() / 8;
		GLfloat c[] = { 0.0f, y, 0.0f,  0.5f, 0.5f,  0.0f, ny, 0.0f };
		vertices.insert(vertices.end(), c, c + 8);
		for (int n = 0; n <= res; n++)
		{
			float x = cos(n * dt), z = sin(n * dt);
			GLfloat v[] = { x, y, z,  0.5f + 0.5f * x, 0.5f + 0.5f * z,  0.0f, ny, 0.0f };
			vertices.insert(vertices.end(), v, v + 8);
		}
		for (int n = 0; n < res; n++)
		{
			unsigned int tri[] = { centro, centro + 1 + n, centro + 2 + n };
			indices.insert(indices.end(), tri, tri + 3);
		}
	}
	MeshModel* cilindro = new MeshModel();
	cilindro->CreateMeshModel(vertices.data(), indices.data(), vertices.size(), indices.size());
	idxCilindroSonda = meshListModel.size();
	meshListModel.push_back(cilindro);
}

//cono: punta en y = 0.5 y base de radio 1 en y = -0.5. Cada lado lleva su propio vertice de la punta
//para que su normal sea la de esa parte del cono. La normal hacia afuera del cono es (x, 1, z) normalizada
//(radio 1 y alto 1), asi que hacia adentro es la contraria
void CrearConoSonda(int res)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	float dt = 2.0f * 3.14159265f / res;
	const float k = 0.70710678f; //1 / raiz(2), para normalizar (x, 1, z)
	for (int n = 0; n < res; n++)
	{
		float x1 = cos(n * dt), z1 = sin(n * dt);
		float x2 = cos((n + 1) * dt), z2 = sin((n + 1) * dt);
		float xm = cos((n + 0.5f) * dt), zm = sin((n + 0.5f) * dt);
		unsigned int i0 = vertices.size() / 8;
		GLfloat lado[] = {
			0.0f, 0.5f, 0.0f,  (n + 0.5f) / res, 1.0f,  -xm * k, -k, -zm * k,
			x1, -0.5f, z1,     (float)n / res, 0.0f,    -x1 * k, -k, -z1 * k,
			x2, -0.5f, z2,     (float)(n + 1) / res, 0.0f, -x2 * k, -k, -z2 * k };
		vertices.insert(vertices.end(), lado, lado + 24);
		unsigned int tri[] = { i0, i0 + 1, i0 + 2 };
		indices.insert(indices.end(), tri, tri + 3);
	}
	//base (normal hacia arriba = hacia adentro)
	unsigned int centro = vertices.size() / 8;
	GLfloat c[] = { 0.0f, -0.5f, 0.0f,  0.5f, 0.5f,  0.0f, 1.0f, 0.0f };
	vertices.insert(vertices.end(), c, c + 8);
	for (int n = 0; n <= res; n++)
	{
		float x = cos(n * dt), z = sin(n * dt);
		GLfloat v[] = { x, -0.5f, z,  0.5f + 0.5f * x, 0.5f + 0.5f * z,  0.0f, 1.0f, 0.0f };
		vertices.insert(vertices.end(), v, v + 8);
	}
	for (int n = 0; n < res; n++)
	{
		unsigned int tri[] = { centro, centro + 1 + n, centro + 2 + n };
		indices.insert(indices.end(), tri, tri + 3);
	}
	MeshModel* cono = new MeshModel();
	cono->CreateMeshModel(vertices.data(), indices.data(), vertices.size(), indices.size());
	idxConoSonda = meshListModel.size();
	meshListModel.push_back(cono);
}

//esfera de radio 1 por meridianos (slices) y paralelos (stacks), como la clase Sphere de la practica 4.
//En una esfera de radio 1 centrada en el origen la normal hacia afuera es la misma posicion,
//asi que hacia adentro es la posicion con signo contrario
void CrearEsferaSonda(int slices, int stacks)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	const float PI = 3.14159265f;
	for (int i = 0; i <= stacks; i++)
	{
		float fi = PI * i / stacks; //de 0 (polo de arriba) a PI (polo de abajo)
		for (int j = 0; j <= slices; j++)
		{
			float teta = 2.0f * PI * j / slices;
			float x = sin(fi) * cos(teta), y = cos(fi), z = sin(fi) * sin(teta);
			GLfloat v[] = { x, y, z,  (float)j / slices, 1.0f - (float)i / stacks,  -x, -y, -z };
			vertices.insert(vertices.end(), v, v + 8);
		}
	}
	for (int i = 0; i < stacks; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			unsigned int a = i * (slices + 1) + j, b = a + slices + 1;
			unsigned int cara[] = { a, b, a + 1,  a + 1, b, b + 1 };
			indices.insert(indices.end(), cara, cara + 6);
		}
	}
	MeshModel* esfera = new MeshModel();
	esfera->CreateMeshModel(vertices.data(), indices.data(), vertices.size(), indices.size());
	idxEsferaSonda = meshListModel.size();
	meshListModel.push_back(esfera);
}

//dibuja una pieza de la sonda: igual que DibujarPrisma/DibujarCilindro de la practica 4,
//recibe la matriz ya colocada y el color, y solo manda los uniforms y dibuja
void DibujaPiezaSonda(int idxMalla, glm::mat4 model, glm::vec3 color, GLuint uniformModel, GLuint uniformColor)
{
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshListModel[idxMalla]->RenderMeshModel();
}

//--- plataforma de lanzamiento y cohete ---

//--- estacion espacial y naves ---

//--- avatar ---

//--- camaras y recorridos ---

//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---


int main()
{
	mainWindow = Window(1366, 768); //tambien jala con 1280x1024 o 1024x768
	mainWindow.Initialise();
	CreateObjects();
	CrearCubo();
	CreateShaders();

	//Camera(posicion inicial, vector up, yaw, pitch, velocidad de movimiento, velocidad de giro)
	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	// ================== 3. CARGA DE RECURSOS ==================

	//texturas: primero se asigna la ruta y luego se carga
	//LoadTextureA para imagenes con canal alfa (png, tga) y LoadTexture para las que no tienen (jpg)
	pruebaTexture = Texture("Textures/prueba.png");
	pruebaTexture.LoadTextureA();
	vegetacionTexture = Texture("Textures/prueba_alpha.png");
	vegetacionTexture.LoadTextureA();

	//modelos
	Prueba_M = Model();
	Prueba_M.LoadModel("Models/prueba.obj");

	//skybox: las 6 caras van en este orden: rt, lf, dn, up, bk, ft
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/sBRight.png");// +X (Derecha)
	skyboxFaces.push_back("Textures/Skybox/Left.png");// -X (Izquierda)
	skyboxFaces.push_back("Textures/Skybox/DownF.png");// -Y (Abajo)
	skyboxFaces.push_back("Textures/Skybox/Top.png");// +Y (Arriba)
	skyboxFaces.push_back("Textures/Skybox/FrontF.png");// +Z (Atras)
	skyboxFaces.push_back("Textures/Skybox/BackF.png");// -Z (Enfrente)

	skybox = Skybox(skyboxFaces);

	//materiales: Material(intensidad especular, shininess)
	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	//--- personajes ---
	capitanFalcon = Model();
	capitanFalcon.LoadModel("Models/capitanFalcon/capitanFalcon.obj");

	//las rutas se arman con el nombre de cada pieza
	for (int i = 0; i < NUM_PIEZAS_CARNIVORA; i++)
	{
		std::string ruta = std::string("Models/Carnivora/carnivora_") + piezasCarnivora[i] + ".obj";
		Carnivora_M[i] = Model();
		Carnivora_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_GORDON; i++)
	{
		std::string ruta = std::string("Models/GordonParts/gordon_") + piezasGordon[i] + ".obj";
		Gordon_M[i] = Model();
		Gordon_M[i].LoadModel(ruta);
	}

	for (int i = 0; i < NUM_PIEZAS_KIRBY; i++)
	{
		std::string ruta = std::string("Models/Kirby/kirby_") + piezasKirby[i] + ".obj";
		Kirby_M[i] = Model();
		Kirby_M[i].LoadModel(ruta);
	}

	//--- planetas: Xen y planeta 2 ---

	//--- pista de carreras y naves ---
	pistaM = Model();
	pistaM.LoadModel("Models/pista/pista.obj");
	nave1Race = Model();
	nave1Race.LoadModel("Models/nave1Race/nave1Race.obj");
	//largo de cada tramo del circuito (distancia entre un punto y el siguiente; el ultimo se une con el primero)
	for (int i = 0; i < NUM_PUNTOS_PISTA; i++)
	{
		largoTramo[i] = glm::length(puntosPista[(i + 1) % NUM_PUNTOS_PISTA] - puntosPista[i]);
		largoCircuito += largoTramo[i];
	}

	//--- satelites y cuerpos espaciales ---
	sateliteRace = Model();
	sateliteRace.LoadModel("Models/sateliteRace/sateliteRace.obj");
	//sonda de la practica 4: textura blanca y sus figuras (el cubo ya esta en meshListModel[2])
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	CrearCilindroSonda(18);
	CrearConoSonda(25);
	CrearEsferaSonda(20, 20);

	//--- plataforma de lanzamiento y cohete ---

	//--- estacion espacial y naves ---

	//--- avatar ---

	//--- camaras y recorridos ---


	// ---- luces ----
	//los parametros van en este orden:
	//DirectionalLight(r, g, b, intensidad ambiental, intensidad difusa, direccion x, y, z)
	//PointLight(r, g, b, ambiental, difusa, posicion x, y, z, constante, lineal, exponente)
	//SpotLight(r, g, b, ambiental, difusa, posicion x, y, z, direccion x, y, z, constante, lineal, exponente, angulo del cono)

	//luz direccional: solo hay 1 y siempre tiene que existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.2f, 0.4f,
		0.0f, -1.0f, -1.0f);

	//contador de luces puntuales (no puede pasar de MAX_POINT_LIGHTS)
	unsigned int pointLightCount = 0;
	//luz puntual blanca
	pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
		0.0f, 0.6f,
		2.0f, 1.5f, -3.0f,
		1.0f, 0.05f, 0.005f);
	pointLightCount++;

	//contador de spotlights (no puede pasar de MAX_SPOT_LIGHTS)
	unsigned int spotLightCount = 0;
	//linterna: su posicion y direccion se actualizan en el while con la camara
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		10.0f);
	spotLightCount++;

	//spotlight azul fija apuntando hacia abajo
	spotLights[1] = SpotLight(0.2f, 0.4f, 1.0f,
		0.0f, 2.0f,
		3.0f, 4.0f, -7.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.05f, 0.005f,
		20.0f);
	spotLightCount++;
	//para agregar otra luz: se pone en el siguiente lugar libre del arreglo y se suma 1 al contador

	//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
	//las luces de la escena van aqui; si otra actividad necesita una luz, se pide a quien lleva esta seccion

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	//ojo: glm::perspective usa radianes, asi que el 45.0f no son 45 grados, pero asi venia del lab y se ve bien
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	// ================== 4. CICLO PRINCIPAL ==================
	//todo lo que esta dentro del while se repite cada frame hasta que se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//eventos de teclado y mouse
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//matriz de vista de la camara libre; con otras camaras se cambia en su seccion
		glm::mat4 view = camera.calculateViewMatrix();

		//--- camaras y recorridos ---
		//zoom con la rueda del mouse: la proyeccion se recalcula cada frame con el angulo de vision actual
		//(glm::perspective pide el angulo en radianes, por eso glm::radians)
		projection = glm::perspective(glm::radians(mainWindow.getZoom()),
			(GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

		//limpiar la pantalla
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//el skybox va primero, antes que todo lo demas
		skybox.DrawSkybox(view, projection, deltaTime);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();

		//intensidad especular y brillo del material
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		//linterna pegada a la camara, un poco abajo de los ojos
		//aqui en el while es donde se pueden cambiar cosas de las luces mientras corre el programa
		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
		//aqui se cambian las luces cada frame, antes de mandarlas al shader

		//mandar todas las luces al shader
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		//para dibujar cada objeto siempre es lo mismo: matriz model, color, textura, material y render
		glm::mat4 model(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //blanco = la textura se ve con sus colores normales

		// ---------- objetos opacos ----------

		//piso
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		pruebaTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[0]->RenderMeshModel();

		//cubo: se mueve en X con las teclas Y/U (ver Window.cpp)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(mainWindow.getmuevex(), -0.5f, -4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		pruebaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[2]->RenderMeshModel();

		//modelo cargado de archivo: trae sus propias texturas, por eso aqui no va UseTexture
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(4.0f, -1.0f, -3.0f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Prueba_M.RenderModel();

		//--- personajes ---
		model = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(240.0f, 15.0f, -130.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		capitanFalcon.RenderModel();

		//modelos con jerarquia: los pies estan en y = 0, por eso se bajan a -1 (altura del piso)
		//para articular se cambian las rotaciones, ej. rotGordon[2].x = -30.0f; mueve el brazo derecho en el hombro
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		//planta carnivora
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.0f, -1.0f, -3.0f));
		DibujaJerarquia(Carnivora_M, padresCarnivora, pivotesCarnivora, rotCarnivora, NUM_PIEZAS_CARNIVORA, model, uniformModel);

		//Gordon
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-2.0f, -1.0f, -3.0f));
		DibujaJerarquia(Gordon_M, padresGordon, pivotesGordon, rotGordon, NUM_PIEZAS_GORDON, model, uniformModel);

		//Kirby
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.5f, -1.0f, -3.0f));
		DibujaJerarquia(Kirby_M, padresKirby, pivotesKirby, rotKirby, NUM_PIEZAS_KIRBY, model, uniformModel);

		//--- planetas: Xen y planeta 2 ---

		//--- pista de carreras y naves ---
		//PISTA RACE
		model = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(240.0f, 10.0f, -130.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		glm::mat4 matPista = model; //se guarda para mover la nave sobre la pista
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pistaM.RenderModel();

		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		//NAVE1 RACE: recorre el circuito de la pista (practica 9-1)
		//la carrera avanza solo si esta activa (tecla P la pausa o la reanuda)
		if (mainWindow.getCarrera())
		{
			distNave += velNave * deltaTime;
			if (distNave >= largoCircuito) //termino la vuelta: empieza otra (ciclo)
				distNave -= largoCircuito;
		}
		//1) en que tramo va: se le restan los largos de los tramos hasta que lo que queda cabe en uno
		int tramo = 0;
		float resto = distNave;
		while (resto > largoTramo[tramo] && tramo < NUM_PUNTOS_PISTA - 1)
		{
			resto -= largoTramo[tramo];
			tramo++;
		}
		int sig = (tramo + 1) % NUM_PUNTOS_PISTA;
		float t = resto / largoTramo[tramo]; //0 = inicio del tramo, 1 = final
		//2) posicion: interpolacion lineal entre el punto del tramo y el siguiente
		glm::vec3 posNave = puntosPista[tramo] + (puntosPista[sig] - puntosPista[tramo]) * t;
		//3) hacia donde ve: la direccion del camino en cada punto se mezcla con t, asi gira suave en las curvas
		int ant = (tramo + NUM_PUNTOS_PISTA - 1) % NUM_PUNTOS_PISTA;
		int sig2 = (sig + 1) % NUM_PUNTOS_PISTA;
		glm::vec3 dirA = glm::normalize(puntosPista[sig] - puntosPista[ant]);    //direccion del camino en el punto "tramo"
		glm::vec3 dirB = glm::normalize(puntosPista[sig2] - puntosPista[tramo]); //direccion del camino en el punto "sig"
		glm::vec3 dirNave = glm::normalize(dirA + (dirB - dirA) * t);
		float peralte = peraltePista[tramo] + (peraltePista[sig] - peraltePista[tramo]) * t;
		//4) la direccion se pasa a angulos: giro sobre Y (curvas) y sobre X (rampas)
		float anguloGiro = atan2(dirNave.x, dirNave.z);
		float anguloRampa = asin(dirNave.y);

		model = matPista; //la nave se mueve en coordenadas de la pista, asi le aplica su misma posicion y escala
		model = glm::translate(model, posNave);
		model = glm::rotate(model, anguloGiro, glm::vec3(0.0f, 1.0f, 0.0f));      //curvas
		model = glm::rotate(model, -anguloRampa, glm::vec3(1.0f, 0.0f, 0.0f));    //rampas: nariz arriba al subir, abajo al bajar
		model = glm::rotate(model, -peralte * toRadians, glm::vec3(0.0f, 0.0f, 1.0f)); //se inclina igual que el camino
		model = glm::translate(model, glm::vec3(0.0f, alturaNave, 0.0f));         //flota sobre el camino
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); //la nariz del modelo ve hacia -Z
		model = glm::scale(model, glm::vec3(escalaNave, escalaNave, escalaNave));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		nave1Race.RenderModel();

		//--- satelites y cuerpos espaciales ---
		//SATELITE RACE
		model = glm::mat4(1.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::translate(model, glm::vec3(340.0f, 50.0f, -530.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		sateliteRace.RenderModel();

		//SONDA ESPACIAL (practica 9-1): vuela en orbita y sus piezas se mueven solas
		if (mainWindow.getSondaActiva())
			tiempoSonda += deltaTime; //con O se pausa: el tiempo de la sonda deja de avanzar

		//1) trayectoria: elipse alrededor de la pista que ademas sube y baja (seno del doble del angulo)
		float anguloOrbita = tiempoSonda * velOrbita * toRadians;
		glm::vec3 posSonda = centroOrbita + glm::vec3(radioOrbitaX * cos(anguloOrbita),
			alturaOrbita * sin(2.0f * anguloOrbita),
			radioOrbitaZ * sin(anguloOrbita));
		//hacia donde avanza: derivada de la posicion respecto al angulo (solo X y Z para el giro sobre Y)
		float dirSondaX = -radioOrbitaX * sin(anguloOrbita);
		float dirSondaZ = radioOrbitaZ * cos(anguloOrbita);
		float giroSonda = atan2(dirSondaX, dirSondaZ); //el frente de la sonda (+Z, donde esta el instrumento rojo) ve hacia donde avanza

		//2) piezas: antes eran las teclas O, P e I de la practica 4, ahora sus angulos salen del tiempo
		float anguloPaneles = tiempoSonda * 1.0f;              //los paneles giran sin parar sobre X ("buscan el sol")
		float anguloAntenaY = tiempoSonda * 2.0f;              //la antena da vueltas sobre Y
		float anguloPlatoX = 35.0f * sin(tiempoSonda * 0.03f); //el plato se inclina de -35 a 35 grados y regresa

		plainTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		glm::mat4 sondaAux(1.0f), sondaAux2(1.0f), sondaAux3(1.0f), sondaAux4(1.0f);

		//ORIGEN de la sonda: la misma jerarquia de la practica 4, ahora colgada de la trayectoria
		model = glm::mat4(1.0f);
		model = glm::translate(model, posSonda);
		model = glm::rotate(model, giroSonda, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(escalaSonda, escalaSonda, escalaSonda));
		sondaAux = model;

		//PLATAFORMA
		model = sondaAux;
		model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f));
		model = glm::scale(model, glm::vec3(2.6f, 0.4f, 2.6f));
		DibujaPiezaSonda(idxCilindroSonda, model, COLOR_OSCURO, uniformModel, uniformColor);

		//MASTIL
		model = sondaAux;
		model = glm::translate(model, glm::vec3(0.0f, 1.9f, 0.0f));
		model = glm::scale(model, glm::vec3(0.6f, 3.0f, 0.6f));
		DibujaPiezaSonda(idxCilindroSonda, model, COLOR_METAL, uniformModel, uniformColor);

		//CUERPO
		model = sondaAux;
		model = glm::translate(model, glm::vec3(0.0f, 4.6f, 0.0f)); //centro del cuerpo
		sondaAux2 = model; //de aqui cuelgan los paneles, la antena y el instrumento
		model = glm::scale(model, glm::vec3(3.0f, 2.4f, 3.0f));
		DibujaPiezaSonda(idxCilindroSonda, model, COLOR_AMARILLO, uniformModel, uniformColor);

		//PANELES SOLARES: uno hacia +X y otro hacia -X
		for (int lado = 1; lado >= -1; lado -= 2)
		{
			//EJE PANEL: gira sobre X (antes tecla O)
			model = sondaAux2;
			model = glm::translate(model, glm::vec3(1.5f * lado, 0.0f, 0.0f));
			model = glm::rotate(model, anguloPaneles * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			sondaAux3 = model;

			model = sondaAux3;
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			DibujaPiezaSonda(idxEsferaSonda, model, COLOR_EJE, uniformModel, uniformColor);

			//BRAZO DEL PANEL
			model = sondaAux3;
			model = glm::translate(model, glm::vec3(0.75f * lado, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.5f, 0.25f, 0.25f));
			DibujaPiezaSonda(2, model, COLOR_METAL, uniformModel, uniformColor); //meshListModel[2] es el cubo

			//PANEL SOLAR
			model = sondaAux3;
			model = glm::translate(model, glm::vec3(4.0f * lado, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(5.0f, 0.15f, 2.6f));
			DibujaPiezaSonda(2, model, COLOR_PANEL, uniformModel, uniformColor);
		}

		//EJE ANTENA: gira toda la antena sobre Y (antes tecla P)
		model = sondaAux2;
		model = glm::translate(model, glm::vec3(0.0f, 1.2f, 0.0f)); //tapa del cuerpo
		model = glm::rotate(model, anguloAntenaY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		sondaAux3 = model;

		model = sondaAux3;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		DibujaPiezaSonda(idxEsferaSonda, model, COLOR_EJE, uniformModel, uniformColor);

		//MASTIL DE LA ANTENA
		model = sondaAux3;
		model = glm::translate(model, glm::vec3(0.0f, 0.9f, 0.0f));
		model = glm::scale(model, glm::vec3(0.35f, 1.8f, 0.35f));
		DibujaPiezaSonda(idxCilindroSonda, model, COLOR_METAL, uniformModel, uniformColor);

		//EJE DE INCLINACION: inclina el plato sobre X (antes tecla I)
		model = sondaAux3;
		model = glm::translate(model, glm::vec3(0.0f, 1.8f, 0.0f));
		model = glm::rotate(model, anguloPlatoX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		sondaAux4 = model;

		model = sondaAux4;
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		DibujaPiezaSonda(idxEsferaSonda, model, COLOR_EJE, uniformModel, uniformColor);

		//PLATO DE LA ANTENA: cono volteado 180 grados sobre X (radio 1.2, como el de la practica 4)
		model = sondaAux4;
		model = glm::translate(model, glm::vec3(0.0f, 0.6f, 0.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		DibujaPiezaSonda(idxConoSonda, model, COLOR_METAL, uniformModel, uniformColor);

		//RECEPTOR DEL PLATO
		model = sondaAux4;
		model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f, 1.4f, 0.12f));
		DibujaPiezaSonda(idxCilindroSonda, model, COLOR_OSCURO, uniformModel, uniformColor);

		model = sondaAux4;
		model = glm::translate(model, glm::vec3(0.0f, 2.2f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		DibujaPiezaSonda(idxEsferaSonda, model, COLOR_ROJO, uniformModel, uniformColor);

		//INSTRUMENTO LATERAL DEL CUERPO (marca el frente de la sonda)
		model = sondaAux2;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.8f));
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.2f));
		DibujaPiezaSonda(2, model, COLOR_ROJO, uniformModel, uniformColor);

		color = glm::vec3(1.0f, 1.0f, 1.0f); //se regresa a blanco para lo que se dibuje despues
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		//--- plataforma de lanzamiento y cohete ---

		//--- estacion espacial y naves ---

		//--- avatar ---

		//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---
		//postes o lamparas de las luminarias

		// ---------- objetos con transparencia ----------
		//van al final, despues de todos los opacos, si no se ven mal
		//cada actividad dibuja aqui sus transparentes, entre glEnable(GL_BLEND) y glDisable(GL_BLEND)

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.5f, 0.0f, -6.5f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		//blending: para transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		vegetacionTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[1]->RenderMeshModel();
		glDisable(GL_BLEND);

		//--- personajes ---

		//--- planetas: Xen y planeta 2 ---

		//--- pista de carreras y naves ---

		//--- satelites y cuerpos espaciales ---

		//--- plataforma de lanzamiento y cohete ---

		//--- estacion espacial y naves ---

		//--- avatar ---

		//--- iluminacion: ciclo dia/noche, luminarias y spotlights ---

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
