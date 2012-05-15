/*____________________________________________________________________________
 *	Copyright (C) 2003 PGP Corporation
 *	All rights reserved.
 *
 *	Size of UTF-16 string is always given in 16 bit characters. This includes 
 *	PGPGetKeyDBObjDataPropertyU16 and PGPGetKeyDBObjAllocatedDataPropertyU16
 *	for which void* is assumed to be PGPChar16* where applicable. 
 *	So you can write like this:
 *
 *		wchar_t s[80];
 *		PGPSize size;
 *		PGPGetPrimaryUserIDNameU16( key, s, sizeof(s)/sizeof(s[0]), &size );
 *            
 *	$Id: pgpAPIAdapter.h,v 1.12 2004/05/07 08:29:12 bgaiser Exp $
 *____________________________________________________________________________*/
#ifndef Included_pgpAPIAdapter_h   /* [ */
#define Included_pgpAPIAdapter_h

#include "pgpKeys.h"
#include "pgpKeyServer.h"

#include "pgpMemoryMgr.h"
#include "pgpUtilities.h"
#include "pgpPFLErrors.h"
#include "pgpFileSpec.h"
#include "pgpHashWords.h"
#include "pgpShareFile.h"

PGP_BEGIN_C_DECLARATIONS

PGPError PGPGetErrorStringU8( PGPError theError, PGPSize bufferSize, PGPChar8 * theString );
PGPError PGPGetErrorStringU16( PGPError theError, PGPSize bufferSize, PGPChar16 * theString );

PGPError PGPGetPGPsdkVersionStringU8( PGPChar8 versionString[ 256 ] );
PGPError PGPGetPGPsdkVersionStringU16( PGPChar16 versionString[ 256 ] );

PGPError PGPNewFileSpecFromFullPathU8( PGPContextRef context, const PGPChar8 *path, PGPFileSpecRef *ref );
PGPError PGPNewFileSpecFromFullPathU16( PGPContextRef context, const PGPChar16 *path, PGPFileSpecRef *ref );

PGPError PGPGetFullPathFromFileSpecU8( PGPFileSpecRef fileRef, PGPChar8 **fullPathPtr);
PGPError PGPGetFullPathFromFileSpecU16( PGPFileSpecRef fileRef, PGPChar16 **fullPathPtr);

PGPError PGPRenameFileU8( PGPFileSpecRef fileRef, const PGPChar8 *newName );
PGPError PGPRenameFileU16( PGPFileSpecRef fileRef, const PGPChar16 *newName );

PGPOptionListRef PGPOPassphraseBufferU8( PGPContextRef context, const PGPChar8 *passphrase, PGPSize passphraseLength);
PGPOptionListRef PGPOPassphraseBufferU16( PGPContextRef context, const PGPChar16 *passphrase, PGPSize passphraseLength);

PGPOptionListRef PGPOPassphraseU8( PGPContextRef context, const PGPChar8 *passphrase);
PGPOptionListRef PGPOPassphraseU16( PGPContextRef context, const PGPChar16 *passphrase);

PGPOptionListRef PGPOPGPMIMEEncodingU8(PGPContextRef context, PGPBoolean mimeEncoding, PGPSize *mimeBodyOffset, PGPChar8 mimeSeparator[ kPGPMimeSeparatorSize ]);
PGPOptionListRef PGPOPGPMIMEEncodingU16(PGPContextRef context, PGPBoolean mimeEncoding, PGPSize *mimeBodyOffset, PGPChar16 mimeSeparator[ kPGPMimeSeparatorSize ]);

PGPOptionListRef PGPOKeyGenNameU8( PGPContextRef context, const void *name, PGPSize nameLength );
PGPOptionListRef PGPOKeyGenNameU16( PGPContextRef context, const void *name, PGPSize nameLength );

