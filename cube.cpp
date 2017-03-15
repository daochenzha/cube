
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

GLuint  texture[1];  // 存储一个纹理
int windowID;  //父窗口ID
float currentX[] = {1, 0, 0};  //当前水平方向
float currentY[] = {0, 1, 0};  //当前垂直方向
float currentMat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };  //当前模型视图矩阵
int currentOpt = -1;  //0上，1下，2左，3右，其它无
int currentZoom = -1; //0放大，1缩小，其它无
int currentMove = -1;  //0上，1下，2左，3右，其它无
int currentZoomState = 0; //记录当前缩放状态
float size = 1;  //物体大小
int autoFlag = 0;  //0手动，1旋转演示，2平移演示，3缩放演示
int rotateSequence[] = { 0, 0, 0, -1, -1, 2, 2, 2, -1, -1, 1, 1, 1, 1, -1, -1, 2, 2, 2, -1, -1, 3, 3, 3 };  //自动旋转序列
int currentRotateIndex = 0;  //自动旋转索引
int moveSequence[] = { 0, 0, -1, -1, -1, -1, 1, 1, 1, 1, -1, -1, 2, 2, -1, -1, -1, -1, 3, 3, -1, -1, 0, 0 }; //自动移动序列
int currentMoveIndex = 0;  //自动移动索引
int zoomSequence[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };  //自动缩放序列
int currentZoomIndex = 0;  //自动缩放索引
int autoSpeed = 0; //控制自动演示速率

AUX_RGBImageRec *LoadBMP(CHAR *Filename)    // 载入位图图象
{
	FILE *File = NULL;         // 文件句柄
	if (!Filename)          // 确保文件名已提供
	{
		return NULL;         // 如果没提供，返回 NULL
	}
	File = fopen(Filename, "r");       // 尝试打开文件
	if (File)           // 文件存在么?
	{
		fclose(File);         // 关闭句柄
		return auxDIBImageLoadA(Filename);    // 载入位图并返回指针
	}
	return NULL;          // 如果载入失败，返回 NULL
}

int LoadGLTextures()         // 载入位图(调用上面的代码)并转换成纹理
{
	int Status = FALSE;         // 状态指示器
	AUX_RGBImageRec *TextureImage[1];     // 创建纹理的存储空间
	memset(TextureImage, 0, sizeof(void *)* 1);   // 将指针设为 NULL
	// 载入位图，检查有无错误，如果位图没找到则退出
	if (TextureImage[0] = LoadBMP("Daochen_Zha.bmp"))
	{
		Status = TRUE;         // 将 Status 设为 TRUE
		glGenTextures(1, &texture[0]);     // 创建纹理
		// 使用来自位图数据生成 的典型纹理
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		// 生成纹理
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 线形滤波
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 线形滤波
	}
	if (TextureImage[0])        // 纹理是否存在
	{
		if (TextureImage[0]->data)      // 纹理图像是否存在
		{
			free(TextureImage[0]->data);    // 释放纹理图像占用的内存
		}
		free(TextureImage[0]);       // 释放图像结构
	}
	return Status;          // 返回 Status
}

//选择字体
void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

//打印字符
void drawString(const char* str) {
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall) { // 如果是第一次调用，执行初始化
		// 为每一个ASCII字符产生一个显示列表
		isFirstCall = 0;

		// 申请MAX_CHAR个连续的显示列表编号
		lists = glGenLists(MAX_CHAR);

		// 把每个字符的绘制命令都装到对应的显示列表中
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	// 调用每个字符对应的显示列表，绘制每个字符
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}

//打印中文字符
void drawCNString(const char* str) {
	int len, i;
	wchar_t* wstring;
	HDC hDC = wglGetCurrentDC();
	GLuint list = glGenLists(1);

	// 计算字符的个数
	// 如果是双字节字符的（比如中文字符），两个字节才算一个字符
	// 否则一个字节算一个字符
	len = 0;
	for (i = 0; str[i] != '\0'; ++i)
	{
		if (IsDBCSLeadByte(str[i]))
			++i;
		++len;
	}

	// 将混合字符转化为宽字符
	wstring = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
	wstring[len] = L'\0';

	// 逐个输出字符
	for (i = 0; i<len; ++i)
	{
		wglUseFontBitmapsW(hDC, wstring[i], 1, list);
		glCallList(list);
	}

	// 回收所有临时资源
	free(wstring);
	glDeleteLists(list, 1);
}

