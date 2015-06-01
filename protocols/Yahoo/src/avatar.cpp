/*
 * $Id: avatar.cpp 14178 2012-03-11 15:02:54Z borkra $
 *
 * myYahoo Miranda Plugin
 *
 * Authors: Gennady Feldman (aka Gena01)
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#include "yahoo.h"

#include <sys/stat.h>

#include <m_langpack.h>
#include "m_folders.h"
#include "avatar.h"
#include "resource.h"

/*
 *31 bit hash function  - this is based on g_string_hash function from glib
 */

int YAHOO_avt_hash(const char *key, DWORD len)
{
	/*
		Thank you Pidgin and Kopete devs. It seems that both clients are using this code now.

	*/

	const unsigned char *p = (const unsigned char *)key;
	int checksum = 0, g, i = len;

	while(i--) {
		checksum = (checksum << 4) + *p++;

		if ((g = (checksum & 0xf0000000)) != 0)
			checksum ^= g >> 23;

		checksum &= ~g;
	}

	return checksum;
}

/**************** Send Avatar ********************/

void upload_avt(int id, INT_PTR fd, int error, void *data)
{
	struct yahoo_file_info *sf = (struct yahoo_file_info*) data;
	unsigned long size = 0;
	char buf[1024];
	int rw;			/* */
	DWORD dw;		/* needed for ReadFile */
	HANDLE myhFile;

	if (fd < 1 || error) {
		LOG(("[get_fd] Connect Failed!"));
		return;
	}

	myhFile  = CreateFileA(sf->filename,
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		0);

	if (myhFile == INVALID_HANDLE_VALUE) {
		LOG(("[get_fd] Can't open file for reading?!"));
		return;
	}

	LOG(("Sending file: %s size: %ld", sf->filename, sf->filesize));

	do {
		rw = ReadFile(myhFile, buf, sizeof(buf), &dw, NULL);

		if (rw != 0) {
			rw = Netlib_Send((HANDLE)fd, buf, dw, MSG_NODUMP);

			if (rw < 1) {
				LOG(("Upload Failed. Send error?"));
				//ShowError(Translate("Yahoo Error"), Translate("Avatar upload failed!?!"));
				break;
			}

			size += rw;
		}
	} while (rw >= 0 && size < sf->filesize);

	CloseHandle(myhFile);

	do {
		rw = Netlib_Recv((HANDLE)fd, buf, sizeof(buf), 0);
		LOG(("Got: %d bytes", rw));
	} while (rw > 0);

	LOG(("File send complete!"));
}

void __cdecl CYahooProto::send_avt_thread(void *psf)
{
	struct yahoo_file_info *sf = ( yahoo_file_info* )psf;
	if (sf == NULL) {
		debugLogA("[yahoo_send_avt_thread] SF IS NULL!!!");
		return;
	}

	setByte("AvatarUL", 1);
	yahoo_send_avatar(m_id, sf->filename, sf->filesize, &upload_avt, sf);

	free(sf->filename);
	free(sf);

	if (getByte("AvatarUL", 1) == 1)
		setByte("AvatarUL", 0);
}

void CYahooProto::SendAvatar(const TCHAR *szFile)
{
	struct _stat statbuf;
	if (_tstat( szFile, &statbuf ) != 0) {
		LOG(("[YAHOO_SendAvatar] Error reading File information?!"));
		return;
	}

	yahoo_file_info *sf = y_new(struct yahoo_file_info, 1);
	sf->filesize = statbuf.st_size;

	wchar_t tszFilename[MAX_PATH];
	wcsncpy(tszFilename, szFile, SIZEOF(tszFilename)-1);
	GetShortPathNameW(szFile, tszFilename, SIZEOF(tszFilename));
	char szFilename[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, tszFilename, -1, szFilename, MAX_PATH, 0, 0);
	sf->filename = strdup(szFilename);

	debugLogA("[Uploading avatar] filename: %s size: %ld", sf->filename, sf->filesize);

	ForkThread(&CYahooProto::send_avt_thread, sf);
}

