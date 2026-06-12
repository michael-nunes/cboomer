#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <math.h>

#include "cboomer-glx.h"
#include "option.h"
#include "sugar.h"
#include "linear-algebra.h"

#define CBOOMER_STD_IMPLEMENTATION
#include "cboomer-std.h"

#define LENGTH_NULL_TERMINATED (-1)

#define ClientMessageGetAtom(_Evt, _Idx) (Atom)((_Evt).xclient.data.l[_Idx])
#define ClientMessageGetFirstAtom(_Evt)  ClientMessageGetAtom(_Evt,0)



static inline void Vec2dToUniform(vec2d Self, GLuint Loc) {
	glUniform2f(Loc, Self.X,Self.Y);
}

typedef struct {
	str * Path;
	str * Contents;
} shader_source;

declare_option(shader_source);

// Shader Funcions
option(shader_source) ShaderSourceFromFile(const char *Path);

void ShaderSourceRelease(shader_source * Src) {
	if(Src==NULL) return;
	StrFree(Src->Path);
	StrFree(Src->Contents);
}

option(shader_source) ShaderSourceFromFile(const char *Path) {
	str * Content = ReadEntireFile(Path);
	shader_source Result = {0};
	if(Content == NULL) return option_result_empty(shader_source,Result);
	Result.Contents = Content;
	Result.Path = StrNew(Path);
	return option_result(shader_source, Result); 
}

GLuint ShaderSourceCompile(shader_source Shader, GLenum Kind) {
	GLuint Result = glCreateShader(Kind);
	GLint Valid;
	str * Code = Shader.Contents;
	char *CodeBuffer = StrGetBuffer(Code);
	glShaderSource(Result, 1, 
			(const GLchar * const *)(&CodeBuffer),
			(const GLint*)(&Code->Length));

	glCompileShader(Result);

	glGetShaderiv(Result,GL_COMPILE_STATUS,&Valid);
	char Buffer[512];
	if(not(Valid)) {
		glGetShaderInfoLog(Result,512,NULL,Buffer);
		fprintf(stderr, "----------------------------------------\n");	
		fprintf(stderr, "Error while compiling:"STRFMT"\n",STRARG(Shader.Path));
		fprintf(stderr, "Log:%s\n", Buffer);
		fprintf(stderr, "----------------------------------------\n");
		exit(1);
	}
	return (Result);
}

GLuint CreateShaderProgram(shader_source Vertex, shader_source Fragment) {
	GLuint Result = glCreateProgram();
	GLuint VertexShader = ShaderSourceCompile(Vertex, GL_VERTEX_SHADER);
	GLuint FragmentShader = ShaderSourceCompile(Fragment, GL_FRAGMENT_SHADER);

	glAttachShader(Result, VertexShader);
	glAttachShader(Result, FragmentShader);

	glLinkProgram(Result);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	GLint Valid;
	char Buffer[512];
	glGetProgramiv(Result, GL_LINK_STATUS, &Valid);
	if(not(Valid)) {
		glGetProgramInfoLog(Result,512,NULL,Buffer);
		printf("Info: %s\n", Buffer);
		exit(1);
	}
	glUseProgram(Result);
	return (Result);
}

typedef struct {
	GLuint ProgramId;
	GLint  CameraPosLoc;
	GLint  CameraZoomLoc;
	GLint  TexLoc;
	GLint  ScreenshotSizeLoc;
	GLint  WindowSizeLoc;
} boomer_shader_program;


XImage * GetScreenshot(Display *Dpy, Window Win) {
	XWindowAttributes Attributes;
	XGetWindowAttributes(Dpy,Win,&Attributes);
	return XGetImage(Dpy,Win,0,0,Attributes.width,Attributes.height,AllPlanes,ZPixmap);
}

typedef struct {
	float ScrollSpeed;
	float ScaleFriction;
	float MinScale;
} config;

typedef struct {
	vec2d Position;
	float Zoom;
	float DeltaScale;
	vec2d Delta;
	vec2d ScalePivot;
} camera;

