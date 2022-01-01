/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma once

#include "stdafx.h"

// basic class - provides interface for various Jabber auth

class TJabberAuth : public MZeroedObject
{
protected:  bool bIsValid = true;
            ptrA szName;
            unsigned complete;
				int priority;
            ThreadData *info;
public:
            TJabberAuth(ThreadData *pInfo, const char *pszMech);
	virtual ~TJabberAuth();

	virtual	char* getInitialRequest();
	virtual	char* getChallenge(const char *challenge);
	virtual	bool  validateLogin(const char *challenge);

	static   int compare(const TJabberAuth *p1, const TJabberAuth *p2)
				{	return p2->priority - p1->priority; // reverse sorting order
				}

	inline   const char* getName() const
				{	return szName;
				}

	inline   bool isValid() const
   			{	return bIsValid;
   			}
};

// plain auth - the most simple one

class TPlainAuth : public TJabberAuth
{
	typedef TJabberAuth CSuper;

	bool bOld;

public:
	TPlainAuth(ThreadData*, bool);

	char* getInitialRequest() override;
};

// md5 auth - digest-based authorization

class TMD5Auth : public TJabberAuth
{
	typedef TJabberAuth CSuper;

	int iCallCount;
public:
	TMD5Auth(ThreadData*);
	~TMD5Auth();

	char* getChallenge(const char *challenge) override;
};

class TScramAuth : public TJabberAuth
{
	typedef TJabberAuth CSuper;

	char *bindFlag, *cnonce = 0, *msg1 = 0, *serverSignature = 0;
	MBinBuffer bindData;
	const EVP_MD *hashMethod;

public:
	TScramAuth(ThreadData *pInfo, const char *pszMech, const EVP_MD *pMethod, int priority);
	~TScramAuth();

	char* getInitialRequest() override;
	char* getChallenge(const char *challenge) override;
	bool  validateLogin(const char *challenge) override;

	void Hi(uint8_t* res , char* passw, size_t passwLen, char* salt, size_t saltLen, int ind);
};

// ntlm auth - LanServer based authorization

class TNtlmAuth : public TJabberAuth
{
	typedef TJabberAuth CSuper;

	HANDLE hProvider;
	ptrA szInitRequest;

public:
	TNtlmAuth(ThreadData*, const char* mechanism);
	~TNtlmAuth();

	char* getInitialRequest() override;
	char* getChallenge(const char *challenge) override;

	bool getSpn(wchar_t* szSpn, size_t dwSpnLen);
};
