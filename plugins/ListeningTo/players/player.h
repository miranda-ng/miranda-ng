/* 
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


class Player
{
protected:
	LISTENINGTOINFO listening_info;
	CRITICAL_SECTION cs;

	void NotifyInfoChanged();

public:
	BOOL enabled;
	BOOL needPoll;
	TCHAR *name; 

	Player();
	virtual ~Player();

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);

	virtual void FreeData();

	// Helpers to write to this object's listening info
	virtual LISTENINGTOINFO * LockListeningInfo();
	virtual void ReleaseListeningInfo();

	// Called everytime options change
	virtual void EnableDisable() {}
};


class ExternalPlayer : public Player
{
protected:
	TCHAR **window_classes;
	int num_window_classes;
	BOOL found_window;

	virtual HWND FindWindow();

public:
	ExternalPlayer();
	virtual ~ExternalPlayer();

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};


class CodeInjectionPlayer : public ExternalPlayer
{
protected:
	char *dll_name;
	TCHAR *message_window_class;
	DWORD next_request_time;

	virtual void InjectCode();

public:
	CodeInjectionPlayer();
	virtual ~CodeInjectionPlayer();

	virtual BOOL GetListeningInfo(LISTENINGTOINFO *lti);
};
