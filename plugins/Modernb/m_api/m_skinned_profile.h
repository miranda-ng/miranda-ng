#ifdef m_skinned_profile_h__
#define m_skinned_profile_h__

#define MS_SKINPROFILE_GET_INTERFACE

#ifdef _cplusplus
extern "C" {
#endif //_cplusplus

struct SKIN_PROFILE_INTERFACE
{
    DWORD   cbSize;  
    
    int  (*pfnGetByte) ( HANDLE, const char *, const char *,  int,  const char *szFile, const int nLine);
    int  (*pfnGetWord) ( HANDLE, const char *, const char *,  int,  const char *szFile, const int nLine);
    int  (*pfnGetDword)( HANDLE, const char *, const char *,  int,  const char *szFile, const int nLine);

};

#ifdef SPI_DECLARE_INTERFACE_INIT
#include <m_database.h>
__forceinline int mir_getSPI( struct SKIN_PROFILE_INTERFACE * pspi)
{
    if ( !ServiceExists( MS_SKINPROFILE_GET_INTERFACE ) )
    {
        pspi->pfnGetByte  = DBGetContactSettingByte_Helper;
        pspi->pfnGetWord  = DBGetContactSettingWord_Helper;
        pspi->pfnGetDword = DBGetContactSettingDword_Helper;
        return 0;
    }
    else
    {
        pspi->cbSize = sizeof( struct SKIN_PROFILE_INTERFACE );
        return CallService( MS_SKINPROFILE_GET_INTERFACE, 0, (LPARAM) pspi );
    }
}
#endif

#ifdef _cplusplus
};
#endif //_cplusplus


#endif // m_skinned_profile_h__