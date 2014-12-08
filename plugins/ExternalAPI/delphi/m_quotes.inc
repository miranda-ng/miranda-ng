{$IFNDEF M_QUOTES}
{$DEFINE M_QUOTES}

const
{*
 *	Export the contact (or all contacts) to xml file
 *		wParam = (WPARAM)(HANDLE)hContact to export or 0 to export all contacts
 *		lParam = (LPARAM)(const char*)pszFileName - pointer to file name to export or
 *				  0 in this case the dialog to select a file to export would be shown
 *		returns 0 if export was successfull, 
 *			    -1 if user canceled export and 
 *				value greater than zero if error occurred during exporting
 *}
  MS_QUOTES_EXPORT:PAnsiChar = 'Quotes/Export';

{*
 *	Import the contact (or all contacts) from xml file
 *		wParam = flags
 *		lParam = (LPARAM)(const char*)pszFileName - pointer to file name to import or
 *				  0 in this case the dialog to select a file to import would be shown
 *		returns 0 if import was successfull, 
 *			    -1 if user canceled import and 
 *				value greater than zero if error occurred during importing
 *}

// if contact(s) exists user would be asked to overwrite these contacts
// #define QUOTES_IMPORT_PROMPT_TO_OVERWRITE_EXISTING_CONTACTS 0x0000
// if contact(s) exists it would be overwrite without any prompt
// #define QUOTES_IMPORT_SILENT_OVERWRITE_EXISTING_CONTACTS 0x0001
// if contact(s) exists during importing it would be ignored
  QUOTES_IMPORT_SKIP_EXISTING_CONTACTS = 2;

  MS_QUOTES_IMPORT:PAnsiChar = 'Quotes/Import';

{$ENDIF}