struct avatar_info{
	char *who;
	char *pic_url;
	int cksum;
};

void __cdecl CYahooProto::recv_avatarthread(void *pavt)
{
	struct avatar_info *avt = ( avatar_info* )pavt;
	int 	error = 0;
	TCHAR  buf[4096];

	if (avt == NULL) {
		debugLogA("AVT IS NULL!!!");
		return;
	}

	if (!m_bLoggedIn) {
		debugLogA("We are not logged in!!!");
		return;
	}

	//    ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);

	LOG(("yahoo_recv_avatarthread who:%s url:%s checksum: %d", avt->who, avt->pic_url, avt->cksum));

	MCONTACT hContact = getbuddyH(avt->who);

	if (!hContact) {
		LOG(("ERROR: Can't find buddy: %s", avt->who));
		error = 1;
	} else if (!error) {
		setDword(hContact, "PictCK", avt->cksum);
		setDword(hContact, "PictLoading", 1);
	}

	if (!error) {

		NETLIBHTTPREQUEST nlhr={0},*nlhrReply;

		nlhr.cbSize		= sizeof(nlhr);
		nlhr.requestType= REQUEST_GET;
		nlhr.flags		= NLHRF_NODUMP|NLHRF_GENERATEHOST|NLHRF_SMARTAUTHHEADER;
		nlhr.szUrl		= avt->pic_url;

		nlhrReply=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,(WPARAM)m_hNetlibUser,(LPARAM)&nlhr);

		if (nlhrReply) {

			if (nlhrReply->resultCode != 200) {
				LOG(("Update server returned '%d' instead of 200. It also sent the following: %s", nlhrReply->resultCode, nlhrReply->szResultDescr));
				// make sure it's a real problem and not a problem w/ our connection
				yahoo_send_picture_info(m_id, avt->who, 3, avt->pic_url, avt->cksum);
				error = 1;
			} else if (nlhrReply->dataLength < 1 || nlhrReply->pData == NULL) {
				LOG(("No data??? Got %d bytes.", nlhrReply->dataLength));
				error = 1;
			} else {
				GetAvatarFileName(hContact, buf, 1024, getByte(hContact, "AvatarType", 0));
				DeleteFile(buf);

				LOG(("Saving file: %s size: %u", buf, nlhrReply->dataLength));
				HANDLE myhFile = CreateFile(buf,
					GENERIC_WRITE,
					FILE_SHARE_WRITE,
					NULL, OPEN_ALWAYS,  FILE_ATTRIBUTE_NORMAL,  0);

				if (myhFile !=INVALID_HANDLE_VALUE) {
					DWORD c;

					WriteFile(myhFile, nlhrReply->pData, nlhrReply->dataLength, &c, NULL);
					CloseHandle(myhFile);

					setDword(hContact, "PictLastCheck", 0);
				} else {
					LOG(("Can not open file for writing: %s", buf));
					error = 1;
				}
			}
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)nlhrReply);
		}
	}

	if (getDword(hContact, "PictCK", 0) != avt->cksum) {
		LOG(("WARNING: Checksum updated during download?!"));
		error = 1; /* don't use this one? */
	}

	setDword(hContact, "PictLoading", 0);
	LOG(("File download complete!?"));

	if (error)
		buf[0]='\0';

	free(avt->who);
	free(avt->pic_url);
	free(avt);
	
	PROTO_AVATAR_INFORMATION AI;
	AI.format = PA_FORMAT_PNG;
	AI.hContact = hContact;
	_tcsncpy(AI.filename, buf, SIZEOF(AI.filename)-1);

	if (error)
		setDword(hContact, "PictCK", 0);

	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, !error ? ACKRESULT_SUCCESS:ACKRESULT_FAILED,(HANDLE) &AI, 0);
}

