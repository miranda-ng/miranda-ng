/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2010 Boris Krasnovskiy
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <richedit.h>

/////////////////////////////////////////////////////////////////////////////////////////
// Replace smileys in a rich edit control... 
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) (SMADD_RICHEDIT3*) &smre;  //pointer to SMADD_RICHEDIT3
// return: TRUE if API succeeded (all parameters were valid) , FALSE if not.

#define SAFLRE_INSERTEMF  2    // insert smiley as EMF into RichEdit, otherwise bitmap inserted
										 // this flag allows "true" transparency
#define SAFLRE_OUTGOING   4    // Parsing outgoing message
#define SAFLRE_NOCUSTOM   8    // Do not use custom smileys
#define SAFLRE_FIREVIEW   16   // use NM_FIREVIEWCHANGE messages

struct SMADD_RICHEDIT
{
	HWND hwndRichEditControl;   // handle to the rich edit control
	CHARRANGE *rangeToReplace;  // same meaning as for normal Richedit use (NULL = replaceall)
	const char *Protocolname;   // protocol to use... if you have defined a protocol, u can 
	                            // use your own protocol name. SmileyAdd will automatically 
	                            // select the smileypack that is defined for your protocol.
	                            // Or, use "Standard" for standard smiley set. Or "ICQ" 
	                            // if you prefer those icons. 
	                            // If not found or NULL, "Standard" will be used
	unsigned flags;             // Flags (SAFLRE_*) that define the behaivior
	BOOL disableRedraw;         // Parameter have been depricated, have no effect on operation
	MCONTACT hContact;          // Contact handle
};

#define MS_SMILEYADD_REPLACESMILEYS  "SmileyAdd/ReplaceSmileys"

/////////////////////////////////////////////////////////////////////////////////////////
// Event notifies that SmileyAdd options have changed 
// Message dialogs usually need to redraw it's content on reception of this event
// wParam = Contact handle which options have changed, NULL if global options changed
// lParam = (LPARAM) 0; not used

#define ME_SMILEYADD_OPTIONSCHANGED  "SmileyAdd/OptionsChanged"

/////////////////////////////////////////////////////////////////////////////////////////
// finds all smileys in text, API parses the provided text and returns all smileys found
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) (SMADD_BATCHPARSE2*) &smgp;  //pointer to SMADD_BATCHPARSE2
// function returns pointer to array SMADD_BATCHPARSERES records for each smiley found
// if no smileys found NULL is returned
// if non NULL value returned pointer must be freed with MS_SMILEYADD_BATCHFREE API

#define SAFL_PATH      1       // provide smiley file path, icon otherwise 
#define SAFL_UNICODE   2       // string fields in OPTIONSDIALOGPAGE are WCHAR*
#define SAFL_OUTGOING  4       // Parsing outgoing message
#define SAFL_NOCUSTOM  8       // Do not use custom smileys

struct SMADD_BATCHPARSE
{
	const char *Protocolname;   // protocol to use... if you have defined a protocol, u can 
	                            // use your own protocol name. Smiley add wil automatically 
	                            // select the smileypack that is defined for your protocol.
	                            // Or, use "Standard" for standard smiley set. Or "ICQ"
	                            // if you prefer those icons. 
	                            // If not found or NULL: "Standard" will be used

	MAllCStrings str;           // A text to parse
	unsigned flag;              // One of the SAFL_ flags specifies parsing requirements
	                            // This parameter should be filled by the user
	unsigned numSmileys;        // Number of Smileys found, this parameter filled by SmileyAdd
	unsigned oflag;             // One of the SAFL_ flags specifies content of the parse results
	                            // this parameter filled by SmileyAdd
	MCONTACT hContact;          // Contact handle
};

struct SMADD_BATCHPARSERES
{
	unsigned startChar;         // Starting smiley character 
	                            // Because of iterative nature of the API caller should set this 
										 // parameter to correct value
	unsigned size;              // Number of characters in smiley (0 if not found)
	                            // Because of iterative nature of the API caller should set this 
										 // parameter to correct value
	union
	{
		const wchar_t *filepath;
		HICON hIcon;             // User is responsible for destroying icon handle 
	};
};

#define MS_SMILEYADD_BATCHPARSE "SmileyAdd/BatchParse"

/////////////////////////////////////////////////////////////////////////////////////////
// Frees memory allocated by MS_SMILEYADD_BATCHPARSE
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) (SMADD_BATCHPARSERES*) &smgp;  //pointer to SMADD_BATCHPARSERES

#define MS_SMILEYADD_BATCHFREE "SmileyAdd/BatchFree"

