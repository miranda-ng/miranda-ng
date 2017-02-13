/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

#include "stdafx.h"
#include <signal_protocol.h>

//TODO: create mutex

void lock(void * /*user_data*/)
{

}

void unlock(void * /*user_data*/)
{

}

int init_omemo()
{
	signal_context *global_context;
	signal_crypto_provider provider;
	signal_context_create(&global_context, NULL);
	signal_context_set_crypto_provider(global_context, &provider);
	signal_context_set_locking_functions(global_context, &lock, &unlock);

	return 0;
}