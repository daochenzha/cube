
#include "stdafx.h"
#include <iostream>

#include <windows.h>
#include <math.h>
#include "gl/glaux.h"
#include <stdio.h>
#define GLUT_DISABLE_ATEXIT_HACK 
#include "GL/glut.h"
#pragma comment(lib, "openGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")\

#define M_PI 3.14159265358979323846
#define MAX_CHAR       128
#define AUTO_SPEED 20

#define NO_AUTO 0
#define AUTO_ROTATE 1
#define AUTO_MOVE 2
#define AUTO_ZOOM 3

GLuint  texture[1];  // �洢һ������
int windowID;  //������ID
float currentX[] = {1, 0, 0};  //��ǰˮƽ����
float currentY[] = {0, 1, 0};  //��ǰ��ֱ����
float currentMat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };  //��ǰģ����ͼ����
int currentOpt = -1;  //0�ϣ�1�£�2��3�ң�������
int currentZoom = -1; //0�Ŵ�1��С��������
int currentMove = -1;  //0�ϣ�1�£�2��3�ң�������
int currentZoomState = 0; //��¼��ǰ����״̬
float size = 1;  //�����С
int autoFlag = 0;  //0�ֶ���1��ת��ʾ��2ƽ����ʾ��3������ʾ
int rotateSequence[] = { 0, 0, 0, -1, -1, 2, 2, 2, -1, -1, 1, 1, 1, 1, -1, -1, 2, 2, 2, -1, -1, 3, 3, 3 };  //�Զ���ת����
int currentRotateIndex = 0;  //�Զ���ת����
int moveSequence[] = { 0, 0, -1, -1, -1, -1, 1, 1, 1, 1, -1, -1, 2, 2, -1, -1, -1, -1, 3, 3, -1, -1, 0, 0 }; //�Զ��ƶ�����
int currentMoveIndex = 0;  //�Զ��ƶ�����
int zoomSequence[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };  //�Զ���������
int currentZoomIndex = 0;  //�Զ���������
int autoSpeed = 0; //�����Զ���ʾ����

AUX_RGBImageRec *LoadBMP(CHAR *Filename)    // ����λͼͼ��
{
	FILE *File = NULL;         // �ļ����
	if (!Filename)          // ȷ���ļ������ṩ
	{
		return NULL;         // ���û�ṩ������ NULL
	}
	File = fopen(Filename, "r");       // ���Դ��ļ�
	if (File)           // �ļ�����ô?
	{
		fclose(File);         // �رվ��
		return auxDIBImageLoadA(Filename);    // ����λͼ������ָ��
	}
	return NULL;          // �������ʧ�ܣ����� NULL
}

int LoadGLTextures()         // ����λͼ(��������Ĵ���)��ת��������
{
	int Status = FALSE;         // ״ָ̬ʾ��
	AUX_RGBImageRec *TextureImage[1];     // ��������Ĵ洢�ռ�
	memset(TextureImage, 0, sizeof(void *)* 1);   // ��ָ����Ϊ NULL
	// ����λͼ��������޴������λͼû�ҵ����˳�
	if (TextureImage[0] = LoadBMP("Daochen_Zha.bmp"))
	{
		Status = TRUE;         // �� Status ��Ϊ TRUE
		glGenTextures(1, &texture[0]);     // ��������
		// ʹ������λͼ�������� �ĵ�������
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		// ��������
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // �����˲�
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // �����˲�
	}
	if (TextureImage[0])        // �����Ƿ����
	{
		if (TextureImage[0]->data)      // ����ͼ���Ƿ����
		{
			free(TextureImage[0]->data);    // �ͷ�����ͼ��ռ�õ��ڴ�
		}
		free(TextureImage[0]);       // �ͷ�ͼ��ṹ
	}
	return Status;          // ���� Status
}

//ѡ������
void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

//��ӡ�ַ�
void drawString(const char* str) {
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall) { // ����ǵ�һ�ε��ã�ִ�г�ʼ��
		// Ϊÿһ��ASCII�ַ�����һ����ʾ�б�
		isFirstCall = 0;

		// ����MAX_CHAR����������ʾ�б���
		lists = glGenLists(MAX_CHAR);

		// ��ÿ���ַ��Ļ������װ����Ӧ����ʾ�б���
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	// ����ÿ���ַ���Ӧ����ʾ�б�����ÿ���ַ�
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}

