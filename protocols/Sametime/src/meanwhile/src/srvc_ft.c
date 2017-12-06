
/*
  Meanwhile - Unofficial Lotus Sametime Community Client Library
  Copyright (C) 2004  Christopher (siege) O'Brien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <glib.h>

#include "mw_channel.h"
#include "mw_common.h"
#include "mw_debug.h"
#include "mw_error.h"
#include "mw_message.h"
#include "mw_service.h"
#include "mw_session.h"
#include "mw_srvc_ft.h"
#include "mw_util.h"


#define PROTOCOL_TYPE  0x00000000
#define PROTOCOL_VER   0x00000001


/** send-on-channel type: FT transfer data */
#define msg_TRANSFER  0x0001


/** ack received transfer data */
#define msg_RECEIVED  0x0002


struct mwServiceFileTransfer {
  struct mwService service;

  struct mwFileTransferHandler *handler;
  GList *transfers;
};


struct mwFileTransfer {
  struct mwServiceFileTransfer *service;
  
  struct mwChannel *channel;
  struct mwIdBlock who;

  enum mwFileTransferState state;

  char *filename;
  char *message;

  guint32 size;
  guint32 remaining;

  struct mw_datum client_data;
};


/** momentarily places a mwLoginInfo into a mwIdBlock */
static void login_into_id(struct mwIdBlock *to, struct mwLoginInfo *from) {
  to->user = from->user_id;
  to->community = from->community;
}


static const char *ft_state_str(enum mwFileTransferState state) {
  switch(state) {
  case mwFileTransfer_NEW:
    return "new";

  case mwFileTransfer_PENDING:
    return "pending";

  case mwFileTransfer_OPEN:
    return "open";

  case mwFileTransfer_CANCEL_LOCAL:
    return "cancelled locally";

  case mwFileTransfer_CANCEL_REMOTE:
    return "cancelled remotely";

  case mwFileTransfer_DONE:
    return "done";

  case mwFileTransfer_ERROR:
    return "error";

  case mwFileTransfer_UNKNOWN:
  default:
    return "UNKNOWN";
  }
}


static void ft_state(struct mwFileTransfer *ft,
		     enum mwFileTransferState state) {

  g_return_if_fail(ft != NULL);

  if(ft->state == state) return;

  // Miranda NG adaptation
  //g_info("setting ft (%s, %s) state: %s",
  //	 NSTR(ft->who.user), NSTR(ft->who.community),
  //	 ft_state_str(state));
  g_message("setting ft (%s, %s) state: %s", NSTR(ft->who.user), NSTR(ft->who.community), ft_state_str(state));

  ft->state = state;
}


static void recv_channelCreate(struct mwServiceFileTransfer *srvc,
			       struct mwChannel *chan,
			       struct mwMsgChannelCreate *msg) {

  struct mwFileTransferHandler *handler;
  struct mwGetBuffer *b;

  char *fnm, *txt;
  guint32 size, junk;
  gboolean b_err;

  g_return_if_fail(srvc->handler != NULL);
  handler = srvc->handler;
  
  b = mwGetBuffer_wrap(&msg->addtl);

  guint32_get(b, &junk); /* unknown */
  mwString_get(b, &fnm); /* offered filename */
  mwString_get(b, &txt); /* offering message */
  guint32_get(b, &size); /* size of offered file */
  /// Miranda NG adaptation - start - http://www.lilotux.net/~mikael/pub/meanwhile/ft_fix.diff
  /* guint32_get(b, &junk); */ /* unknown */
  /// Miranda NG adaptation - end
  /* and we just skip an unknown guint16 at the end */

  b_err = mwGetBuffer_error(b);
  mwGetBuffer_free(b);

  if(b_err) {
    g_warning("bad/malformed addtl in File Transfer service");
    mwChannel_destroy(chan, ERR_FAILURE, NULL);

  } else {
    struct mwIdBlock idb;
    struct mwFileTransfer *ft;

    login_into_id(&idb, mwChannel_getUser(chan));
    ft = mwFileTransfer_new(srvc, &idb, txt, fnm, size);
    ft->channel = chan;
    ft_state(ft, mwFileTransfer_PENDING);

    mwChannel_setServiceData(chan, ft, NULL);

    if(handler->ft_offered)
      handler->ft_offered(ft);
  }

  g_free(fnm);
  g_free(txt);
}


