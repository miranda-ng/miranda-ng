{
  Miranda IM: the free IM client for Microsoft  Windows

  Copyright 2000-2003 Miranda ICQ/IM project,
  all portions of this codebase are copyrighted to the people
  listed in contributors.txt.

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
}
{$IFNDEF M_SYSTEM}
{$DEFINE M_SYSTEM}

{$IFNDEF MIRANDANAME}
{$DEFINE MIRANDANAME}
const
  MIRANDANAME = 'Miranda NG';
{$ENDIF}
{$IFNDEF MIRANDACLASS}
{$DEFINE MIRANDACLASS}
const
  MIRANDACLASS = 'Miranda';
{$ENDIF}

  { miranda/system/modulesloaded
    called after all modules have been successfully initialised
    wParam=lParam=0
    used to resolve double-dependencies in the module load order
  }
const
  ME_SYSTEM_MODULESLOADED:PAnsiChar = 'Miranda/System/ModulesLoaded';

  { miranda/system/shutdown event
    called just before the application terminates
    the database is still guaranteed to be running during this hook.
    wParam=lParam=0
  }
  ME_SYSTEM_SHUTDOWN:PAnsiChar = 'Miranda/System/Shutdown';

  { restarts miranda ( 0.8+ )
    wParam = 0 or 1.
      1 - restart with current profile,
      0 - restart in default profile or profile manager
    lParam = 0
  }
  MS_SYSTEM_RESTART:PAnsiChar = 'Miranda/System/Restart';

  { miranda/system/oktoexit event
    called before the app goes into shutdown routine to make sure everyone is
    happy to exit
    wParam=lParam=0
    return nonzero to stop the exit cycle
  }
  ME_SYSTEM_OKTOEXIT:PAnsiChar = 'Miranda/System/OkToExitEvent';

  {
    wParam=0
    lParam=0

    This hook is fired just before the thread unwind stack is used,
    it allows MT plugins to shutdown threads if they have any special
    processing to do, etc.
  }
  ME_SYSTEM_PRESHUTDOWN:PAnsiChar = 'Miranda/System/PShutdown';

function Miranda_IsTerminated : bytebool; stdcall; external AppDll;

procedure GetFileReceivedFolder(hContact:TMCONTACT; szFolder:PWideChar); external AppDll;

{$ENDIF}
