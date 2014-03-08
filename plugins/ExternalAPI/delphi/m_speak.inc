{
Copyright (C) 2007 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
}

{$IFNDEF M_SPEAK}
{$DEFINE M_SPEAK}

(*
There is 2 ways of using the speak plugin:

1. Older and simple way: just call
	Speak_Say(hContact, _T("text to speak"))
and the text will be spoken using contact settings. If hContact is NULL, it will use
system settings.
Previous versions only had an ascii version, so if you want to support then you need
to call
	Speak_SayA(hContact, "text to speak")


2. Integrating with meSpeak GUI: for that you have first to register a speak type and
then call the speak functions. In both case you have 2 options:

2.1 Sending the full text: meSpeak GUI will only allow to enable/disable the type.
To register call (in modules loaded):
	Speak_Register("PluginName (DB key)", "name", "Prety name for GUI", "icon_xyz")
And to speak call:
	Speak_SayEx("name", hContact, _T("text to speak"))

2.2 Using templates: you will not pass the text, but some variables. meSpeak handles
the GUI to allow the user to create the text for those variables. These functions
end with WT (with templates).
To register call (in modules loaded):
	AnsiChar *templates[] = { "Name\nDefault\n%var1%\tDescription 1\n%var2%\tDescription2\n%var3%\tDescription 3" };
	Speak_RegisterWT("PluginName (DB key)", "name", "Prety name for GUI", "icon_xyz",
					 templates, 1);
And to speak call:
	TCHAR *variables[] = { _T("var1"), _T("Value 1"), _T("var2"), _T("Value 2"), _T("var3"), _T("Value 3") };
	Speak_SayExWT("name", hContact, 0, variables, 3);
*)

const
  MIID_SPEAK = '{1EF72725-6A83-483B-AA50-8953E359EEAD}';

  {*
  Speak a text

  wParam: (HANDLE) hContact
  lParam: (AnsiChar *) text
  return: 0 on success
  *}
  MS_SPEAK_SAY_A = 'Speak/Say';

  {*
  Speak a unicode text

  wParam: (HANDLE) hContact
  lParam: (WCHAR *) text
  return: 0 on success
  *}
  MS_SPEAK_SAY_W = 'Speak/SayW';

type
  PSPEAK_TYPE = ^TSPEAK_TYPE;
  TSPEAK_TYPE = record
    cbSize: integer;
	  module: PAnsiChar;
	  name: PAnsiChar;					// Internal type name
	  description: PAnsiChar;		// Will be translated
	  icon: PAnsiChar;					// Name off icolib icon
  	// Aditional data if wants to use add to history services
	  templates: ^PAnsiChar;    // Each entry is: "Name\nDefault\n%var%\tDescription\n%var%\tDescription\n%var%\tDescription"
	  numTemplates: integer;
  end;

const
  {*
  Register and speak type

  wParam: (SPEAK_TYPE *) type
  lParam: 0
  return: 0 on success
  *}
  MS_SPEAK_REGISTER = 'Speak/Register';

const
  SPEAK_CHAR  = 1;
  SPEAK_WCHAR = 2;

type
  PSPEAK_ITEM = ^TSPEAK_ITEM;
  TSPEAK_ITEM = record
    cbSize: integer;
    _type: PAnsiChar;		      // Internal type name
  	hContact: TMCONTACT;
	  flags: integer;       // SPEAK_*
	  templateNum: integer;	// -1 to use text
    case boolean of
      true:  (text: PAnsiChar);
      false: (
        variables: pointer;
        numVariables: integer;
      );
  end;

const
  {*
  Speak a text

  wParam: (SPEAK_ITEM *) Item
  lParam: 0
  return: 0 on success
  *}
  MS_SPEAK_SAYEX = 'Speak/SayEx';


{$IFDEF I_AM_A_CONSTANT_THAT_IS_NEVER_DEFINED_BUT_ALLOWS_THE_CODE_BELOW_NOT_TO_BE_COMMENTED}
// Helper functions

static int Speak_SayA(HANDLE hContact, const AnsiChar *text)
{
	return CallService(MS_SPEAK_SAY_A, (WPARAM) hContact, (LPARAM) text);
}

static int Speak_SayW(HANDLE hContact, const WCHAR *text)
{
	return CallService(MS_SPEAK_SAY_W, (WPARAM) hContact, (LPARAM) text);
}

static int Speak_Register(AnsiChar *module, AnsiChar *name, AnsiChar *description, AnsiChar *icon)
{
	SPEAK_TYPE type;

	if (!ServiceExists(MS_SPEAK_REGISTER))
		return -1;

	type.cbSize = sizeof(type);
	type.module = module;
	type.name = name;
	type.description = description;
	type.icon = icon;
	type.templates = NULL;
	type.numTemplates = 0;

	return CallService(MS_SPEAK_REGISTER, (WPARAM) &type, 0);
}

static int Speak_RegisterWT(const AnsiChar *module, const AnsiChar *name, const AnsiChar *description,
							const AnsiChar *icon, AnsiChar **templates, int numTemplates)
{
	SPEAK_TYPE type;

	if (!ServiceExists(MS_SPEAK_REGISTER))
		return -1;

	type.cbSize = sizeof(type);
	type.module = module;
	type.name = name;
	type.description = description;
	type.icon = icon;
	type.templates = templates;
	type.numTemplates = numTemplates;

	return CallService(MS_SPEAK_REGISTER, (WPARAM) &type, 0);
}

static int Speak_SayExA(AnsiChar *type, HANDLE hContact, const AnsiChar *text)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		// Try old service
		return Speak_SayA(hContact, text);

	item.cbSize = sizeof(item);
	item.flags = SPEAK_CHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = -1;
	item.text = text;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}

static int Speak_SayExW(AnsiChar *type, HANDLE hContact, const WCHAR *text)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		// Try old service
		return Speak_SayW(hContact, text);

	item.cbSize = sizeof(item);
	item.flags = SPEAK_WCHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = -1;
	item.text = text;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}

static int Speak_SayExWTA(AnsiChar *type, HANDLE hContact, int templateNum, AnsiChar **variables, int numVariables)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		return -1;

	item.cbSize = sizeof(item);
	item.flags = SPEAK_CHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = templateNum;
	item.variables = variables;
	item.numVariables = numVariables;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}

static int Speak_SayExWTW(AnsiChar *type, HANDLE hContact, int templateNum, WCHAR **variables, int numVariables)
{
	SPEAK_ITEM item;

	if (!ServiceExists(MS_SPEAK_SAYEX))
		return -1;

	item.cbSize = sizeof(item);
	item.flags = SPEAK_WCHAR;
	item.type = type;
	item.hContact = hContact;
	item.templateNum = templateNum;
	item.variables = variables;
	item.numVariables = numVariables;

	return CallService(MS_SPEAK_SAYEX, (WPARAM) &item, 0);
}


#ifdef UNICODE
#  define MS_SPEAK_SAY MS_SPEAK_SAY_W
#  define Speak_Say Speak_SayW
#  define Speak_SayEx Speak_SayExW
#  define Speak_SayExWT Speak_SayExWTW
#else
#  define MS_SPEAK_SAY MS_SPEAK_SAY_A
#  define Speak_Say Speak_SayA
#  define Speak_SayEx Speak_SayExA
#  define Speak_SayExWT Speak_SayExWTA
#endif

{$ENDIF}
{$ENDIF}