static void recv_channelAccept(struct mwServiceFileTransfer *srvc,
			       struct mwChannel *chan,
			       struct mwMsgChannelAccept *msg) {

  struct mwFileTransferHandler *handler;
  struct mwFileTransfer *ft;

  g_return_if_fail(srvc->handler != NULL);
  handler = srvc->handler;

  ft = mwChannel_getServiceData(chan);
  g_return_if_fail(ft != NULL);

  ft_state(ft, mwFileTransfer_OPEN);

  if(handler->ft_opened)
    handler->ft_opened(ft);
}


static void recv_channelDestroy(struct mwServiceFileTransfer *srvc,
				struct mwChannel *chan,
				struct mwMsgChannelDestroy *msg) {

  struct mwFileTransferHandler *handler;
  struct mwFileTransfer *ft;
  guint32 code;

  code = msg->reason;

  g_return_if_fail(srvc->handler != NULL);
  handler = srvc->handler;

  ft = mwChannel_getServiceData(chan);
  g_return_if_fail(ft != NULL);

  ft->channel = NULL;

  if(! mwFileTransfer_isDone(ft))
    ft_state(ft, mwFileTransfer_CANCEL_REMOTE);

  mwFileTransfer_close(ft, code);
}


static void recv_TRANSFER(struct mwFileTransfer *ft,
			  struct mwOpaque *data) {

  struct mwServiceFileTransfer *srvc;
  struct mwFileTransferHandler *handler;
  
  srvc = ft->service;
  handler = srvc->handler;

  g_return_if_fail(mwFileTransfer_isOpen(ft));

  if(data->len > ft->remaining) {
    /* @todo handle error */

  } else {
    ft->remaining -= data->len;

    if(! ft->remaining)
      ft_state(ft, mwFileTransfer_DONE);
    
    if(handler->ft_recv)
      handler->ft_recv(ft, data);
  }
}


static void recv_RECEIVED(struct mwFileTransfer *ft,
			  struct mwOpaque *data) {

  struct mwServiceFileTransfer *srvc;
  struct mwFileTransferHandler *handler;
  
  srvc = ft->service;
  handler = srvc->handler;

  if(! ft->remaining)
    ft_state(ft, mwFileTransfer_DONE);

  if(handler->ft_ack)
    handler->ft_ack(ft);

  if(! ft->remaining)
    mwFileTransfer_close(ft, mwFileTransfer_SUCCESS);
}


static void recv(struct mwService *srvc, struct mwChannel *chan,
		 guint16 type, struct mwOpaque *data) {

  struct mwFileTransfer *ft;
  
  ft = mwChannel_getServiceData(chan);
  g_return_if_fail(ft != NULL);

  switch(type) {
  case msg_TRANSFER:
    recv_TRANSFER(ft, data);
    break;

  case msg_RECEIVED:
    recv_RECEIVED(ft, data);
    break;

  default:
    mw_mailme_opaque(data, "unknown message in ft service: 0x%04x", type);
  }
}


static void clear(struct mwServiceFileTransfer *srvc) {
  struct mwFileTransferHandler *h;
  
  h = srvc->handler;
  if(h && h->clear)
    h->clear(srvc);
  srvc->handler = NULL;
}


static const char *name(struct mwService *srvc) {
  return "File Transfer";
}


static const char *desc(struct mwService *srvc) {
  return "Provides file transfer capabilities through the community server";
}


static void start(struct mwService *srvc) {
  mwService_started(srvc);
}


static void stop(struct mwServiceFileTransfer *srvc) {
  while(srvc->transfers) {
    mwFileTransfer_free(srvc->transfers->data);
  }

  mwService_stopped(MW_SERVICE(srvc));
}


struct mwServiceFileTransfer *
mwServiceFileTransfer_new(struct mwSession *session,
			  struct mwFileTransferHandler *handler) {

  struct mwServiceFileTransfer *srvc_ft;
  struct mwService *srvc;

  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(handler != NULL, NULL);

  srvc_ft = g_new0(struct mwServiceFileTransfer, 1);
  srvc = MW_SERVICE(srvc_ft);

  mwService_init(srvc, session, mwService_FILE_TRANSFER);
  srvc->recv_create = (mwService_funcRecvCreate) recv_channelCreate;
  srvc->recv_accept = (mwService_funcRecvAccept) recv_channelAccept;
  srvc->recv_destroy = (mwService_funcRecvDestroy) recv_channelDestroy;
  srvc->recv = recv;
  srvc->clear = (mwService_funcClear) clear;
  srvc->get_name = name;
  srvc->get_desc = desc;
  srvc->start = start;
  srvc->stop = (mwService_funcStop) stop;

  srvc_ft->handler = handler;

  return srvc_ft;
}