void CYahooProto::ext_got_picture(const char *me, const char *who, const char *pic_url, int cksum, int type)
{
	MCONTACT hContact = 0;

	LOG(("[ext_yahoo_got_picture] for %s with url %s (checksum: %d) type: %d", who, pic_url, cksum, type));

	/*
	  Type:

		1 - Send Avatar Info
		2 - Got Avatar Info
		3 - YIM6 didn't like my avatar? Expired? We need to invalidate and re-load
	 */
	switch (type) {
	case 1:
		{
			int cksum=0;
			DBVARIANT dbv;

			/* need to send avatar info */
			if (!getByte("ShowAvatars", 1 )) {
				LOG(("[ext_yahoo_got_picture] We are not using/showing avatars!"));
				yahoo_send_picture_update(m_id, who, 0); // no avatar (disabled)
				return;
			}

			LOG(("[ext_yahoo_got_picture] Getting ready to send info!"));
			/* need to read CheckSum */
			cksum = getDword("AvatarHash", 0);
			if (cksum) {
				if (!getString("AvatarURL", &dbv)) {
					LOG(("[ext_yahoo_got_picture] Sending url: %s checksum: %d to '%s'!", dbv.pszVal, cksum, who));
					//void yahoo_send_picture_info(int id, const char *me, const char *who, const char *pic_url, int cksum)
					yahoo_send_picture_info(m_id, who, 2, dbv.pszVal, cksum);
					db_free(&dbv);
					break;
				} else
					LOG(("No AvatarURL???"));

				/*
				 * Try to re-upload the avatar
				 */
				if (getByte("AvatarUL", 0) != 1) {
					// NO avatar URL??
					if (!getTString("AvatarFile", &dbv)) {
						struct _stat statbuf;

						if (_tstat( dbv.ptszVal, &statbuf ) != 0) {
							LOG(("[ext_yahoo_got_picture] Avatar File Missing? Can't find file: %s", dbv.ptszVal));
						} else {
							setString("AvatarInv", who);
							SendAvatar(dbv.ptszVal);
						}

						db_free(&dbv);
					} else {
						LOG(("[ext_yahoo_got_picture] No Local Avatar File??? "));
					}
				} else
						LOG(("[ext_yahoo_got_picture] Another avatar upload in progress?"));
			}
		}
		break;
	case 2: /*
		     * We got Avatar Info for our buddy.
		     */
			if (!getByte("ShowAvatars", 1 )) {
				LOG(("[ext_yahoo_got_picture] We are not using/showing avatars!"));
				return;
			}

			/* got avatar info, so set miranda up */
			hContact = getbuddyH(who);

			if (!hContact) {
				LOG(("[ext_yahoo_got_picture] Buddy not on my buddy list?."));
				return;
			}

			if (!cksum && pic_url) {
				const char *chk = strstr(pic_url, "chksum=");
				if (chk)
					cksum = strtol(chk + 7, NULL, 10);
			}

			if (!cksum || cksum == -1) {
				LOG(("[ext_yahoo_got_picture] Resetting avatar."));
				setDword(hContact, "PictCK", 0);
				reset_avatar(hContact);
			}
			else {
				if (pic_url == NULL) {
					LOG(("[ext_yahoo_got_picture] WARNING: Empty URL for avatar?"));
					return;
				}

				TCHAR z[1024];
				GetAvatarFileName(hContact, z, 1024, getByte(hContact, "AvatarType", 0));

				if (getDword(hContact, "PictCK", 0) != cksum || _taccess( z, 0) != 0) {

					debugLogA("[ext_yahoo_got_picture] Checksums don't match or avatar file is missing. Current: %d, New: %d", 
						getDword(hContact, "PictCK", 0), cksum);

					struct avatar_info *avt = ( avatar_info* )malloc(sizeof(struct avatar_info));
					avt->who = strdup(who);
					avt->pic_url = strdup(pic_url);
					avt->cksum = cksum;

					ForkThread(&CYahooProto::recv_avatarthread, avt);
				}
			}

		break;
	case 3:
		/*
		 * Our Avatar is not good anymore? Need to re-upload??
		 */
		 /* who, pic_url, cksum */
		{
			int mcksum=0;
			DBVARIANT dbv;

			/* need to send avatar info */
			if (!getByte("ShowAvatars", 1 )) {
				LOG(("[ext_yahoo_got_picture] We are not using/showing avatars!"));
				yahoo_send_picture_update(m_id, who, 0); // no avatar (disabled)
				return;
			}

			LOG(("[ext_yahoo_got_picture] Getting ready to send info!"));
			/* need to read CheckSum */
			mcksum = getDword("AvatarHash", 0);
			if (mcksum == 0) {
				/* this should NEVER Happen??? */
				LOG(("[ext_yahoo_got_picture] No personal checksum? and Invalidate?!"));
				yahoo_send_picture_update(m_id, who, 0); // no avatar (disabled)
				return;
			}

			LOG(("[ext_yahoo_got_picture] My Checksum: %d", mcksum));

			if (!getString("AvatarURL", &dbv)) {
					if (mir_strcmpi(pic_url, dbv.pszVal) == 0) {
						DBVARIANT dbv2;
						/*time_t  ts;
						DWORD	ae;*/

						if (mcksum != cksum)
							LOG(("[ext_yahoo_got_picture] WARNING: Checksums don't match!"));

						/*time(&ts);
						ae = getDword("AvatarExpires", 0);

						if (ae != 0 && ae > (ts - 300)) {
							LOG(("[ext_yahoo_got_picture] Current Time: %lu Expires: %lu ", ts, ae));
							LOG(("[ext_yahoo_got_picture] We just reuploaded! Stop screwing with Yahoo FT. "));

							// don't leak stuff
							db_free(&dbv);

							break;
						}*/

						LOG(("[ext_yahoo_got_picture] Buddy: %s told us this is bad??Expired??. Re-uploading", who));
						delSetting("AvatarURL");

						if (!getTString("AvatarFile", &dbv2)) {
							setString("AvatarInv", who);
							SendAvatar(dbv2.ptszVal);
							db_free(&dbv2);
						} else {
							LOG(("[ext_yahoo_got_picture] No Local Avatar File??? "));
						}
					} else {
						LOG(("[ext_yahoo_got_picture] URL doesn't match? Tell them the right thing!!!"));
						yahoo_send_picture_info(m_id, who, 2, dbv.pszVal, mcksum);
					}
					// don't leak stuff
					db_free(&dbv);
			} else {
				LOG(("[ext_yahoo_got_picture] no AvatarURL?"));
			}
		}
		break;
	default:
		LOG(("[ext_yahoo_got_picture] Unknown request/packet type exiting!"));
	}

	LOG(("ext_yahoo_got_picture exiting"));
}

