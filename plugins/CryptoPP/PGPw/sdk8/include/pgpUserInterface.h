/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	This file contains the prototypes for functions which use UI to interact
	with the user.

	$Id: pgpUserInterface.h,v 1.21 2003/09/24 03:09:32 ajivsov Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUserInterface_h	/* [ */
#define Included_pgpUserInterface_h

#include "pgpPubTypes.h"
#include "pgpTLS.h"

#if PGP_WIN32
#include "windows.h"
#endif

PGP_BEGIN_C_DECLARATIONS

#if PGP_MACINTOSH
#pragma options align=mac68k
#endif

enum PGPAdditionalRecipientRequestEnforcement_
{
	kPGPARREnforcement_Invalid	= 0,
	kPGPARREnforcement_None		= 1,
	kPGPARREnforcement_Warn		= 2,
	kPGPARREnforcement_Strict	= 3,

	PGP_ENUM_FORCE( PGPAdditionalRecipientRequestEnforcement_ )
};
PGPENUM_TYPEDEF( PGPAdditionalRecipientRequestEnforcement_,
				PGPAdditionalRecipientRequestEnforcement );

enum PGPRecipientSpecType_
{
	kPGPRecipientSpecType_Invalid	= 0,
	kPGPRecipientSpecType_Key		= 1,
	kPGPRecipientSpecType_UserID	= 2,
	kPGPRecipientSpecType_KeyID		= 3,
	
	PGP_ENUM_FORCE( PGPRecipientSpecType_ )
};
PGPENUM_TYPEDEF( PGPRecipientSpecType_, PGPRecipientSpecType );

enum PGPRecipientSpecFlags_
{
	kPGPRecipientSpecFlags_Locked	= (1UL << 0),
	
	PGP_ENUM_FORCE( PGPRecipientSpecFlags_ )
};

typedef PGPFlags	PGPRecipientSpecFlags;

typedef struct PGPRecipientSpec
{
	PGPRecipientSpecType	type;
	PGPRecipientSpecFlags	flags;
	PGPUInt32				reserved32[3];	/* Must be zero */
	
	union
	{
		PGPKeyDBObjRef	key;
		PGPChar8			userIDStr[256];		/* Null terminated string */
		PGPKeyID		keyID;
	} u;

} PGPRecipientSpec;

typedef struct PGPKeyServerSpec
{
	PGPKeyServerRef		server;
	const PGPChar8		*serverName;	/* Optional */
	const PGPChar8		*serverDomain;	/* Optional */

} PGPKeyServerSpec;

#if PGP_MACINTOSH
#pragma options align=reset
#endif

PGPError	PGPRecipientDialog( PGPContextRef context, PGPKeyDBRef sourceKeys,
					PGPBoolean alwaysDisplayDialog, PGPKeyDBRef *recipientKeys,
					PGPOptionListRef firstOption, ... );

PGPError	PGPPassphraseDialog( PGPContextRef context,
					PGPOptionListRef firstOption, ... );

PGPError	PGPConfirmationPassphraseDialog( PGPContextRef context,
					PGPOptionListRef firstOption, ... );

PGPError	PGPKeyPassphraseDialog( PGPContextRef context,
					PGPKeyDBObjRef keyDBObject, PGPOptionListRef firstOption, ... );

PGPError	PGPSigningPassphraseDialog( PGPContextRef context,
					PGPKeyDBRef sourceKeys, PGPKeyDBObjRef *signingKey,
					PGPOptionListRef firstOption, ... );

PGPError	PGPDecryptionPassphraseDialog( PGPContextRef context,							
					PGPKeySetRef recipientKeys, PGPUInt32 keyIDCount,
					const PGPKeyID keyIDList[], PGPKeyDBObjRef *decryptionKey,
					PGPOptionListRef firstOption, ... );

PGPError	PGPConventionalEncryptionPassphraseDialog( PGPContextRef context,
					PGPOptionListRef firstOption, ... );

PGPError	PGPConventionalDecryptionPassphraseDialog( PGPContextRef context,							
					PGPOptionListRef firstOption, ... );

PGPError	PGPOptionsDialog( PGPContextRef context,
					PGPOptionListRef firstOption, ... );

PGPError	PGPCollectRandomDataDialog( PGPContextRef context,
					PGPUInt32 neededEntropyBits,
					PGPOptionListRef firstOption, ... );

PGPError	PGPSearchKeyServerDialog(
					PGPContextRef 			context,
					PGPUInt32				serverCount,
					const PGPKeyServerSpec 	serverList[],
					PGPtlsContextRef		tlsContext,
					PGPBoolean				searchAllServers,
					PGPKeyDBRef 			*foundKeys,
					PGPOptionListRef 		firstOption, ... );

PGPError	PGPSendToKeyServerDialog(
					PGPContextRef 			context,
					const PGPKeyServerSpec 	*server,
					PGPtlsContextRef		tlsContext,
					PGPKeySetRef 			keysToSend,
					PGPKeySetRef 			*failedKeys,
					PGPOptionListRef 		firstOption, ... );
									
/*
** Returns a value in the range 0-100 which crudely estimates
** the "quality" of a passphrase.
*/

#undef		PGPEstimatePassphraseQuality
PGPUInt32	PGPEstimatePassphraseQuality( const PGPChar8 *passphrase );

/* General dialog options */

