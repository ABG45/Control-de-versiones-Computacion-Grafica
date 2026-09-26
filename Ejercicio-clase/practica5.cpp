/*
Practica 5: Optimizacion y Carga de Modelos
Rover separado en 17 piezas + jerarquia + rotacion de patas limitada a +-45 grados

JERARQUIA
---------
  Cuerpo
	|- Pata (x6) ---- Rueda        <- la rueda cuelga de su pata y se mueve CON ella
	|- Brazo_Base -- Seg1 -- Seg2 -- Pinza

CONTROLES
---------
  W A S D / mouse .... camara
  P .................. VISTA EXPLOTADA: separa las patas del cuerpo (la rueda se va con su pata)
  1 2 3 4 5 6 ........ selecciona la pata (Del-Izq, Del-Der, Med-Izq, Med-Der, Tra-Izq, Tra-Der)
  R / F .............. gira la pata seleccionada (la rueda gira junto con ella), limite +-45
  T / G .............. hace rodar la rueda de esa pata sobre su propio eje (sin limite)
  M .................. marcha: las 6 patas se mecen y las ruedas ruedan al mismo tiempo
  Z / X .............. base del brazo (gira como torreta sobre Y)
  C / V .............. primer segmento del brazo
  B / N .............. segundo segmento del brazo
  K / L .............. pinza
  0 .................. regresa todo a cero
  ESC ................ salir
*/

//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <string>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

#include "jerarquia.h"   // tabla kPartesRover[] con padres, offsets y ejes

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;           //solo recibe xyz
std::vector<MeshColor*> meshListColor; // recibe xyz rgb
std::vector<MeshModel*> meshListModel; // recibe xyz uv nx ny nz
std::vector<Shader> shaderList;

Camera camera;

//====================== MEDIDAS DEL ROVER ======================
// Medidas sacadas de los .obj, con el pivote del Cuerpo en el origen:
//   ancho (X): 13.558    alto (Y): 7.536    largo (Z): 9.358
//   el punto mas bajo (las llantas) queda 4.986 abajo del pivote del Cuerpo
//   el techo del chasis queda 2.550 arriba del pivote del Cuerpo
//   el costado del chasis llega a 3.470 en Z
const GLfloat kBajoDelRover = -4.986f;  // punto mas bajo respecto al pivote del Cuerpo
const GLfloat kTechoChasis = 2.550f;   // techo del chasis respecto al pivote del Cuerpo

const GLfloat kAlturaPiso = -2.0f;   // el piso se dibuja a esta altura
const GLfloat kEscalaRover = 0.35f;   // que tan grande se ve el rover
const GLfloat kHolgura = 0.02f;   // para que no se encaje en el piso

// Altura a la que hay que poner el pivote del Cuerpo para que las llantas
// queden justo apoyadas en el piso:
const GLfloat kAlturaRover = kAlturaPiso - kBajoDelRover * kEscalaRover + kHolgura;

// El brazo viene parado aparte en el modelo original (a 13 unidades sobre -Z).
// Con esto lo montamos sobre el techo del chasis. Ponlo en false para dejarlo donde estaba.
const bool kMontarBrazoEnChasis = true;
const glm::vec3 kOffsetBrazoChasis = glm::vec3(3.5f, kTechoChasis, 0.0f);

//====================== VISTA EXPLOTADA ======================
// Cuanto se aleja cada pieza DE SU PADRE al presionar P, en unidades del modelo.
// Ojo con las ruedas: van en CERO a proposito, para que se queden pegadas a su
// pata y se separen junto con ella, no por su cuenta.
// Las patas se van hacia los costados (eje Z) y el brazo hacia arriba (eje Y),
// asi ninguna pieza se mete debajo del piso.
const GLfloat kSepPata = 4.5f;   // que tanto se abren las patas hacia los lados
const GLfloat kSepBrazo = 1.8f;   // que tanto se separa cada segmento del brazo