void CYahooProto::ext_got_picture_checksum(const char *me, const char *who, int cksum)
{
	LOG(("ext_yahoo_got_picture_checksum for %s checksum: %d", who, cksum));

	MCONTACT hContact = getbuddyH(who);
	if (hContact == NULL) {
		LOG(("Buddy Not Found. Skipping avatar update"));
		return;
	}

	/* Last thing check the checksum and request new one if we need to */
	if (!cksum || cksum == -1) {
		setDword(hContact, "PictCK", 0);
		reset_avatar(hContact);
	}
	else {
		if (getDword(hContact, "PictCK", 0) != cksum) {
			// Now save the new checksum. No rush requesting new avatar yet.
			setDword(hContact, "PictCK", cksum);

			// Need to delete the Avatar File!!
			TCHAR szFile[MAX_PATH];
			GetAvatarFileName(hContact, szFile, SIZEOF(szFile)-1, 0);
			DeleteFile(szFile);

			// Reset the avatar and cleanup.
			reset_avatar(hContact);

			// Request new avatar here... (might also want to check the sharing status?)

			if (getByte("ShareAvatar", 0) == 2)
				request_avatar(who);
		}
	}
}

void CYahooProto::ext_got_picture_update(const char *me, const char *who, int buddy_icon)
{
	LOG(("ext_got_picture_update for %s buddy_icon: %d", who, buddy_icon));

	MCONTACT hContact = getbuddyH(who);
	if (hContact == NULL) {
		LOG(("Buddy Not Found. Skipping avatar update"));
		return;
	}

	setByte(hContact, "AvatarType", buddy_icon);

	/* Last thing check the checksum and request new one if we need to */
	reset_avatar(hContact);
}

