/*
    Copyright (C) 2005-2007 Tom Beaumont

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "i18n.h"

#include "state.h"
#include "sfx.h"
#include "text.h"
#include "system-relative.h"
#include <cassert>
#include <SDL.h>
#include <SDL_image.h>
#ifdef __NDS__
#include <filesystem.h>
#elif defined(__3DS__)
#include <3ds.h>
#endif

#ifdef WIN32
/*	#include <SDL_syswm.h>
	#include <shellapi.h> // Windows header for drag & drop
	#ifdef USE_BBTABLET
		#include "bbtablet/bbtablet.h"
	#endif*/
#else
	#undef USE_BBTABLET
#endif

#include <algorithm>
#include <string>

StateMakerBase* StateMakerBase::first = 0;
State* StateMakerBase::current = 0;

int SDL_focus = SDL_APPACTIVE | SDL_APPINPUTFOCUS;	// Initial focus state

#ifdef WIN32
	#include <windows.h>
	#include <winuser.h>
	#include <commdlg.h>
	#include <direct.h>

	bool tablet_system = false;

	char* LoadSaveDialog(bool save, bool levels, const char * title)
	{
		OPENFILENAME f;
		static char filename[1025] = "";
		static char path[1025] = "C:\\WINDOWS\\Desktop\\New Folder\\Foo\\Levels";
		char backupPath[1025];
		_getcwd(backupPath, sizeof(backupPath)/sizeof(backupPath[0])-1);

		memset(&f, 0, sizeof(f));

		#define FILTER(desc, f) desc " (" f ")\0" f "\0"
		f.lpstrFilter = FILTER("All known files","*.lev;*.sol")
						FILTER("Level files","*.lev")
						FILTER("Solution files","*.sol")
						FILTER("All files","*.*");
		#undef FILTER

		f.lStructSize = sizeof(f);
		f.lpstrFile = filename;
		f.nMaxFile = sizeof(filename);
		f.lpstrInitialDir = path;
		f.lpstrTitle = title;

		if (GetSaveFileName(&f)==TRUE)
		{
			// Remember user's choice of path!
			_getcwd(path, sizeof(path)/sizeof(path[0])-1);

			if (save)
			{
				int i = strlen(filename)-1;
				while (i>0 && filename[i]!='.' && filename[i]!='\\' && filename[i]!='/') i--;
				if (filename[i]!='.' && levels)
					strcat(filename, ".lev");
				if (filename[i]!='.' && !levels)
					strcat(filename, ".sol");
			}
			_chdir(backupPath);
			return filename;
		}

		_chdir(backupPath);
		return 0;
	}
#else
	char* LoadSaveDialog(bool /*save*/, bool /*levels*/, const char * /*title*/)
	{
		return 0;
	}
#endif

extern void test();

int mouse_buttons = 0;
int mousex= 10, mousey = 10;
int noMouse = 0;
int quitting = 0;

double stylusx= 0, stylusy= 0;
int stylusok= 0;
float styluspressure = 0;
SDL_Surface * screen = 0;
SDL_Surface * realScreen = 0;

extern State* MakeWorld();

bool fullscreen = false;

void InitScreen()
{
#ifdef __NDS__
	realScreen = SDL_SetVideoMode(
		SCREEN_W, SCREEN_H, 16, // Width, Height, BPP
		SDL_SWSURFACE | SDL_FULLSCREEN | SDL_BOTTOMSCR );

	screen = realScreen;
#elif defined(__3DS__)
	realScreen = SDL_SetVideoMode(
		SCREEN_W, SCREEN_H, 16, // Width, Height, BPP
		SDL_SWSURFACE | SDL_FULLSCREEN | SDL_CONSOLETOP | SDL_BOTTOMSCR );

	screen = realScreen;
#else
	realScreen = SDL_SetVideoMode(
		SCREEN_W, SCREEN_H, // Width, Height
		0, // Current BPP
		SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0) );

	if (screen)
		SDL_FreeSurface(screen);

	SDL_Surface* tempscreen = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		SCREEN_W, SCREEN_H,
		16, 0xf800, 0x07e0, 0x001f, 0);

	screen = SDL_DisplayFormat(tempscreen);
	SDL_FreeSurface(tempscreen);
#endif
}

void ToggleFullscreen()
{
	fullscreen = !fullscreen;
	InitScreen();
	StateMakerBase::current->ScreenModeChanged();
}
String base_path;

int TickTimer()
{
	static int time = SDL_GetTicks();
	int cap=40;

	int x = SDL_GetTicks() - time;
	time += x;
	if (x<0) x = 0, time = SDL_GetTicks();
	if (x>cap) x = cap;

	return x;
}

