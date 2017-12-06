/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file contains the prototypes for functions which use UI to interact
	with the user.

	$Id: pgpUserInterface.h,v 1.40 1999/03/10 02:59:37 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUserInterface_h	/* [ */
#define Included_pgpUserInterface_h

#include "pgpPubTypes.h"
#include "pgpGroups.h"

#if PGP_WIN32
#include "windows.h"
#endif

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#if PRAGMA_ALIGN_SUPPORTED
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

typedef struct PGPRecipientSpec
{
	PGPRecipientSpecType	type;
	PGPBoolean				locked;
	PGPBoolean				reserved8[3];	/* Must be zero */
	PGPUInt32				reserved32[3];	/* Must be zero */
	
	union
	{
		PGPKeyRef	key;
		char		userIDStr[256];		/* Null terminated string */
		
		struct
		{
			PGPKeyID				keyID;
			PGPPublicKeyAlgorithm	algorithm;
		} id;
		
	} u;

} PGPRecipientSpec;

typedef struct PGPKeyServerSpec
{
	PGPKeyServerRef		server;
	const char			*serverName;	/* Optional */
	PGPUInt32			reserved[5];	/* Must be zero */

} PGPKeyServerSpec;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

PGPError	PGPRecipientDialog(
					PGPContextRef 		context,
					PGPKeySetRef 		allKeys,
					PGPBoolean			alwaysDisplayDialog,
					PGPKeySetRef 		*recipientKeys,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPPassphraseDialog(
					PGPContextRef 		context,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPConfirmationPassphraseDialog(
					PGPContextRef 		context,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPKeyPassphraseDialog(
					PGPContextRef 		context,
					PGPKeyRef			theKey,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPSigningPassphraseDialog(
					PGPContextRef 		context,
					PGPKeySetRef		allKeys,
					PGPKeyRef			*signingKey,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPDecryptionPassphraseDialog(
					PGPContextRef		context,							
					PGPKeySetRef		recipientKeys,
					PGPUInt32			keyIDCount,
					const PGPKeyID		keyIDList[],		
					PGPKeyRef 			*decryptionKey,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPConventionalEncryptionPassphraseDialog(
					PGPContextRef 		context,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPConventionalDecryptionPassphraseDialog(
					PGPContextRef 		context,							
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPOptionsDialog(
					PGPContextRef 		context,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPCollectRandomDataDialog(
					PGPContextRef 		context,
					PGPUInt32			neededEntropyBits,
					PGPOptionListRef 	firstOption, ... );

PGPError	PGPSearchKeyServerDialog(
					PGPContextRef 			context,
					PGPUInt32				serverCount,
					const PGPKeyServerSpec 	serverList[],
					PGPtlsContextRef		tlsContext,
					PGPBoolean				searchAllServers,
					PGPKeySetRef 			*foundKeys,
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

PGPUInt32	PGPEstimatePassphraseQuality(const char *passphrase);

/* General dialog options */

PGPOptionListRef	PGPOUIDialogPrompt(PGPContextRef context,
							const char *prompt);

PGPOptionListRef	PGPOUIWindowTitle(PGPContextRef context,
							const char *title);

PGPOptionListRef	PGPOUIDialogOptions(PGPContextRef context,
							PGPOptionListRef firstOption, ...);

#if PGP_WIN32
PGPOptionListRef	PGPOUIParentWindowHandle(PGPContextRef context,
							HWND hwndParent);
#endif

/* All passphrase dialogs */

/* Caller should free passphrase with PGPFreeData() */
PGPOptionListRef	PGPOUIOutputPassphrase(PGPContextRef context,
							char **passphrase);

PGPOptionListRef	PGPOUIMinimumPassphraseQuality(PGPContextRef context,
							PGPUInt32 minimumPassphraseQuality);

PGPOptionListRef	PGPOUIMinimumPassphraseLength(PGPContextRef context,
							PGPUInt32 minimumPassphraseLength);

/* PGPConfirmationPassphraseDialog() options */

PGPOptionListRef	PGPOUIShowPassphraseQuality(PGPContextRef context,
							PGPBoolean showPassphraseQuality);

/* PGPSigningPassphraseDialog() and PGPDecryptionPassphraseDialog() options */

PGPOptionListRef	PGPOUIDefaultKey(PGPContextRef context,
							PGPKeyRef defaultKey);

PGPOptionListRef	PGPOUIVerifyPassphrase(PGPContextRef context,
							PGPBoolean verifyPassphrase);

PGPOptionListRef	PGPOUIFindMatchingKey(PGPContextRef context,
							PGPBoolean findMatchingKey);

PGPOptionListRef	PGPOUITextUI(PGPContextRef context, PGPBoolean textUI);

/* PGPRecipientDialog() options: */

PGPOptionListRef	PGPOUIRecipientList(PGPContextRef context,
							PGPUInt32 *recipientCount,
							PGPRecipientSpec **recipientList);

PGPOptionListRef	PGPOUIDefaultRecipients(PGPContextRef context,
							PGPUInt32 recipientCount,
							const PGPRecipientSpec recipientList[]);

PGPOptionListRef	PGPOUIDisplayMarginalValidity(PGPContextRef context,
							PGPBoolean displayMarginalValidity);

PGPOptionListRef	PGPOUIIgnoreMarginalValidity(PGPContextRef context,
							PGPBoolean ignoreMarginalValidity);

PGPOptionListRef	PGPOUIRecipientGroups(PGPContextRef context,
							PGPGroupSetRef groupSet);

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
							PGPKeySetRef 			*foundKeys,
							PGPOptionListRef 		firstOption, ...);

/* Key server search dialog options */

PGPOptionListRef	PGPOUIKeyServerSearchFilter(PGPContextRef context,
							PGPFilterRef filter);

PGPOptionListRef	PGPOUIKeyServerSearchKey(PGPContextRef context,
							PGPKeyRef key);

PGPOptionListRef	PGPOUIKeyServerSearchKeySet(PGPContextRef context,
							PGPKeySetRef keySet);

PGPOptionListRef	PGPOUIKeyServerSearchKeyIDList(PGPContextRef context,
							PGPUInt32 keyIDCount, const PGPKeyID keyIDList[] );

/*
** These options are used to build the options dialog and are only
** applicable for the PGPOptionsDialog() and PGPOUIDialogOptions() calls.
** The "description" parameters are optional. 
*/

PGPOptionListRef	PGPOUICheckbox(PGPContextRef context, PGPUInt32 itemID,
							const char *title, const char *description,
							PGPUInt32 initialValue, PGPUInt32 *resultPtr,
							PGPOptionListRef firstOption, ...);

PGPOptionListRef	PGPOUIPopupList(PGPContextRef context, PGPUInt32 itemID,
							const char *title, const char *description,
							PGPUInt32 listItemCount, const char *listItems[],
							PGPUInt32 initialValue, PGPUInt32 *resultPtr,
							PGPOptionListRef firstOption, ...);
							
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
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
