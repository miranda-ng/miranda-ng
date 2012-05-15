#ifndef __m_nconvers_h__
#define __m_nconvers_h__

//replace smiley tags in a rich edit control... 
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (NCONVERS_GETICON*) &ncgi;
//return: TRUE if found, FALSE if not

typedef struct 
{
  int cbSize;             // = sizeof(NCONVERS_GETSMILEY)
  char* Protocolname;     // NULL means 'default'
  char* SmileySequence;   // character string containing the smiley 
  HICON SmileyIcon;       // RETURN VALUE: this is filled with the icon handle... 
                          // do not destroy!
  int Smileylength;       //l ength of the smiley that is found.
} NCONVERS_GETICON;

#define MS_NCONVERS_GETSMILEYICON "nConvers/GetSmileyIcon"


#endif // __m_nconvers_h__