const glm::vec3 kSeparacion[17] = {
	glm::vec3(0.0f,        0.0f,  0.0f),        //  0 Cuerpo        (se queda quieto)
	glm::vec3(0.0f,  kSepBrazo,  0.0f),        //  1 Brazo_Base
	glm::vec3(0.0f,  kSepBrazo,  0.0f),        //  2 Brazo_Seg1
	glm::vec3(0.0f,  kSepBrazo,  0.0f),        //  3 Brazo_Seg2
	glm::vec3(0.0f,  kSepBrazo,  0.0f),        //  4 Brazo_Pinza
	glm::vec3(0.0f,       0.0f,  kSepPata),    //  5 Pata_Del_Der
	glm::vec3(0.0f,       0.0f,  0.0f),        //  6 Rueda_Del_Der   <- pegada a su pata
	glm::vec3(0.0f,       0.0f, -kSepPata),    //  7 Pata_Del_Izq
	glm::vec3(0.0f,       0.0f,  0.0f),        //  8 Rueda_Del_Izq   <- pegada a su pata
	glm::vec3(0.0f,       0.0f,  kSepPata),    //  9 Pata_Med_Der
	glm::vec3(0.0f,       0.0f,  0.0f),        // 10 Rueda_Med_Der   <- pegada a su pata
	glm::vec3(0.0f,       0.0f, -kSepPata),    // 11 Pata_Med_Izq
	glm::vec3(0.0f,       0.0f,  0.0f),        // 12 Rueda_Med_Izq   <- pegada a su pata
	glm::vec3(0.0f,       0.0f,  kSepPata),    // 13 Pata_Tra_Der
	glm::vec3(0.0f,       0.0f,  0.0f),        // 14 Rueda_Tra_Der   <- pegada a su pata
	glm::vec3(0.0f,       0.0f, -kSepPata),    // 15 Pata_Tra_Izq
	glm::vec3(0.0f,       0.0f,  0.0f),        // 16 Rueda_Tra_Izq   <- pegada a su pata
};

//====================== ESTADO DE LAS PIEZAS ======================
// Un Model por cada pieza. El indice coincide con kPartesRover[].
Model RoverParte[kNumPartesRover];

// Angulo actual de cada pieza, en grados.
// Patas y brazo se limitan a [-45, 45]; las ruedas giran libres sobre su eje.
GLfloat anguloParte[kNumPartesRover] = { 0.0f };

// Matriz final de cada pieza, ya con la jerarquia aplicada.
glm::mat4 matrizParte[kNumPartesRover];

// Indices de las 6 patas dentro de kPartesRover[], en el orden de las teclas 1..6
const int kPatas[6] = { 7, 5, 11, 9, 15, 13 };
//                      1  2   3  4   5   6
//                   DelIzq DelDer MedIzq MedDer TraIzq TraDer

// La rueda que cuelga de cada una de esas patas
const int kRuedas[6] = { 8, 6, 12, 10, 16, 14 };

const char* kNombrePata[6] = { "delantera izquierda", "delantera derecha",
							   "media izquierda",     "media derecha",
							   "trasera izquierda",   "trasera derecha" };

// Desfase de cada pata para que la marcha se vea alternada (tipo tripode)
const GLfloat kFasePata[6] = { 3.1416f, 0.0f, 0.0f, 3.1416f, 3.1416f, 0.0f };

int  pataActiva = 0;       // cual pata mueven las teclas R / F
bool marchaActiva = false;  // animacion automatica encendida o apagada
bool explotarActivo = false;  // vista explotada encendida o apagada
GLfloat factorExplosion = 0.0f;    // va de 0 (armado) a 1 (separado), para que se vea suave

//Lista de Skybox a crear
Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Vertex Shader
static const char* vShader = "shaders/shader_m.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_m.frag";


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

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);
}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


// Carga las 17 piezas desde la carpeta Models/
void CargarRover()
{
	for (int i = 0; i < kNumPartesRover; i++)
	{
		std::string ruta = std::string("Models/") + kPartesRover[i].archivo;
		RoverParte[i].LoadModel(ruta);
		printf("[%2d] %-16s padre: %s\n", i, kPartesRover[i].nombre,
			(kPartesRover[i].padre < 0) ? "(raiz)" : kPartesRover[kPartesRover[i].padre].nombre);
	}
	printf("\n%d piezas cargadas. P = vista explotada.\n\n", kNumPartesRover);
}


// True si la pieza es una de las 6 ruedas
bool EsRueda(int i)
{
	return strstr(kPartesRover[i].nombre, "Rueda") != NULL;
}


// Deja el angulo dentro de [min, max] que marca la tabla
GLfloat Limitar(GLfloat angulo, GLfloat minGrados, GLfloat maxGrados)
{
	if (angulo < minGrados) return minGrados;
	if (angulo > maxGrados) return maxGrados;
	return angulo;
}


