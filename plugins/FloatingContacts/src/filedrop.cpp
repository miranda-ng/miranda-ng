#include "stdhdr.h"

static	void	ProcessDroppedItems	( char **ppDroppedItems, int nCount, char **ppFiles );
static	int		CountDroppedFiles	( char **ppDroppedItems, int nCount  );
static	BOOL	OnDropFiles			( HDROP hDrop, ThumbInfo *pThumb );

HRESULT STDMETHODCALLTYPE CDropTarget::QueryInterface(REFIID riid,LPVOID *ppvObj)
{
	if ( IsEqualIID( riid, IID_IDropTarget )) 
	{
		*ppvObj = this;
		this->AddRef();
		return S_OK;
	}
	
	*ppvObj = NULL;

	return ( E_NOINTERFACE );
}

ULONG STDMETHODCALLTYPE CDropTarget::AddRef( )
{
	return ++this->refCount;
}

ULONG STDMETHODCALLTYPE CDropTarget::Release( )
{
	int res = --this->refCount;
	if (!res) delete this;
	return res;
}


HRESULT STDMETHODCALLTYPE CDropTarget::DragOver( DWORD fKeyState, POINTL pt, DWORD *pdwEffect )
{
	*pdwEffect = 0;
	
	if ( hwndCurDrag == NULL ) 
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
	{
		*pdwEffect |= DROPEFFECT_COPY;
	}
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CDropTarget::DragEnter( IDataObject *pData, DWORD fKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC	feFile	 = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	FORMATETC	feText	 = { CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if ( S_OK == pData->QueryGetData( &feFile ) ||
		 S_OK == pData->QueryGetData( &feText ))
	{
		POINT shortPt;
		shortPt.x = pt.x; 
		shortPt.y = pt.y;
		
		HWND hwnd = WindowFromPoint( shortPt );
		
		ThumbInfo *pThumb = thumbList.FindThumb(hwnd);
		if (pThumb)
		{
			hwndCurDrag = hwnd;
			pThumb->ThumbSelect( TRUE );
		}
	}

	return DragOver( fKeyState, pt, pdwEffect);
}


HRESULT STDMETHODCALLTYPE CDropTarget::DragLeave( )
{
	ThumbInfo *pThumb = thumbList.FindThumb( hwndCurDrag );

	if ( NULL != pThumb )
	{
		pThumb->ThumbDeselect( TRUE );
	}

	hwndCurDrag = NULL;
	
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CDropTarget::Drop( IDataObject *pData,DWORD fKeyState,POINTL pt,DWORD *pdwEffect)
{
	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	*pdwEffect = DROPEFFECT_NONE;
	
	if (hwndCurDrag == NULL)
		return S_OK;

	ThumbInfo *pThumb = (ThumbInfo*) GetWindowLongPtr( hwndCurDrag, GWLP_USERDATA );
	if (pThumb == NULL)
		return S_OK;

	STGMEDIUM stg;
	bool bFormatText = false;
	if ( S_OK != pData->GetData(&fe,&stg))
	{
		fe.cfFormat = CF_UNICODETEXT;

		if ( S_OK != pData->GetData(&fe,&stg))
		{
			return S_OK;
		}
		else
		{
			bFormatText = true;
		}
	}
	
	if (!bFormatText)
	{
		HDROP hDrop = (HDROP) stg.hGlobal;
		if (hDrop != NULL)
		{
			OnDropFiles(hDrop, pThumb);
		}
	}
	else
	{
		TCHAR *pText = (TCHAR*) GlobalLock(stg.hGlobal);
		if (pText != NULL)
		{
			SendMsgDialog(hwndCurDrag, pText);
			GlobalUnlock(stg.hGlobal);
		}
	}

	if (stg.pUnkForRelease != NULL)
	{
		stg.pUnkForRelease->Release();
	}
	else 
	{
		GlobalFree(stg.hGlobal);
	}

	DragLeave();
	
	return S_OK;
}

///////////////////////////////////////////////////////
// Send files processing

BOOL OnDropFiles( HDROP hDrop, ThumbInfo *pThumb )
{
	UINT nDroppedItemsCount = DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0);

	char **ppDroppedItems = ( char** )malloc( sizeof(char*)*( nDroppedItemsCount + 1));
	
	if ( ppDroppedItems == NULL )
	{
		return FALSE;
	}

	ppDroppedItems[ nDroppedItemsCount ] = NULL;
	
	char  szFilename[MAX_PATH];
	for (UINT iItem = 0; iItem < nDroppedItemsCount; ++iItem ) 
	{
		DragQueryFileA( hDrop, iItem, szFilename, sizeof( szFilename ));
		ppDroppedItems[ iItem ] = _strdup( szFilename );
	}
	
	UINT nFilesCount = CountDroppedFiles( ppDroppedItems, nDroppedItemsCount );
	
	char **ppFiles = ( char** )malloc( sizeof( char *)* ( nFilesCount+1));
	
	BOOL bSuccess = FALSE;
	if (ppFiles != NULL)
	{
		ppFiles[nFilesCount] = NULL;

		ProcessDroppedItems(ppDroppedItems, nDroppedItemsCount, ppFiles);

		bSuccess = (BOOL)CallService(MS_CLIST_CONTACTFILESDROPPED, (WPARAM)pThumb->hContact, (LPARAM)ppFiles); 

		for (UINT iItem = 0; iItem < nFilesCount ; ++iItem )
			free(ppFiles[iItem]);

		free(ppFiles);
	}

	// Cleanup
	for (UINT iItem = 0; ppDroppedItems[ iItem ]; ++iItem ) 
	{
		free( ppDroppedItems[ iItem ] );
	}

	free( ppDroppedItems );

	return bSuccess;
}


static int CountFiles( char *szItem )
{
	int nCount = 0;
	WIN32_FIND_DATAA	fd;
	
	HANDLE hFind = FindFirstFileA( szItem, &fd );

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				// Skip parent directories
				if (( 0 != strcmp( fd.cFileName, "." )) &&
					 ( 0 != strcmp( fd.cFileName, ".." )) )
				{
					char szDirName[MAX_PATH];
					strncpy( szDirName, szItem, MAX_PATH - 1);

					if ( NULL != strstr( szItem, "*.*" ))
					{
						size_t offset = mir_strlen( szDirName ) - 3;
						mir_snprintf(szDirName + offset, SIZEOF( szDirName) - offset, "%s\0", fd.cFileName);
					}
					
					++nCount;
					strcat( szDirName, "\\*.*" );
					nCount += CountFiles( szDirName );
				}
			}
			else
			{
				++nCount;
			}
		}
		while( FALSE != FindNextFileA( hFind, &fd ));
	}

	return nCount;
}



