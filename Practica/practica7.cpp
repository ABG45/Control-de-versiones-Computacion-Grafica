/*
Práctica 7: Iluminación 1 - REPORTE DE PRÁCTICA
- spotLights[2]: faro frontal del rover. Se calcula con la matriz del rover, así que al avanzar o retroceder (I/K) la luz va con él.
- spotLights[3]: faro del avión hacia abajo. Al subir o bajar el avión (O/L) la luz va con él.
- spotLights[4]: faro del avión hacia adelante. Al avanzar o retroceder el avión (flechas arriba/abajo) la luz va con él.
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
Texture dadoTexture;     //dado Star Wars (práctica 6)
Texture holocronTexture; //holocron Jedi texturizado por código (práctica 6)


//Rover con jerarquía: un Model por pieza, en el mismo orden que kPartesRover (jerarquia.h)
Model RoverPartes[kNumPartesRover];
float anguloRover[kNumPartesRover] = { 0.0f }; //ángulo actual de cada articulación (grados)
glm::vec3 posRover(1.0f, -1.0f, 6.0f); //posición del rover (I/K lo mueve sobre su eje X, que es su frente)
float giroLlantas = 0.0f;              //giro de las llantas al avanzar (grados)
glm::vec3 posAvion(6.0f, 3.0f, 3.0f);  //posición del avión
const float rotAvion = 30.0f;          //orientación del avión sobre Y (grados)

//Matrices de los modelos que llevan faro: se usan para dibujarlos Y para colocar sus luces,
//por eso la luz siempre queda pegada al modelo.
glm::mat4 MatrizRover()
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, posRover);
	m = glm::scale(m, glm::vec3(0.4f, 0.4f, 0.4f));
	return m;
}
glm::mat4 MatrizAvion()
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, posAvion);
	m = glm::rotate(m, rotAvion * 3.14159265f / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::scale(m, glm::vec3(0.6f, 0.6f, 0.6f));
	return m;
}
//punto y dirección en coordenadas del modelo -> mundo
glm::vec3 PuntoMundo(const glm::mat4& m, glm::vec3 p) { return glm::vec3(m * glm::vec4(p, 1.0f)); }
glm::vec3 DirMundo(const glm::mat4& m, glm::vec3 d) { return glm::normalize(glm::vec3(m * glm::vec4(d, 0.0f))); }
//Modelos de las prácticas anteriores
Model Dado_M;     //dado texturizado en el programa de modelado (práctica 6)
Model Holocron_M; //holocron texturizado importado (práctica 6)
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


}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


void CrearDado()
{
	//Dado de 6 caras con logos de Star Wars (textura optimizada: Textures/dado_starwars.png, 1024x512, celdas de 256x256)
	//Cada cara tiene sus 4 vértices propios para poder asignar coordenadas S,T distintas por cara.
	//Orden de vértices por cara: abajo-izq, abajo-der, arriba-der, arriba-izq (vistos desde afuera) -> la imagen no sale espejeada
	//Normales invertidas (convención del curso, igual que en Model.cpp)
	unsigned int cubo_indices[] = {
		0, 1, 2,
		2, 3, 0,  // front
		4, 5, 6,
		6, 7, 4,  // right
		8, 9, 10,
		10, 11, 8,  // back
		12, 13, 14,
		14, 15, 12,  // left
		16, 17, 18,
		18, 19, 16,  // top
		20, 21, 22,
		22, 23, 20,  // bottom
	};

	GLfloat cubo_vertices[] = {
		//x		y		z			S			T				NX		NY		NZ
		// front
		-0.50f,	-0.50f,	0.50f,		0.0029f,	0.5059f,		0.0f,	0.0f,	-1.0f,	//0
		0.50f,	-0.50f,	0.50f,		0.2471f,	0.5059f,		0.0f,	0.0f,	-1.0f,	//1
		0.50f,	0.50f,	0.50f,		0.2471f,	0.9941f,		0.0f,	0.0f,	-1.0f,	//2
		-0.50f,	0.50f,	0.50f,		0.0029f,	0.9941f,		0.0f,	0.0f,	-1.0f,	//3
		// right
		0.50f,	-0.50f,	0.50f,		0.2529f,	0.5059f,		-1.0f,	0.0f,	0.0f,	//4
		0.50f,	-0.50f,	-0.50f,		0.4971f,	0.5059f,		-1.0f,	0.0f,	0.0f,	//5
		0.50f,	0.50f,	-0.50f,		0.4971f,	0.9941f,		-1.0f,	0.0f,	0.0f,	//6
		0.50f,	0.50f,	0.50f,		0.2529f,	0.9941f,		-1.0f,	0.0f,	0.0f,	//7
		// back
		0.50f,	-0.50f,	-0.50f,		0.5029f,	0.5059f,		0.0f,	0.0f,	1.0f,	//8
		-0.50f,	-0.50f,	-0.50f,		0.7471f,	0.5059f,		0.0f,	0.0f,	1.0f,	//9
		-0.50f,	0.50f,	-0.50f,		0.7471f,	0.9941f,		0.0f,	0.0f,	1.0f,	//10
		0.50f,	0.50f,	-0.50f,		0.5029f,	0.9941f,		0.0f,	0.0f,	1.0f,	//11
		// left
		-0.50f,	-0.50f,	-0.50f,		0.7529f,	0.5059f,		1.0f,	0.0f,	0.0f,	//12
		-0.50f,	-0.50f,	0.50f,		0.9971f,	0.5059f,		1.0f,	0.0f,	0.0f,	//13
		-0.50f,	0.50f,	0.50f,		0.9971f,	0.9941f,		1.0f,	0.0f,	0.0f,	//14
		-0.50f,	0.50f,	-0.50f,		0.7529f,	0.9941f,		1.0f,	0.0f,	0.0f,	//15
		// top
		-0.50f,	0.50f,	0.50f,		0.0029f,	0.0059f,		0.0f,	-1.0f,	0.0f,	//16
		0.50f,	0.50f,	0.50f,		0.2471f,	0.0059f,		0.0f,	-1.0f,	0.0f,	//17
		0.50f,	0.50f,	-0.50f,		0.2471f,	0.4941f,		0.0f,	-1.0f,	0.0f,	//18
		-0.50f,	0.50f,	-0.50f,		0.0029f,	0.4941f,		0.0f,	-1.0f,	0.0f,	//19
		// bottom
		-0.50f,	-0.50f,	-0.50f,		0.2529f,	0.0059f,		0.0f,	1.0f,	0.0f,	//20
		0.50f,	-0.50f,	-0.50f,		0.4971f,	0.0059f,		0.0f,	1.0f,	0.0f,	//21
		0.50f,	-0.50f,	0.50f,		0.4971f,	0.4941f,		0.0f,	1.0f,	0.0f,	//22
		-0.50f,	-0.50f,	0.50f,		0.2529f,	0.4941f,		0.0f,	1.0f,	0.0f,	//23
	};
	//caras: front=Alianza Rebelde, right=Imperio, back=Orden Jedi, left=Imperio Sith, top=Mandalorian, bottom=Primera Orden

	MeshModel* dado = new MeshModel();
	dado->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(dado);

}


void CrearHolocron()
{
	//Reporte Ejercicio 1: Holocron Jedi texturizado por código
	//Geometría: holocron_simple (cubo con esquinas recortadas) = 14 caras:
	//  6 caras grandes (arriba, 4 lados y fondo octagonal), 4 trapecios inferiores y 4 triángulos superiores.
	//Cada una de las 14 caras lleva un logo distinto del universo Star Wars.
	//Cada cara tiene vértices propios (56 en total) para asignarle su propia región S,T
	//dentro de Textures/holocron_jedi.png (1024x1024, rejilla de 4x4 celdas de 256 px).
	//Las caras se triangulan en abanico (0,1,2 / 0,2,3 / ...). Normales invertidas (convención del curso).
	unsigned int holocron_indices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23, 20, 23, 24, 20, 24, 25, 20, 25, 26, 20, 26, 27,
		28, 29, 30, 28, 30, 31,
		32, 33, 34, 32, 34, 35,
		36, 37, 38, 36, 38, 39,
		40, 41, 42, 40, 42, 43,
		44, 45, 46,
		47, 48, 49,
		50, 51, 52,
		53, 54, 55,
	};

	GLfloat holocron_vertices[] = {
		//x				y				z				S			T				NX			NY			NZ
		// cara big
		-2.451372f,	2.421759f,	0.077538f,		0.0137f,	0.8744f,		0.011f,	-1.000f,	-0.008f,	//0
		0.051152f,	2.399781f,	2.545043f,		0.1257f,	0.7639f,		0.011f,	-1.000f,	-0.008f,	//1
		2.521206f,	2.448241f,	0.063388f,		0.2363f,	0.8750f,		0.011f,	-1.000f,	-0.008f,	//2
		0.049649f,	2.440128f,	-2.417217f,		0.1257f,	0.9861f,		0.011f,	-1.000f,	-0.008f,	//3
		// cara big
		-2.517274f,	-2.579095f,	0.002115f,		0.3747f,	0.7637f,		0.998f,	-0.040f,	0.038f,	//4
		-2.646786f,	-0.139047f,	2.493757f,		0.4858f,	0.8722f,		0.998f,	-0.040f,	0.038f,	//5
		-2.451372f,	2.421759f,	0.077538f,		0.3779f,	0.9863f,		0.998f,	-0.040f,	0.038f,	//6
		-2.453980f,	-0.063388f,	-2.477451f,		0.2642f,	0.8756f,		0.998f,	-0.040f,	0.038f,	//7
		// cara big
		2.593003f,	-0.085222f,	2.505755f,		0.5137f,	0.8736f,		-1.000f,	-0.022f,	0.006f,	//8
		2.632201f,	-2.553892f,	0.007226f,		0.6230f,	0.7655f,		-1.000f,	-0.022f,	0.006f,	//9
		2.534944f,	-0.061146f,	-2.580280f,		0.7363f,	0.8747f,		-1.000f,	-0.022f,	0.006f,	//10
		2.521206f,	2.448241f,	0.063388f,		0.6206f,	0.9845f,		-1.000f,	-0.022f,	0.006f,	//11
		// cara big
		-2.646786f,	-0.139047f,	2.493757f,		0.7637f,	0.8734f,		0.022f,	-0.004f,	-1.000f,	//12
		-0.140696f,	-2.601934f,	2.558561f,		0.8702f,	0.7687f,		0.022f,	-0.004f,	-1.000f,	//13
		2.593003f,	-0.085222f,	2.505755f,		0.9863f,	0.8757f,		0.022f,	-0.004f,	-1.000f,	//14
		0.051152f,	2.399781f,	2.545043f,		0.8784f,	0.9813f,		0.022f,	-0.004f,	-1.000f,	//15
		// cara big
		2.534944f,	-0.061146f,	-2.580280f,		0.0137f,	0.6239f,		0.064f,	-0.001f,	0.998f,	//16
		0.081432f,	-2.513204f,	-2.426005f,		0.1235f,	0.5144f,		0.064f,	-0.001f,	0.998f,	//17
		-2.453980f,	-0.063388f,	-2.477451f,		0.2363f,	0.6238f,		0.064f,	-0.001f,	0.998f,	//18
		0.049649f,	2.440128f,	-2.417217f,		0.1249f,	0.7356f,		0.064f,	-0.001f,	0.998f,	//19
		// cara big
		-2.517274f,	-2.579095f,	0.002115f,		0.2637f,	0.6222f,		-0.011f,	1.000f,	0.003f,	//20
		-1.185055f,	-2.530499f,	-1.198261f,		0.3213f,	0.5703f,		-0.011f,	1.000f,	0.003f,	//21
		0.081432f,	-2.513204f,	-2.426005f,		0.3761f,	0.5172f,		-0.011f,	1.000f,	0.003f,	//22
		1.344182f,	-2.542571f,	-1.294387f,		0.4306f,	0.5662f,		-0.011f,	1.000f,	0.003f,	//23
		2.632201f,	-2.553892f,	0.007226f,		0.4863f,	0.6224f,		-0.011f,	1.000f,	0.003f,	//24
		1.385028f,	-2.549476f,	1.308677f,		0.4324f,	0.6787f,		-0.011f,	1.000f,	0.003f,	//25
		-0.140696f,	-2.601934f,	2.558561f,		0.3664f,	0.7328f,		-0.011f,	1.000f,	0.003f,	//26
		-1.328995f,	-2.578386f,	1.297838f,		0.3150f,	0.6783f,		-0.011f,	1.000f,	0.003f,	//27
		// cara trap
		0.081432f,	-2.513204f,	-2.426005f,		0.5137f,	0.5358f,		0.529f,	0.589f,	0.611f,	//28
		-1.185055f,	-2.530499f,	-1.198261f,		0.6240f,	0.5313f,		0.529f,	0.589f,	0.611f,	//29
		-2.517274f,	-2.579095f,	0.002115f,		0.7363f,	0.5275f,		0.529f,	0.589f,	0.611f,	//30
		-2.453980f,	-0.063388f,	-2.477451f,		0.6316f,	0.7225f,		0.529f,	0.589f,	0.611f,	//31
		// cara trap
		2.632201f,	-2.553892f,	0.007226f,		0.7637f,	0.5263f,		-0.538f,	0.577f,	0.615f,	//32
		1.344182f,	-2.542571f,	-1.294387f,		0.8791f,	0.5317f,		-0.538f,	0.577f,	0.615f,	//33
		0.081432f,	-2.513204f,	-2.426005f,		0.9863f,	0.5340f,		-0.538f,	0.577f,	0.615f,	//34
		2.534944f,	-0.061146f,	-2.580280f,		0.8760f,	0.7237f,		-0.538f,	0.577f,	0.615f,	//35
		// cara trap
		-0.140696f,	-2.601934f,	2.558561f,		0.0137f,	0.2812f,		-0.593f,	0.568f,	-0.570f,	//36
		1.385028f,	-2.549476f,	1.308677f,		0.1296f,	0.2916f,		-0.593f,	0.568f,	-0.570f,	//37
		2.632201f,	-2.553892f,	0.007226f,		0.2363f,	0.2913f,		-0.593f,	0.568f,	-0.570f,	//38
		2.593003f,	-0.085222f,	2.505755f,		0.1281f,	0.4688f,		-0.593f,	0.568f,	-0.570f,	//39
		// cara trap
		-2.517274f,	-2.579095f,	0.002115f,		0.2637f,	0.2793f,		0.593f,	0.589f,	-0.548f,	//40
		-1.328995f,	-2.578386f,	1.297838f,		0.3758f,	0.2796f,		0.593f,	0.589f,	-0.548f,	//41
		-0.140696f,	-2.601934f,	2.558561f,		0.4863f,	0.2778f,		0.593f,	0.589f,	-0.548f,	//42
		-2.646786f,	-0.139047f,	2.493757f,		0.3748f,	0.4722f,		0.593f,	0.589f,	-0.548f,	//43
		// cara tri
		-2.453980f,	-0.063388f,	-2.477451f,		0.6223f,	0.2775f,		0.574f,	-0.587f,	0.571f,	//44
		-2.451372f,	2.421759f,	0.077538f,		0.7363f,	0.4711f,		0.574f,	-0.587f,	0.571f,	//45
		0.049649f,	2.440128f,	-2.417217f,		0.5137f,	0.4725f,		0.574f,	-0.587f,	0.571f,	//46
		// cara tri
		2.534944f,	-0.061146f,	-2.580280f,		0.8819f,	0.2753f,		-0.566f,	-0.599f,	0.566f,	//47
		0.049649f,	2.440128f,	-2.417217f,		0.9863f,	0.4740f,		-0.566f,	-0.599f,	0.566f,	//48
		2.521206f,	2.448241f,	0.063388f,		0.7637f,	0.4747f,		-0.566f,	-0.599f,	0.566f,	//49
		// cara tri
		2.593003f,	-0.085222f,	2.505755f,		0.1306f,	0.0264f,		-0.573f,	-0.577f,	-0.582f,	//50
		2.521206f,	2.448241f,	0.063388f,		0.2363f,	0.2236f,		-0.573f,	-0.577f,	-0.582f,	//51
		0.051152f,	2.399781f,	2.545043f,		0.0137f,	0.2199f,		-0.573f,	-0.577f,	-0.582f,	//52
		// cara tri
		-2.646786f,	-0.139047f,	2.493757f,		0.3618f,	0.0246f,		0.565f,	-0.589f,	-0.578f,	//53
		0.051152f,	2.399781f,	2.545043f,		0.4863f,	0.2236f,		0.565f,	-0.589f,	-0.578f,	//54
		-2.451372f,	2.421759f,	0.077538f,		0.2637f,	0.2254f,		0.565f,	-0.589f,	-0.578f,	//55
	};

	MeshModel* holocron = new MeshModel();
	holocron->CreateMeshModel(holocron_vertices, holocron_indices, 448, 84);
	meshListModel.push_back(holocron);

}




int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	CreateObjects();
	CrearDado();
	CrearHolocron();
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
	dadoTexture = Texture("Textures/dado_starwars.png");
	dadoTexture.LoadTextureA();
	holocronTexture = Texture("Textures/holocron_jedi.png");
	holocronTexture.LoadTextureA();

	
	//Rover: se carga cada pieza por separado (Models/<pieza>.obj)
	for (int i = 0; i < kNumPartesRover; i++)
	{
		RoverPartes[i].LoadModel(std::string("Models/") + kPartesRover[i].archivo);
	}
	
	//Modelos de la práctica 6
	Dado_M.LoadModel("Models/dado_starwars.obj");
	Holocron_M.LoadModel("Models/holocron_texturizado.obj");
	Avion_M.LoadModel("Models/avion_rochelle.obj");

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

	//faro del avión hacia abajo (azul claro)
	spotLights[3] = SpotLight(0.5f, 0.7f, 1.0f,
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
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

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


		//movimiento del rover y del avión
		{
			bool* teclas = mainWindow.getsKeys();
			float paso = 0.05f * deltaTime;
			float dxRover = 0.0f;
			if (teclas[GLFW_KEY_I]) dxRover += paso; //avanzar (el frente del rover es +X)
			if (teclas[GLFW_KEY_K]) dxRover -= paso; //retroceder
			posRover.x += dxRover;
			giroLlantas -= (dxRover / (1.24f * 0.4f)) / toRadians; //las llantas ruedan con el avance (radio 1.24 * escala 0.4)

			glm::vec3 frenteAvion = DirMundo(MatrizAvion(), glm::vec3(0.0f, 0.0f, 1.0f)); //la nariz del avión es +Z
			if (teclas[GLFW_KEY_UP])   posAvion += frenteAvion * paso; //avanzar
			if (teclas[GLFW_KEY_DOWN]) posAvion -= frenteAvion * paso; //retroceder
			if (teclas[GLFW_KEY_O])    posAvion.y += paso;             //subir
			if (teclas[GLFW_KEY_L])    posAvion.y -= paso;             //bajar
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
		}

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);



		glm::mat4 model(1.0);
		glm::mat4 modelaux(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[2]->RenderMeshModel();
	
		//Rover con jerarquía
		//Teclas: Z/X base del brazo, C/V segmento 1, B/N segmento 2, M/, pinza, R/T ruedas
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
			glm::mat4 matRover[kNumPartesRover];
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			plainTexture.UseTexture();
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
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
				RoverPartes[i].RenderModel(uniformColor); //usa el color Kd de rover.mtl
			}
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		}


		//Dado Star Wars por código (práctica 6)
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-5.0f, 5.0f, 6.0));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		meshListModel[4]->RenderMeshModel();

		//Dado Star Wars importado (práctica 6)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-3.0f, 3.0f, 6.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Dado_M.RenderModel(uniformColor);

		//Holocron Jedi texturizado por código (práctica 6)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-7.0f, 7.0f, 6.0));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		holocronTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();

		//Holocron Jedi importado (práctica 6)
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-9.0f, 4.0f, 6.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Holocron_M.RenderModel(uniformColor);

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

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
