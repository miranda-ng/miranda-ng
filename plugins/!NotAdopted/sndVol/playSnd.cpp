#include "plugin.h"
#include "playSnd.h"

namespace playSnd
{

CMusicManager*     g_pMusicManager          = NULL;
CMusicSegment*     g_pMusicSegment          = NULL;
HANDLE             g_hDMusicMessageEvent    = NULL;
DWORD              g_dwVolume               = -77777;
BOOL               g_bInOption              = FALSE;


HRESULT Init()
{
	HRESULT hr;
	g_pMusicManager = new CMusicManager();
	
    if( FAILED( hr = g_pMusicManager->Initialize( NULL ) ) )
        return DXTRACE_ERR( TEXT("Initialize"), hr );

	return S_OK;	
}

void Destroy()
{
	SAFE_DELETE( g_pMusicSegment );
	SAFE_DELETE( g_pMusicManager );
}

//-----------------------------------------------------------------------------
// Name: LoadSegmentFile()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LoadSegmentFile( char* strFileName )
{
	HRESULT hr;

	// Free any previous segment, and make a new one
	SAFE_DELETE( g_pMusicSegment );

	// Have the loader collect any garbage now that the old 
	// segment has been released
	g_pMusicManager->CollectGarbage();

	// Set the media path based on the file name (something like C:\MEDIA)
	// to be used as the search directory for finding DirectMusic content
	// related to this file.
	TCHAR strMediaPath[MAX_PATH];
	_tcscpy( strMediaPath, strFileName );
	TCHAR* strLastSlash = _tcsrchr(strMediaPath, TEXT('\\'));
	if(strLastSlash != NULL)
	{
		*strLastSlash = 0;
		if( FAILED( hr = g_pMusicManager->SetSearchDirectory( strMediaPath ) ) )
			return DXTRACE_ERR( TEXT("SetSearchDirectory"), hr );
	}
	else
	{
		if( FAILED( hr = g_pMusicManager->SetSearchDirectory( "" ) ) )
			return DXTRACE_ERR( TEXT("SetSearchDirectory"), hr );
	}

	// For DirectMusic must know if the file is a standard MIDI file or not
	// in order to load the correct instruments.
	BOOL bMidiFile = FALSE;
	if( strstr( strFileName, ".mid" ) != NULL ||
		strstr( strFileName, ".rmi" ) != NULL ) 
	{
		bMidiFile = TRUE;
	}

	BOOL bWavFile = FALSE;
	if( strstr( strFileName, ".wav" ) != NULL )
	{
		bWavFile = TRUE;
	}        

	// Load the file into a DirectMusic segment 
	if( FAILED( g_pMusicManager->CreateSegmentFromFile( &g_pMusicSegment, strFileName, 
														TRUE, bMidiFile ) ) )
	{
		// Not a critical failure, so just update the status
		return E_FAIL; 
	}

	return S_OK;
}

BOOL WINAPI PlaySound(char* pszSound, HMODULE hmod, DWORD fdwSound)
{
	/*
	if(!g_bInOption)
	{
		DWORD volume = (DWORD)DBGetContactSettingDword(NULL, SERVICENAME, "Volume", 100);
			
		if(volume != g_dwVolume)
		{
			g_dwVolume = volume;
			playSnd::SetVolume(g_dwVolume);
		}
	}
	*/
    HRESULT hr;
	
    if( FAILED( hr = LoadSegmentFile(pszSound) ) )
        return FALSE;
	
	if( FAILED( hr = g_pMusicSegment->SetRepeats( 0 ) ) )
		return FALSE;
	
    // Play the segment and wait. The DMUS_SEGF_BEAT indicates to play on the 
    // next beat if there is a segment currently playing. 
    if( FAILED( hr = g_pMusicSegment->Play( DMUS_SEGF_BEAT ) ) )
        return FALSE;

	return TRUE;
}


BOOL SetVolume(long Value)
{
	long value = (int)(log10(Value)/2.0*5000.0-4000.0);
	IDirectMusicPerformance* pPerf = NULL;               
	if( g_pMusicManager )
		pPerf = g_pMusicManager->GetPerformance();
	if( NULL == pPerf )
		return FALSE;

	// Adjust the slider position to match GUID_PerfMasterTempo range
	pPerf->SetGlobalParam(GUID_PerfMasterVolume, (void*)&value, sizeof(long));
	
	return TRUE;
}


}
