#pragma once

#define LANG_INLINE_PREFIX	            "[OTR INFO] "

#define LANG_START_OTR						LPGEN("Start OTR")
#define LANG_STOP_OTR						LPGEN("Stop OTR")

#define LANG_YES							LPGENW("Yes")
#define LANG_NO								LPGENW("No")

#define LANG_OTR_TOOLTIP					LPGEN("OTR status (Click for more options)")
#define LANG_STATUS_DISABLED				LPGENW("OTR Encryption: Disabled")
#define LANG_STATUS_UNVERIFIED				LPGENW("OTR Encryption: UNVERIFIED")
#define LANG_STATUS_PRIVATE					LPGENW("OTR Encryption: Private")
#define LANG_STATUS_FINISHED				LPGENW("OTR Encryption: Finished")

#define LANG_SESSION_TERMINATED_OTR			LPGENW("OTR encrypted session with '%s' has been terminated")
#define LANG_SESSION_REQUEST_OTR			LPGENW("Requesting OTR encrypted session with '%s'")
#define LANG_SESSION_START_OTR				LPGENW("Beginning OTR encrypted session with '%s'")
#define LANG_SESSION_START_OTR_VERIFY		LPGENW("Beginning OTR encrypted session with '%s' (NOT VERIFIED)")
#define LANG_SESSION_TRY_CONTINUE_OTR		LPGENW("Trying to refresh OTR encrypted session with '%s'")
#define LANG_SESSION_HAS_CONTINUE_OTR		LPGENW("OTR encrypted session was refreshed by '%s'")
#define LANG_SESSION_HAS_CONTINUE_OTR_VERIFY LPGENW("OTR encrypted session was refreshed by '%s' (NOT VERIFIED)")
#define LANG_SESSION_CONTINUE_OTR			LPGENW("OTR encrypted session with '%s' successfully refreshed")
#define LANG_SESSION_CONTINUE_OTR_VERIFY	LPGENW("OTR encrypted session with '%s' successfully refreshed (NOT VERIFIED)")
#define LANG_SESSION_TERMINATED_BY_OTR		LPGENW("OTR encrypted session has been terminated by '%s'. You should do that, too!")
#define LANG_SESSION_NOT_STARTED_OTR		LPGENW("OTR encrypted session with '%s' could not be started")

#define LANG_ENCRYPTION_ERROR				LPGENW("OTR could not encrypt when sending message")

#define LANG_OTR_INFO						LPGEN("OTR Information")

#define LANG_FINGERPRINT_VERIFIED			LPGENW("OTR encrypted session with '%s' is now using a VERIFIED fingerprint")
#define LANG_FINGERPRINT_NOT_VERIFIED		LPGENW("OTR encrypted session with '%s' is now using a NOT VERIFIED fingerprint")

#define LANG_GENERATE_KEY					LPGENW("Generating new private key for account '%s'.\nPlease Wait...")

#define LANG_CONTACT						LPGENW("Contact")
#define LANG_PROTO							LPGENW("Account")
#define LANG_POLICY							LPGENW("Policy")
#define LANG_VERIFIED						LPGENW("Verified")
#define LANG_ACTIVE							LPGENW("Active")
#define LANG_FINGERPRINT					LPGENW("Fingerprint")
#define LANG_HTMLCONV						LPGENW("Conv. HTML")

#define LANG_POLICY_DEFAULT					LPGENW("Default")
#define LANG_POLICY_ALWAYS					LPGENW("Always")
#define LANG_POLICY_OPP						LPGENW("Opportunistic")
#define LANG_POLICY_MANUAL					LPGENW("Manual")
#define LANG_POLICY_NEVER 					LPGENW("Never")

#define LANG_OTR_ASK_NEWKEY					LPGENW("Generating new key for '%s'. Continue?")
#define LANG_OTR_ASK_REMOVEKEY				LPGENW("Removing key for '%s'. Continue?")

#define LANG_OTR_FPVERIFY_TITLE				LPGENW("OTR Fingerprint Verification")
#define LANG_OTR_FPVERIFY_DESC				LPGENW("OTR encrypted session with '%s'.\nThe OTR fingerprint used by your contact is NOT VERIFIED.\nDo you trust it?")
#define LANG_OTR_FPVERIFIED_DESC			LPGENW("OTR encrypted session with '%s'.\nThe OTR fingerprint used by your contact is already verified.\nDo you still trust it?")
#define LANG_YOUR_PRIVKEY					LPGENW("Your Fingerprint to tell your contact (use a trusted channel!)")
#define LANG_CONTACT_FINGERPRINT			LPGENW("VERIFY: Fingerprint from contact")

#define LANG_SMP_VERIFY_TITLE				LPGENW("OTR Authenticate: %s (%s)")
#define LANG_SMPTYPE_QUESTION				LPGENW("Challenge Question")
#define	LANG_SMPTYPE_PASSWORD				LPGENW("Known Password")
#define LANG_SMPTYPE_FINGERPRINT			LPGENW("Manual fingerprint comparison")
#define LANG_SMP_ANSWER						LPGENW("Secret Answer")
#define LANG_SMP_QUESTION					LPGENW("Question")
#define LANG_SMP_PASSWORD					LPGENW("Password")

#define LANG_SMP_SUCCESS					LPGENW("Authentication successful.")
#define LANG_SMP_SUCCESS_VERIFY				LPGENW("Your contact authenticated you successfully. You can send your own request to authenticate him.")
#define LANG_SMP_ERROR						LPGENW("Error during authentication.")
#define LANG_SMP_FAILED						LPGENW("Authentication failed")
#define LANG_SMP_IN_PROGRESS				LPGENW("Authentication for '%s' is already in progress.")

#define LANG_SMP_PROGRESS_TITLE				LPGENW("OTR Authenticating: %s (%s)")
#define LANG_SMP_PROGRESS_DESC				LPGENW("Authenticating contact:\n%s (%s)")

#define LANG_OTR_SMPQUESTION_VERIFY_DESC	LPGENW("OTR encrypted session with '%s' (UNVERIFIED).\nUse a question only your partner can answer.")
#define LANG_OTR_SMPQUESTION_VERIFIED_DESC	LPGENW("OTR encrypted session with '%s' (VERIFIED).\nVerify the session again using a question only your partner can answer.")

#define LANG_OTR_SMPPASSWORD_VERIFY_DESC	LPGENW("OTR encrypted session with '%s' (UNVERIFIED).\nUse a known password.")
#define LANG_OTR_SMPPASSWORD_VERIFIED_DESC	LPGENW("OTR encrypted session with '%s' (VERIFIED).\nVerify the session again using a known password.")

#define LANG_OTR_SMPQUESTION_RESPOND_DESC	LPGENW("Your contact '%s' wants to verify your identity with a question only you can answer.")

#define LANG_OTR_SMPPASSWORD_RESPOND_DESC	LPGENW("Your contact '%s' wants to verify your identity with a secret password you should know.")

#define LANG_FINGERPRINT_STILL_IN_USE		LPGENW("Fingerprint '%s' still in use in conversation with '%s' (%s). You cannot delete it!")
#define LANG_FINGERPRINT_NOT_DELETED		LPGENW("Fingerprint '%s' in use in conversation with '%s' (%s). It could not be deleted!")

#define LANG_OTR_SECUREIM_STARTED			LPGENW("Cannot start OTR for '%s'. SecureIM is already running")
#define LANG_OTR_SECUREIM_TITLE				LPGENW("OTR: SecureIM installed")
#define LANG_OTR_SECUREIM_TEXT				LPGENW("You are using SecureIM. MirOTR will only work for contacts with SecureIM disabled")
