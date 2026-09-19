#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	GLfloat getrotay() { return rotay; }
	GLfloat getrotax() { return rotax; }
	GLfloat getrotaz() { return rotaz; }
	GLfloat getarticulacion1() { return articulacion1; }
	GLfloat getarticulacion2() { return articulacion2; }
	GLfloat getarticulacion3() { return articulacion3; }
	GLfloat getarticulacion4() { return articulacion4; }
	GLfloat getarticulacion5() { return articulacion5; }
	GLfloat getarticulacion6() { return articulacion6; }

	/*===== ARTICULACIONES DEL BRAZO =====
	brazoRot[articulacion][eje]
	  articulacion: 0 = EJE BB-BR1 (hombro), 1 = EJE BR1-BR2 (codo), 2 = EJE BR2-PINZA (muneca)
	  eje:          0 = X, 1 = Y, 2 = Z
	Cada articulacion se puede girar en los TRES ejes.*/
	GLfloat getBrazoRot(int articulacion, int eje) { return brazoRot[articulacion][eje]; }
	GLfloat getBaseBrazo() { return baseBrazo; }	//EJE BRAZO: gira todo el brazo sobre Y
	GLfloat getPinza() { return pinza; }		//apertura de la pinza
	int getArticulacionSel() { return articulacionSel; }

	/*===== LLANTAS =====
	llantaRot[i] es el EJE PL-LL de cada llanta (su giro individual)
	  0 = delantera cerca   1 = media cerca   2 = trasera cerca
	  3 = delantera lejos   4 = media lejos   5 = trasera lejos   */
	GLfloat getLlantaRot(int i) { return llantaRot[i]; }
	int getLlantaSel() { return llantaSel; }

	//===== EJES DE LA SUSPENSION =====
	GLfloat getEjeFrontal() { return ejeFrontal; }		//EJE FRONTAL: gira la barra delantera
	GLfloat getEjeBarraPata() { return ejeBarraPata; }	//EJE B-PL: gira la pata delantera
	GLfloat getEjeLL2() { return ejeLL2; }			//EJE LL2: pata de en medio
	GLfloat getEjeLL3() { return ejeLL3; }			//EJE LL3: pata trasera

	//===== SONDA ESPACIAL (version 2) =====
	GLfloat getPanelSolar() { return panelSolar; }
	GLfloat getAntenaY() { return antenaY; }
	GLfloat getAntenaX() { return antenaX; }

	~Window();
private: 
	GLFWwindow *mainWindow;
	GLint width, height;
	GLfloat rotax,rotay,rotaz, articulacion1, articulacion2, articulacion3, articulacion4, articulacion5, articulacion6;

	GLfloat brazoRot[3][3];
	GLfloat baseBrazo, pinza;
	int articulacionSel;

	GLfloat llantaRot[6];
	int llantaSel;

	GLfloat ejeFrontal, ejeBarraPata, ejeLL2, ejeLL3;
	GLfloat panelSolar, antenaY, antenaX;

	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;
	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
