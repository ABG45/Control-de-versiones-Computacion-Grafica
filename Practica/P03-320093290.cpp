//pr�ctica 3: Modelado Geom�trico y C�mara Sint�tica.
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
vector<MeshColor*> colorMeshList; //mallas con color distinto por cada vertice/cara (diamante de la cruz de 4 brazos)
vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks




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

//Un "diamante" = 2 piramides cuadrangulares unidas base-con-base (bipiramide).
//El apice interno queda FIJO en el origen (0,0,0) -> ese es el punto que se
//comparte con los otros 3 diamantes para formar la cruz de 4 brazos.
//El apice externo queda en (0,2,0) -> la punta libre que apunta hacia afuera.
//La base cuadrada compartida esta en y=1 y solo se dibuja una vez (queda oculta
//entre las dos piramides, dibujarla dos veces solo causaria parpadeo/z-fighting).
//IMPORTANTE: la base mide un poco mas que 1x1 (de -0.7071 a 0.7071, es decir
//1/raiz(2) de semi-ancho), a proposito: con altura=1 y semi-ancho=1/raiz(2)
//cada arista queda exactamente a 45 grados de su eje. Ese es el angulo exacto
//que se necesita porque, en una cruz de 4 brazos (los ejes estan a 90 grados
//entre si), la direccion que conecta a 2 brazos vecinos es su bisectriz, que
//tambien esta a 45 grados de cada uno de esos 2 ejes.
//Cada piramide (interna y externa) usa el mismo esquema de color:
//frontal=roja, izquierda=verde, trasera=amarilla, derecha=magenta, base=azul.
void CrearDiamanteColor()
{
	GLfloat diamantecolor_vertices[] = {
		//pos.x, pos.y, pos.z,		color.r, color.g, color.b

		//========== Piramide interna (apice en el origen) ==========
		//cara frontal (roja)
		 0.70710678f, 1.0f,  0.70710678f,		1.0f, 0.0f, 0.0f,
		-0.70710678f, 1.0f,  0.70710678f,		1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f,  0.0f,		1.0f, 0.0f, 0.0f,

		 //cara izquierda (verde)
		 -0.70710678f, 1.0f,  0.70710678f,		0.0f, 1.0f, 0.0f,
		 -0.70710678f, 1.0f, -0.70710678f,		0.0f, 1.0f, 0.0f,
		  0.0f, 0.0f,  0.0f,		0.0f, 1.0f, 0.0f,

		  //cara trasera (amarilla)
		  -0.70710678f, 1.0f, -0.70710678f,		1.0f, 1.0f, 0.0f,
		   0.70710678f, 1.0f, -0.70710678f,		1.0f, 1.0f, 0.0f,
		   0.0f, 0.0f,  0.0f,		1.0f, 1.0f, 0.0f,

		   //cara derecha (magenta)
			0.70710678f, 1.0f, -0.70710678f,		1.0f, 0.0f, 1.0f,
			0.70710678f, 1.0f,  0.70710678f,		1.0f, 0.0f, 1.0f,
			0.0f, 0.0f,  0.0f,		1.0f, 0.0f, 1.0f,

			//========== Base compartida (azul), se dibuja una sola vez ==========
			 0.70710678f, 1.0f,  0.70710678f,		0.0f, 0.0f, 1.0f,
			 0.70710678f, 1.0f, -0.70710678f,		0.0f, 0.0f, 1.0f,
			-0.70710678f, 1.0f, -0.70710678f,		0.0f, 0.0f, 1.0f,

			 0.70710678f, 1.0f,  0.70710678f,		0.0f, 0.0f, 1.0f,
			-0.70710678f, 1.0f, -0.70710678f,		0.0f, 0.0f, 1.0f,
			-0.70710678f, 1.0f,  0.70710678f,		0.0f, 0.0f, 1.0f,

			//========== Piramide externa (apice en (0,2,0)) ==========
			//cara frontal (roja)
			 0.70710678f, 1.0f,  0.70710678f,		1.0f, 0.0f, 0.0f,
			-0.70710678f, 1.0f,  0.70710678f,		1.0f, 0.0f, 0.0f,
			 0.0f, 2.0f,  0.0f,		1.0f, 0.0f, 0.0f,

			 //cara izquierda (verde)
			 -0.70710678f, 1.0f,  0.70710678f,		0.0f, 1.0f, 0.0f,
			 -0.70710678f, 1.0f, -0.70710678f,		0.0f, 1.0f, 0.0f,
			  0.0f, 2.0f,  0.0f,		0.0f, 1.0f, 0.0f,

			  //cara trasera (amarilla)
			  -0.70710678f, 1.0f, -0.70710678f,		1.0f, 1.0f, 0.0f,
			   0.70710678f, 1.0f, -0.70710678f,		1.0f, 1.0f, 0.0f,
			   0.0f, 2.0f,  0.0f,		1.0f, 1.0f, 0.0f,

			   //cara derecha (magenta)
				0.70710678f, 1.0f, -0.70710678f,		1.0f, 0.0f, 1.0f,
				0.70710678f, 1.0f,  0.70710678f,		1.0f, 0.0f, 1.0f,
				0.0f, 2.0f,  0.0f,		1.0f, 0.0f, 1.0f,
	};

	MeshColor* diamantecolor = new MeshColor();
	diamantecolor->CreateMeshColor(diamantecolor_vertices, 30); //30 vertices reales (10 triangulos x 3)
	colorMeshList.push_back(diamantecolor);
}