#undef				PGPOUIDialogPrompt
PGPOptionListRef	PGPOUIDialogPrompt( PGPContextRef context,
							const PGPChar8 *prompt );

#undef				PGPOUIWindowTitle
PGPOptionListRef	PGPOUIWindowTitle( PGPContextRef context,
							const PGPChar8 *title );

PGPOptionListRef	PGPOUIDialogOptions( PGPContextRef context,
							PGPOptionListRef firstOption, ... );

PGPOptionListRef	PGPOUIDialogContextHelpButton(PGPContextRef	context,	
							PGPBoolean		showDialogContextHelpButton);

#if PGP_WIN32
PGPOptionListRef	PGPOUIParentWindowHandle( PGPContextRef context,
							HWND hwndParent );
#endif

/* All passphrase dialogs */

/* Caller should free passphrase with PGPFreeData() */
#undef				PGPOUIOutputPassphrase
PGPOptionListRef	PGPOUIOutputPassphrase( PGPContextRef context,
							PGPChar8 **passphrase );

PGPOptionListRef	PGPOUIMinimumPassphraseQuality( PGPContextRef context,
							PGPUInt32 minimumPassphraseQuality );

PGPOptionListRef	PGPOUIMinimumPassphraseLength( PGPContextRef context,
							PGPUInt32 minimumPassphraseLength );

/* Will cause the dialog to cancel if there has been no activity for x seconds */							
PGPOptionListRef	PGPOUIDialogTimeout( PGPContextRef context,
							PGPUInt32 seconds);

/* PGPConfirmationPassphraseDialog() options */

PGPOptionListRef	PGPOUIShowPassphraseQuality( PGPContextRef context,
							PGPBoolean showPassphraseQuality );

/* PGPSigningPassphraseDialog() and PGPDecryptionPassphraseDialog() options */

PGPOptionListRef	PGPOUIDefaultKey( PGPContextRef context,
							PGPKeyDBObjRef defaultKey );

PGPOptionListRef	PGPOUIVerifyPassphrase( PGPContextRef context,
							PGPBoolean verifyPassphrase );

PGPOptionListRef	PGPOUIFindMatchingKey( PGPContextRef context,
							PGPBoolean findMatchingKey );

PGPOptionListRef	PGPOUITextUI( PGPContextRef context, PGPBoolean textUI );

/* PGPRecipientDialog() options: */

PGPOptionListRef	PGPOUIRecipientList( PGPContextRef context,
							PGPUInt32 *recipientCount,
							PGPRecipientSpec **recipientList );

PGPOptionListRef	PGPOUIDefaultRecipients( PGPContextRef context,
							PGPUInt32 recipientCount,
							const PGPRecipientSpec recipientList[] );

PGPOptionListRef	PGPOUIDisplayMarginalValidity( PGPContextRef context,
							PGPBoolean displayMarginalValidity );

PGPOptionListRef	PGPOUIIgnoreMarginalValidity( PGPContextRef context,
							PGPBoolean ignoreMarginalValidity );

PGPOptionListRef	PGPOUIEnforceAdditionalRecipientRequests(
						PGPContextRef context,
						PGPAdditionalRecipientRequestEnforcement enforcement,
						PGPBoolean alwaysDisplayDialogWithARRs);

/* PGPDecryptionPassphraseDialog() and PGPRecipientDialog() only: */

PGPOptionListRef	PGPOUIKeyServerUpdateParams(PGPContextRef context,
							PGPUInt32 				serverCount,
							const PGPKeyServerSpec 	serverList[],
							PGPtlsContextRef 		tlsContext,
							PGPBoolean 				searchBeforeDisplay,
							PGPKeyDBRef 			*foundKeys,
							PGPOptionListRef 		firstOption, ...);

/* Key server search dialog options */

PGPOptionListRef	PGPOUIKeyServerSearchFilter(PGPContextRef context,
							PGPFilterRef filter);

PGPOptionListRef	PGPOUIKeyServerSearchKey(PGPContextRef context,
							PGPKeyDBObjRef searchKeyObject);

PGPOptionListRef	PGPOUIKeyServerSearchKeySet(PGPContextRef context,
							PGPKeySetRef keySet);

PGPOptionListRef	PGPOUIKeyServerSearchKeyIDList(PGPContextRef context,
							PGPUInt32 keyIDCount, const PGPKeyID keyIDList[] );

/*
** These options are used to build the options dialog and are only
** applicable for the PGPOptionsDialog() and PGPOUIDialogOptions() calls.
** The "description" parameters are optional. 
*/
#undef				PGPOUICheckbox
PGPOptionListRef	PGPOUICheckbox(PGPContextRef context, PGPUInt32 itemID,
							const PGPChar8 *title, const PGPChar8 *description,
							PGPUInt32 initialValue, PGPUInt32 *resultPtr,
							PGPOptionListRef firstOption, ...);
#undef				PGPOUIPopupList
PGPOptionListRef	PGPOUIPopupList(PGPContextRef context, PGPUInt32 itemID,
							const PGPChar8 *title, const PGPChar8 *description,
							PGPUInt32 listItemCount, const PGPChar8 *listItems[],
							PGPUInt32 initialValue, PGPUInt32 *resultPtr,
							PGPOptionListRef firstOption, ...);
							
#if PGP_DEPRECATED

#include "pgpGroups.h"

PGPOptionListRef	PGPOUIRecipientGroups(PGPContextRef context,
							PGPGroupSetRef groupSet);
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpUserInterface_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
