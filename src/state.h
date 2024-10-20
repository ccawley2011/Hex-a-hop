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

#ifndef __HHOP_STATE_H__
#define __HHOP_STATE_H__

#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "video.h"
#ifdef __3DS__
#include <3ds.h>
#endif

// Forward compatibility with SDL2
#if SDL_MAJOR_VERSION < 2
#define SDLK_KP_1 SDLK_KP1
#define SDLK_KP_2 SDLK_KP2
#define SDLK_KP_3 SDLK_KP3
#define SDLK_KP_5 SDLK_KP5
#define SDLK_KP_7 SDLK_KP7
#define SDLK_KP_8 SDLK_KP8
#define SDLK_KP_9 SDLK_KP9
#endif

#if defined(__NDS__) || defined(__3DS__)
#define JOYSTICK_PAUSE   0 /* Start */
#define JOYSTICK_SELECT  1 /* A */
#define JOYSTICK_BACK    2 /* B */
#define JOYSTICK_DELETE  3 /* X */
#define JOYSTICK_RESTART 7 /* Select */
#else
#define JOYSTICK_SELECT  0
#define JOYSTICK_BACK    1
#define JOYSTICK_DELETE  2
#define JOYSTICK_PAUSE   3
#define JOYSTICK_RESTART 4
#endif


#ifdef WIN32
	// Trigger debugger
//	#define FATAL(string, string2) do{__asm{int 3};}while(0)
#endif

static inline void FATAL(const char * string="Unknown", const char * string2="")
{
#ifdef __3DS__
	errorConf errCnf;
	char text[1024];

	snprintf(text, 1024, "Fatal error: %s \"%s\"\n", string, string2);
	errorInit(&errCnf, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
	errorText(&errCnf, text);
	errorDisp(&errCnf);
#else
	fprintf(stderr, "Fatal error: %s \"%s\"\n", string, string2);
#endif
#ifdef __NDS__
	while (1);
#else
	exit(1);
#endif
}

class String
{
	int len;
	char* data;
public:
	void reserve(int i)
	{
		if (i < 0)
			FATAL("-ve string length.");
		if (i <= len)
			return;
		char* newdata = (char*)realloc(data, (i+1)*sizeof(char));
		if(!newdata) return;
		else data = newdata;
		len = i;
		data[len] = '\0';
	}
	String() : len(0), data(NULL) { reserve(32); *data = '\0'; }
	String(String const & s) : len(0), data(NULL) { reserve(s.len); strcpy(data, s.data); }
	String(const char* s) : len(0), data(NULL) { *this = s; }
	~String() { free(data); }
	operator const char* () const {return data ? data : "";}
	void operator = (String const & a) { *this = (const char*)a; }
	void operator = (const char * a)
	{
		if(!a || !a[0])
		{
			truncate(0);
			return;
		}
		reserve(strlen(a));
		strncpy(data, a, len);
		data[len] = '\0';
	}
	void operator += (const char * a) { reserve(strlen(a)+len); strcat(data, a); }
	String operator + (const char * a) const { String ret(*this); ret += a; return ret; }
	void truncate (int pos)
	{
		if(data && pos < len)
		{
			data[pos] = '\0';
			len = 0;
		}
	}
	void fix_backslashes()
	{
		if(data)
		{
			for (int i=0; data[i]; i++)
			{
				if (data[i]=='\\')
					data[i]='/';
			}
		}
	}
};

class State
{
public:
	virtual ~State() {}

	virtual bool KeyPressed(int key, int mod) = 0;
	virtual void KeyReleased(int /*key*/) {};
	virtual bool JoyPressed(int button) = 0;
	virtual void JoyReleased(int /*button*/) {};
	virtual bool JoyHatMotion(Uint8 /*value*/) = 0;
	virtual void Mouse(int x, int y, int dx, int dy, int buttons_pressed, int buttons_released, int buttons) = 0;
	virtual void Update(double timedelta) = 0;
	virtual void Render() = 0;
	virtual void FileDrop(const char* filename) = 0;
	virtual void ScreenModeChanged() {};
};

/************************************************************************
// TEMPLATE - copy & paste

#define ClassName NEWSTATE
class ClassName : public State
{
public:
	virtual bool KeyPressed(int key, int mod)
	{
		return false;
	}
	virtual void KeyReleased(int key)
	{
	}
	virtual bool JoyPressed(int button)
	{
		return false;
	}
	virtual void JoyReleased(int button)
	{
	}
	virtual bool JoyHatMotion(Uint8 value)
	{
	}
	virtual void Mouse(int x, int y, int dx, int dy, int buttons_pressed, int buttons_released, int buttons)
	{
	}
	virtual void FileDrop(const char* filename)
	{
	}
	virtual void Update(double timedelta)
	{
		// TODO
	}
	virtual void Render()
	{
		// TODO
	}
	virtual void ScreenModeChanged()
	{
		// TODO
	}
};
MAKE_STATE(ClassName, _KEY_, false);

************************************************************************/

class StateMakerBase
{
	static StateMakerBase* first;
	StateMakerBase* next;
	int key;
	bool start;
protected:
	State* state;
public:
	static State* current;

public:
	StateMakerBase(int key, bool start) : state(NULL)
	{
		for (StateMakerBase* s = first; s; s=s->next)
			if(key == s->key)
			{
				FATAL("Duplicate key in StateMakerBase::StateMakerBase");
				return;
			}
		this->key = key;
		this->start = start;
		next = first;
		first = this;
	}
	virtual ~StateMakerBase() {}
	virtual State* Create() = 0;
	void Destroy()
	{
		delete state;
		state = 0;
	}
	static State* GetNew(int k)
	{
		for (StateMakerBase* s = first; s; s=s->next)
			if(k==s->key)
				return current = s->Create();
		return current;
	}
	static State* GetNew()
	{
		if (!first)
		{
			FATAL("StateMakerBase::GetNew - first is NULL");
			return 0;
		}
		for (StateMakerBase* s = first; s; s=s->next)
			if(s->start)
				return current = s->Create();
		return current = first->Create();
	}
	static void DestroyAll()
	{
		for (StateMakerBase* s = first; s; s=s->next)
			s->Destroy();
		current = 0;
	}
};

template<class X>
class StateMaker : public StateMakerBase
{
public:
	StateMaker(int key, bool start=false) : StateMakerBase(key, start)
	{}
	State* Create()
	{
		if (!state) state = new X;
		return state;
	}
};

#define MAKE_STATE(x,key,start) static StateMaker<x> _maker_##x(key, start);

extern int mouse_buttons, mousex, mousey, noMouse;
extern double stylusx, stylusy;
extern float styluspressure;
extern int stylusok;
extern int quitting;

char* LoadSaveDialog(bool save, bool levels, const char * title);

#endif

