
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


#ifndef _MW_SRVC_FT_H
#define _MW_SRVC_FT_H


/** @file mw_srvc_ft.h

    A file transfer is a simple way to get large chunks of binary data
    from one client to another.
*/


#include "mw_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @struct mwServiceFileTransfer
    File transfer service
*/
struct mwServiceFileTransfer;


/** @struct mwFileTransfer
    A single file trasfer session
 */
struct mwFileTransfer;


#define mwService_FILE_TRANSFER  0x00000038


enum mwFileTransferState {
  mwFileTransfer_NEW,   /**< file transfer is not open */
  mwFileTransfer_PENDING,  /**< file transfer is opening */
  mwFileTransfer_OPEN,     /**< file transfer is open */
  mwFileTransfer_CANCEL_LOCAL,
  mwFileTransfer_CANCEL_REMOTE,
  mwFileTransfer_DONE,
  mwFileTransfer_ERROR,    /**< error in file transfer */
  mwFileTransfer_UNKNOWN,  /**< unknown state */
};


#define mwFileTransfer_isState(ft, state) \
  (mwFileTransfer_getState(ft) == (state))

#define mwFileTransfer_isNew(ft) \
  mwFileTransfer_isState((ft), mwFileTransfer_NEW)

#define mwFileTransfer_isPending(ft) \
  mwFileTransfer_isState((ft), mwFileTransfer_PENDING)

#define mwFileTransfer_isOpen(ft) \
  mwFileTransfer_isState((ft), mwFileTransfer_OPEN)

#define mwFileTransfer_isDone(ft) \
  mwFileTransfer_isState((ft), mwFileTransfer_DONE)

#define mwFileTransfer_isCancelLocal(ft) \
  mwFileTransfer_isState((ft), mwFileTransfer_CANCEL_LOCAL)

#define mwFileTransfer_isCancelRemote(ft) \
  mwFileTransfer_isState((ft), mwFileTransfer_CANCEL_REMOTE)


enum mwFileTranferCode {
  mwFileTransfer_SUCCESS   = 0x00000000,
  mwFileTransfer_REJECTED  = 0x08000606,
};


struct mwFileTransferHandler {

  /** an incoming file transfer has been offered */
  void (*ft_offered)(struct mwFileTransfer *ft);

  /** a file transfer has been fully initiated */
  void (*ft_opened)(struct mwFileTransfer *ft);

  /** a file transfer has been closed. Check the status of the file
      transfer to determine if the transfer was complete or if it had
      been interrupted */
  void (*ft_closed)(struct mwFileTransfer *ft, guint32 code);

  /** receive a chunk of a file from an inbound file transfer. */
  void (*ft_recv)(struct mwFileTransfer *ft, struct mwOpaque *data);

  /** received an ack for a sent chunk on an outbound file transfer.
      this indicates that a previous call to mwFileTransfer_send has
      reached the target and that the target has responded. */
  void (*ft_ack)(struct mwFileTransfer *ft);

  /** optional. called from mwService_free */
  void (*clear)(struct mwServiceFileTransfer *srvc);
};


struct mwServiceFileTransfer *
mwServiceFileTransfer_new(struct mwSession *session,
			  struct mwFileTransferHandler *handler);


struct mwFileTransferHandler *
mwServiceFileTransfer_getHandler(struct mwServiceFileTransfer *srvc);


const GList *
mwServiceFileTransfer_getTransfers(struct mwServiceFileTransfer *srvc);


/// Miranda NG adaptation start - new method
struct mwService *
mwServiceFileTransfer_getService(struct mwServiceFileTransfer *srvc);
/// Miranda NG adaptation end


struct mwFileTransfer *
mwFileTransfer_new(struct mwServiceFileTransfer *srvc,
		   const struct mwIdBlock *who, const char *msg,
		   const char *filename, guint32 filesize);


/** deallocate a file transfer. will call mwFileTransfer_close if
    necessary */
void
mwFileTransfer_free(struct mwFileTransfer *ft);


/** the status of this file transfer */
enum mwFileTransferState
mwFileTransfer_getState(struct mwFileTransfer *ft);


struct mwServiceFileTransfer *
mwFileTransfer_getService(struct mwFileTransfer *ft);


/** the user on the other end of the file transfer */
const struct mwIdBlock *
mwFileTransfer_getUser(struct mwFileTransfer *ft);


/** the message sent along with an offered file transfer */
const char *
mwFileTransfer_getMessage(struct mwFileTransfer *ft);


/** the publicized file name. Not necessarily related to any actual
    file on either system */
const char *
mwFileTransfer_getFileName(struct mwFileTransfer *ft);


/** total bytes intended to be sent/received */
guint32 mwFileTransfer_getFileSize(struct mwFileTransfer *ft);


/** bytes remaining to be received/send */
guint32 mwFileTransfer_getRemaining(struct mwFileTransfer *ft);


/** count of bytes sent/received over this file transfer so far */
#define mwFileTransfer_getSent(ft) \
  (mwFileTransfer_getFileSize(ft) - mwFileTransfer_getRemaining(ft))


/** initiate an outgoing file transfer */
int mwFileTransfer_offer(struct mwFileTransfer *ft);


/** accept an incoming file transfer */
int mwFileTransfer_accept(struct mwFileTransfer *ft);


/** reject an incoming file transfer */
#define mwFileTransfer_reject(ft) \
  mwFileTransfer_close((ft), mwFileTransfer_REJECTED)


/** cancel an open file transfer */
#define mwFileTransfer_cancel(ft) \
  mwFileTransfer_close((ft), mwFileTransfer_SUCCESS);


/** Close a file transfer. This will trigger the ft_close function of the
    session's handler.

    @see mwFileTransfer_reject
    @see mwFileTransfer_cancel
*/
int mwFileTransfer_close(struct mwFileTransfer *ft, guint32 code);


/** send a chunk of data over an outbound file transfer. The client at
    the other end of the transfer should respond with an acknowledgement
    message, which can be caught in the service's handler.

    @see mwFileTransferHandler::ft_ack
*/
int mwFileTransfer_send(struct mwFileTransfer *ft,
			struct mwOpaque *data);


/** acknowledge the receipt of a chunk of data from an inbound file
    transfer.  This should be done after every received chunk, or the
    transfer will stall. However, not all clients will wait for an ack
    after sending a chunk before sending the next chunk, so it is
    possible to have the handler's ft_recv function triggered again
    even if no ack was sent.

    @see mwFileTransferHandler::ft_recv
*/
int mwFileTransfer_ack(struct mwFileTransfer *ft);


void mwFileTransfer_setClientData(struct mwFileTransfer *ft,
				  gpointer data, GDestroyNotify clean);


gpointer mwFileTransfer_getClientData(struct mwFileTransfer *ft);


void mwFileTransfer_removeClientData(struct mwFileTransfer *ft);


#ifdef __cplusplus
}
#endif


#endif /* _MW_SRVC_FT_H */