struct mwFileTransferHandler *
mwServiceFileTransfer_getHandler(struct mwServiceFileTransfer *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return srvc->handler;
}


const GList *
mwServiceFileTransfer_getTransfers(struct mwServiceFileTransfer *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return srvc->transfers;
}


/// Miranda NG adaptation start - new method
struct mwService *
mwServiceFileTransfer_getService(struct mwServiceFileTransfer *srvc) {
  g_return_val_if_fail(srvc != NULL, NULL);
  return &(srvc->service);
}
/// Miranda NG adaptation end


struct mwFileTransfer *
mwFileTransfer_new(struct mwServiceFileTransfer *srvc,
		   const struct mwIdBlock *who, const char *msg,
		   const char *filename, guint32 filesize) {
  
  struct mwFileTransfer *ft;

  g_return_val_if_fail(srvc != NULL, NULL);
  g_return_val_if_fail(who != NULL, NULL);
  
  ft = g_new0(struct mwFileTransfer, 1);
  ft->service = srvc;
  mwIdBlock_clone(&ft->who, who);
  ft->filename = g_strdup(filename);
  ft->message = g_strdup(msg);
  ft->size = ft->remaining = filesize;

  ft_state(ft, mwFileTransfer_NEW);

  /* stick a reference in the service */
  srvc->transfers = g_list_prepend(srvc->transfers, ft);

  return ft;
}


struct mwServiceFileTransfer *
mwFileTransfer_getService(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, NULL);
  return ft->service;
}


enum mwFileTransferState
mwFileTransfer_getState(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, mwFileTransfer_UNKNOWN);
  return ft->state;
}


const struct mwIdBlock *
mwFileTransfer_getUser(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, NULL);
  return &ft->who;
}


const char *
mwFileTransfer_getMessage(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, NULL);
  return ft->message;
}


const char *
mwFileTransfer_getFileName(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, NULL);
  return ft->filename;
}


guint32 mwFileTransfer_getFileSize(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, 0);
  return ft->size;
}


guint32 mwFileTransfer_getRemaining(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, 0);
  return ft->remaining;
}


int mwFileTransfer_accept(struct mwFileTransfer *ft) {
  struct mwServiceFileTransfer *srvc;
  struct mwFileTransferHandler *handler;
  int ret;

  g_return_val_if_fail(ft != NULL, -1);
  g_return_val_if_fail(ft->channel != NULL, -1);
  g_return_val_if_fail(mwFileTransfer_isPending(ft), -1);
  g_return_val_if_fail(mwChannel_isIncoming(ft->channel), -1);
  g_return_val_if_fail(mwChannel_isState(ft->channel, mwChannel_WAIT), -1);

  g_return_val_if_fail(ft->service != NULL, -1);
  srvc = ft->service;

  g_return_val_if_fail(srvc->handler != NULL, -1);
  handler = srvc->handler;

  ret = mwChannel_accept(ft->channel);

  if(ret) {
    mwFileTransfer_close(ft, ERR_FAILURE);

  } else {
    ft_state(ft, mwFileTransfer_OPEN);
    if(handler->ft_opened)
      handler->ft_opened(ft);
  }

  return ret;
}


static void ft_create_chan(struct mwFileTransfer *ft) {
  struct mwSession *s;
  struct mwChannelSet *cs;
  struct mwChannel *chan;
  struct mwLoginInfo *login;
  struct mwPutBuffer *b;
  
  /* we only should be calling this if there isn't a channel already
     associated with the conversation */
  g_return_if_fail(ft != NULL);
  g_return_if_fail(mwFileTransfer_isNew(ft));
  g_return_if_fail(ft->channel == NULL);
		   
  s = mwService_getSession(MW_SERVICE(ft->service));
  cs = mwSession_getChannels(s);

  chan = mwChannel_newOutgoing(cs);
  mwChannel_setService(chan, MW_SERVICE(ft->service));
  mwChannel_setProtoType(chan, PROTOCOL_TYPE);
  mwChannel_setProtoVer(chan, PROTOCOL_VER);

  /* offer all known ciphers */
  mwChannel_populateSupportedCipherInstances(chan);

  /* set the target */
  login = mwChannel_getUser(chan);
  login->user_id = g_strdup(ft->who.user);
  login->community = g_strdup(ft->who.community);

  /* compose the addtl create */
  b = mwPutBuffer_new();
  guint32_put(b, 0x00);
  mwString_put(b, ft->filename);
  mwString_put(b, ft->message);
  guint32_put(b, ft->size);
  guint32_put(b, 0x00);
  guint16_put(b, 0x00);

  mwPutBuffer_finalize(mwChannel_getAddtlCreate(chan), b);

  ft->channel = mwChannel_create(chan)? NULL: chan;
  if(ft->channel) {
    mwChannel_setServiceData(ft->channel, ft, NULL);
  }
}


