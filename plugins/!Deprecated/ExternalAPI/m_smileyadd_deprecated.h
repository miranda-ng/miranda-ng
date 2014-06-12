/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2009 Boris Krasnovskiy
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




//find smileys in unicode text  
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_PARSEW*) &smgp;  //pointer to SMADD_PARSEW
typedef struct 
{
	int cbSize;                 //size of the structure
	const char* Protocolname;	//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. Smiley add wil automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
	wchar_t* str;                  //String to parse 
	HICON SmileyIcon;           //RETURN VALUE: the Icon handle is responsibility of the reciever 
							    //it must be destroyed with DestroyIcon when not needed.
	unsigned startChar;         //Starting smiley character 
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value
	unsigned size;              //Number of characters in smiley (0 if not found)
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value 
} SMADD_PARSEW;

#define MS_SMILEYADD_PARSEW "SmileyAdd/ParseW"


//find smiley in text, API could be called iterativly, on each iteration the remainder 
//of the string after last smiley processed  
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_PARSE*) &smgp;  //pointer to SMADD_PARSE
//return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
typedef struct 
{
	int cbSize;                 //size of the structure
	const char* Protocolname;	//protocol to use... if you have defined a protocol, u can 
								//use your own protocol name. Smiley add wil automatically 
								//select the smileypack that is defined for your protocol.
								//Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
								//if you prefer those icons. 
								//If not found or NULL: "Standard" will be used
	char* str;                  //String to parse 
	HICON SmileyIcon;           //RETURN VALUE: the Icon handle is responsibility of the reciever 
							    //it must be destroyed with DestroyIcon when not needed.
	unsigned startChar;         //Starting smiley character 
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value
	unsigned size;              //Number of characters in smiley (0 if not found)
								//Because of iterative nature of the API caller should set this 
								//parameter to correct value 
} SMADD_PARSE;

#define MS_SMILEYADD_PARSE "SmileyAdd/Parse"


//get button smiley icon
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_INFO*) &smgi;  //pointer to SMADD_INFO
//return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
#define MS_SMILEYADD_GETINFO  "SmileyAdd/GetInfo"


//find smiley, this API have been supreceeded with MS_SMILEYADD_PARSE[W]
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_GETICON*) &smgi;  //pointer to SMADD_GETICON
//return: TRUE if SmileySequence starts with a smiley, FALSE if not
typedef struct 
{
	int cbSize;             //size of the structure
	char* Protocolname;     //   "             "
	char* SmileySequence;   //character string containing the smiley 
	HICON SmileyIcon;       //RETURN VALUE: this is filled with the icon handle... 
							//do not destroy!
	int  Smileylength;		//length of the smiley that is found.
} SMADD_GETICON;

#define MS_SMILEYADD_GETSMILEYICON "SmileyAdd/GetSmileyIcon"

#define SMADD_SHOWSEL_SIZE_V1 32
#define SMADD_SHOWSEL_SIZE_V2 36

#define SMADD_RICHEDIT_SIZE_V1 16
#define SMADD_RICHEDIT_SIZE_V2 24

#define SMADD_INFO_SIZE_V1 20

#define SMADD_BATCHPARSE_SIZE_V1 24