//��ӡ�����ַ�
void drawCNString(const char* str) {
	int len, i;
	wchar_t* wstring;
	HDC hDC = wglGetCurrentDC();
	GLuint list = glGenLists(1);

	// �����ַ��ĸ���
	// �����˫�ֽ��ַ��ģ����������ַ����������ֽڲ���һ���ַ�
	// ����һ���ֽ���һ���ַ�
	len = 0;
	for (i = 0; str[i] != '\0'; ++i)
	{
		if (IsDBCSLeadByte(str[i]))
			++i;
		++len;
	}

	// ������ַ�ת��Ϊ���ַ�
	wstring = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
	wstring[len] = L'\0';

	// �������ַ�
	for (i = 0; i<len; ++i)
	{
		wglUseFontBitmapsW(hDC, wstring[i], 1, list);
		glCallList(list);
	}

	// ����������ʱ��Դ
	free(wstring);
	glDeleteLists(list, 1);
}

// ����������
void drawCube(void)
{
	glBindTexture(GL_TEXTURE_2D, texture[0]);      // ѡ������

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);

	// ǰ��
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f); // ������ı��ε�����
	// ����
	glNormal3f(0, 0, -1);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); // ������ı��ε�����
	// ����
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // ������ı��ε�����
	// ����
	glNormal3f(0, -1, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); // ������ı��ε�����
	// ����
	glNormal3f(1, 0, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // ������ı��ε�����
	// ����
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f); // ������ı��ε�����
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f); // ������ı��ε�����
	glEnd();
}

//��ά������ת����
void rotateVector(float* vector, float* axis, float rad)
{
	float x1 = vector[0];
	float y1 = vector[1];
	float z1 = vector[2];
	float x2 = axis[0];
	float y2 = axis[1];
	float z2 = axis[2];

	vector[0] = (pow(x2, 2) + (1 - pow(x2, 2))*cos(rad)) * x1 +
		(x2*y2*(1 - cos(rad)) - z2*sin(rad)) * y1 +
		(x2*z2*(1-cos(rad))+y2*sin(rad)) * z1;
	vector[1] = (y2*x2*(1 - cos(rad)) + z2*sin(rad)) * x1 +
		(pow(y2, 2) + (1 - pow(y2, 2))*cos(rad)) * y1 +
		(y2*z2*(1 - cos(rad)) - x2*sin(rad)) * z1;
	vector[2] = (z2*x2*(1 - cos(rad)) - y2*sin(rad)) * x1 +
		(z2*y2*(1 - cos(rad)) + x2*sin(rad)) * y1 +
		(pow(z2, 2) + (1 - pow(z2, 2))*cos(rad)) * z1;;
}