void CYahooProto::ext_got_picture_status(const char *me, const char *who, int buddy_icon)
{
	MCONTACT hContact = 0;

	LOG(("ext_yahoo_got_picture_status for %s buddy_icon: %d", who, buddy_icon));

	hContact = getbuddyH(who);
	if (hContact == NULL) {
		LOG(("Buddy Not Found. Skipping avatar update"));
		return;
	}

	setByte(hContact, "AvatarType", buddy_icon);

	/* Last thing check the checksum and request new one if we need to */
	reset_avatar(hContact);
}

void CYahooProto::ext_got_picture_upload(const char *me, const char *url,unsigned int ts)
{
	int cksum = 0;
	DBVARIANT dbv;

	LOG(("[ext_yahoo_got_picture_upload] url: %s timestamp: %d", url, ts));

	if (!url) {
		LOG(("[ext_yahoo_got_picture_upload] Problem with upload?"));
		return;
	}


	cksum = getDword("TMPAvatarHash", 0);
	if (cksum != 0) {
		LOG(("[ext_yahoo_got_picture_upload] Updating Checksum to: %d", cksum));
		setDword("AvatarHash", cksum);
		delSetting("TMPAvatarHash");

		// This is only meant for message sessions, but we don't got those in miranda yet
		//YAHOO_bcast_picture_checksum(cksum);
		yahoo_send_picture_checksum(m_id, NULL, cksum);

		// need to tell the stupid Yahoo that our icon updated
		//YAHOO_bcast_picture_update(2);
	}else
		cksum = getDword("AvatarHash", 0);

	setString("AvatarURL", url);
	//YAHOO_SetDword("AvatarExpires", ts);

	if  (!getString("AvatarInv", &dbv)) {
		LOG(("[ext_yahoo_got_picture_upload] Buddy: %s told us this is bad??", dbv.pszVal));

		LOG(("[ext_yahoo_got_picture] Sending url: %s checksum: %d to '%s'!", url, cksum, dbv.pszVal));
		//void yahoo_send_picture_info(int id, const char *me, const char *who, const char *pic_url, int cksum)
		yahoo_send_picture_info(m_id, dbv.pszVal, 2, url, cksum);

		delSetting("AvatarInv");
		db_free(&dbv);
	}
}

void CYahooProto::ext_got_avatar_share(int buddy_icon)
{
	LOG(("[ext_yahoo_got_avatar_share] buddy icon: %d", buddy_icon));

	setByte("ShareAvatar", buddy_icon );
}

void CYahooProto::reset_avatar(MCONTACT hContact)
{
	LOG(("[YAHOO_RESET_AVATAR]"));

	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
}

void CYahooProto::request_avatar(const char* who)
{
	if (!getByte("ShowAvatars", 1 )) {
		LOG(("Avatars disabled, but available for: %s", who));
		return;
	}

	MCONTACT hContact = getbuddyH(who);

	if (!hContact)
		return;

	time_t cur_time;
	time(&cur_time);
	time_t last_chk = getDword(hContact, "PictLastCheck", 0);

	/*
	* time() - in seconds ( 60*60 = 1 hour)
	*/
	if (getDword(hContact, "PictCK", 0) == 0 || last_chk == 0 || (cur_time - last_chk) > 60) {
		setDword(hContact, "PictLastCheck", (DWORD)cur_time);
		LOG(("Requesting Avatar for: %s", who));
		yahoo_request_buddy_avatar(m_id, who);
	}
	else LOG(("Avatar Not Available for: %s Last Check: %ld Current: %ld (Flood Check in Effect)", who, last_chk, cur_time));
}

