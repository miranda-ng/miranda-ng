/**************************************************************************\

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
all portions of this code base are copyrighted to Artem Shpynov and/or
the people listed in contributors.txt.

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

****************************************************************************

Created: Mar 19, 2007

Author and Copyright:  Artem Shpynov aka FYR:  ashpynov@gmail.com

****************************************************************************

File contains implementation of vertical and horizontal frames layout
Module will substitute existed CLUIFrames  feature

The goal is to be:
1. Back compatibable with CLUIFrames services
2. Support Transparent frames
3. Be able to create Horizontal and Vertical layouted frames
4. Implement fast and unflicker resizing (one pass batch resizing) including during autoresize
5. Support snapped to its edges frames, and probably other windows

\**************************************************************************/

#include "..\commonheaders.h"

#define __modern_ext_frames_c__include_c_file

#include "modern_ext_frames_private.h"		//static definitions
#include "modern_ext_frames_extern.c"		//external available procedures
#include "modern_ext_frames_intern.c"		//static internal procedures
#include "modern_ext_frames_services.c"		//services
#include "modern_ext_frames_opt.c"		//options

#undef __modern_ext_frames_c__include_c_file