// Devuelve el vector del eje local de giro de la pieza
glm::vec3 EjeDeGiro(char eje)
{
	if (eje == 'X') return glm::vec3(1.0f, 0.0f, 0.0f);
	if (eje == 'Y') return glm::vec3(0.0f, 1.0f, 0.0f);
	return glm::vec3(0.0f, 0.0f, 1.0f);   // Z por defecto
}


// Color con el que se pinta cada pieza
glm::vec3 ColorDeParte(int i)
{
	const char* nombre = kPartesRover[i].nombre;
	if (strstr(nombre, "Rueda") != NULL)  return glm::vec3(0.10f, 0.10f, 0.12f); // llantas negras
	if (strstr(nombre, "Pata") != NULL)   return glm::vec3(0.55f, 0.55f, 0.58f); // patas gris
	if (strstr(nombre, "Brazo") != NULL)  return glm::vec3(0.95f, 0.45f, 0.10f); // brazo naranja
	return glm::vec3(0.85f, 0.85f, 0.88f);                                       // cuerpo blanco
}


// Lee el teclado y actualiza los angulos de las piezas
void ActualizarAngulos(bool* keys, GLfloat dt)
{
	const GLfloat velocidad = 0.6f * dt;   // grados por cuadro, para patas y brazo
	const GLfloat velRueda = 2.0f * dt;   // las ruedas giran mas rapido

	// --- Vista explotada (solo al momento de presionar P) ---
	static bool teclaPAnterior = false;
	if (keys[GLFW_KEY_P] && !teclaPAnterior)
	{
		explotarActivo = !explotarActivo;
		printf(explotarActivo ? "Vista explotada: ENCENDIDA\n" : "Vista explotada: apagada\n");
	}
	teclaPAnterior = keys[GLFW_KEY_P];

	// El factor sube o baja poco a poco para que la separacion se vea suave
	GLfloat paso = 0.02f * dt;
	if (explotarActivo) factorExplosion += paso;
	else                factorExplosion -= paso;
	if (factorExplosion > 1.0f) factorExplosion = 1.0f;
	if (factorExplosion < 0.0f) factorExplosion = 0.0f;

	// --- Regresar todo a cero ---
	if (keys[GLFW_KEY_0])
	{
		for (int i = 0; i < kNumPartesRover; i++) anguloParte[i] = 0.0f;
		marchaActiva = false;
	}

	// --- Elegir cual pata se mueve con R / F ---
	static int pataAnterior = -1;
	if (keys[GLFW_KEY_1]) pataActiva = 0;
	if (keys[GLFW_KEY_2]) pataActiva = 1;
	if (keys[GLFW_KEY_3]) pataActiva = 2;
	if (keys[GLFW_KEY_4]) pataActiva = 3;
	if (keys[GLFW_KEY_5]) pataActiva = 4;
	if (keys[GLFW_KEY_6]) pataActiva = 5;
	if (pataActiva != pataAnterior)
	{
		printf("Pata seleccionada: %s   (R/F la gira, T/G rueda su llanta)\n",
			kNombrePata[pataActiva]);
		pataAnterior = pataActiva;
	}

	// --- Prender o apagar la marcha (solo al momento de presionar) ---
	static bool teclaMAnterior = false;
	if (keys[GLFW_KEY_M] && !teclaMAnterior)
	{
		marchaActiva = !marchaActiva;
		printf(marchaActiva ? "Marcha: ENCENDIDA\n" : "Marcha: apagada\n");
	}
	teclaMAnterior = keys[GLFW_KEY_M];

	if (marchaActiva)
	{
		// Las 6 patas se mecen con su desfase, sin pasar de 45 grados,
		// y al mismo tiempo las 6 ruedas van rodando sobre su propio eje.
		GLfloat tiempo = (GLfloat)glfwGetTime();
		for (int p = 0; p < 6; p++)
		{
			int pata = kPatas[p];
			int rueda = kRuedas[p];

			anguloParte[pata] = 45.0f * sinf(tiempo * 2.0f + kFasePata[p]);
			anguloParte[pata] = Limitar(anguloParte[pata],
				kPartesRover[pata].minGrados, kPartesRover[pata].maxGrados);

			anguloParte[rueda] -= velRueda * 3.0f;   // la llanta rueda, sin limite
		}
	}
	else
	{
		// Movimiento manual de la pata seleccionada.
		// La rueda cuelga de la pata, asi que gira junto con ella sin hacer nada extra.
		int pata = kPatas[pataActiva];
		int rueda = kRuedas[pataActiva];

		if (keys[GLFW_KEY_R]) anguloParte[pata] += velocidad;
		if (keys[GLFW_KEY_F]) anguloParte[pata] -= velocidad;
		anguloParte[pata] = Limitar(anguloParte[pata],
			kPartesRover[pata].minGrados, kPartesRover[pata].maxGrados);

		// Y si ademas quieres que la llanta ruede sobre su propio eje:
		if (keys[GLFW_KEY_T]) anguloParte[rueda] += velRueda;
		if (keys[GLFW_KEY_G]) anguloParte[rueda] -= velRueda;
	}

	// --- Brazo: base (1), segmento 1 (2), segmento 2 (3) y pinza (4) ---
	if (keys[GLFW_KEY_Z]) anguloParte[1] += velocidad;
	if (keys[GLFW_KEY_X]) anguloParte[1] -= velocidad;
	if (keys[GLFW_KEY_C]) anguloParte[2] += velocidad;
	if (keys[GLFW_KEY_V]) anguloParte[2] -= velocidad;
	if (keys[GLFW_KEY_B]) anguloParte[3] += velocidad;
	if (keys[GLFW_KEY_N]) anguloParte[3] -= velocidad;
	if (keys[GLFW_KEY_K]) anguloParte[4] += velocidad;
	if (keys[GLFW_KEY_L]) anguloParte[4] -= velocidad;

	for (int i = 1; i <= 4; i++)
		anguloParte[i] = Limitar(anguloParte[i],
			kPartesRover[i].minGrados, kPartesRover[i].maxGrados);
}