//��Ⱦ����
void renderScene(void)
{
	//ʶ����һ���Զ�ģʽ����
	if (autoFlag == 1) {
		autoSpeed++;
		currentOpt = rotateSequence[currentRotateIndex];
		if (autoSpeed == AUTO_SPEED) {
			autoSpeed = 0;
			currentRotateIndex++;
			if (currentRotateIndex > 23) {
				currentRotateIndex = 0;
			}
		}
		
	}
	else if (autoFlag == 2) {
		autoSpeed++;
		currentMove = moveSequence[currentMoveIndex];
		if (autoSpeed == AUTO_SPEED) {
			autoSpeed = 0;
			currentMoveIndex++;
			if (currentMoveIndex > 23) {
				currentMoveIndex = 0;
			}
		}
	}
	else if (autoFlag == 3) {
		autoSpeed++;
		currentZoom = zoomSequence[currentZoomIndex];
		if (autoSpeed == AUTO_SPEED) {
			autoSpeed = 0;
			currentZoomIndex++;
			if (currentZoomIndex > 23) {
				currentZoomIndex = 0;
			}
		}
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(currentMat);
	glPushMatrix();

	//����ƽ�Ʋ���
	if (currentMove == 0)
	{
		glTranslatef(currentY[0] / 50, currentY[1] / 50, currentY[2] / 50); // ƽ��
		currentMove = -1;
	}
	else if (currentMove == 1)
	{
		glTranslatef(-currentY[0] / 50, -currentY[1] / 50, -currentY[2] / 50); // ƽ��
		currentMove = -1;
	}
	else if (currentMove == 2)
	{
		glTranslatef(-currentX[0] / 50, -currentX[1] / 50, -currentX[2] / 50); // ƽ��
		currentMove = -1;
	}
	else if (currentMove == 3)
	{
		glTranslatef(currentX[0] / 50, currentX[1] / 50, currentX[2] / 50); // ƽ��
		currentMove = -1;
	}

	//�������Ų���
	if (currentZoom == 1 && currentZoomState > -20)
	{
		glScalef(0.98, 0.98, 0.98);    // ����
		currentZoom = -1;
		currentZoomState--;
	}
	else if (currentZoom == 0 && currentZoomState < 7)
	{
		glScalef(1.02, 1.02, 1.02);    // ����
		currentZoom = -1;
		currentZoomState++;
	}

	//������ת����
	if (currentOpt == 0) {
		glRotatef(-3, currentX[0], currentX[1], currentX[2]);
		currentOpt = -1;
		rotateVector(currentY, currentX, 3 * M_PI / 180);
	}
	else if (currentOpt == 1) {
		glRotatef(3, currentX[0], currentX[1], currentX[2]);
		currentOpt = -1;
		rotateVector(currentY, currentX, -3 * M_PI / 180);
	}
	else if (currentOpt == 2) {
		glRotatef(-3, currentY[0], currentY[1], currentY[2]);
		currentOpt = -1;
		rotateVector(currentX, currentY, 3 * M_PI / 180);
	}
	else if (currentOpt == 3) {
		glRotatef(3, currentY[0], currentY[1], currentY[2]);
		currentOpt = -1;
		rotateVector(currentX, currentY, -3 * M_PI / 180);
	}

	glGetFloatv(GL_MODELVIEW_MATRIX, currentMat);
	drawCube();
	glPopMatrix();

	glLoadIdentity();
	glPushMatrix();
	selectFont(16, ANSI_CHARSET, "Comic Sans MS");
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos2f(0.7f, -0.9f);
	drawString("Designed by");
	glRasterPos2f(0.7f, -0.95f);
	drawString("Daochen Zha");
	glRasterPos2f(-1.0f, 0.95f);
	drawCNString("����˵����");
	glRasterPos2f(-1.0f, 0.90f);
	drawCNString("WASD��ת");
	glRasterPos2f(-1.0f, 0.85f);
	drawCNString("IJKLƽ��");
	glRasterPos2f(-1.0f, 0.80f);
	drawCNString("ZX����");
	glRasterPos2f(-1.0f, 0.75f);
	drawCNString("�Ҽ��򿪲˵�");
	glPopMatrix();
	glutSwapBuffers();
}

//�����������
void keyboard(unsigned char key, int x, int y)
{
	if (!autoFlag) {
		if (key == 's' || key == 'S') {
			currentOpt = 1;
		}
		else if (key == 'w' || key == 'W') {
			currentOpt = 0;
		}
		else if (key == 'd' || key == 'D') {
			currentOpt = 3;
		}
		else if (key == 'a' || key == 'A') {
			currentOpt = 2;
		}
		else if (key == 'z' || key == 'Z') {
			currentZoom = 0;
		}
		else if (key == 'x' || key == 'X') {
			currentZoom = 1;
		}
		else if (key == 'i' || key == 'I') {
			currentMove = 0;
		}
		else if (key == 'k' || key == 'K') {
			currentMove = 1;
		}
		else if (key == 'j' || key == 'J') {
			currentMove = 2;
		}
		else if (key == 'l' || key == 'L') {
			currentMove = 3;
		}
	}
}

//��ʼ������
void init(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);            //������ɫ��Ϊ��ɫ����Ҳ����Ϊ�Ǳ�����ɫ��
	glCullFace(GL_BACK);                        //����ü�(���治�ɼ�)
	glEnable(GL_CULL_FACE);                        //���òü�
	glEnable(GL_TEXTURE_2D);
	LoadGLTextures();            //����������ͼ
}


void processMenuEvents(int option) {
	//option�����Ǵ��ݹ�����value��ֵ��
	switch (option) {
	case AUTO_ROTATE:
		autoFlag = 1;
		autoSpeed = 0;
		currentRotateIndex = 0;
		break;
	case AUTO_MOVE:
		autoFlag = 2;
		autoSpeed = 0;
		currentMoveIndex = 0;
		break;
	case AUTO_ZOOM:
		autoFlag = 3;
		autoSpeed = 0;
		currentZoomIndex = 0;
		break;
	case NO_AUTO:
		autoFlag = 0;
		break;
	}
}

//�����˵�
void createGLUTMenus() {

	int menu;
	int submenu;
	
	//�����Ӳ˵�
	submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("��ת��ʾ", AUTO_ROTATE);
	glutAddMenuEntry("ƽ����ʾ", AUTO_MOVE);
	glutAddMenuEntry("������ʾ", AUTO_ZOOM);


	//�������˵�
	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("�ֶ�����", NO_AUTO);
	glutAddSubMenu("�Զ���ʾ", submenu);

	// �Ѳ˵�������Ҽ�����������
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	windowID = glutCreateWindow("��̬������ By ����2014301500040");
	init();

	glutKeyboardFunc(keyboard);

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);

	createGLUTMenus();
	glutMainLoop();
}
