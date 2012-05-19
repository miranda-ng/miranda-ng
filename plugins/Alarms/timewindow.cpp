#include "timewindow.h"

HDC hdc;
HGLRC hglrc;

HWND pluginwind = 0;
int FrameId = 0;

// Select the pixel format for a given device context
BOOL SetDCPixelFormat(HDC hDC) {
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size of this structure
		1,                              // Version of this structure

		PFD_DRAW_TO_WINDOW |            // Draw to window (not bitmap)

		PFD_SUPPORT_OPENGL |            // Support OpenGL calls
		PFD_DOUBLEBUFFER,               // Double-buffered mode
		PFD_TYPE_RGBA,                  // RGBA Color mode
		24,                             // Want 24bit color
		0,0,0,0,0,0,                    // Not used to select mode
		0,0,                            // Not used to select mode
		0,0,0,0,0,                      // Not used to select mode
		32,                             // Size of depth buffer
		0,                              // Not used to select mode
		0,                              // Not used to select mode
		PFD_MAIN_PLANE,                 // Draw in main plane
		0,                              // Not used to select mode
		0,0,0 
	};                   // Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	if((nPixelFormat = ChoosePixelFormat(hDC, &pfd)) == 0) {
		MessageBox(0, "ChoosePixelFormat failed", "Error", MB_OK);
		return FALSE;
	}

	// Set the pixel format for the device context
	if(SetPixelFormat(hDC, nPixelFormat, &pfd) == FALSE) {
		MessageBox(0, "SetPixelFormat failed", "Error", MB_OK);
		return FALSE;
	}

	return TRUE;
}

int InitGL() {
	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	glClearColor(0, 0, 0, 0);

	glEnable(GL_SCISSOR_TEST);

	// back face culling
	//glEnable(GL_CULL_FACE);

	//glEnable(GL_LIGHTING);

	glShadeModel(GL_SMOOTH);
	//glShadeModel(GL_FLAT);

	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.4151, 0.4151, 0.4151, 1);

	return 0;
}

int SizeGL(int width, int height) {
	if(width == 0 || height == 0) return 0;

	GLfloat n_range = 0.5, x_range, y_range;

	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);

	// prevent divide by zero
	if(width == 0) width = 1;
	if(height == 0) height = 1;

	// Establish clipping volume (left, right, bottom, top, near, far)
	if (width <= height) {
		x_range = n_range;
		y_range = n_range*height/width;
	} else {
		x_range = n_range*width/height;
		y_range = n_range;
	}	

	// Reset projection matrix stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustum(-x_range, x_range, y_range, -y_range, n_range, 50 /*n_range * 100*/);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return 0;
}

void drawSmallMinute() {

	GLfloat size = 0.08f;

	glBegin(GL_POLYGON);
		glColor3f(1, 1, 1);
		glVertex3f(-size, -0.5*size, 0);
		glVertex3f(-size, 0.5*size, 0);
		glVertex3f(size, 0.5*size, 0);
		glVertex3f(size, -0.5*size, 0);
	glEnd();
}

void drawLargeMinute() {

	GLfloat size = 0.08f;

	glBegin(GL_POLYGON);
		glColor3f(1, 1, 1);
		glVertex3f(-size, -1.25*size, 0);
		glVertex3f(-size, 1.25*size, 0);
		glVertex3f(size, 1.25*size, 0);
		glVertex3f(size, -1.25*size, 0);
	glEnd();
}

void drawHourHand() {

	glBegin(GL_POLYGON);
		glColor3f(0.7f, 0.7f, 0.85f);
		glVertex3f(0, 0, 0);
		glVertex3f(-0.1f, 0.8f*1.5f, 0);
		glVertex3f(0, 1.5f, 0);
		glVertex3f(0.1f, 0.8f*1.5f, 0);
	glEnd();
}

