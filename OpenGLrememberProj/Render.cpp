#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"
#include <tuple>
#include <vector>
#include <stdlib.h>

#include <math.h>




bool textureMode = true;
bool lightMode = true;

std::vector<double> matr(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{

	double sum = 0;
	//double v1 = 0, v2 = 0, v3 = 0; 
	auto ir = (y2 - y1) * (z3 - z1) - (y3 - y1) * (z2 - z1);//координаты нормального вектора 
	auto jr = -((x2 - x1) * (z3 - z1) - (z2 - z1) * (x3 - x1));
	auto kr = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
	sum = sqrt(ir * ir + jr * jr + kr * kr);
	ir /= sum;
	jr /= sum;
	kr /= sum;
	return std::vector<double>({ ir,jr,kr });
}

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}


/*std::vector<double> matr(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{

	double sum = 0;
	//double v1 = 0, v2 = 0, v3 = 0;
	auto ir = (y2 - y1) * (z3 - z1) - (y3 - y1) * (z2 - z1);//координаты нормального вектора
	auto jr = -((x2 - x1) * (z3 - z1) - (z2 - z1) * (x3 - x1));
	auto kr = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
	sum = sqrt(ir * ir + jr * jr + kr * kr);
	ir /= sum;
	jr /= sum;
	kr /= sum;
	return std::vector<double>({ ir,jr,kr });
}*/