String GetBasePath()
{
	String base_path;

#ifdef RELATIVE_PATHS
	char* exedir = lisys_relative_exedir();
	if (exedir != NULL)
	{
		base_path += exedir;
		base_path += "/data/";
		free(exedir);
	}
	else
		base_path = "./data/";
	return base_path;
#elif defined(__NDS__)
	nitroFSInit(NULL);

	base_path = "nitro:/";
	return base_path;
#elif defined(__3DS__)
	romfsInit();

	base_path = "romfs:/";
	return base_path;
#else
	base_path = DATADIR "/";

	for (int i=strlen(base_path)-1; i>=0; i--)
	{
#ifdef WIN32
		if (base_path[i]=='/' || base_path[i]=='\\')
#else
		if (base_path[i]=='/')
#endif
		{
			base_path.truncate(i+1);
			break;
		}
	}

	// Check the path ends with a directory separator.
	if (strlen(base_path)>0)
	{
		char last = base_path[strlen(base_path)-1];
#ifdef WIN32
		if (last!='/' && last!='\\')
#else
		if (last!='/')
#endif
			base_path = "";
	}

#ifdef WIN32
	if (strstr(base_path, "\\foo2___Win32_Debug\\"))
		strstr(base_path, "\\foo2___Win32_Debug\\")[1] = '\0';
	if (strstr(base_path, "\\Release\\"))
		strstr(base_path, "\\Release\\")[1] = '\0';
#endif

	return base_path;
#endif
}

int main(int /*argc*/, char * /*argv*/[])
{
	base_path = GetBasePath();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE) < 0)
		FATAL("Cannot initialize SDL", SDL_GetError());

	InitScreen();

	if (!TextInit(base_path))
		FATAL("Cannot initialize text");

	SDL_Surface* icon = IMG_Load(base_path + "hex-a-hop-16.png");
	if (icon)
	{
		SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 0, 255, 255));
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface(icon);
	}

	InitSound(base_path);

	SDL_WarpMouse(SCREEN_W/2, SCREEN_H/2);

	int videoExposed = 1;

#ifdef WIN32
	HWND hwnd = 0;
