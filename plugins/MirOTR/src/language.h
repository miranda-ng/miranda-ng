#pragma once

#define LANG_START_OTR						LPGEN("Start OTR")
#define LANG_STOP_OTR						LPGEN("Stop OTR")

#define LANG_YES							LPGEN("Yes")
#define LANG_NO								LPGEN("No")

#define LANG_OTR_TOOLTIP					LPGEN("OTR status (Click for more options)")
#define LANG_STATUS_DISABLED				LPGEN("OTR Encryption: Disabled")
#define LANG_STATUS_UNVERIFIED				LPGEN("OTR Encryption: UNVERIFIED")
#define LANG_STATUS_PRIVATE					LPGEN("OTR Encryption: Private")
#define LANG_STATUS_FINISHED				LPGEN("OTR Encryption: Finished")

#define LANG_MENU_START						LPGENT("&Start OTR session")
#define LANG_MENU_REFRESH					LPGENT("&Refresh OTR session")
#define LANG_MENU_STOP						LPGENT("Sto&p OTR session")
#define LANG_MENU_VERIFY					LPGENT("&Verify Fingerprint")
#define LANG_MENU_TOGGLEHTML				LPGENT("&Convert HTML (for Pidgin)")

#define LANG_SESSION_TERMINATED_OTR			LPGEN("OTR encrypted session with '%s' has been terminated")
#define LANG_SESSION_REQUEST_OTR			LPGEN("Requesting OTR encrypted session with '%s'")
#define LANG_SESSION_START_OTR				LPGEN("Beginning OTR encrypted session with '%s'")
#define LANG_SESSION_START_OTR_VERIFY		LPGEN("Beginning OTR encrypted session with '%s' (NOT VERIFIED)")
#define LANG_SESSION_TRY_CONTINUE_OTR		LPGEN("Trying to refresh OTR encrypted session with '%s'")
#define LANG_SESSION_HAS_CONTINUE_OTR		LPGEN("OTR encrypted session was refreshed by '%s'")
#define LANG_SESSION_HAS_CONTINUE_OTR_VERIFY LPGEN("OTR encrypted session was refreshed by '%s' (NOT VERIFIED)")
#define LANG_SESSION_CONTINUE_OTR			LPGEN("OTR encrypted session with '%s' successfully refreshed")
#define LANG_SESSION_CONTINUE_OTR_VERIFY	LPGEN("OTR encrypted session with '%s' successfully refreshed (NOT VERIFIED)")
#define LANG_SESSION_TERMINATED_BY_OTR		LPGEN("OTR encrypted session has been terminated by '%s'. You should do that, too!")
#define LANG_SESSION_NOT_STARTED_OTR		LPGEN("OTR encrypted session with '%s' could not be started")

#define LANG_ENCRYPTION_ERROR				LPGEN("OTR could not encrypt when sending message")

#define LANG_OTR_INFO						LPGEN("OTR Information")
#define LANG_INLINE_PREFIX					LPGEN("[OTR INFO] ")

#define LANG_FINGERPRINT_VERIFIED			LPGEN("OTR encrypted session with '%s' is now using a VERIFIED fingerprint")
#define LANG_FINGERPRINT_NOT_VERIFIED		LPGEN("OTR encrypted session with '%s' is now using a NOT VERIFIED fingerprint")

#define LANG_ICON_UNVERIFIED				LPGEN("OTR unverified")
#define LANG_ICON_FINISHED					LPGEN("OTR finished")
#define LANG_ICON_PRIVATE					LPGEN("OTR private")
#define LANG_ICON_NOT_PRIVATE				LPGEN("OTR not secure")
#define LANG_ICON_OTR						LPGEN("OTR")
#define LANG_ICON_REFRESH					LPGEN("Refresh")

#define LANG_GENERATE_KEY					LPGEN("Generating new private key for protocol '%s'.\nPlease Wait...")

#define LANG_OPT_GENERAL					LPGENT("General")
#define LANG_OPT_PROTO						LPGENT("Protocols")
#define LANG_OPT_CONTACTS					LPGENT("Contacts")
#define LANG_OPT_FINGER						LPGENT("Fingerprints")

#define LANG_CONTACT						LPGEN("Contact")
#define LANG_PROTO							LPGEN("Account")
#define LANG_POLICY							LPGEN("Policy")
#define LANG_VERIFIED						LPGEN("Verified")
#define LANG_ACTIVE							LPGEN("Active")
#define LANG_FINGERPRINT					LPGEN("Fingerprint")
#define LANG_HTMLCONV						LPGEN("Conv. HTML")