void glMyNormal9d(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{
	auto n = matr(x1, y1, z1, x2, y2, z2, x3, y3, z3);
	glNormal3d(n[0], n[1], n[2]);
}


void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);//включение текстур
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;//размер блика


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();
	//конец рисования квадратика станкина

	//рисование 3д многоугольника
	glBindTexture(GL_TEXTURE_2D, texId);

	glBegin(GL_QUADS);
	glColor3d(0.0, 1.0, 1.0);
	glMyNormal9d(5, 0, 4, 5, 0, 1, 6, 4, 4);
	glTexCoord2d(0, 1);
	glVertex3d(5, 0, 4);//1 
	glTexCoord2d(0, 0);
	glVertex3d(5, 0, 1);//2 
	glTexCoord2d(1, 0);
	glVertex3d(6, 4, 1);//3 
	glTexCoord2d(1, 1);
	glVertex3d(6, 4, 4);//4 

	glColor3d(1.0, 0.0, 1.0);
	glMyNormal9d(6, 4, 4, 6, 4, 1, 4, 5, 4);
	glTexCoord2d(0, 0);
	glVertex3d(6, 4, 1);//3 
	glTexCoord2d(0, 1);
	glVertex3d(6, 4, 4);//4 
	glTexCoord2d(1, 1);
	glVertex3d(4, 5, 4);//4* 
	glTexCoord2d(1, 0);
	glVertex3d(4, 5, 1);//3* 

	glColor3d(1.0, 0.0, 0.0);
	glMyNormal9d(4, 5, 4, 4, 5, 1, 4, 7, 4);
	glTexCoord2d(0, 1);
	glVertex3d(4, 5, 4);//4* 
	glTexCoord2d(0, 0);
	glVertex3d(4, 5, 1);//3* 
	glTexCoord2d(1, 0);
	glVertex3d(4, 7, 1);//6* 
	glTexCoord2d(1, 1);
	glVertex3d(4, 7, 4);//5* 

	glColor3d(0.0, 0.0, 1.0);
	glMyNormal9d(4, 7, 4, 4, 7, 1, 5, 9, 4);
	glTexCoord2d(0, 0);
	glVertex3d(4, 7, 1);//6* 
	glTexCoord2d(0, 1);
	glVertex3d(4, 7, 4);//5* 
	glTexCoord2d(1, 1);
	glVertex3d(5, 9, 4);//5 
	glTexCoord2d(1, 0);
	glVertex3d(5, 9, 1);//6 



	glColor3d(0.4, 1.0, 1.0);
	glMyNormal9d(5, 9, 4, 5, 9, 1, 1, 6, 4);
	glTexCoord2d(0, 1);
	glVertex3d(5, 9, 4);//5 
	glTexCoord2d(0, 0);
	glVertex3d(5, 9, 1);//6 
	glTexCoord2d(1, 0);
	glVertex3d(1, 6, 1);//7 
	glTexCoord2d(1, 1);
	glVertex3d(1, 6, 4);//8 

	glColor3d(0.7, 1.2, 0.0);
	glMyNormal9d(1, 6, 4, 1, 6, 1, 1, 4, 4);
	glVertex3d(1, 6, 1);//7 
	glVertex3d(1, 6, 4);//8 
	glVertex3d(1, 4, 4);//9 
	glVertex3d(1, 4, 1);//10 

	glColor3d(0.3, 1.0, 0.3);
	glMyNormal9d(1, 4, 4, 1, 4, 1, 3, 4, 4);
	glTexCoord2d(0, 1);
	glVertex3d(1, 4, 4);//9 
	glTexCoord2d(0, 0);
	glVertex3d(1, 4, 1);//10 
	glTexCoord2d(1, 0);
	glVertex3d(3, 4, 1);//11 
	glTexCoord2d(1, 1);
	glVertex3d(3, 4, 4);//12 

	glColor3d(1.0, 1.0, 0.5);//последняя бокова сторона
	glMyNormal9d(3, 4, 4, 3, 4, 1, 5, 0, 4);
	glTexCoord2d(0, 0);
	glVertex3d(3, 4, 1);//11 
	glTexCoord2d(0, 1);
	glVertex3d(3, 4, 4);//12 
	glTexCoord2d(1, 1);
	glVertex3d(5, 0, 4);//1 
	glTexCoord2d(1, 0);
	glVertex3d(5, 0, 1);//2 

	glColor3d(0.0, 0.7, 0.9);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex3d(5, 0, 4);//1 
	glTexCoord2d(1, 0);
	glVertex3d(6, 4, 4);//4 
	glTexCoord2d(1, 1);
	glVertex3d(4, 5, 4);//4* 
	glTexCoord2d(0, 1);
	glVertex3d(3, 4, 4);//12 

	glColor3d(1.0, 0.2, 0.3);
	glNormal3d(0, 0, 1);
	glVertex3d(4, 5, 4);//4* 
	glVertex3d(3, 4, 4);//12 
	glVertex3d(1, 4, 4);//9 
	glVertex3d(4, 7, 4);//5* 

	glColor3d(1.0, 0.5, 0.5);
	glNormal3d(0, 0, 1);
	glVertex3d(1, 4, 4);//9 
	glVertex3d(4, 7, 4);//5* 
	glVertex3d(5, 9, 4);//5 
	glVertex3d(1, 6, 4);//8 


	glColor3d(0.2, 1.0, 0.2);
	glNormal3d(0, 0, -1);
	glVertex3d(5, 0, 1);//1 
	glVertex3d(6, 4, 1);//4 
	glVertex3d(4, 5, 1);//4* 
	glVertex3d(3, 4, 1);//12 

	glColor3d(0.9, 0.8, 0.7);
	glNormal3d(0, 0, -1);
	glVertex3d(4, 5, 1);//4* 
	glVertex3d(3, 4, 1);//12 
	glVertex3d(1, 4, 1);//9 
	glVertex3d(4, 7, 1);//5* 

	glColor3d(0.3, 0.3, 0.0);
	glNormal3d(0, 0, -1);
	glVertex3d(1, 4, 1);//9 
	glVertex3d(4, 7, 1);//5* 
	glVertex3d(5, 9, 1);//5 
	glVertex3d(1, 6, 1);//8 


	glEnd();
	glBegin(GL_TRIANGLES);

	glColor3d(0.0, 0.0, 0.0);
	for (double i = 0; i < 250; i++)
	{
	
		if ((sin(i) > -0.98) && (cos(i) > -0.988))
		{
			double x = 1 + 0.99 * cos(i), y = 5 + 0.99 * sin(i), x1 = 1 + 0.99 * cos(i + 1), y1 = 5 + 0.99 * sin(i + 1);

			glMyNormal9d(x, y, 1.005, 1, 5, 1.005, x1, y1, 1.005);
			glVertex3d(x, y, 1.005);
			glVertex3d(1, 5, 1.005);
			glVertex3d(x1, y1, 1.005);

			glMyNormal9d(x, y, 3.995, 1, 5, 3.995, x1, y1, 3.995);
			glNormal3d(0, 0, 1);
			glVertex3d(x, y, 3.995);
			glVertex3d(1, 5, 3.995);
			glVertex3d(x1, y1, 3.995);


		}
	}

	glEnd();
	glBegin(GL_QUADS);
	for (double i = 0; i < 250; i++)
	{
		glNormal3d(0, 0, 1);
		if ((sin(i) > -0.98) && (cos(i) > -0.988))
		{
			double x = 1 + 0.99 * cos(i), y = 5 + 0.99 * sin(i), x1 = 1 + 0.99 * cos(i + 1), y1 = 5 + 0.99 * sin(i + 1);
			double x2 = 1 + 0.99 * cos(i + 0.5), y2 = 5 + 0.99 * sin(i + 0.5);
			glMyNormal9d(x, y, 1.005, x2, y2, 1.005, x, y, 3.995);

			//glNormal3d(0, 0, 1);
			glVertex3d(x, y, 1.005);
			glVertex3d(x2, y2, 1.005);
			glVertex3d(x2, y2, 3.995);
			glVertex3d(x, y, 3.995);



		}
	}

	glEnd();

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}