#endif
#ifdef USE_BBTABLET
	bbTabletDevice &td = bbTabletDevice::getInstance( );
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	SDL_Joystick *joystick = SDL_JoystickOpen(0);
	if (!joystick)
		fprintf(stderr, "Could not open joystick: %s\n", SDL_GetError());

	StateMakerBase::GetNew();

	while(!quitting)
	{
		SDL_Event e;
		UpdateSound(-1.0);
		while(!SDL_PollEvent(&e) && !quitting)
		{
			int x = 0;

			if ((SDL_focus & 6)==6)
			{
				videoExposed = 1;

				x = TickTimer();

				while (x<10)
				{
					SDL_Delay(10-x);
					x += TickTimer();
				}

				StateMakerBase::current->Update(x / 1000.0);
			}
			else
			{
				// Not focused. Try not to eat too much CPU!
				SDL_Delay(150);
			}

			// experimental...
			if (!noMouse)
				StateMakerBase::current->Mouse(mousex, mousey, 0, 0, 0, 0, mouse_buttons);

			if (videoExposed)
			{
				StateMakerBase::current->Render();

				if (screen && realScreen!=screen)
				{
					SDL_Rect r = {0,0,SCREEN_W,SCREEN_H};
					SDL_BlitSurface(screen, &r, realScreen, &r);
				}
				SDL_Flip(realScreen);
				videoExposed = 0;
			}

			SDL_Delay(10);

#ifdef USE_BBTABLET
			// Tablet ////////////////////////
			bbTabletEvent evt;
			while(hwnd!=NULL && td.getNextEvent(evt))
			{
				stylusok = 1;
				RECT r;
				if (tablet_system)
				{
					GetWindowRect(hwnd, &r);
					stylusx = evt.x * GetSystemMetrics(SM_CXSCREEN);
					stylusy = (1.0 - evt.y) * GetSystemMetrics(SM_CYSCREEN);
					stylusx -= (r.left + GetSystemMetrics(SM_CXFIXEDFRAME));
					stylusy -= (r.top + GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION));;
				}
				else
				{
					GetClientRect(hwnd, &r);
					stylusx = evt.x * r.right;
					stylusy = (1.0 - evt.y) * r.bottom;
				}
				styluspressure = (evt.buttons & 1) ? evt.pressure : 0;

				/*
				printf("id=%d csrtype=%d b=%x (%0.3f, %0.3f, %0.3f) p=%0.3f tp=%0.3f\n",
					   evt.id,
					   evt.type,
					   evt.buttons,
					   evt.x,
					   evt.y,
					   evt.z,
					   evt.pressure,
					   evt.tpressure
					   );
				*/
			}

#endif
		}

		switch (e.type)
		{
			case SDL_VIDEOEXPOSE:
				videoExposed = 1;
				break;

#ifdef WIN32
/*			case SDL_SYSWMEVENT:
			{
				SDL_SysWMmsg* m = e.syswm.msg;
				hwnd = m->hwnd;
				static bool init=false;
				if (!init)
				{
					init = true;
					DragAcceptFiles(hwnd, TRUE);
					#ifdef USE_BBTABLET
						td.initTablet(hwnd, tablet_system ? bbTabletDevice::SYSTEM_POINTER : bbTabletDevice::SEPARATE_POINTER );
						if (!td.isValid())
							 printf("No tablet/driver found\n");
					#endif
				}
				if (m->msg == WM_DROPFILES)
				{
					HDROP h = (HDROP)m->wParam;

					char name[512];
					if (DragQueryFile(h, 0xffffffff, 0, 0) == 1)
					{
						DragQueryFile(h, 0, name, sizeof(name)/sizeof(name[0]));

						StateMakerBase::current->FileDrop(name);
					}

					DragFinish(h);
				}

				break;
			}*/
#endif

			case SDL_ACTIVEEVENT:
			{
				int gain = e.active.gain ? e.active.state : 0;
				int loss = e.active.gain ? 0 : e.active.state;
				SDL_focus = (SDL_focus | gain) & ~loss;
				if (gain & SDL_APPACTIVE)
					StateMakerBase::current->ScreenModeChanged();
				if (loss & SDL_APPMOUSEFOCUS)
					noMouse = 1;
				else if (gain & SDL_APPMOUSEFOCUS)
					noMouse = 0;

				break;
			}

			case SDL_MOUSEMOTION:
				noMouse = false;
				StateMakerBase::current->Mouse(e.motion.x, e.motion.y, e.motion.x-mousex, e.motion.y-mousey, 0, 0, mouse_buttons);
				mousex = e.motion.x; mousey = e.motion.y;
				break;
			case SDL_MOUSEBUTTONUP:
				noMouse = false;
				mouse_buttons &= ~(1<<(e.button.button-1));
				StateMakerBase::current->Mouse(e.button.x, e.button.y, e.button.x-mousex, e.button.y-mousey,
										0, 1<<(e.button.button-1), mouse_buttons);
				mousex = e.button.x; mousey = e.button.y ;
				break;
			case SDL_MOUSEBUTTONDOWN:
				noMouse = false;
				mouse_buttons |= 1<<(e.button.button-1);
				StateMakerBase::current->Mouse(e.button.x, e.button.y, e.button.x-mousex, e.button.y-mousey,
										1<<(e.button.button-1), 0, mouse_buttons);
				mousex = e.button.x; mousey = e.button.y ;
				break;

			case SDL_KEYUP:
				StateMakerBase::current->KeyReleased(e.key.keysym.sym);
				break;

			case SDL_KEYDOWN:
			{
				SDL_KeyboardEvent & k = e.key;

				if (k.keysym.sym==SDLK_F4 && (k.keysym.mod & KMOD_ALT))
				{
					quitting = 1;
				}
				else if (k.keysym.sym==SDLK_F12)
				{
					// Toggle system pointer controlled by tablet or not
					#ifdef USE_BBTABLET
						if (td.isValid())
						{
							tablet_system = !tablet_system;
							td.setPointerMode(tablet_system ? bbTabletDevice::SYSTEM_POINTER : bbTabletDevice::SEPARATE_POINTER);
						}
					#endif
				}
				else if (k.keysym.sym==SDLK_RETURN && (k.keysym.mod & KMOD_ALT) && !(k.keysym.mod & KMOD_CTRL))
				{
					ToggleFullscreen();
				}
				else if (StateMakerBase::current->KeyPressed(k.keysym.sym, k.keysym.mod))
				{
				}
				else if ((k.keysym.mod & (KMOD_ALT | KMOD_CTRL))==0)
				{
					StateMakerBase::GetNew(k.keysym.sym);
				}
			}
			break;

			case SDL_JOYBUTTONUP:
				StateMakerBase::current->JoyReleased(e.jbutton.button);
				break;

			case SDL_JOYBUTTONDOWN:
				StateMakerBase::current->JoyPressed(e.jbutton.button);
				break;

			case SDL_JOYHATMOTION:
				StateMakerBase::current->JoyHatMotion(e.jhat.value);
				break;

			case SDL_QUIT:
				quitting = 1;
				break;
		}
	}

	TextFree();
	FreeSound();
	SDL_JoystickClose(joystick);
	SDL_Quit();
#ifdef __3DS__
	romfsExit();
#endif
	return 0;
}