// Calcula la matriz de cada pieza aplicando la jerarquia y dibuja el rover.
// modelBase es donde queda parado el rover dentro de la escena.
void DibujarRover(glm::mat4 modelBase, GLuint uniformModel, GLuint uniformColor)
{
	for (int i = 0; i < kNumPartesRover; i++)
	{
		const ParteRover& parte = kPartesRover[i];

		// De quien cuelga esta pieza
		glm::mat4 matrizPadre = (parte.padre < 0) ? modelBase : matrizParte[parte.padre];

		// Offset del pivote respecto al padre
		glm::vec3 offset(parte.offset[0], parte.offset[1], parte.offset[2]);
		if (parte.padre < 0) offset = glm::vec3(0.0f); // el cuerpo queda en el origen
		if (kMontarBrazoEnChasis && strcmp(parte.nombre, "Brazo_Base") == 0)
			offset = kOffsetBrazoChasis;               // el brazo montado sobre el chasis

		// La separacion de la vista explotada se suma al offset, o sea que se
		// aplica DENTRO de la jerarquia. Por eso los hijos se van con su padre:
		// la rueda tiene separacion cero, asi que sigue pegada a su pata y se
		// separa del cuerpo junto con ella.
		offset += kSeparacion[i] * factorExplosion;

		// M_pieza = M_padre * Traslacion(offset) * Rotacion(eje, angulo)
		glm::mat4 m = matrizPadre;
		m = glm::translate(m, offset);
		if (parte.eje != '\0' && anguloParte[i] != 0.0f)
			m = glm::rotate(m, anguloParte[i] * toRadians, EjeDeGiro(parte.eje));

		matrizParte[i] = m;   // los hijos cuelgan de aqui

		// Dibujar la pieza
		glm::vec3 color = ColorDeParte(i);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
		RoverParte[i].RenderModel();
	}
}


int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 2.0f, 12.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -10.0f, 0.3f, 0.3f);

	//Cargar el rover pieza por pieza
	CargarRover();

	//Crear Skybox con sus 6 texturas
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0, uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = (GLfloat)glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / (GLfloat)limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//Mover las patas, las ruedas y el brazo
		ActualizarAngulos(mainWindow.getsKeys(), deltaTime);

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Se dibuja el Skybox
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// INICIA DIBUJO DEL PISO
		color = glm::vec3(0.5f, 0.5f, 0.5f); //piso de color gris
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, kAlturaPiso, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshListModel[2]->RenderMeshModel();

		//------------ INICIA DIBUJO DEL ROVER POR JERARQUIA ------------
		// La altura sale de la medida del modelo, para que las llantas
		// queden apoyadas en el piso y no enterradas.
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, kAlturaRover, 0.0f));
		model = glm::scale(model, glm::vec3(kEscalaRover, kEscalaRover, kEscalaRover));
		DibujarRover(model, uniformModel, uniformColor);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}