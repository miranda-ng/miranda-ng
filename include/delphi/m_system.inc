(*

Miranda IM: the free IM client for Microsoft* Windows*

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

*)
{$IFNDEF M_SYSTEM}
{$DEFINE M_SYSTEM}

type

    TMM_INTERFACE = record
        cbSize: int;
        _malloc: function(cbSize: Integer): Pointer; cdecl;
        _realloc: function (pb: Pointer; cbSize: Integer): Pointer; cdecl;
        _free: procedure(pb: Pointer); cdecl;
    end;

const

    MIRANDANAME                 = 'Miranda';

    {
        wParam : 0
        lParam : 0
        affect : called after all modules have been successfully initialised
                 used to resolve double-dependencies in the module load order, see notes
        return : 0
        notes  : Can be used to call services, etc that have not yet loaded
                 when your module has.
    }
    ME_SYSTEM_MODULESLOADED     = 'Miranda/System/ModulesLoaded';

    {
        wParam : 0
        lParam : 0
        affect : called just before Miranda terminates, the database is still running
                 during this hook
        return : 0
    }
    ME_SYSTEM_SHUTDOWN          = 'Miranda/System/Shutdown';

    {
        wParam : 0
        lParam : 0
        affect : called before Miranda actually shuts down -- everyone has to agree
                 or it is not shut down.
        return : non zero to stop the shutdown
    }
    ME_SYSTEM_OKTOEXIT          = 'Miranda/System/OkToExitEvent';

    {
        wParam : 0
        lParam : 0
        affect : service which sends everyone the ME_SYSTEM_OKTOEXIT event
        return : true if everyone is okay to exit, otherwise false
    }
    MS_SYSTEM_OKTOEXIT          = 'Miranda/System/OkToExit';

    {
        wParam : 0
        lParam : 0
        return : returns the version number -- each byte set with version index,
                 e.g. 1.2.3.4 $01020304
    }
    MS_SYSTEM_GETVERSION        = 'Miranda/System/GetVersion';

    {
        wParam : size in bytes of the buffer to be filled
        lParam : pointer to the buffer to be filled
        affect : returns Miranda's version as text with build type such as '1.2.3.4 alpha'
        return : 0 on success -- non zero on failure
    }
    MS_SYSTEM_GETVERSIONTEXT    = 'Miranda/System/GetVersionText';

    {
        wParam : Handle of a wait object to be used
        lParam : pointer to service name
        affect : causes the service name to be called whenever the wait object
                 is signalled with CallService(Service, wParam=hWaitObjeect, lParam=0)
        return : 0 on success, non zero on failure, will always fail if
                 more than 64 event objects are already being waited on because
                 of the limit imposed by Windows.
        version: implemented after v0.1.2.0+
        other  : QueueUserAPC() can be used instead of this service to wait
                 for notifications, BUT *only* after v0.1.2.2+ since that deals
                 with APC's
    }
    MS_SYSTEM_WAITONHANDLE      = 'Miranda/System/WaitOnHandle';

    {
        wParam : hWaitObject to be removed
        lParam : 0
        affect : removes the wait object from the list, see above.
        returns: 0 on success, nonzero on failure
        version: implemented after v0.1.2.0+
    }
    MS_SYSTEM_REMOVEWAIT        = 'Miranda/System/RemoveWait';

    {
        wParam : 0
        lParam : Pointer to an initialised TMM_INTERFACE
        affect : Get function pointers to, malloc(), free() and realloc() used by Miranda
        note   : this should only be used carefully, make sure .cbSize is initialised with sizeof(TMM_INTERFACE)
        version: 0.1.2.2+
    }
    MS_SYSTEM_GET_MMI           = 'Miranda/System/GetMMI';

    {
        wParam=0
        lParam=0

        Add a thread to the unwind wait stack that Miranda will poll on
        when it is tearing down modules.

        This must be called in the context of the thread that is to be pushed
        i.e. there are no args, it works out what thread is being called
        and gets itself a handle to the calling thread.
    }
    MS_SYSTEM_THREAD_PUSH	   = 'Miranda/Thread/Push';

    {
        wParam=0
        lParam=0

        Remove a thread from the unwind wait stack -- it is expected
        that the call be made in the context of the thread to be removed.

        Miranda will begin to tear down modules and plugins if/when the
        last thread from the unwind stack is removed.
    }
    MS_SYSTEM_THREAD_POP	   = 'Miranda/Thread/Pop';

    {
        wParam=0
        lParam=0

        This hook is fired just before the thread unwind stack is used,
        it allows MT plugins to shutdown threads if they have any special
        processing to do, etc.
    }
    ME_SYSTEM_PRESHUTDOWN	   = 'Miranda/System/PShutdown';

    {
        wParam=0
        lParam=0

        Returns TRUE when Miranda has got WM_QUIT and is in the process
        of shutting down
    }
    MS_SYSTEM_TERMINATED	   = 'Miranda/SysTerm';

{$ENDIF}
