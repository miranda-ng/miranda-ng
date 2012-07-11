/*
Miranda SmileyAdd Plugin
Plugin support header file
Copyright (C) 2004-2006 borkra, portions by Rein-Peter de Boer

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


#define SAFLRE_INSERTEMF  2     // insert smiley as EMF into RichEdit, otherwise bitmap inserted
                                // this flag allows "true" transparency

typedef struct 
{
    int cbSize;                 //size of the structure
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
    int cbSize;                 //size of the structure
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
    int cbSize;                 //size of the structure
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

#define SAFL_PATH   1           // provide smiley file path, icon otherwise 
#define SAFL_UNICODE  2         // string fields in OPTIONSDIALOGPAGE are WCHAR*

#if defined _UNICODE || defined UNICODE
  #define SAFL_TCHAR     SAFL_UNICODE
#else
  #define SAFL_TCHAR     0
#endif

typedef struct 
{
    int cbSize;                 //size of the structure
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
    int cbSize;                 //size of the structure
    char* name;                 //smiley category name for reference
    char* dispname;             //smiley category name for display 
} SMADD_REGCAT;

//Register smiley category
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_REGCAT*) &smgp;  //pointer to SMADD_REGCAT
#define MS_SMILEYADD_REGISTERCATEGORY "SmileyAdd/RegisterCategory"