/////////////////////////////////////////////////////////////////////////////////////////
// Register smiley category
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) (SMADD_REGCAT*) &smgp; pointer to SMADD_REGCAT

struct SMADD_REGCAT
{
	char *name;                 // smiley category name for reference
	char *dispname;             // smiley category name for display 
};

#define MS_SMILEYADD_REGISTERCATEGORY "SmileyAdd/RegisterCategory"

/////////////////////////////////////////////////////////////////////////////////////////
// Reloads smiley category
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) Pointer to protocol name or NULL for all;

#define MS_SMILEYADD_RELOAD "SmileyAdd/Reload"

/**
	NM_FIREVIEWCHANGE is WM_NOTIFY Message for notify parent of host window about smiley are going to be repaint

	The proposed action is next: Owner of RichEdit windows received NM_FIREVIEWCHANGE through WM_NOTIFY
	twice first time before painting|invalidating (FVCN_PREFIRE) and second time - after (FVCN_POSTFIRE).
	The Owner window may change any values of received FVCNDATA_NMHDR structure in order to raise needed action.
	For example it may substitute FVCA_INVALIDATE to FVCA_CUSTOMDRAW event to force painting on self offscreen context.
	
	It can be:
	FVCA_CUSTOMDRAW - in this case you need to provide valid HDC to draw on and valid RECT of smiley
	FVCA_INVALIDATE - to invalidate specified rect of window 
	FVCA_NONE		- skip any action. But be aware - animation will be stopped till next repainting of smiley.
	FVCA_SENDVIEWCHANGE - to notify richedit ole about object changed. Be aware Richedit will fully reconstruct itself

	Another point is moment of received smiley rect - it is only valid if FVCA_DRAW is initially set, 
	and it is PROBABLY valid if FVCA_INVALIDATE is set. And it most probably invalid in case of FVCA_SENDVIEWCHANGE.
	The smiley position is relative last full paint HDC. Usually it is relative to top-left corner of host 
	richedit (NOT it client area) in windows coordinates. 

*/

// Type of Event one of
#define FVCN_PREFIRE        1
#define FVCN_POSTFIRE       2

// Action of event are going to be done
#define FVCA_NONE           0
#define FVCA_DRAW           1   // do not modify hdc in case of _DRAW, Use _CUSTOMDRAW
#define FVCA_CUSTOMDRAW     2
#define FVCA_SKIPDRAW		 5

// Extended NMHDR structure for WM_NOTIFY
struct FVCNDATA_NMHDR
{
	//NMHDR structure
	HWND        hwndFrom;       // Window of smiley host
	UINT_PTR    idFrom;         // ignored
	UINT        code;           // NM_FIREVIEWCHANGE

	BYTE        bEvent;         // FVCN_ value - pre- or post- painting
	BYTE        bAction;        // FVCA_ keys
	HDC         hDC;            // Canvas to draw on
	RECT        rcRect;         // Valid/should be in case of FVCA_DRAW
	COLORREF    clrBackground;  // color to fill background if fTransparent is not set
	BOOL        fTransparent;   // if need to fill back color (not supported)
	LPARAM      lParam;         // used by host window PreFire and PostFire event
};

// Code of WM_NOTIFY message (code)
#define NM_FIREVIEWCHANGE   NM_FIRST+1;

/////////////////////////////////////////////////////////////////////////////////////////
// Loads all smileys for the contact
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) (SMADD_CONT*) &dir;  // pointer to directory to load smiley from

#define SMADD_SMILEPACK 0
#define SMADD_FILE      1
#define SMADD_FOLDER    2

struct SMADD_CONT
{
	int type;                   // SMADD_* constant
	const char *pszModule;      // module name
	const wchar_t *path;        // smiley category name for reference
};

void __forceinline SmileyAdd_LoadContactSmileys(int type, const char *pszModule, const wchar_t *path)
{
	SMADD_CONT cont = { type, pszModule, path };
	CallService("SmileyAdd/LoadContactSmileys", 0, LPARAM(&cont));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Displays SmileyTool window to choose a smiley
// wParam = (WPARAM) 0; not used
// lParam = (LPARAM) (SMADD_SELECTSMILEY*) &param;

struct SMADD_SELECTSMILEY
{
	const char *pszProto;   // protocol module or NULL for emoji
	const char *pszSmileys; // list of smileys or zero to display all pack
	int xPosition;
	int yPosition;
	int direction;
	HWND hWndTarget;
	HWND hWndParent;
	UINT targetMessage;
	WPARAM targetWParam;
};

#define MS_SMILEYADD_SELECTSMILEY  "SmileyAdd/SelectSmiley"