void drawMinuteHand() {

	glBegin(GL_POLYGON);
		glColor3f(0.7f, 0.85f, 0.7f);
		glVertex3f(0, 0, 0);
		glVertex3f(-0.15f, 0.8f*2.1f, 0);
		glVertex3f(0, 2.1f, 0);
		glVertex3f(0.15f, 0.8f*2.1f, 0);
	glEnd();
}

void drawSecondsHand() {

	glBegin(GL_POLYGON);
		glColor3f(0.85f, 0.7f, 0.7f);
		glVertex3f(-0.02f, 0, 0);
		glVertex3f(-0.02f, 2.1f, 0);
		glVertex3f(0.02f, 2.1f, 0);
		glVertex3f(0.02f, 0, 0);
	glEnd();
}

int RenderGL(HWND hwnd, HDC hdc) {

	SYSTEMTIME ctime;
	//GLfloat rot;
	//int i;
	char buf[512];

	GetLocalTime(&ctime);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glLoadIdentity();
	glTranslated(0, 0, -30);

	//gluLookAt(0, 0, 10,  0, 0, 0,  0, 1, 0);
	glPushMatrix();

/*	
	for (i=0;i<60;i++) {
		glPushMatrix();
		rot = (GLfloat)i/60*360;
		glRotatef(-rot, 0, 0, 1.0);
		glTranslatef(0, 2.2f, 0);
		if (i%5!=0) drawSmallMinute();
		else drawLargeMinute();
		glPopMatrix();
	}

	glPopMatrix();

	glPushMatrix();
	rot = (GLfloat)(ctime.wSecond)/60*360;
	glRotatef(-rot, 0, 0, 1.0);
	drawSecondsHand();
	glPopMatrix();

	glPushMatrix();
	rot = (GLfloat)(ctime.wMinute)/60*360 + (GLfloat)(ctime.wSecond)/300*360/12;
	glRotatef(-rot, 0, 0, 1.0);
	drawMinuteHand();
	glPopMatrix();

	glPushMatrix();
	rot = (GLfloat)(ctime.wHour%12)/12*360 + (GLfloat)(ctime.wMinute)/60*360/12;
	glRotatef(-rot, 0, 0, 1.0);
	drawHourHand();

  */
	GetLocalTime(&ctime);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ctime, 0, buf, 512);
	draw_string_centered(buf);

	glPopMatrix();

	SwapBuffers(hdc);
	
	return 0;
}