void CameraUpdate(camera *Camera, config *Conf, vec2d Mouse, vec2d WindowSize,  float Dt) {
	float Friction = Conf->ScaleFriction;

	if(fabs(Camera->DeltaScale) > 0.01f) {

		vec2d Pivot = Camera->ScalePivot;

		vec2d P0 = Vec2d_Div1f(
				Vec2d_Sub(Pivot, Vec2d_Mul1f(WindowSize,0.5f)), 
				Camera->Zoom
		);


		float Step = Camera->DeltaScale * Dt;
		Camera->Zoom = Camera->Zoom + Step;
		
		vec2d P1 = Vec2d_Div1f(
				Vec2d_Sub(
					Pivot, Vec2d_Mul1f(WindowSize,0.5f)), 
				Camera->Zoom
		);


		Camera->Position = Vec2d_Add(Camera->Position, Vec2d_Sub(P0,P1));

		Camera->DeltaScale -= Step * Friction;
	}

	if(Camera->Zoom < Conf->MinScale) Camera->Zoom = Conf->MinScale;

	if(fabs(Camera->Delta.X) > 1.0f) {
		float Step = Camera->Delta.X * Dt;
		Camera->Position.X += Step;
		Camera->Delta.X -= Step * 2.2f;
	}
	if(fabs(Camera->Delta.Y) > 1.0f) {
		float Step = Camera->Delta.Y * Dt;
		Camera->Position.Y += Step;
		Camera->Delta.Y -= Step * 2.2f;
	}
}

typedef struct {
	config *Config;
	camera *Camera;
	vec2d  Mouse;
} scroll_env;

static inline void ScrollUp(scroll_env env) {
	env.Camera->DeltaScale += env.Config->ScrollSpeed;
	env.Camera->ScalePivot = env.Mouse;
}

static inline void ScrollDown(scroll_env env) {
	env.Camera->DeltaScale -= env.Config->ScrollSpeed;
	env.Camera->ScalePivot = env.Mouse;
}


typedef struct {
	da_fields(int);
} numbers;

