#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	rotax = 0.0f; rotay = 0.0f; rotaz = 0.0f;
	articulacion1 = 0.0f; articulacion2 = 0.0f; articulacion3 = 0.0f;
	articulacion4 = 0.0f; articulacion5 = 0.0f; articulacion6 = 0.0f;
	for (int a = 0; a < 3; a++)
		for (int e = 0; e < 3; e++) brazoRot[a][e] = 0.0f;
	baseBrazo = 0.0f; pinza = 0.0f; articulacionSel = 0;
	for (int i = 0; i < 6; i++) llantaRot[i] = 0.0f;
	llantaSel = 0;
	ejeFrontal = 0.0f; ejeBarraPata = 0.0f; ejeLL2 = 0.0f; ejeLL3 = 0.0f;
	panelSolar = 0.0f; antenaY = 0.0f; antenaX = 0.0f;
	xChange = 0.0f; yChange = 0.0f; lastX = 0.0f; lastY = 0.0f;
	mouseFirstMoved = true;
	for (size_t i = 0; i < 1024; i++) keys[i] = 0;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	rotax = 0.0f; rotay = 0.0f; rotaz = 0.0f;
	articulacion1 = 0.0f; articulacion2 = 0.0f; articulacion3 = 0.0f;
	articulacion4 = 0.0f; articulacion5 = 0.0f; articulacion6 = 0.0f;
	for (int a = 0; a < 3; a++)
		for (int e = 0; e < 3; e++) brazoRot[a][e] = 0.0f;
	baseBrazo = 0.0f; pinza = 0.0f; articulacionSel = 0;
	for (int i = 0; i < 6; i++) llantaRot[i] = 0.0f;
	llantaSel = 0;
	ejeFrontal = 0.0f; ejeBarraPata = 0.0f; ejeLL2 = 0.0f; ejeLL3 = 0.0f;
	panelSolar = 0.0f; antenaY = 0.0f; antenaX = 0.0f;
	/*Inicializaciones que faltaban en la version original: sin esto la camara
	pega un brinco en el primer movimiento del mouse*/
	xChange = 0.0f; yChange = 0.0f; lastX = 0.0f; lastY = 0.0f;
	mouseFirstMoved = true;
	for (size_t i = 0; i < 1024; i++) keys[i] = 0;
}

int Window::Initialise()
{
	//Inicializacion de GLFW
	if (!glfwInit())
	{
		printf("Fallo inicializar GLFW");
		glfwTerminate();
		return 1;
	}
	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	mainWindow = glfwCreateWindow(width, height, "Practica 4: Modelado Jerarquico - Rover", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamano de Buffer
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow);

	//MANEJAR TECLADO y MOUSE
	createCallbacks();

	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Fallo inicializacion de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST); //HABILITAR BUFFER DE PROFUNDIDAD

	//Asignar Viewport
	glViewport(0, 0, bufferWidth, bufferHeight);
	//Callback para detectar que se esta usando la ventana
	glfwSetWindowUserPointer(mainWindow, this);

	return 0; //faltaba el return en la version original
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//========== SELECTORES (solo al presionar) ==========
	if (action == GLFW_PRESS)
	{
		//teclas 1,2,3: cual articulacion del brazo se va a mover
		if (key == GLFW_KEY_1) { theWindow->articulacionSel = 0; printf("Articulacion: HOMBRO (EJE BB-BR1)\n"); }
		if (key == GLFW_KEY_2) { theWindow->articulacionSel = 1; printf("Articulacion: CODO (EJE BR1-BR2)\n"); }
		if (key == GLFW_KEY_3) { theWindow->articulacionSel = 2; printf("Articulacion: MUNECA (EJE BR2-PINZA)\n"); }
		//teclas 4..9: cual llanta se va a girar
		if (key == GLFW_KEY_4) { theWindow->llantaSel = 0; printf("Llanta: DELANTERA CERCA\n"); }
		if (key == GLFW_KEY_5) { theWindow->llantaSel = 1; printf("Llanta: MEDIA CERCA\n"); }
		if (key == GLFW_KEY_6) { theWindow->llantaSel = 2; printf("Llanta: TRASERA CERCA\n"); }
		if (key == GLFW_KEY_7) { theWindow->llantaSel = 3; printf("Llanta: DELANTERA LEJOS\n"); }
		if (key == GLFW_KEY_8) { theWindow->llantaSel = 4; printf("Llanta: MEDIA LEJOS\n"); }
		if (key == GLFW_KEY_9) { theWindow->llantaSel = 5; printf("Llanta: TRASERA LEJOS\n"); }
	}

	/*Se ignora el evento de soltar la tecla: antes el angulo se incrementaba dos
	veces por cada pulsacion (al presionar y al soltar).
	Si se mantiene presionado SHIFT el giro se hace en sentido contrario.*/
	if (action != GLFW_RELEASE)
	{
		GLfloat paso = (mode & GLFW_MOD_SHIFT) ? -5.0f : 5.0f;
		int art = theWindow->articulacionSel;
		int ll = theWindow->llantaSel;

		//----- rotacion global del rover -----
		if (key == GLFW_KEY_E) theWindow->rotax += paso;
		if (key == GLFW_KEY_R) theWindow->rotay += paso;
		if (key == GLFW_KEY_T) theWindow->rotaz += paso;

		//----- brazo: la articulacion seleccionada gira en X, Y o Z -----
		if (key == GLFW_KEY_F) theWindow->brazoRot[art][0] += paso; //eje X
		if (key == GLFW_KEY_G) theWindow->brazoRot[art][1] += paso; //eje Y
		if (key == GLFW_KEY_H) theWindow->brazoRot[art][2] += paso; //eje Z
		if (key == GLFW_KEY_J) theWindow->baseBrazo += paso;        //EJE BRAZO (gira todo el brazo)

		//----- pinza -----
		if (key == GLFW_KEY_C)
		{
			theWindow->pinza += 3.0f;
			if (theWindow->pinza > 35.0f) theWindow->pinza = 35.0f;
		}
		if (key == GLFW_KEY_V)
		{
			theWindow->pinza -= 3.0f;
			if (theWindow->pinza < 0.0f) theWindow->pinza = 0.0f;
		}

		//----- llantas -----
		if (key == GLFW_KEY_K) theWindow->llantaRot[ll] += paso * 2.0f;   //solo la seleccionada
		if (key == GLFW_KEY_L)                                           //las 6 al mismo tiempo
		{
			for (int i = 0; i < 6; i++) theWindow->llantaRot[i] += paso * 2.0f;
		}

		//----- suspension -----
		if (key == GLFW_KEY_B) theWindow->ejeFrontal += paso;
		if (key == GLFW_KEY_N) theWindow->ejeBarraPata += paso;
		if (key == GLFW_KEY_M) theWindow->ejeLL2 += paso;
		if (key == GLFW_KEY_Y) theWindow->ejeLL3 += paso;

		//----- sonda espacial (version 2) -----
		if (key == GLFW_KEY_O) theWindow->panelSolar += paso;
		if (key == GLFW_KEY_P) theWindow->antenaY += paso;
		if (key == GLFW_KEY_I) theWindow->antenaX += paso;

		//se conservan las articulaciones originales por compatibilidad
		if (key == GLFW_KEY_U) theWindow->articulacion1 += paso;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS) theWindow->keys[key] = true;
		else if (action == GLFW_RELEASE) theWindow->keys[key] = false;
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}


Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
