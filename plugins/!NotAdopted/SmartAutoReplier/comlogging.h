/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once 

/// define hlobal object..
#ifndef CRUSHLOGOBJ
	#define CRUSHLOGOBJ	g_crushLog
#endif

/// this macro should be placed at the begining of any func of plugin..
#ifndef BEGIN_PROTECT_AND_LOG_CODE	
		#define BEGIN_PROTECT_AND_LOG_CODE		try {			\
												SetLastError(0);
#endif

/// this macro should be placed at the end of any func of plugin..
#ifndef END_PROTECT_AND_LOG_CODE
		#define END_PROTECT_AND_LOG_CODE		}	\
		catch (...)									\
		{											\
			CCrushLog::InternalProtect(__FUNCTION__, __FILE__, __DATE__, __LINE__,	GetLastError());\
		}
#endif
