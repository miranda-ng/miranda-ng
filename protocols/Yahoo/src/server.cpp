/*
 * $Id: server.cpp 9810 2009-05-19 01:43:22Z gena01 $
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
#include <time.h>

int PASCAL send(SOCKET s, const char FAR *buf, int len, int flags)
{
	int rlen = Netlib_Send((HANDLE)s, buf, len, 0);

#ifdef HTTP_GATEWAY				
	if (iHTTPGateway)
		lLastSend = time(NULL);
#endif

	if (rlen == SOCKET_ERROR) {
		LOG(("SEND Error."));
		return -1;
	}

	return len;
}

int PASCAL recv(SOCKET s, char FAR *buf, int len, int flags)
{
	int RecvResult = Netlib_Recv((HANDLE)s, buf, len, (len == 1) ? MSG_NODUMP : 0);

	if (RecvResult == 0) {
		LOG(("[recv] Connection closed gracefully."));
		return 0;
	} 
	if (RecvResult == SOCKET_ERROR) {
		LOG(("[recv] Connection abortively closed"));
		return -1;
	}

	return RecvResult;
}

void __cdecl CYahooProto::server_main(void *empty)
{
	enum yahoo_status status = (enum yahoo_status) (int)empty;
	time_t lLastPing, lLastKeepAlive, t;
	YList *l;
	NETLIBSELECTEX nls = {0};
	int recvResult, ridx = 0, widx = 0, i;

	debugLogA("Server Thread Starting status: %d", status);

	do_yahoo_debug=YAHOO_LOG_DEBUG;
	yahoo_set_log_level(( yahoo_log_level )do_yahoo_debug);

	poll_loop = 1; /* set this so we start looping */

	ext_login(status);

	lLastKeepAlive = lLastPing = time(NULL);

	while (poll_loop) {
		nls.cbSize = sizeof(nls);
		nls.dwTimeout = 1000; // 1000 millis = 1 sec 

		FD_ZERO(&nls.hReadStatus);
		FD_ZERO(&nls.hWriteStatus);
		FD_ZERO(&nls.hExceptStatus);
		nls.hExceptConns[0] = NULL;
		ridx = 0; widx = 0; 

		for(l=m_connections; l; ) {
			struct _conn *c = ( _conn * )l->data;
			//LOG(("Connection tag:%d id:%d fd:%d remove:%d", c->tag, c->id, c->fd, c->remove));
			if (c->remove) {
				YList *n = y_list_next(l);
				//LOG(("Removing id:%d fd:%d tag:%d", c->id, c->fd, c->tag));
				m_connections = y_list_remove_link(m_connections, l);
				y_list_free_1(l);
				FREE(c);
				l=n;
			} else {
				if (c->cond & YAHOO_INPUT_READ) {
					//LOG(("[YAHOO_INPUT_READ] Waiting on read. Tag: %d fd: %d", c->tag, c->fd ));
					nls.hReadConns[ridx++] = (HANDLE)c->fd;
				}

				if (c->cond & YAHOO_INPUT_WRITE) {
					//LOG(("[YAHOO_INPUT_WRITE] Waiting on write. Tag: %d fd: %d", c->tag, c->fd ));
					nls.hWriteConns[widx++] =(HANDLE) c->fd;
				}

				l = y_list_next(l);
			}
		}

		//debugLogA("[Yahoo_Server] ridx:%d widx:%d", ridx, widx);

		nls.hReadConns[ridx] = NULL;
		nls.hWriteConns[widx] = NULL;

		if (m_connections == NULL) {
			debugLogA("Last connection closed.");
			break;
		}
		recvResult = CallService(MS_NETLIB_SELECTEX, (WPARAM) 0, (LPARAM)&nls);

		/* do the timer check */
		if (m_id > 0) {
#ifdef	HTTP_GATEWAY			
			//debugLogA("HTTPGateway: %d", iHTTPGateway);
			if	(!iHTTPGateway) {
#endif					
				t = time(NULL); 

				if (m_bLoggedIn && t - lLastKeepAlive >= 60) {
					LOG(("[TIMER] Sending a keep alive message"));
					yahoo_keepalive(m_id);

					lLastKeepAlive = t;
				}

				if (m_bLoggedIn && t - lLastPing >= 3600) {
					LOG(("[TIMER] Sending ping"));
					yahoo_send_ping(m_id);

					lLastPing = t;
				}

#ifdef HTTP_GATEWAY					
			} else {
				debugLogA("[SERVER] Got packets: %d", ylad->rpkts);

				if ( m_bLoggedIn && ( (ylad->rpkts > 0 && (time(NULL) - lLastSend) >=3) ||
					( (time(NULL) - lLastSend) >= 13)) ) {

						LOG(("[TIMER] Sending an idle message..."));
						yahoo_send_idle_packet(m_id);
				}

				//
				// need to sleep, cause netlibselectex is too fast?
				//
				SleepEx(500, TRUE);
			}
#endif				
		}
		/* do the timer check ends */

		for(l = m_connections; l; l = y_list_next(l)) {
			struct _conn *c = ( _conn * )l->data;

			if (c->remove) 
				continue;

			/* are we waiting for a Read request? */
			if (c->cond & YAHOO_INPUT_READ) {
				
				for (i = 0; i  < ridx; i++) {
					if ((HANDLE)c->fd == nls.hReadConns[i]) {
						if (nls.hReadStatus[i]) {
							//LOG(("[YAHOO_INPUT_READ] Read Ready. Tag: %d fd: %d", c->tag, c->fd ));
							yahoo_callback(c, YAHOO_INPUT_READ);
						}
					}//if c->fd=
				}//for i = 0
				
			}

			/* are we waiting for a Write request? */
			if (c->cond & YAHOO_INPUT_WRITE) {
				
				for (i = 0; i  < widx; i++) {
					if ((HANDLE)c->fd == nls.hWriteConns[i]) {
						if (nls.hWriteStatus[i]) {
							//LOG(("[YAHOO_INPUT_WRITE] Write ready. Tag: %d fd: %d", c->tag, c->fd ));
							yahoo_callback(c, YAHOO_INPUT_WRITE);
						}
					} // if c->fd == nls
				}// for i = 0
			}

		}// for l=connections

		/* Check for Miranda Exit Status */
		if (Miranda_Terminated()) {
			debugLogA("Miranda Exiting... stopping the loop.");
			break;
		}
	}
	debugLogA("Exited loop");

	/* cleanup the data stuff and close our connection handles */
	while(m_connections) {
		YList *tmp = m_connections;
		struct _conn * c = ( _conn * )m_connections->data;
		Netlib_CloseHandle((HANDLE)c->fd);
		FREE(c);
		m_connections = y_list_remove_link(m_connections, m_connections);
		y_list_free_1(tmp);
	}

	yahoo_close(m_id);

	m_bLoggedIn = FALSE; 

	m_status = YAHOO_STATUS_OFFLINE;
	m_id = 0;

	/* now set ourselves to offline */
	BroadcastStatus(ID_STATUS_OFFLINE);
	logoff_buddies();	

	debugLogA("Server thread ending");
}