int mwFileTransfer_offer(struct mwFileTransfer *ft) {
  struct mwServiceFileTransfer *srvc;
  struct mwFileTransferHandler *handler;

  g_return_val_if_fail(ft != NULL, -1);
  g_return_val_if_fail(ft->channel == NULL, -1);
  g_return_val_if_fail(mwFileTransfer_isNew(ft), -1);

  g_return_val_if_fail(ft->service != NULL, -1);
  srvc = ft->service;

  g_return_val_if_fail(srvc->handler != NULL, -1);
  handler = srvc->handler;

  ft_create_chan(ft);
  if(ft->channel) {
    ft_state(ft, mwFileTransfer_PENDING);
  } else {
    ft_state(ft, mwFileTransfer_ERROR);
    mwFileTransfer_close(ft, ERR_FAILURE);
  }

  return 0;
}


int mwFileTransfer_close(struct mwFileTransfer *ft, guint32 code) {
  struct mwServiceFileTransfer *srvc;
  struct mwFileTransferHandler *handler;
  int ret = 0;

  g_return_val_if_fail(ft != NULL, -1);

  if(mwFileTransfer_isOpen(ft))
    ft_state(ft, mwFileTransfer_CANCEL_LOCAL);

  if(ft->channel) {
    ret = mwChannel_destroy(ft->channel, code, NULL);
    ft->channel = NULL;
  }

  srvc = ft->service;
  g_return_val_if_fail(srvc != NULL, ret);

  handler = srvc->handler;
  g_return_val_if_fail(handler != NULL, ret);

  if(handler->ft_closed)
    handler->ft_closed(ft, code);

  return ret;
}


void mwFileTransfer_free(struct mwFileTransfer *ft) {
  struct mwServiceFileTransfer *srvc;

  if(! ft) return;

  srvc = ft->service;
  if(srvc)
    srvc->transfers = g_list_remove(srvc->transfers, ft);

  if(ft->channel) {
    mwChannel_destroy(ft->channel, mwFileTransfer_SUCCESS, NULL);
    ft->channel = NULL;
  }

  mwFileTransfer_removeClientData(ft);

  mwIdBlock_clear(&ft->who);
  g_free(ft->filename);
  g_free(ft->message);
  g_free(ft);
}


int mwFileTransfer_send(struct mwFileTransfer *ft,
			struct mwOpaque *data) {

  struct mwChannel *chan;
  int ret;

  g_return_val_if_fail(ft != NULL, -1);
  g_return_val_if_fail(mwFileTransfer_isOpen(ft), -1);
  g_return_val_if_fail(ft->channel != NULL, -1);
  chan = ft->channel;

  g_return_val_if_fail(mwChannel_isOutgoing(chan), -1);

  if(data->len > ft->remaining) {
    /* @todo handle error */
    return -1;
  }

  ret = mwChannel_send(chan, msg_TRANSFER, data);
  if(! ret) ft->remaining -= data->len;
  
  /* we're not done until we receive an ACK for the last piece of
     outgoing data */

  return ret;
}


int mwFileTransfer_ack(struct mwFileTransfer *ft) {
  struct mwChannel *chan;

  g_return_val_if_fail(ft != NULL, -1);

  chan = ft->channel;
  g_return_val_if_fail(chan != NULL, -1);
  g_return_val_if_fail(mwChannel_isIncoming(chan), -1);

  return mwChannel_sendEncrypted(chan, msg_RECEIVED, NULL, FALSE);
}


void mwFileTransfer_setClientData(struct mwFileTransfer *ft,
				  gpointer data, GDestroyNotify clean) {
  g_return_if_fail(ft != NULL);
  mw_datum_set(&ft->client_data, data, clean);
}


gpointer mwFileTransfer_getClientData(struct mwFileTransfer *ft) {
  g_return_val_if_fail(ft != NULL, NULL);
  return mw_datum_get(&ft->client_data);
}


void mwFileTransfer_removeClientData(struct mwFileTransfer *ft) {
  g_return_if_fail(ft != NULL);
  mw_datum_clear(&ft->client_data);
}