static void SaveFiles( char *szItem, char **ppFiles, int *pnCount )
{
	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA( szItem, &fd );

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				// Skip parent directories
				if (( 0 != strcmp( fd.cFileName, "." )) &&
					 ( 0 != strcmp( fd.cFileName, ".." )) )
				{
					char szDirName[MAX_PATH];
					strncpy( szDirName, szItem, MAX_PATH - 1);

					if ( NULL != strstr( szItem, "*.*" ))
					{
						size_t offset = mir_strlen( szDirName ) - 3;
						mir_snprintf(szDirName + offset, SIZEOF( szDirName) - offset, "%s\0", fd.cFileName);
					}
					
					ppFiles[ *pnCount ] = _strdup( szDirName );
					++( *pnCount );

					strcat( szDirName, "\\*.*" );
					SaveFiles( szDirName, ppFiles, pnCount );
					
				}
			}
			else
			{
				size_t nSize = sizeof(char) * ( mir_strlen( szItem ) + mir_strlen( fd.cFileName ) + sizeof( char ));
				char  *szFile = (char*) malloc( nSize ) ;
				
				strncpy( szFile, szItem, nSize - 1); 
				
				if ( NULL != strstr( szFile, "*.*" ))
				{
					szFile[ mir_strlen( szFile ) - 3 ] = '\0';
					strncat(szFile, fd.cFileName, nSize - mir_strlen(szFile));
				}
								
				ppFiles[ *pnCount ] = szFile;
				++( *pnCount );
			}
		}
		while( FALSE != FindNextFileA( hFind, &fd ));
	}
}


static void ProcessDroppedItems( char **ppDroppedItems, int nCount, char **ppFiles  )
{
	int fileCount = 0;

	for (int i = 0; i < nCount; ++i )
	{
		SaveFiles( ppDroppedItems[ i ], ppFiles, &fileCount );
	}
}


static int CountDroppedFiles( char **ppDroppedItems, int nCount  )
{
	int fileCount = 0;
	
	for (int i = 0; i < nCount; ++i )
	{
		fileCount += CountFiles( ppDroppedItems[ i ] );
	}

	return fileCount;
}


///////////////////////////////////////////////////////////////////////////////
// Init/destroy
void InitFileDropping()
{
}

void FreeFileDropping(void)
{
}

void RegisterFileDropping( HWND hwnd, CDropTarget* pdropTarget )
{
	RegisterDragDrop( hwnd, (IDropTarget*)pdropTarget );
}

void UnregisterFileDropping( HWND hwnd )
{
	RevokeDragDrop( hwnd );
}
