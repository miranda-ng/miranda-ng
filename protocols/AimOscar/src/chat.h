#ifndef CHAT_H
#define CHAT_H

struct chatnav_param
{
    char* id;
    unsigned short exchange;
    unsigned short instance;

    char* message;
    char* sn;
    char* icbm_cookie;

    bool isroom;

    chatnav_param(char* tid, unsigned short ex, unsigned short in, char* msg, char* nm, char* icki)
    { id = tid; exchange = ex; instance = in; isroom = false; 
      message = mir_strdup(msg); sn = mir_strdup(nm); icbm_cookie = (char*)mir_alloc(8); memcpy(icbm_cookie, icki, 8); }

    chatnav_param(char* tid, unsigned short ex)
    { id = mir_strdup(tid); exchange = ex; isroom = true; 
      message = NULL; sn = NULL; icbm_cookie = NULL; }

    ~chatnav_param()
    { 
        mir_free(id); 
        mir_free(message);
        mir_free(sn);
        mir_free(icbm_cookie);
    }
}; 

struct chat_list_item
{
    char* id;
    char* cookie;
    HANDLE hconn;
    unsigned short cid;
	unsigned short seqno;
    unsigned short exchange;
    unsigned short instance;
	char* CHAT_COOKIE;
	int CHAT_COOKIE_LENGTH;

    chat_list_item(char* tid, char* tcookie, unsigned short ex, unsigned short in)
    { id = mir_strdup(tid); cid = get_random(); seqno = 0; hconn = NULL;
      cookie = mir_strdup(tcookie); exchange = ex; instance = in; }

    ~chat_list_item()
    { mir_free(id); mir_free(cookie); }
};

#endif