void CYahooProto::GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, int cbLen, int type)
{
	int tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), VARST( _T("%miranda_avatarcache%")), m_szModuleName);

	if ( _taccess(pszDest, 0))
		CreateDirectoryTreeT(pszDest);

	if (hContact != NULL) {
		int ck_sum = getDword(hContact, "PictCK", 0);
		tPathLen += mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%lX"), ck_sum);
	} else
		tPathLen += mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%S avatar"), m_szModuleName);

	_tcsncpy_s((pszDest + tPathLen), (cbLen - tPathLen), (type == 1 ? _T(".swf") :  _T(".png")), _TRUNCATE);
}

INT_PTR __cdecl CYahooProto::GetAvatarInfo(WPARAM wParam,LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;
	DBVARIANT dbv;
	int avtType;

	if (!getString(AI->hContact, YAHOO_LOGINID, &dbv)) {
		debugLogA("[YAHOO_GETAVATARINFO] For: %s", dbv.pszVal);
		db_free(&dbv);
	}else {
		debugLogA("[YAHOO_GETAVATARINFO]");
	}

	if (!getByte("ShowAvatars", 1 ) || !m_bLoggedIn) {
		debugLogA("[YAHOO_GETAVATARINFO] %s", m_bLoggedIn ? "We are not using/showing avatars!" : "We are not logged in. Can't load avatars now!");

		return GAIR_NOAVATAR;
	}

	avtType = getByte(AI->hContact, "AvatarType", 0);
	debugLogA("[YAHOO_GETAVATARINFO] Avatar Type: %d", avtType);

	if ( avtType != 2) {
		if (avtType != 0)
			debugLogA("[YAHOO_GETAVATARINFO] Not handling this type yet!");

		return GAIR_NOAVATAR;
	}

	if (getDword(AI->hContact, "PictCK", 0) == 0)
		return GAIR_NOAVATAR;

	GetAvatarFileName(AI->hContact, AI->filename, SIZEOF(AI->filename), getByte(AI->hContact, "AvatarType", 0));
	AI->format = PA_FORMAT_PNG;
	debugLogA("[YAHOO_GETAVATARINFO] filename: %s", AI->filename);

	if (_taccess( AI->filename, 0) == 0)
		return GAIR_SUCCESS;

	if (( wParam & GAIF_FORCE ) != 0 && AI->hContact != NULL) {
		/* need to request it again? */
		if (getDword(AI->hContact, "PictLoading", 0) != 0 &&
			(time(NULL) - getDword(AI->hContact, "PictLastCheck", 0) < 500)) {
				debugLogA("[YAHOO_GETAVATARINFO] Waiting for avatar to load!");
				return GAIR_WAITFOR;
		} else if ( m_bLoggedIn ) {
			DBVARIANT dbv;

			if (!getString(AI->hContact, YAHOO_LOGINID, &dbv)) {
				debugLogA("[YAHOO_GETAVATARINFO] Requesting avatar!");

				request_avatar(dbv.pszVal);
				db_free(&dbv);

				return GAIR_WAITFOR;
			} else {
				debugLogA("[YAHOO_GETAVATARINFO] Can't retrieve user id?!");
			}
		}
	}

	debugLogA("[YAHOO_GETAVATARINFO] NO AVATAR???");
	return GAIR_NOAVATAR;
}

/*
 * --=[ AVS / LoadAvatars API/Services ]=--
 */
INT_PTR __cdecl CYahooProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		LOG(("[YahooGetAvatarCaps] AF_MAXSIZE"));

		((POINT*)lParam)->x = 96;
		((POINT*)lParam)->y = 96;

		break;

	case AF_PROPORTION:
		LOG(("[YahooGetAvatarCaps] AF_PROPORTION"));

		res = PIP_NONE;
		break;

	case AF_FORMATSUPPORTED:
		LOG(("[YahooGetAvatarCaps] AF_FORMATSUPPORTED"));
		res = lParam == PA_FORMAT_PNG;
		break;

	case AF_ENABLED:
		LOG(("[YahooGetAvatarCaps] AF_ENABLED"));

		res = (getByte("ShowAvatars", 1 )) ? 1 : 0;
		break;

	case AF_DONTNEEDDELAYS:
		res = 1; /* don't need to delay avatar loading */
		break;

	case AF_MAXFILESIZE:
		res = 0; /* no max filesize for now */
		break;

	case AF_DELAYAFTERFAIL:
		res = 15 * 60 * 1000; /* 15 mins */
		break;

	default:
		LOG(("[YahooGetAvatarCaps] Unknown: %d", wParam));
	}

	return res;
}

