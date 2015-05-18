#include "stdafx.h"

#include "csocket.h"
#include "cserver.h"
#include "http_api.h"

#define MS_FAVCONTACTS_OPEN_CONTACT			"FavContacts/OpenContact"

class CHttpProcessor : public IConnectionProcessor
{
private:
	CSocket *m_socket;

	char *FetchURL(char *s)
	{
		char *p;
		if (p = strstr(s, "\r\n")) *p = 0;
		if (p = strrchr(s, ' ')) *p = 0;
		if (p = strchr(s, ' ')) while (*p && *p == ' ') p++;
		return mir_strdup(p);
	}

public:
	CHttpProcessor(CSocket *s) : m_socket(s) {}

	void ProcessConnection()
	{
		char buf[1024];
		int n = m_socket->Recv(buf, sizeof(buf));
		if (n > 0) {
			buf[n] = 0;

			char *s = FetchURL(buf);

			if (!strncmp(s, "/fav/list/", 10))
			{
				SendList();
			}
			else if (!strncmp(s, "/fav/open/", 10))
			{
				OpenContact(s);
			}

			mir_free(s);
		}
		m_socket->Close();
	}

	void OpenContact(char *s)
	{
		m_socket->Send("HTTP 200 OK\r\n\r\n");

		int hContact;
		sscanf(s, "/fav/open/%d", &hContact);
		if (CallService(MS_DB_CONTACT_IS, hContact, 0))
			CallServiceSync(MS_FAVCONTACTS_OPEN_CONTACT, hContact, 0);
	}

	void SendList()
	{
		TFavContacts favList;
		favList.build();

		m_socket->Send(
			"HTTP 200 OK\r\n"
			"Content-Type: text/javascript\r\n"
			"\r\n");

		Send("try {\r\n");
		Send("SetContactCount(");
		Send(favList.getCount());
		Send(");\r\n");

		for (int i = 0; i < favList.getCount(); ++i)
		{
			MCONTACT hContact = favList[i]->getHandle();
			TCHAR *name = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
			AVATARCACHEENTRY *avatar = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
			int status = db_get_w(hContact, GetContactProto(hContact), "Status", ID_STATUS_OFFLINE);

			Send("SetContact(");
			Send(i);
			Send(", ");
			Send((int)hContact);// XXX: x64 broken!?
			Send(", '");
			SendQuoted(name);
			Send("', ");
			Send(status);
			Send(", '");
			SendQuoted(avatar ? avatar->szFilename : _T(""));
			Send("');\r\n");
		}
		Send("} catch(e) {}\r\n");
	}

	void Send(char *s)
	{
		m_socket->Send(s);
	}


	void Send(WCHAR *ws)
	{
		char *s = mir_utf8encodeW(ws);
		m_socket->Send(s);
		mir_free(s);
	}


	void Send(int i)
	{
		char buf[32];
		mir_snprintf(buf, SIZEOF(buf), "%d", i);
		Send(buf);
	}

	template<class XCHAR>
	void SendQuoted(const XCHAR *s)
	{
		int length = 0;
		const XCHAR *p;
		for (p = s; *p; p++)
		{
			if (*p == '\'' || *p == '\\' || *p == '\"')
				length++;
			length++;
		}
		XCHAR *buf = (XCHAR *)mir_alloc(sizeof(XCHAR) * (length + 1));
		XCHAR *q = buf;
		for (p = s; *p; p++)
		{
			if (*p == '\'' || *p == '\\' || *p == '\"')
			{
				*q = '\\';
				q++;
			}
			*q = *p;
			q++;
		}
		*q = 0;
		Send(buf);
		mir_free(buf);
	}
};

class CHttpProcessorFactory : public IConnectionProcessorFactory
{
public:
	IConnectionProcessor *Create(CSocket *s)
	{
		return new CHttpProcessor(s);
	}
};

static CHttpProcessorFactory g_httpProcessorFactory;
static CServer g_httpServer;

void LoadHttpApi()
{
	g_httpServer.Start(60888, &g_httpProcessorFactory, true);
}

int UnloadHttpApi()
{
	g_httpServer.Stop();
	return 0;
}
