

#include "hdr/modern_commonheaders.h"

#ifdef _DEBUG // REMOVE ME, as soon as ghazan will implement msvcprt.lib msvcrt.lib analogs to vc6tovc7.lib
#include "hdr/modern_skinned_profile.h"


//need to be linked in: atls.lib msvcprt.lib msvcrt.lib 

CSkinnedProfile CSkinnedProfile::_me;


CSkinnedProfile::CSkinnedProfile()
{
	Init();	
}

CSkinnedProfile::~CSkinnedProfile()
{

}

HRESULT CSkinnedProfile::Init()
{
	CAutoCriticalSection Lock( SkinProfile()->_Lock, true );
	SkinnedProfile.clear();
	return S_OK;
}

HRESULT CSkinnedProfile::Clear()
{
	CAutoCriticalSection Lock( SkinProfile()->_Lock, true );
	SkinnedProfile.clear();
	return S_OK;
}


ValueVariant* CSkinnedProfile::_GetValue( const char * szSection, const char * szKey )
{
	KeyList_t::iterator k_it = SkinnedProfile.find( szSection );
	if ( k_it == SkinnedProfile.end() ) 
		return NULL; // Not skinned

	ValueList_t::iterator v_it = k_it->second.find( szKey );
	if ( v_it == k_it->second.end() )
		return NULL; // Not skinned

	return &( v_it->second );
};

BYTE CSkinnedProfile::SpiGetSkinByte( HANDLE hContact, const char * szSection, const char * szKey, const BYTE defValue )
{
	if ( hContact ) 
		return ModernGetSettingByte( hContact, szSection, szKey, defValue ); //per-contact settings are not skinnablr at all

	CAutoCriticalSection Lock( SkinProfile()->_Lock, true );

	ValueVariant* value = SkinProfile()->_GetValue( szSection, szKey );

	if ( value == NULL ) // not skinned => return DB
		return ModernGetSettingWord( hContact, szSection, szKey, defValue );

	else if ( !value->IsEmpty() ) 
		return value->GetByte();

	return defValue; // skinned but empty => return default value
}

WORD CSkinnedProfile::SpiGetSkinWord( HANDLE hContact, const char * szSection, const char * szKey, const WORD defValue )
{
	if ( hContact ) 
		return ModernGetSettingWord( hContact, szSection, szKey, defValue ); //per-contact settings are not skinnablr at all

	CAutoCriticalSection Lock( SkinProfile()->_Lock, true );

	ValueVariant* value = SkinProfile()->_GetValue( szSection, szKey );
	
	if ( value == NULL ) // not skinned => return DB
		return ModernGetSettingWord( hContact, szSection, szKey, defValue );
	
	else if ( !value->IsEmpty() ) 
		return value->GetWord();

	return defValue; // skinned but empty => return default value
}

DWORD CSkinnedProfile::SpiGetSkinDword( HANDLE hContact, const char * szSection, const char * szKey, const DWORD defValue )
{
	if ( hContact ) 
		return ModernGetSettingDword( hContact, szSection, szKey, defValue ); //per-contact settings are not skinnablr at all

	CAutoCriticalSection Lock( SkinProfile()->_Lock, true );

	ValueVariant* value = SkinProfile()->_GetValue( szSection, szKey );

	if ( value == NULL ) // not skinned => return DB
		return ModernGetSettingDword( hContact, szSection, szKey, defValue );
	
	else if ( !value->IsEmpty() ) 
		return value->GetDword();	
	
	return defValue; // skinned but empty => return default value
}

BOOL CSkinnedProfile::SpiCheckSkinned( HANDLE hContact, const char * szSection, const char * szKey )
{
	if ( hContact ) 
		return FALSE;

	CAutoCriticalSection Lock( SkinProfile()->_Lock, true );

	return ( SkinProfile()->_GetValue( szSection, szKey ) == NULL );
}
#endif // _DEBUG // REMOVE ME, as soon as ghazan will implement msvcprt.lib msvcrt.lib analogs to vc6tovc7.lib