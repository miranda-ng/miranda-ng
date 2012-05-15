/*
Miranda SmileyAdd Plugin
Copyright (C) 2005-2008 Boris Krasnovskiy
Copyright (C) 2003-2004 Rein-Peter de Boer

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
#include <richedit.h>

#define SAFLRE_INSERTEMF  2     // insert smiley as EMF into RichEdit, otherwise bitmap inserted
                                // this flag allows "true" transparency
#define SAFLRE_OUTGOING  4      // Parsing outgoing message
#define SAFLRE_NOCUSTOM  8      // Do not use custom smileys

typedef struct 
{
    unsigned cbSize;            //size of the structure
    HWND hwndRichEditControl;   //handle to the rich edit control
    CHARRANGE* rangeToReplace;  //same meaning as for normal Richedit use (NULL = replaceall)
    const char* Protocolname;   //protocol to use... if you have defined a protocol, u can 
                                //use your own protocol name. SmileyAdd will automatically 
                                //select the smileypack that is defined for your protocol.
                                //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                                //if you prefer those icons. 
                                //If not found or NULL, "Standard" will be used
    unsigned flags;             //Flags (SAFLRE_*) that define the behaivior
    BOOL disableRedraw;         //Parameter have been depricated, have no effect on operation
    HANDLE hContact;            //Contact handle
} SMADD_RICHEDIT3;

//Replace smileys in a rich edit control... 
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_RICHEDIT3*) &smre;  //pointer to SMADD_RICHEDIT3
//return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
#define MS_SMILEYADD_REPLACESMILEYS  "SmileyAdd/ReplaceSmileys"


typedef struct 
{
    unsigned cbSize;            //size of the structure
    char* Protocolname;         //protocol to use... if you have defined a protocol, you can 
                                //use your own protocol name. Smiley add will automatically 
                                //select the smileypack that is defined for your protocol.
                                //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                                //if you prefer those icons. 
                                //If not found or NULL: "Standard" will be used
    int xPosition;              //Postition to place the selectwindow
    int yPosition;              // "
    int Direction;              //Direction (i.e. size upwards/downwards/etc) of the window 0, 1, 2, 3

    HWND hwndTarget;            //Window, where to send the message when smiley is selected.
    UINT targetMessage;         //Target message, to be sent.
    LPARAM targetWParam;        //Target WParam to be sent (LParam will be char* to select smiley)
                                //see the example file.
    HWND hwndParent;            //Parent window for smiley dialog 
    HANDLE hContact;            //Contact handle
} SMADD_SHOWSEL3;

//Show smiley selection window 
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_SHOWSEL3*) &smre;  //pointer to SMADD_SHOWSEL3
//return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
#define MS_SMILEYADD_SHOWSELECTION  "SmileyAdd/ShowSmileySelection"


typedef struct 
{
    unsigned cbSize;            //size of the structure
    char* Protocolname;         //   "             "
    HICON ButtonIcon;           //RETURN VALUE: this is filled with the icon handle
                                //of the smiley that can be used on the button
                                //if used with GETINFO2 handle must be destroyed by user!
                                //NULL if the buttonicon is not defined...
    int NumberOfVisibleSmileys; //Number of visible smileys defined.
    int NumberOfSmileys;        //Number of total smileys defined
    HANDLE hContact;            //Contact handle
} SMADD_INFO2;

//get button smiley icon
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_INFO2*) &smgi;  //pointer to SMADD_INFO2
//return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
#define MS_SMILEYADD_GETINFO2 "SmileyAdd/GetInfo2"

// Event notifies that SmileyAdd options have changed 
// Message dialogs usually need to redraw it's content on reception of this event
//wParam = Contact handle which options have changed, NULL if global options changed
//lParam = (LPARAM) 0; not used
#define ME_SMILEYADD_OPTIONSCHANGED  "SmileyAdd/OptionsChanged"

#define SAFL_PATH      1        // provide smiley file path, icon otherwise 
#define SAFL_UNICODE   2        // string fields in OPTIONSDIALOGPAGE are WCHAR*
#define SAFL_OUTGOING  4        // Parsing outgoing message
#define SAFL_NOCUSTOM  8        // Do not use custom smileys

#if defined _UNICODE || defined UNICODE
  #define SAFL_TCHAR     SAFL_UNICODE
#else
  #define SAFL_TCHAR     0
#endif

typedef struct 
{
    unsigned cbSize;            //size of the structure
    const char* Protocolname;   //protocol to use... if you have defined a protocol, u can 
                                //use your own protocol name. Smiley add wil automatically 
                                //select the smileypack that is defined for your protocol.
                                //Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                                //if you prefer those icons. 
                                //If not found or NULL: "Standard" will be used
    union {
       TCHAR*   str;            //String to parse
       char*    astr;
       wchar_t* wstr;
    };
    unsigned flag;              //One of the SAFL_ flags specifies parsing requirements
                                //This parameter should be filled by the user

    unsigned numSmileys;        //Number of Smileys found, this parameter filled by SmileyAdd
    unsigned oflag;             //One of the SAFL_ flags specifies content of the parse results
                                //this parameter filled by SmileyAdd
    HANDLE hContact;            //Contact handle
} SMADD_BATCHPARSE2;

typedef struct 
{
    unsigned startChar;         //Starting smiley character 
                                //Because of iterative nature of the API caller should set this 
                                //parameter to correct value
    unsigned size;              //Number of characters in smiley (0 if not found)
                                //Because of iterative nature of the API caller should set this 
                                //parameter to correct value
    union {
      const TCHAR*   filepath;
      const char*    afilepath;
      const wchar_t* wfilepath;
      HICON          hIcon;     //User responsible for destroying icon handle 
    };
} SMADD_BATCHPARSERES;

//find all smileys in text, API parses the provided text and returns all smileys found
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_BATCHPARSE2*) &smgp;  //pointer to SMADD_BATCHPARSE2
//function returns pointer to array SMADD_BATCHPARSERES records for each smiley found
//if no smileys found NULL is returned
//if non NULL value returned pointer must be freed with MS_SMILEYADD_BATCHFREE API
#define MS_SMILEYADD_BATCHPARSE "SmileyAdd/BatchParse"

//Free memory allocated by MS_SMILEYADD_BATCHPARSE
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_BATCHPARSERES*) &smgp;  //pointer to SMADD_BATCHPARSERES
#define MS_SMILEYADD_BATCHFREE "SmileyAdd/BatchFree"

typedef struct 
{
    unsigned cbSize;            //size of the structure
    char* name;                 //smiley category name for reference
    char* dispname;             //smiley category name for display 
} SMADD_REGCAT;

//Register smiley category
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_REGCAT*) &smgp; pointer to SMADD_REGCAT
#define MS_SMILEYADD_REGISTERCATEGORY "SmileyAdd/RegisterCategory"

//Register smiley category
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) Pointer to protocol name or NULL for all;
#define MS_SMILEYADD_RELOAD "SmileyAdd/Reload"

#ifndef MIID_SMILEY
// {E03C71B2-6DEE-467e-A4F0-DD516745876A}
#define MIID_SMILEY	{ 0xe03c71b2, 0x6dee, 0x467e, { 0xa4, 0xf0, 0xdd, 0x51, 0x67, 0x45, 0x87, 0x6a } }
#endif

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
//#define FVCA_INVALIDATE     3 (not supported)
//#define FVCA_SENDVIEWCHANGE 4 (not supported)
#define FVCA_SKIPDRAW		5

// Extended NMHDR structure for WM_NOTIFY
typedef struct  
{   
    //NMHDR structure
    HWND hwndFrom;              // Window of smiley host
    UINT idFrom;                // ignored
    UINT code;                  // NM_FIREVIEWCHANGE
    
    size_t      cbSize;
    BYTE        bEvent;         // FVCN_ value - pre- or post- painting
    BYTE        bAction;        // FVCA_ keys
    HDC         hDC;            // Canvas to draw on
    RECT        rcRect;         // Valid/should be in case of FVCA_DRAW
    COLORREF    clrBackground;  // color to fill background if fTransparent is not set
    BOOL        fTransparent;   // if need to fill back color (not supported)
    LPARAM      lParam;         // used by host window PreFire and PostFire event
} FVCNDATA_NMHDR;

// Code of WM_NOTIFY message (code)
#define NM_FIREVIEWCHANGE   NM_FIRST+1;

typedef struct 
{
    unsigned cbSize;             // size of the structure
	HANDLE hContact;
	int type;					 // 0 - directory, 1 - file;
    TCHAR* path;                  // smiley category name for reference
} SMADD_CONT;

//Loads all smileys for the contact
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_CONT*) &dir;  // pointer to directory to load smiley from
#define MS_SMILEYADD_LOADCONTACTSMILEYS  "SmileyAdd/LoadContactSmileys"