PGPOptionListRef PGPOPreferredKeyServerU8( PGPContextRef context, PGPChar8 const * server );
PGPOptionListRef PGPOPreferredKeyServerU16( PGPContextRef context, PGPChar16 const * server );

PGPOptionListRef PGPOCommentStringU8( PGPContextRef context,PGPChar8 const *comment);
PGPOptionListRef PGPOCommentStringU16( PGPContextRef context,PGPChar16 const *comment);

PGPOptionListRef PGPOVersionStringU8( PGPContextRef context, PGPChar8 const *version);
PGPOptionListRef PGPOVersionStringU16( PGPContextRef context, PGPChar16 const *version);

PGPOptionListRef PGPOFileNameStringU8( PGPContextRef context, PGPChar8 const *fileName);
PGPOptionListRef PGPOFileNameStringU16( PGPContextRef context, PGPChar16 const *fileName);

PGPOptionListRef PGPOSigRegularExpressionU8(PGPContextRef context, PGPChar8 const *regularExpression);
PGPOptionListRef PGPOSigRegularExpressionU16(PGPContextRef context, PGPChar16 const *regularExpression);

PGPError PGPGetKeyDBObjDataPropertyU8( PGPKeyDBObjRef key, PGPKeyDBObjProperty whichProperty, void *buffer, PGPSize bufferSize, PGPSize *dataSize);
PGPError PGPGetKeyDBObjDataPropertyU16( PGPKeyDBObjRef key, PGPKeyDBObjProperty whichProperty, void *buffer, PGPSize bufferSize, PGPSize *dataSize);

PGPError PGPGetKeyDBObjAllocatedDataPropertyU8( PGPKeyDBObjRef obj, PGPKeyDBObjProperty prop, void **buffer, PGPSize *dataSize );
PGPError PGPGetKeyDBObjAllocatedDataPropertyU16( PGPKeyDBObjRef obj, PGPKeyDBObjProperty prop, void **buffer, PGPSize *dataSize );