int main(int ArgCount, char **Args) {



 	string_builder ConfigFile = GetConfigDir("cboomer/config");

//	string_builder ConfigFile = {0};
//	Path_Append(&ConfigFile, "frag.glsl");
	int Exists = Path_Exists(&ConfigFile);
	
	printf(":%.*s: Found: %s\n", flatten(ConfigFile,.Count,.Items), Exists ? "yes" : "no");

//	return 0;
	shader_source VertexShader, FragmentShader;
	unwrap(shader_source, VertexShader,   ShaderSourceFromFile("vertex.glsl"));
	unwrap(shader_source, FragmentShader, ShaderSourceFromFile("frag.glsl"));

	printf("Vertex Shader:"STRFMT"\n", STRARG(VertexShader.Path));
	printf("Fragment Shader:"STRFMT"\n", STRARG(FragmentShader.Path));

	Display *Dpy;
	Window   MainWindow;
	XWindowAttributes Attributes;
	int Screen;
	struct { int Major, Minor; } GlVersion = {0};
	int HasGl;
	XVisualInfo *Vis;
	XSetWindowAttributes Swa = {0};
	Atom WmDeleteMessage;

	expect_ptr(Display*, Dpy, XOpenDisplay(NULL), "Cannot open X11 Display");

	Screen = XDefaultScreen(Dpy);

	HasGl = glXQueryVersion(Dpy, flatten_ex(addr_of, GlVersion, .Major, .Minor));
	if(!HasGl || (GlVersion.Major < 1) || (GlVersion.Major == 1 && GlVersion.Minor < 3)){
		printf("Expected GLX 1.3, Founded:");
		if(HasGl) 
			printf("%d.%d\n", flatten(GlVersion, .Major, .Minor));
		else
			printf("None");
		exit(1);
	}
	printf("GLX Version: %d.%d\n", flatten(GlVersion, .Major, .Minor)); 

	int VAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

	unwrap_ptr(XVisualInfo*, Vis, 
			glXChooseVisual(Dpy,Screen, VAttributes));

	Swa.colormap = XCreateColormap(Dpy, DefaultRootWindow(Dpy), Vis->visual, AllocNone);

	XSync(Dpy, False);
	printf("Colormap XID: %lu\n", Swa.colormap);

	Swa.event_mask = 
		ButtonPressMask 
		bit_or ButtonReleaseMask 
		bit_or KeyPressMask 
		bit_or KeyReleaseMask 
		bit_or PointerMotionMask 
		bit_or ExposureMask 
		bit_or ClientMessage;

	Swa.override_redirect = 1;
	Swa.save_under = 1;

	XGetWindowAttributes(Dpy, 
			DefaultRootWindow(Dpy), 
			&Attributes);

	printf("Creating window %dx%d\n\tDepth: %d\n", 
			Attributes.width, Attributes.height, Attributes.depth);

	MainWindow = XCreateWindow(Dpy,
			DefaultRootWindow(Dpy),
			0, 0, Attributes.width, Attributes.height, 0,
			Vis->depth, InputOutput, Vis->visual,
			CWColormap 
			bit_or CWEventMask 
			bit_or CWOverrideRedirect 
			bit_or CWSaveUnder, &Swa);

	XMapWindow(Dpy, MainWindow);


	char *WmName  = "C-Boomer";
	char *WmClass = "C-Boomer";
	XClassHint Hints = { WmName, WmClass };
	XStoreName(Dpy, MainWindow, WmName);
	XSetClassHint(Dpy, MainWindow, &Hints);

	WmDeleteMessage = XInternAtom(Dpy, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(Dpy,MainWindow,&WmDeleteMessage,1);

	GLXContext GlCtx = glXCreateContext(Dpy,Vis,NULL, GL_TRUE);
	glXMakeCurrent(Dpy,MainWindow, GlCtx);

	float RefreshRate = 30.0f;
	int num, den;
	if(glXGetMscRateOML && glXGetMscRateOML(Dpy,MainWindow,&num,&den)) {
		RefreshRate = (float)num/den;
	}
	printf("Refresh Rate: %.2f\n", RefreshRate);

	CBoomerLoadGLExtensions();

	boomer_shader_program ShaderProgram = {
		.ProgramId = CreateShaderProgram(VertexShader, FragmentShader)
	};

	ShaderSourceRelease(addr_of(VertexShader));
	ShaderSourceRelease(addr_of(FragmentShader));


	printf("Shader Program Id: %u\n", ShaderProgram.ProgramId);

	glViewport(0,0,Attributes.width,Attributes.height);
	ShaderProgram.CameraPosLoc  = 
		glGetUniformLocation(ShaderProgram.ProgramId, "cameraPos");
	ShaderProgram.CameraZoomLoc = 
		glGetUniformLocation(ShaderProgram.ProgramId, "cameraZoom");
	ShaderProgram.TexLoc = 
		glGetUniformLocation(ShaderProgram.ProgramId, "tex");
	ShaderProgram.ScreenshotSizeLoc = 
		glGetUniformLocation(ShaderProgram.ProgramId, "screenshotSize");

	ShaderProgram.WindowSizeLoc = 
		glGetUniformLocation(ShaderProgram.ProgramId, "windowSize");

	XImage *Screenshot = GetScreenshot(Dpy, DefaultRootWindow(Dpy));



	GLuint VAO,VBO,EBO;
	float w = (float)Screenshot->width;
	float h = (float)Screenshot->height;

	printf("Texture Dim: %.fx%.f\n", w,h);

	GLfloat Vertices[] = {
		//  [ POS     ]   [ Tex Coords ]
		w,    0.0f,   1.0f, 1.0f, // TR
		w,       h,   1.0f, 0.0f, // BR
		0.0f,    h,   0.0f, 0.0f, // BL
		0.0f, 0.0f,   0.0f, 1.0f  // TL
	};

	// TR->BR->TL BR->BL-TL
	GLuint Indices[]= { 0, 1, 3, 1, 2, 3};

	glGenVertexArrays(1, addr_of(VAO));
	glGenBuffers(1, addr_of(VBO));
	glGenBuffers(1, addr_of(EBO));

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 
			sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
			sizeof(Indices), Indices, GL_STATIC_DRAW);

	int Stride = 4 * sizeof(GLfloat);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 
			Stride, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
			Stride, (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	GLuint Texture = 0;
	glGenTextures(1, addr_of(Texture));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexImage2D(GL_TEXTURE_2D, 0,
			GL_RGB,
			Screenshot->width,
			Screenshot->height,
			0,
			GL_BGRA,
			GL_UNSIGNED_BYTE,
			Screenshot->data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glUniform1i(ShaderProgram.TexLoc, 0);

	glEnable(GL_TEXTURE_2D);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);	

	int Running = 1;

	config Config = (config){ 0.5f, 3.0f, 0.1f };

	camera Camera = { .Zoom = 1.0f, .DeltaScale = 0 };

	Window FocusedBefore;
	int    RevertToReturn;
	XGetInputFocus(Dpy,addr_of(FocusedBefore), addr_of(RevertToReturn));

	glDisable(GL_DEPTH_TEST);

	vec2d Cursor = {0};
	while(Running) {

		/*IF NOT WINDOWED */
		XSetInputFocus(Dpy, MainWindow, RevertToParent, CurrentTime);

		XEvent Evt;
		XGetWindowAttributes(Dpy, MainWindow, &Attributes);
		vec2d WindowSize = Vec2d2f(flatten(Attributes, .width, .height));
		vec2d ScreenshotSize = Vec2d2f(flatten(Screenshot, ->width, ->height));

		//glViewport(0,0,WindowSize.X,WindowSize.Y);
		glViewport(0,0,flatten(WindowSize,.X,.Y));

		while(XPending(Dpy) > 0) {
			XNextEvent(Dpy,&Evt);

#define _ScrollEnv (scroll_env){ \
	.Camera = &Camera, \
	.Config = &Config, \
	.Mouse = Cursor }

			switch(Evt.type) {
				case_begin(Expose)
					ignore;
				case_of(ClientMessage)
					if(ClientMessageGetFirstAtom(Evt) == WmDeleteMessage) {
						Running = 0;
					}
				case_of(MotionNotify)
					Cursor = Vec2d2f(flatten(Evt.xmotion, .x, .y));
				case_of(KeyPress) {
					KeySym Key = XLookupKeysym(&Evt.xkey, 0);
					// "No, we have closure at home"
					#define key_invert_dir(_k) ((_k) == Key ? -1.0f : 1.0f)
					switch(Key) {
						case_begin(XK_0) { 
							Camera.Zoom = 1.0f; 
							Camera.Position = Vec2d1f(0);
							Camera.Delta = Vec2d1f(0);
							Camera.DeltaScale = 0;

						}
						case_of(XK_equal) ScrollUp(_ScrollEnv);
						case_of(XK_minus) ScrollDown(_ScrollEnv);
						case_of(XK_q,XK_Escape) Running = 0;
						case_of(XK_Left, XK_Right) {
							float Dir = key_invert_dir(XK_Left);
							Camera.Delta.X += 100 * Dir;
						}
						case_of(XK_Up, XK_Down) {
							float Dir = key_invert_dir(XK_Up);
							Camera.Delta.Y += 100 * Dir;
						}
						case_end
					}
				}
#undef key_invert_dir
				case_of(ButtonPress)
					switch(Evt.xbutton.button) {
						case_begin(Button4) ScrollUp(_ScrollEnv);
						case_of(Button5)    ScrollDown(_ScrollEnv);
						case_end
					}
				case_end;
			}
		}

		float Dt = 1.0f / RefreshRate;

		CameraUpdate(&Camera, &Config, Cursor, WindowSize, Dt);

		glUseProgram(ShaderProgram.ProgramId);
		glClearColor(0.1f,0.1f,0.1f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT bit_or GL_DEPTH_BUFFER_BIT);

		glUniform1f(ShaderProgram.CameraZoomLoc, Camera.Zoom);
		Vec2dToUniform(Camera.Position, ShaderProgram.CameraPosLoc);
		Vec2dToUniform(ScreenshotSize, ShaderProgram.ScreenshotSizeLoc);
		Vec2dToUniform(WindowSize, ShaderProgram.WindowSizeLoc);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_INT, 0);


		glXSwapBuffers(Dpy,MainWindow);
		glFinish();
	}

	// All GL stuff will be released when the process finishes
	// It is needless to spend time on it
	//
	//
	// glDeleteVertexArrays(1, addr_of(VAO));
	// glDeleteBuffers(1, addr_of(VBO));
	// glDeleteBuffers(1, addr_of(EBO));

	XSetInputFocus(Dpy, FocusedBefore, RevertToParent, CurrentTime);
	XSync(Dpy,False);
	return 0;
}