//Calcula la matriz de rotacion que lleva el eje +Y local hacia la direccion dada.
//Se usa para orientar cada diamante de la cruz hacia una direccion distinta.
glm::mat4 RotacionDesdeYHacia(glm::vec3 direccion)
{
	glm::vec3 ejeY(0.0f, 1.0f, 0.0f);
	direccion = glm::normalize(direccion);
	float cosAngulo = glm::dot(ejeY, direccion);

	if (cosAngulo > 0.9999f)
		return glm::mat4(1.0f); //ya apunta hacia +Y, no hace falta rotar

	if (cosAngulo < -0.9999f) //apunta exactamente hacia -Y
		return glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glm::vec3 eje = glm::normalize(glm::cross(ejeY, direccion));
	float angulo = acos(cosAngulo);
	return glm::rotate(glm::mat4(1.0f), angulo, eje);
}

//Calcula el angulo (en radianes) que hay que girar alrededor de 'eje' para llevar
//el vector 'desde' exactamente sobre el vector 'hacia'. Ambos vectores deben estar
//al mismo angulo respecto a 'eje' (mismo "cono") para que el giro los empalme bien;
//si no, el resultado los deja lo mas cerca posible pero no exactos.
//Se usa para calcular el giro extra que hace que una arista de cada diamante
//quede unida exactamente con la de su diamante pareja.
float AnguloAlrededorDeEje(glm::vec3 desde, glm::vec3 hacia, glm::vec3 eje)
{
	eje = glm::normalize(eje);
	//se proyectan ambos vectores sobre el plano perpendicular al eje
	glm::vec3 desdePerp = glm::normalize(desde - eje * glm::dot(desde, eje));
	glm::vec3 haciaPerp = glm::normalize(hacia - eje * glm::dot(hacia, eje));

	float coseno = glm::dot(desdePerp, haciaPerp);
	float seno = glm::dot(eje, glm::cross(desdePerp, haciaPerp));
	return atan2(seno, coseno);
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

//Dibuja el cohete espacial usando SOLO instancias de las mallas que ya existen
//en meshList y la esfera global sp -- no crea geometria nueva, solo manda cada
//malla varias veces con una matriz de modelo distinta (traslacion/rotacion/
//escala), que es la idea de "instanciar" primitivas.
//
//'modeloBase' es la transformacion del cohete completo (posicion en la escena
//+ las rotaciones de las teclas E/R/T); cada parte se ubica en coordenadas
//LOCALES relativas al cohete y luego se multiplica por modeloBase, para que
//el cohete se mueva/rote como un solo objeto rigido.
//
//Indices en meshList: 0=cubo, 1=piramide triangular, 2=cilindro, 3=cono,
//4=piramide cuadrangular.
void DibujarCohete(glm::mat4 modeloBase, GLuint uniformModel, GLuint uniformColor)
{
	glm::mat4 modelo;
	glm::vec3 color;

	//---------- Cuerpo: cilindro ----------
	//el cilindro base tiene radio 1 y altura 1 (centrado en el origen);
	//lo escalamos a radio 0.5 y altura 3.
	modelo = glm::scale(modeloBase, glm::vec3(0.5f, 3.0f, 0.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelo));
	color = glm::vec3(0.85f, 0.85f, 0.9f); //blanco grisaceo
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[2]->RenderMeshGeometry();

	//---------- Nariz: cono ----------
	//el cono tiene radio 2 horneado en su malla (apice arriba, base abajo);
	//lo escalamos a radio 0.5 (factor 0.25) y lo subimos hasta la punta del cuerpo.
	modelo = glm::translate(modeloBase, glm::vec3(0.0f, 2.0f, 0.0f));
	modelo = glm::scale(modelo, glm::vec3(0.25f, 1.0f, 0.25f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelo));
	color = glm::vec3(0.8f, 0.1f, 0.1f); //rojo
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[3]->RenderMeshGeometry();

	//---------- Tobera del motor: otra instancia del cilindro, mas chica ----------
	modelo = glm::translate(modeloBase, glm::vec3(0.0f, -1.7f, 0.0f));
	modelo = glm::scale(modelo, glm::vec3(0.35f, 0.4f, 0.35f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelo));
	color = glm::vec3(0.3f, 0.3f, 0.3f); //gris oscuro
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[2]->RenderMeshGeometry();

	//---------- Ojo de buey (ventana): esfera ----------
	modelo = glm::translate(modeloBase, glm::vec3(0.0f, 0.5f, 0.55f));
	modelo = glm::scale(modelo, glm::vec3(0.2f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelo));
	color = glm::vec3(0.3f, 0.6f, 0.9f); //azul cielo
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	sp.render();

	//---------- Panel/modulo lateral: cubo ----------
	modelo = glm::translate(modeloBase, glm::vec3(0.0f, -0.5f, 0.55f));
	modelo = glm::scale(modelo, glm::vec3(0.3f, 0.3f, 0.1f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelo));
	color = glm::vec3(0.9f, 0.7f, 0.1f); //amarillo/dorado
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	meshList[0]->RenderMesh();

	//---------- Aletas: 4 instancias de la piramide triangular alrededor de la base ----------
	color = glm::vec3(0.8f, 0.1f, 0.1f); //rojo, igual que la nariz
	glUniform3fv(uniformColor, 1, glm::value_ptr(color));
	for (int i = 0; i < 4; i++)
	{
		float angulo = glm::radians(90.0f * i);
		modelo = glm::rotate(modeloBase, angulo, glm::vec3(0.0f, 1.0f, 0.0f));
		modelo = glm::translate(modelo, glm::vec3(0.0f, -1.4f, 0.55f)); //empuja la aleta hacia afuera del cuerpo
		modelo = glm::scale(modelo, glm::vec3(0.5f, 0.8f, 0.25f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelo));
		meshList[1]->RenderMesh();
	}
}



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
	glEnable(GL_DEPTH_TEST); //asegura que las caras se dibujen respetando la profundidad (por si Window::Initialise no lo hace ya)
	//Cilindro y cono reciben resoluci�n (slices, rebanadas) y Radio de circunferencia de la base y tapa

	CrearCubo();//�ndice 0 en MeshList
	CrearPiramideTriangular();//�ndice 1 en MeshList
	CrearCilindro(10, 1.0f);//�ndice 2 en MeshList
	CrearCono(25, 2.0f);//�ndice 3 en MeshList
	CrearPiramideCuadrangular();//�ndice 4 en MeshList
	CrearDiamanteColor();//indice 0 en colorMeshList (diamante = 2 piramides base con base, para la estrella de 4 brazos)
	CreateShaders();



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

	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f); //inicializar Color para enviar a variable Uniform;

	//---- Control de movimiento con las flechas para el cohete y la cruz ----
	//MOVER_AMBAS: las flechas mueven las 2 figuras juntas (modo por defecto)
	//MOVER_COHETE / MOVER_CRUZ: las flechas solo mueven esa figura
	//TAB alterna entre los 3 modos, en ese orden
	enum ModoMovimiento { MOVER_AMBAS, MOVER_COHETE, MOVER_CRUZ };
	ModoMovimiento modoMovimiento = MOVER_AMBAS;
	bool tabPresionadoAntes = false; //para detectar el momento en que se presiona TAB, no mientras se mantiene
	glm::vec3 desplazamientoCohete(0.0f, 0.0f, 0.0f);
	glm::vec3 desplazamientoCruz(0.0f, 0.0f, 0.0f);
	const GLfloat velocidadMovimiento = 2.0f; //unidades por segundo

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

		//---- Movimiento de las figuras con las flechas ----
		bool* teclas = mainWindow.getsKeys();

		//TAB alterna el modo (deteccion de flanco: solo cambia en el instante
		//en que se presiona, no se queda ciclando mientras se mantiene oprimida)
		if (teclas[GLFW_KEY_TAB] && !tabPresionadoAntes)
		{
			modoMovimiento = (ModoMovimiento)((modoMovimiento + 1) % 3);
			if (modoMovimiento == MOVER_AMBAS)  printf("Modo de movimiento: AMBAS figuras\n");
			if (modoMovimiento == MOVER_COHETE) printf("Modo de movimiento: solo el COHETE\n");
			if (modoMovimiento == MOVER_CRUZ)   printf("Modo de movimiento: solo la CRUZ\n");
		}
		tabPresionadoAntes = teclas[GLFW_KEY_TAB];

		glm::vec3 movimiento(0.0f, 0.0f, 0.0f);
		if (teclas[GLFW_KEY_LEFT])  movimiento.x -= velocidadMovimiento * deltaTime;
		if (teclas[GLFW_KEY_RIGHT]) movimiento.x += velocidadMovimiento * deltaTime;
		if (teclas[GLFW_KEY_UP])    movimiento.y += velocidadMovimiento * deltaTime;
		if (teclas[GLFW_KEY_DOWN])  movimiento.y -= velocidadMovimiento * deltaTime;

		if (modoMovimiento == MOVER_AMBAS || modoMovimiento == MOVER_COHETE)
			desplazamientoCohete += movimiento;
		if (modoMovimiento == MOVER_AMBAS || modoMovimiento == MOVER_CRUZ)
			desplazamientoCruz += movimiento;

		//Limpiar la ventana
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Se agrega limpiar el buffer de profundidad
		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		model = glm::mat4(1.0);
		//Rotacion de las teclas E/R/T: se calcula una sola vez y se reutiliza tanto
		//para el cohete como para la cruz, asi las 2 figuras giran igual con esas teclas.
		glm::mat4 rotacionERT = glm::mat4(1.0f);
		rotacionERT = glm::rotate(rotacionERT, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
		rotacionERT = glm::rotate(rotacionERT, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));  //al presionar la tecla R se rota sobre el eje y
		rotacionERT = glm::rotate(rotacionERT, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f));

		//Traslaci�n inicial para posicionar en -Z a los objetos
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0f));
		model = glm::translate(model, desplazamientoCohete); //movimiento acumulado con las flechas
		//otras transformaciones para el objeto
		//model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
		model = model * rotacionERT;
		//la l�nea de proyecci�n solo se manda una vez a menos que en tiempo de ejecuci�n
		//se programe cambio entre proyecci�n ortogonal y perspectiva
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		//=========================================================
		//Figura de 8 piramides = 4 "diamantes" (cada diamante = 2 piramides
		//unidas base con base, formando una bipiramide de 2 puntas).
		//colorMeshList[0] YA es un diamante completo (las 2 piramides + su
		//base compartida ya estan horneadas en una sola malla, con el apice
		//interno en el origen local (0,0,0) y el apice externo en (0,2,0)).
		//
		//Los 4 diamantes se acomodan en CRUZ: sus ejes son +X, +Y, -X, -Y
		//(90 grados entre cada uno y el siguiente). Ademas de compartir el
		//vertice central, cada diamante se GIRA sobre su propio eje para que
		//DOS de sus aristas queden perfectamente unidas: una con el brazo
		//siguiente y otra con el brazo anterior. Esto SI es exacto (a
		//diferencia de un acomodo tetraedrico): como los brazos estan a 90
		//grados, la conexion entre 2 vecinos cae justo a 45 grados de cada
		//uno, que es el mismo angulo al que quedan las aristas de la
		//piramide (ver el ancho de base en CrearDiamanteColor).
		//=========================================================
		shaderList[1].useShader();
		GLuint uniformModelColor = shaderList[1].getModelLocation();
		GLuint uniformProjectionColor = shaderList[1].getProjectLocation();
		GLuint uniformViewColor = shaderList[1].getViewLocation();

		glUniformMatrix4fv(uniformProjectionColor, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformViewColor, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		float escalaDiamante = 0.6f; //tamano de cada brazo/diamante de la cruz

		//Los 4 brazos en cruz: +X, +Y, -X, -Y, en ese orden ciclico (cada uno
		//es vecino del siguiente y del anterior en esta lista; el arreglo
		//"da la vuelta" completa: +X -> +Y -> -X -> -Y -> +X).
		glm::vec3 direccionesCruz[4] = {
			glm::vec3(1.0f,  0.0f,  0.0f), //brazo 0: +X
			glm::vec3(0.0f,  1.0f,  0.0f), //brazo 1: +Y
			glm::vec3(-1.0f,  0.0f,  0.0f), //brazo 2: -X
			glm::vec3(0.0f, -1.0f,  0.0f)  //brazo 3: -Y
		};
		glm::vec3 posicionCruz = glm::vec3(3.0f, 0.0f, -4.0f) + desplazamientoCruz; //base + movimiento acumulado con las flechas

		//Una de las 4 aristas de la piramide (la de la cara frontal/roja), en espacio local.
		//OJO: debe coincidir con el ancho de base usado en CrearDiamanteColor (1/raiz(2)).
		glm::vec3 aristaLocalDeReferencia = glm::normalize(glm::vec3(0.70710678f, 1.0f, 0.70710678f));

		for (int i = 0; i < 4; i++)
		{
			glm::vec3 direccion = glm::normalize(direccionesCruz[i]);
			glm::mat4 rotacionBase = RotacionDesdeYHacia(direccion); //orienta el diamante hacia este brazo

			//El brazo vecino (siguiente en la cruz). La direccion exacta a la
			//que debe alinearse una de las aristas es la bisectriz entre este
			//brazo y su vecino; como estan a 90 grados, esa bisectriz esta a
			//45 de cada uno, que es justo el angulo al que quedaron las aristas.
			glm::vec3 vecino = glm::normalize(direccionesCruz[(i + 1) % 4]);
			glm::vec3 objetivoExacto = glm::normalize(direccion + vecino);

			//A donde cae la arista de referencia si no se aplica ningun giro extra
			glm::vec3 aristaSinGirar = glm::normalize(glm::vec3(rotacionBase * glm::vec4(aristaLocalDeReferencia, 0.0f)));

			//Giro extra (alrededor de este mismo eje) para que esa arista caiga
			//exactamente sobre la de su vecino. Por la simetria de 90 grados de
			//la cruz, esto autom�ticamente deja OTRA arista de este mismo
			//diamante tambien exacta con el vecino del otro lado.
			float giroDeEmpalme = AnguloAlrededorDeEje(aristaSinGirar, objetivoExacto, direccion);

			glm::mat4 modelo = glm::mat4(1.0f);
			modelo = glm::translate(modelo, posicionCruz); //el punto central compartido de la cruz
			modelo = modelo * rotacionERT; //rotacion de las teclas E/R/T, igual que el cohete (gira la cruz completa alrededor de su propio centro)
			modelo = modelo * rotacionBase;
			modelo = glm::rotate(modelo, giroDeEmpalme, glm::vec3(0.0f, 1.0f, 0.0f)); //el giro de empalme, en espacio local
			modelo = glm::scale(modelo, glm::vec3(escalaDiamante));

			glUniformMatrix4fv(uniformModelColor, 1, GL_FALSE, glm::value_ptr(modelo));
			colorMeshList[0]->RenderMeshColor();
		}
		glUseProgram(0);

		shaderList[0].useShader(); //se regresa al shader normal para seguir dibujando las demas figuras
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		//El cohete se dibuja usando 'model' (la transformacion con las teclas
		//E/R/T) como base, asi que se mueve/rota como un solo objeto.
		DibujarCohete(model, uniformModel, uniformColor);

		/*
		//ejercicio: Instanciar primitivas geom�tricas para recrear las figuras 2 y 3 de la pr�ctica pasada en 3D,
		//se requiere que exista piso
		*/

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}