PGPError PGPNewKeyDBObjDataFilterU8( PGPContextRef context, PGPKeyDBObjProperty prop, const void *val, PGPSize len, PGPMatchCriterion match, PGPFilterRef *outFilter );
PGPError PGPNewKeyDBObjDataFilterU16( PGPContextRef context, PGPKeyDBObjProperty prop, const void *val, PGPSize len, PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError PGPLDAPQueryFromFilterU8( PGPFilterRef filter, PGPChar8 **queryOut );
PGPError PGPLDAPQueryFromFilterU16( PGPFilterRef filter, PGPChar16 **queryOut );

PGPError PGPLDAPX509QueryFromFilterU8( PGPFilterRef filter,PGPChar8 **queryOut );
PGPError PGPLDAPX509QueryFromFilterU16( PGPFilterRef filter,PGPChar16 **queryOut );

PGPError PGPHKSQueryFromFilterU8( PGPFilterRef filter, PGPChar8 **queryOut );
PGPError PGPHKSQueryFromFilterU16( PGPFilterRef filter, PGPChar16 **queryOut );

PGPError PGPNetToolsCAHTTPQueryFromFilterU8( PGPFilterRef filter, PGPChar8 **queryOut );
PGPError PGPNetToolsCAHTTPQueryFromFilterU16( PGPFilterRef filter, PGPChar16 **queryOut );

PGPError PGPAddUserIDU8( PGPKeyDBObjRef key, PGPChar8 const *userID, PGPOptionListRef firstOption, ...);
PGPError PGPAddUserIDU16( PGPKeyDBObjRef key, PGPChar16 const *userID, PGPOptionListRef firstOption, ...);

PGPInt32 PGPCompareUserIDStringsU8(PGPChar8 const *a, PGPChar8 const *b);
PGPInt32 PGPCompareUserIDStringsU16(PGPChar16 const *a, PGPChar16 const *b);

PGPError PGPGetKeyIDStringU8( PGPKeyID const * ref, PGPKeyIDStringType type, PGPChar8 outString[ kPGPMaxKeyIDStringSize ] );
PGPError PGPGetKeyIDStringU16( PGPKeyID const * ref, PGPKeyIDStringType type, PGPChar16 outString[ kPGPMaxKeyIDStringSize ] );

PGPError PGPNewKeyIDFromStringU8( const PGPChar8 *string, PGPPublicKeyAlgorithm pkalg, PGPKeyID * outID );
PGPError PGPNewKeyIDFromStringU16( const PGPChar16 *string, PGPPublicKeyAlgorithm pkalg, PGPKeyID * outID );

PGPError PGPSetPKCS11DrvFileU8( PGPChar8 *module );
PGPError PGPSetPKCS11DrvFileU16( PGPChar16 *module );

PGPError PGPGetTokenInfoDataPropertyU8( PGPContextRef context, PGPUInt32 tokenNumber, PGPTokenProperty prop, PGPByte *value, PGPSize size, PGPSize *sizeout );
PGPError PGPGetTokenInfoDataPropertyU16( PGPContextRef context, PGPUInt32 tokenNumber, PGPTokenProperty prop, void *value, PGPSize size, PGPSize *sizeout );

PGPError PGPCreateDistinguishedNameU8( PGPContextRef context, PGPChar8 const *str, PGPByte **pdname, PGPSize *pdnamelen );
PGPError PGPCreateDistinguishedNameU16( PGPContextRef context, PGPChar16 const *str, PGPByte **pdname, PGPSize *pdnamelen );

PGPError PGPGetPrimaryUserIDNameU8(PGPKeyDBObjRef key, PGPChar8 *buffer, PGPSize bufferSize, PGPSize *dataSize);
PGPError PGPGetPrimaryUserIDNameU16(PGPKeyDBObjRef key, PGPChar16 *buffer, PGPSize bufferSize, PGPSize *dataSize);

PGPError PGPGetHashWordStringU8( PGPUInt32 index, PGPHashWordList list, PGPChar8 hashWordString[ 12 ] );
PGPError PGPGetHashWordStringU16( PGPUInt32 index, PGPHashWordList list, PGPChar16 hashWordString[ 12 ] );

PGPError PGPSetShareFileUserIDU8( PGPShareFileRef shareFileRef, const PGPUTF8 *userID );
PGPError PGPSetShareFileUserIDU16( PGPShareFileRef shareFileRef, const PGPChar16 *userID );

PGPError PGPGetShareFileUserIDU8( PGPShareFileRef shareFileRef, PGPSize bufferSize, PGPUTF8 *userID, PGPSize *fullSize );
PGPError PGPGetShareFileUserIDU16( PGPShareFileRef shareFileRef, PGPSize bufferSize, PGPChar16 *userID, PGPSize *fullSize );

PGPError PGPGetTARCacheObjDataPropertyU8( PGPTARCacheObjRef obj, PGPTARCacheObjProperty prop, void *buffer, PGPSize bufSize, PGPSize *dataSize );
PGPError PGPGetTARCacheObjDataPropertyU16( PGPTARCacheObjRef obj, PGPTARCacheObjProperty prop, void *buffer, PGPSize bufSize, PGPSize *dataSize );


/* ----- SDK UI ----- */
PGPUInt32 PGPEstimatePassphraseQualityU8( const PGPChar8 *passphrase );
PGPUInt32 PGPEstimatePassphraseQualityU16( const PGPChar16 *passphrase );

PGPOptionListRef PGPOUIDialogPromptU8( PGPContextRef context, const PGPChar8 *prompt );
PGPOptionListRef PGPOUIDialogPromptU16( PGPContextRef context, const PGPChar16 *prompt );

PGPOptionListRef PGPOUIWindowTitleU8( PGPContextRef context, const PGPChar8 *title );
PGPOptionListRef PGPOUIWindowTitleU16( PGPContextRef context, const PGPChar16 *title );

PGPOptionListRef PGPOUIOutputPassphraseU8( PGPContextRef context, PGPChar8 **passphrase );
PGPOptionListRef PGPOUIOutputPassphraseU16( PGPContextRef context, PGPChar16 **passphrase );

PGPOptionListRef PGPOUICheckboxU8(PGPContextRef context, PGPUInt32 itemID, const PGPChar8 *title, const PGPChar8 *description, PGPUInt32 initialValue, PGPUInt32 *resultPtr, PGPOptionListRef firstOption, ...);
PGPOptionListRef PGPOUICheckboxU16(PGPContextRef context, PGPUInt32 itemID, const PGPChar16 *title, const PGPChar16 *description, PGPUInt32 initialValue, PGPUInt32 *resultPtr, PGPOptionListRef firstOption, ...);

PGPOptionListRef PGPOUIPopupListU8(PGPContextRef context, PGPUInt32 itemID, const PGPChar8 *title, const PGPChar8 *description,PGPUInt32 listItemCount, const PGPChar16 *listItems[], PGPUInt32 initialValue, PGPUInt32 *resultPtr, PGPOptionListRef firstOption, ...);
PGPOptionListRef PGPOUIPopupListU16(PGPContextRef context, PGPUInt32 itemID, const PGPChar16 *title, const PGPChar16 *description,PGPUInt32 listItemCount, const PGPChar16 *listItems[], PGPUInt32 initialValue, PGPUInt32 *resultPtr, PGPOptionListRef firstOption, ...);


/* ----- SDK Network ----- */
PGPOptionListRef PGPONetURLU8(PGPContextRef context, const PGPChar8 *url);
PGPOptionListRef PGPONetURLU16(PGPContextRef context, const PGPChar16 *url);

PGPOptionListRef PGPONetHostNameU8(PGPContextRef context, const PGPChar8 *hostName, PGPUInt16 port);
PGPOptionListRef PGPONetHostNameU16(PGPContextRef context, const PGPChar16 *hostName, PGPUInt16 port);

PGPOptionListRef PGPOKeyServerKeyStoreDNU8(PGPContextRef context, const PGPChar8 *szKeyStoreDn);
PGPOptionListRef PGPOKeyServerKeyStoreDNU16(PGPContextRef context, const PGPChar16 *szKeyStoreDn);

PGPError PGPGetKeyServerHostNameU8(PGPKeyServerRef inKeyServerRef, PGPChar8 ** outHostName); /* Use PGPFreeData to free */
PGPError PGPGetKeyServerHostNameU16(PGPKeyServerRef inKeyServerRef, PGPChar16 ** outHostName); /* Use PGPFreeData to free */

PGPError PGPGetKeyServerPathU8(PGPKeyServerRef inKeyServerRef, PGPChar8 ** outPath); /* Use PGPFreeData to free */
PGPError PGPGetKeyServerPathU16(PGPKeyServerRef inKeyServerRef, PGPChar16 ** outPath); /* Use PGPFreeData to free */

PGPError PGPGetLastKeyServerErrorStringU8(PGPKeyServerRef inKeyServerRef,PGPChar8 ** outErrorString);
PGPError PGPGetLastKeyServerErrorStringU16(PGPKeyServerRef inKeyServerRef,PGPChar16 ** outErrorString);

PGPError PGPGetProxyServerU8(PGPContextRef context, PGPProxyServerType type, PGPChar8 **proxyAddress, PGPUInt16 *proxyPort );
PGPError PGPGetProxyServerU16(PGPContextRef context, PGPProxyServerType type, PGPChar16 **proxyAddress, PGPUInt16 *proxyPort );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpAPIAdapter_h   */


/*__Editor_settings____
 *
 *	Local Variables:
 *	tab-width: 4
 *	End:
 *	vi: ts=4 sw=4
 *	vim: si
 *_____________________*/