LRESULT CALLBACK FrameContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_SIZE:
			{
				HWND child = (HWND)GetWindowLong(hwnd, GWL_USERDATA);
				RECT r;
				GetClientRect(hwnd, &r);

				//SetWindowPos(Frames[framepos].hWnd,HWND_TOP,0,DEFAULT_TITLEBAR_HEIGHT,width,height-DEFAULT_TITLEBAR_HEIGHT,SWP_SHOWWINDOW);
				SetWindowPos(child, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
			}
			break;
		case WM_CLOSE:
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	RECT r;
	//HDC hdc;

	switch(msg) {

		case WM_CREATE:
			hdc = GetDC(hwnd);

			if(SetDCPixelFormat(hdc) == FALSE) {
				return FALSE;
			}

			hglrc = wglCreateContext(hdc);
			
			wglMakeCurrent(hdc, hglrc);

			GetClientRect(hwnd, &r);

			InitGL();
			SizeGL(r.right - r.left, r.bottom - r.top);
			wglMakeCurrent(hdc, NULL);
			
			SetTimer(hwnd, 1011, 20, 0);

			return 0;

		case WM_ERASEBKGND:
			//RenderGL(hwnd, hdc);
			return TRUE;

		case WM_PAINT:
			//ps.fErase = FALSE;
			if(BeginPaint(hwnd, &ps)) {
				glScissor(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.left);

				wglMakeCurrent(hdc, hglrc);
				
				RenderGL(hwnd, hdc);

				wglMakeCurrent(hdc, NULL);

				EndPaint(hwnd, &ps);
			}
			//wglMakeCurrent(NULL, NULL);
			return 0;

		case WM_MOVE:
		case WM_SIZE:
			if(hdc) {
				wglMakeCurrent(hdc, hglrc);

				//SizeGL(LOWORD(lParam), HIWORD(lParam));
				
				GetClientRect(hwnd, &r);
				SizeGL(r.right - r.left, r.bottom - r.top);
				
				wglMakeCurrent(hdc, NULL);
				//RenderGL(hwnd, hdc);

				InvalidateRect(hwnd, 0, FALSE);
			}

			//return DefWindowProc(hwnd, msg, wParam, lParam);
			return TRUE;

		case WM_DESTROY:
			KillTimer(hwnd, 1011);
			hdc = GetDC(hwnd);
			wglMakeCurrent(hdc,NULL);
			wglDeleteContext(hglrc);
			ReleaseDC(hwnd, hdc);

			//DestroyWindow(hwnd);
			//return TRUE;
			break;

		case WM_TIMER:
			//RenderGL(hwnd, hdc);
			InvalidateRect(hwnd, 0, FALSE);
			return 0;

	};

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int CreateFrame(HWND parent) 
{
	WNDCLASS wndclass;
	wndclass.style         = 0; //CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = FrameWindowProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "TestFrame";
	RegisterClass(&wndclass);

	//HWND pluginwind = CreateWindow("TestFrame",Translate("Test"), 
	//	WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 
	//	0,0,0,0, parent, NULL,hInst,NULL);

	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {

		pluginwind = CreateWindow("TestFrame",Translate("Test"), 
			(WS_CHILD | WS_CLIPCHILDREN) & ~CS_VREDRAW & ~CS_HREDRAW,
			0,0,100,100, parent, NULL,hInst,NULL);

		CLISTFrame Frame;

		memset(&Frame,0,sizeof(Frame));
		Frame.name=(char *)malloc(255);
		memset(Frame.name,0,255);

		strcpy(Frame.name,"TestFrame");

		Frame.cbSize=sizeof(CLISTFrame);
		Frame.hWnd=pluginwind;
		Frame.align=alBottom;
		Frame.Flags=F_VISIBLE|F_SHOWTB|F_SHOWTBTIP;
		Frame.height=30;

		FrameId=CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);

	} else {
		wndclass.style         = 0;//CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc   = FrameContainerWindowProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInst;
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = 0; //(HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = "TestFrameContainer";
		RegisterClass(&wndclass);

		//pluginwind = CreateWindow(WS_EX_TOOLWINDOW, "TestFrame",Translate("Test"), 
		//	WS_POPUPWINDOW | WS_THICKFRAME | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, 
		//	0,0,100,100, parent, NULL,hInst,NULL);

		//ShowWindow(pluginwind, SW_SHOW);
		//UpdateWindow(pluginwind);

		HWND framewind = CreateWindowEx(WS_EX_TOOLWINDOW, "TestFrameContainer",Translate("Test"), 
			WS_POPUPWINDOW | WS_THICKFRAME | WS_VISIBLE | WS_CAPTION,
			0,0,100,100, parent, NULL,hInst,NULL);
	
		pluginwind = CreateWindow("TestFrame",Translate("Test"), 
			WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
			0,0,100,100, framewind, NULL,hInst,NULL);

		SetWindowLong(framewind, GWL_USERDATA, (LONG)pluginwind);

		ShowWindow(framewind, SW_SHOW);
		UpdateWindow(framewind);
	}

	return 0;
}

void InitFrame()
{
	CreateFrame((HWND)CallService(MS_CLUI_GETHWND, 0, 0));
}

void DeinitFrame()
{
	if(ServiceExists(MS_CLIST_FRAMES_REMOVEFRAME)) {
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)FrameId, 0);
	}
	DestroyWindow(pluginwind);
}