/*
Service: /GetMyAvatar
wParam=(char *)Buffer to file name
lParam=(int)Buffer size
return=0 on success, else on error
*/
INT_PTR __cdecl CYahooProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *buffer = ( TCHAR* )wParam;
	int size = (int)lParam;

	debugLogA("[YahooGetMyAvatar]");

	if (buffer == NULL || size <= 0)
		return -1;

	if (!getByte("ShowAvatars", 1 ))
		return -2;

	DBVARIANT dbv;
	int ret = -3;

	if (getDword("AvatarHash", 0)) {
		if (!getTString("AvatarFile", &dbv)) {
			if (_taccess(dbv.ptszVal, 0) == 0) {
				mir_tstrncpy(buffer, dbv.ptszVal, size-1);
				buffer[size-1] = '\0';

				ret = 0;
			}
			db_free(&dbv);
		}
	}

	return ret;
}

/*
#define PS_SETMYAVATAR "/SetMyAvatar"
wParam=0
lParam=(const char *)Avatar file name
return=0 for sucess
*/

INT_PTR __cdecl CYahooProto::SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR* tszFile = ( TCHAR* )lParam;
	TCHAR  tszMyFile[MAX_PATH+1];

	GetAvatarFileName(NULL, tszMyFile, MAX_PATH, 2);

	if (tszFile == NULL) {
		debugLogA("[Deleting Avatar Info]");

		/* remove ALL our Avatar Info Keys */
		delSetting("AvatarFile");
		delSetting("AvatarHash");
		delSetting("AvatarURL");
		delSetting("AvatarTS");

		/* Send a Yahoo packet saying we don't got an avatar anymore */
		yahoo_send_picture_status(m_id, 0);

		setByte("ShareAvatar",0);

		DeleteFile(tszMyFile);
	} else {
		HANDLE hFile = CreateFile(tszFile,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			0);

		if ( hFile ==  INVALID_HANDLE_VALUE )
			return 1;

		DWORD dwPngSize = GetFileSize( hFile, NULL);
		BYTE *pResult = ( BYTE* )malloc(dwPngSize);
		if (pResult == NULL) {
			CloseHandle(hFile);
			return 2;
		}

		DWORD dw;
		ReadFile( hFile, pResult, dwPngSize, &dw, NULL);
		CloseHandle( hFile );

		hFile = CreateFile(tszMyFile,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, 0);
		if ( hFile ==  INVALID_HANDLE_VALUE )
			return 1;

		WriteFile( hFile, pResult, dwPngSize, &dw, NULL);
		SetEndOfFile( hFile);
		CloseHandle( hFile );

		unsigned int hash = YAHOO_avt_hash(( const char* )pResult, dwPngSize);
		free( pResult );

		if ( hash ) {
			LOG(("[YAHOO_SetAvatar] File: '%s' CK: %d", tszMyFile, hash));

			/* now check and make sure we don't reupload same thing over again */
			if (hash != getDword("AvatarHash", 0)) {
				setTString("AvatarFile", tszMyFile);
				setDword("TMPAvatarHash", hash);

				/*	Set Sharing to ON if it's OFF */
				if (getByte("ShareAvatar", 0) != 2) {
					setByte("ShareAvatar", 2 );
					yahoo_send_picture_status(m_id, 2);
				}

				SendAvatar(tszMyFile);
			}
			else LOG(("[YAHOO_SetAvatar] Same checksum and avatar on YahooFT. Not Reuploading."));
	}	}

	return 0;
}

/*
 * --=[ ]=--
 */