#define LANG_POLICY_DEFAULT					LPGEN("Default")
#define LANG_POLICY_ALWAYS					LPGEN("Always")
#define LANG_POLICY_OPP						LPGEN("Opportunistic")
#define LANG_POLICY_MANUAL					LPGEN("Manual")
#define LANG_POLICY_NEVER 					LPGEN("Never")

#define LANG_OTR_ASK_NEWKEY					LPGEN("Generating new key for '%s'. Continue?")
#define LANG_OTR_ASK_REMOVEKEY				LPGEN("Removing key for '%s'. Continue?")

#define LANG_OTR_FPVERIFY_TITLE				LPGEN("OTR Fingerprint Verification")
#define LANG_OTR_FPVERIFY_DESC				LPGEN("OTR encrypted session with '%s'.\nThe OTR fingerprint used by your contact is NOT VERIFIED.\nDo you trust it?")
#define LANG_OTR_FPVERIFIED_DESC			LPGEN("OTR encrypted session with '%s'.\nThe OTR fingerprint used by your contact is already verified.\nDo you still trust it?")
#define LANG_YOUR_PRIVKEY					LPGEN("Your Fingerprint to tell your contact (use a trusted channel!)")
#define LANG_CONTACT_FINGERPRINT			LPGEN("VERIFY: Fingerprint from contact")

#define LANG_SMP_VERIFY_TITLE				LPGEN("OTR Authenticate: %s (%s)")
#define LANG_SMPTYPE_QUESTION				LPGEN("Challenge Question")
#define	LANG_SMPTYPE_PASSWORD				LPGEN("Known Password")
#define LANG_SMPTYPE_FINGERPRINT			LPGEN("Manual fingerprint comparison")
#define LANG_SMP_ANSWER						LPGEN("Secret Answer")
#define LANG_SMP_QUESTION					LPGEN("Question")
#define LANG_SMP_PASSWORD					LPGEN("Password")

#define LANG_SMP_SUCCESS					LPGEN("Authentication successful.")
#define LANG_SMP_SUCCESS_VERIFY				LPGEN("Your contact authenticated you successfully. You can send your own request to authenticate him.")
#define LANG_SMP_ERROR						LPGEN("Error during authentication.")
#define LANG_SMP_FAILED						LPGEN("Authentication failed")
#define LANG_SMP_IN_PROGRESS				LPGEN("Authentication for '%s' is already in progress.")

#define LANG_SMP_PROGRESS_TITLE				LPGEN("OTR Authenticating: %s (%s)")
#define LANG_SMP_PROGRESS_DESC				LPGEN("Authenticating contact:\n%s (%s)")

#define LANG_OTR_SMPQUESTION_VERIFY_DESC	LPGEN("OTR encrypted session with '%s' (UNVERIFIED).\nUse a question only your partner can answer.")
#define LANG_OTR_SMPQUESTION_VERIFIED_DESC	LPGEN("OTR encrypted session with '%s' (VERIFIED).\nVerify the session again using a question only your partner can answer.")

#define LANG_OTR_SMPPASSWORD_VERIFY_DESC	LPGEN("OTR encrypted session with '%s' (UNVERIFIED).\nUse a known password.")
#define LANG_OTR_SMPPASSWORD_VERIFIED_DESC	LPGEN("OTR encrypted session with '%s' (VERIFIED).\nVerify the session again using a known password.")

#define LANG_OTR_SMPQUESTION_RESPOND_DESC	LPGEN("Your contact '%s' wants to verify your identity with a question only you can answer.")

#define LANG_OTR_SMPPASSWORD_RESPOND_DESC	LPGEN("Your contact '%s' wants to verify your identity with a secret password you should know.")

#define LANG_FINGERPRINT_STILL_IN_USE		LPGEN("Fingerprint '%s' still in use in conversation with '%s' (%s). You cannot delete it!")
#define LANG_FINGERPRINT_NOT_DELETED		LPGEN("Fingerprint '%s' in use in conversation with '%s' (%s). It could not be deleted!")

#define LANG_OTR_SECUREIM_STARTED			LPGEN("Cannot start OTR for '%s'. SecureIM is already running")
#define LANG_OTR_SECUREIM_TITLE				LPGEN("OTR: SecureIM installed")
#define LANG_OTR_SECUREIM_TEXT				LPGEN("You are using SecureIM. MirOTR will only work for contacts with SecureIM disabled")