// 绘制立方体
void drawCube(void)
{
	glBindTexture(GL_TEXTURE_2D, texture[0]);      // 选择纹理

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);

	// 前面
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); // 纹理和四边形的左下
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // 纹理和四边形的右下
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f); // 纹理和四边形的右上
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f); // 纹理和四边形的左上
	// 后面
	glNormal3f(0, 0, -1);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); // 纹理和四边形的右下
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f); // 纹理和四边形的右上
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // 纹理和四边形的左上
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); // 纹理和四边形的左下
	// 顶面
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f); // 纹理和四边形的左上
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f); // 纹理和四边形的左下
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f); // 纹理和四边形的右下
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // 纹理和四边形的右上
	// 底面
	glNormal3f(0, -1, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f); // 纹理和四边形的右上
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f); // 纹理和四边形的左上
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // 纹理和四边形的左下
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); // 纹理和四边形的右下
	// 右面
	glNormal3f(1, 0, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); // 纹理和四边形的右下
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // 纹理和四边形的右上
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f); // 纹理和四边形的左上
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // 纹理和四边形的左下
	// 左面
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); // 纹理和四边形的左下
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); // 纹理和四边形的右下
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f); // 纹理和四边形的右上
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f); // 纹理和四边形的左上
	glEnd();
}

//三维向量旋转函数
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

//渲染函数
void renderScene(void)
{
	//识别下一个自动模式操作
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

	//处理平移操作
	if (currentMove == 0)
	{
		glTranslatef(currentY[0] / 50, currentY[1] / 50, currentY[2] / 50); // 平移
		currentMove = -1;
	}
	else if (currentMove == 1)
	{
		glTranslatef(-currentY[0] / 50, -currentY[1] / 50, -currentY[2] / 50); // 平移
		currentMove = -1;
	}
	else if (currentMove == 2)
	{
		glTranslatef(-currentX[0] / 50, -currentX[1] / 50, -currentX[2] / 50); // 平移
		currentMove = -1;
	}
	else if (currentMove == 3)
	{
		glTranslatef(currentX[0] / 50, currentX[1] / 50, currentX[2] / 50); // 平移
		currentMove = -1;
	}

	//处理缩放操作
	if (currentZoom == 1 && currentZoomState > -20)
	{
		glScalef(0.98, 0.98, 0.98);    // 缩放
		currentZoom = -1;
		currentZoomState--;
	}
	else if (currentZoom == 0 && currentZoomState < 7)
	{
		glScalef(1.02, 1.02, 1.02);    // 缩放
		currentZoom = -1;
		currentZoomState++;
	}

	//处理旋转操作
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
	drawCNString("操作说明：");
	glRasterPos2f(-1.0f, 0.90f);
	drawCNString("WASD旋转");
	glRasterPos2f(-1.0f, 0.85f);
	drawCNString("IJKL平移");
	glRasterPos2f(-1.0f, 0.80f);
	drawCNString("ZX缩放");
	glRasterPos2f(-1.0f, 0.75f);
	drawCNString("右键打开菜单");
	glPopMatrix();
	glutSwapBuffers();
}

//处理键盘输入
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

//初始化函数
void init(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);            //清理颜色，为黑色，（也可认为是背景颜色）
	glCullFace(GL_BACK);                        //背面裁剪(背面不可见)
	glEnable(GL_CULL_FACE);                        //启用裁剪
	glEnable(GL_TEXTURE_2D);
	LoadGLTextures();            //载入纹理贴图
}


void processMenuEvents(int option) {
	//option，就是传递过来的value的值。
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

//创建菜单
void createGLUTMenus() {

	int menu;
	int submenu;
	
	//创建子菜单
	submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("旋转演示", AUTO_ROTATE);
	glutAddMenuEntry("平移演示", AUTO_MOVE);
	glutAddMenuEntry("缩放演示", AUTO_ZOOM);


	//创建主菜单
	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("手动控制", NO_AUTO);
	glutAddSubMenu("自动演示", submenu);

	// 把菜单和鼠标右键关联起来。
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	windowID = glutCreateWindow("动态正方体 By Daochen Zha");
	init();

	glutKeyboardFunc(keyboard);

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);

	createGLUTMenus();
	glutMainLoop();
}
