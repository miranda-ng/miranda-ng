/*
 * libvqproto: Vypress/QChat protocol interface library
 * (c) Saulius Menkevicius 2005
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: vqproto.h,v 1.14 2005/03/17 11:02:44 bobas Exp $
 */

#ifndef __VQPROTO_H
#define __VQPROTO_H

/* required headers */
#include <sys/types.h>

/* notes
 *-----------------------------------

I. All the functions (except some of them) return 0 on success, and non-0
   as unix-style error constant (ENOMEM, EACCESS, etc).

*/

/* API constants & macros
 *-----------------------------------*/
#define VQP_MAX_PACKET_SIZE	0x2000
#define VQP_MAIN_CHANNEL	"Main"
#define VQP_MAKE_SWVERSION(hi, lo) \
		((((hi) & 0xffff) << 16) | ((lo) & 0xffff))

/* type definitions
 *-----------------------------------*/
typedef void * vqp_link_t;
typedef void * vqp_msg_t;
typedef void * vqp_dconn_t;

typedef struct vqp_uuid_struct {
	unsigned long data1;
	unsigned short data2, data3;
	unsigned char data4[8];
} vqp_uuid_t;

enum vqp_protocol_type {
	VQP_PROTOCOL_QUICKCHAT,
	VQP_PROTOCOL_VYPRESSCHAT
}
;
enum vqp_protocol_options {
	VQP_PROTOCOL_OPT_MULTICAST	= 1,
	VQP_PROTOCOL_OPT_FILETRANSFER	= 2
};

enum vqp_protocol_connection {
	VQP_PROTOCOL_CONN_UDP,
	VQP_PROTOCOL_CONN_IPX
};

enum vqp_status {
	VQP_STATUS_AVAILABLE,
	VQP_STATUS_AWAY,
	VQP_STATUS_DND,
	VQP_STATUS_NOTAVAILABLE
};

enum vqp_active {
	VQP_ACTIVE_ACTIVE,
	VQP_ACTIVE_INACTIVE
};

enum vqp_gender {
	VQP_GENDER_MALE,
	VQP_GENDER_FEMALE
};

enum vqp_codepage {
	VQP_CODEPAGE_LOCALE,
	VQP_CODEPAGE_UTF8
};

typedef struct vqp_addr_struct {
	enum vqp_protocol_connection conn;

	union vqp_addr_node_union {
		unsigned long ip;
		unsigned char ipx[6];
	} node;
} vqp_addr_t;

struct vqp_parse_func_struct {
	unsigned int struct_size;

	/* user detection */
	void (* func_refresh_req)(
		const vqp_msg_t msg, void * user_data,
		const char * src, enum vqp_codepage src_codepage);

	void (* func_refresh_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst,
		enum vqp_status src_mode, enum vqp_active src_active, enum vqp_gender src_gender,
		unsigned int src_swversion, const vqp_uuid_t * src_uuid,
		enum vqp_codepage src_codepage, unsigned int src_pref_color);

	void (* func_ping)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, const char * time_stamp);

	void (* func_pong)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, const char * orig_time_stamp);

	/* flood notification */
	void (* func_flood_notification)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, const char * secs_blocked);

	/* remote execution */
	void (* func_remote_exec)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst,
		const char * cmdline, const char * password);

	void (* func_remote_exec_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, const char * exec_text);

	/* user status */
	void (* func_nick_change)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * src_new_nick, enum vqp_gender src_gender);

	void (* func_status_change)(
		const vqp_msg_t msg, void * user_data,
		const char * src, enum vqp_status src_status,
		enum vqp_gender src_gender, const char * src_autoanswer);

	void (* func_active_change)(
		const vqp_msg_t msg, void * user_data,
		const char * src, enum vqp_active src_is_active);

	/* channels */
	void (* func_channel_join)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * src,
		enum vqp_status src_status, enum vqp_gender src_gender,
		unsigned int src_swversion, const vqp_uuid_t * src_uuid,
		enum vqp_codepage src_codepage, unsigned int src_pref_color);

	void (* func_channel_leave)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * src,
		enum vqp_gender src_gender);

	void (* func_channel_text)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * src,
		const char * text, int is_action_text);

	void (* func_channel_sound)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * src,
		const char * sound_filename);

	void (* func_channel_topic_change)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * topic_text);

	void (* func_channel_current_topic)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * dst,
		const char * topic_text);

	void (* func_channel_whohere_req)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * src);

	void (* func_channel_whohere_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * channel, const char * src, const char * dst,
		enum vqp_active src_is_active);

	void (* func_channel_list_req)(
		const vqp_msg_t msg, void * user_data,
		const char * src);

	void (* func_channel_list_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * dst, const char * channel_list);

	/* message delivery/ack */
	void (* func_message)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst,
		const char * text, int is_multiaddress_msg);

	void (* func_message_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst,
		enum vqp_status src_status, enum vqp_gender src_gender,
		const char * src_autoanswer, const char * orig_packetsig);

	/* info req-ack */
	void (* func_info_req)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst);

	void (* func_info_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst,
		const char * src_computer_name, const char * src_user_name,
		const char * src_ip_addresses, const char * src_channel_list,
		const char * src_autoanswer, const char * src_workgroup,
		const char * src_platform, const char * src_software);

	/* beep's */
	void (* func_beep_signal)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst);

	void (* func_beep_ack)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, enum vqp_gender src_gender);

	/* privates */
	void (* func_private_open)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, enum vqp_gender src_gender);

	void (* func_private_close)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst, enum vqp_gender src_gender);

	void (* func_private_text)(
		const vqp_msg_t msg, void * user_data,
		const char * src, const char * dst,
		const char * text, int is_action_text);
};

struct vqp_dconn_parse_funcs {
	unsigned int struct_size;

	/* user detection */
	void (* func_file_transfer) (
		const vqp_dconn_t dconn, void * user_data,
		size_t content_len, const char * file_name,
		const char * file_desc, int files_left, vqp_uuid_t * p_uuid);
	
	void (* func_file_transfer_ready) (
		const vqp_dconn_t dconn, void * user_data);
	
	void (* func_data)(
		const vqp_dconn_t dconn, void * user_data,
		void * data, size_t data_sz);
};

/* link functions
 *-----------------------------------*/

/* vqp_init:
 *	initializes the library.
 *
 *	you can override the default memory interface (malloc/free) with user-supplied
 *	routines by specifying mmi_malloc and mmi_free parameters
 */
void vqp_init(
	void * (* mmi_malloc)(size_t),
	void (* mmi_free)(void *));

/* vqp_uninit:
 *	deinitializes the library.
 *
 *	currently does nothing, but you should call it anyway
 */
void vqp_uninit();

/* vqp_link_open:
 *	allocates new link to the network
 *
 * returns:
 *	link identifier (vqp_link_t), or NULL on error
 *	unix error num in *p_error
 */
vqp_link_t vqp_link_open(
	enum vqp_protocol_type protocol,
	enum vqp_protocol_options options,
	enum vqp_protocol_connection connection,
	unsigned long local_address,		/* address to bind to (can be 0) (with IP only) */
	unsigned long * p_broadcast_addresses,	/* 0UL terminated list, (with IP only) */
	unsigned long multicast_address,	/* (with IP only) */
	unsigned short port,
	int * p_error);

/* vqp_link_close:
 *	closes the link
 */
int vqp_link_close(vqp_link_t link);

/* vqp_link_rx_socket:
 *	returns RX (receiving) socket of the link
 *	which can be used to select() the link message socket
 */
int vqp_link_rx_socket(vqp_link_t link);

/* vqp_link_file_rx:
 *	returns server socket of the link that you can select() on and accept()
 *	(this works with VQP_PROTOCOL_CONN_UDP only)
 */
int vqp_link_file_rx(vqp_link_t link);

/* vqp_link_protocol:
 *	returns links's protocol
 */
enum vqp_protocol_type vqp_link_protocol(vqp_link_t);

/* vqp_link_send:
 *	sends specified message
 */
int vqp_link_send(vqp_msg_t msg);

/* vqp_link_recv:
 *	receives message from the link
 *
 * returns:
 *	0, on success, *p_in_msg is set to new message
 *	unix-style errno (Exxx), on error
 */
int vqp_link_recv(vqp_link_t link, vqp_msg_t * p_in_msg);

/* address funcs
 *-----------------------------------*/
/* vqp_addr_nil:
 *	returns a 'nil' address for specified link
 */
void vqp_addr_nil(vqp_link_t link, vqp_addr_t * p_addr);

/* vqp_addr_is_nil:
 *	returns if address is a nil address
 */
int vqp_addr_is_nil(vqp_addr_t * p_addr);

/* uuid (unique user identifier) funcs
 *-----------------------------------*/

/* vqp_uuid_create:
 *	creates unique uuid,
 */
void vqp_uuid_create(vqp_uuid_t * uuid);

/* vqp_uuid_create_nil:
 *	creates a nil uuid,
 */
void vqp_uuid_create_nil(vqp_uuid_t * uuid);

/* vqp_uuid_is_nil:
 *	returns non-0, if the uuid is nil
 */
int vqp_uuid_is_nil(const vqp_uuid_t * uuid);

/* vqp_uuid_is_equal:
 *	returns non-0 if the uuids are equal
 */
int vqp_uuid_is_equal(const vqp_uuid_t * uuid_a, const vqp_uuid_t * uuid_b);

/* vqp_uuid_to_string:
 *	returns a malloc'ed string representation of the uuid
 */
char * vqp_uuid_to_string(const vqp_uuid_t * uuid);

/* vqp_uuid_from_string:
 *	tries to parse the uuid string and copy its contents
 *	to @uuid;
 * returns:
 *	0 on success,
 *	non-o on failure
 */
int vqp_uuid_from_string(vqp_uuid_t * uuid, const char * str);

/* message handling
 *-----------------------------------*/
const char * vqp_msg_signature(const vqp_msg_t msg);
vqp_link_t vqp_msg_link(const vqp_msg_t msg);
vqp_addr_t vqp_msg_src_addr(const vqp_msg_t msg);

void vqp_msg_parse(
	const struct vqp_parse_func_struct * parse_func,
	const vqp_msg_t msg,
	void * user_data);
	
void vqp_msg_free(vqp_msg_t msg);

void vqp_msg_set_dst_addr(vqp_msg_t msg, vqp_addr_t dst_addr);

/* user detection */
vqp_msg_t vqp_msg_refresh_req(
	vqp_link_t link,
	const char * src, enum vqp_codepage src_codepage);

vqp_msg_t vqp_msg_refresh_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	enum vqp_status src_mode, enum vqp_active src_active, enum vqp_gender src_gender,
	unsigned int src_swversion, const vqp_uuid_t * src_uuid,
	enum vqp_codepage src_codepage, unsigned int src_pref_color);

vqp_msg_t vqp_msg_ping(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * time_stamp);

vqp_msg_t vqp_msg_pong(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * orig_time_stamp);

/* flood notification */
vqp_msg_t vqp_msg_flood_notification(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * secs_blocked);

/* remote execution */
vqp_msg_t vqp_msg_remote_exec(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * cmdline, const char * password);

vqp_msg_t vqp_msg_remote_exec_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * exec_text);

/* user status */
vqp_msg_t vqp_msg_nick_change(
	vqp_link_t link,
	const char * src, const char * src_new_nick, enum vqp_gender src_gender);

vqp_msg_t vqp_msg_status_change(
	vqp_link_t link,
	const char * src, enum vqp_status src_status,
	enum vqp_gender src_gender, const char * src_autoanswer);

vqp_msg_t vqp_msg_active_change(
	vqp_link_t link,
	const char * src, enum vqp_active src_is_active);

/* channels */
vqp_msg_t vqp_msg_channel_join(
	vqp_link_t link,
	const char * src, const char * channel,
	enum vqp_status src_status, enum vqp_gender src_gender,
	unsigned int src_swversion, const vqp_uuid_t * src_uuid,
	enum vqp_codepage src_codepage, unsigned int src_pref_color);

vqp_msg_t vqp_msg_channel_leave(
	vqp_link_t link,
	const char * channel, const char * src,
	enum vqp_gender src_gender);

vqp_msg_t vqp_msg_channel_text(
	vqp_link_t link,
	const char * channel, const char * src,
	const char * text, int is_action_text);

vqp_msg_t vqp_msg_channel_sound(
	vqp_link_t link,
	const char * channel, const char * src,
	const char * sound_filename);

vqp_msg_t vqp_msg_channel_topic_change(
	vqp_link_t link,
	const char * channel, const char * topic_text);

vqp_msg_t vqp_msg_channel_current_topic(
	vqp_link_t link,
	const char * channel, const char * dst,
	const char * topic_text);

vqp_msg_t vqp_msg_channel_whohere_req(
	vqp_link_t link,
	const char * channel, const char * src);

vqp_msg_t vqp_msg_channel_whohere_ack(
	vqp_link_t link,
	const char * channel, const char * src, const char * dst,
	enum vqp_active src_is_active);

vqp_msg_t vqp_msg_channel_list_req(
	vqp_link_t link,
	const char * src);

vqp_msg_t vqp_msg_channel_list_ack(
	vqp_link_t link,
	const char * dst, const char * channel_list);

/* message delivery/ack */
vqp_msg_t vqp_msg_message(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * text, int is_multiaddress_msg);

vqp_msg_t vqp_msg_message_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	enum vqp_status src_status, enum vqp_gender src_gender,
	const char * src_autoanswer, const char * orig_packetsig);

/* info req-ack */
vqp_msg_t vqp_msg_info_req(
	vqp_link_t link,
	const char * src, const char * dst);

vqp_msg_t vqp_msg_info_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * src_computer_name, const char * src_user_name,
	const char * src_ip_addresses, const char * src_channel_list,
	const char * src_autoanswer, const char * src_workgroup,
	const char * src_platform, const char * src_software);

/* beep's */
vqp_msg_t vqp_msg_beep_signal(
	vqp_link_t link,
	const char * src, const char * dst);

vqp_msg_t vqp_msg_beep_ack(
	vqp_link_t link,
	const char * src, const char * dst, enum vqp_gender src_gender);

/* privates */
vqp_msg_t vqp_msg_private_open(
	vqp_link_t link,
	const char * src, const char * dst, enum vqp_gender src_gender);

vqp_msg_t vqp_msg_private_close(
	vqp_link_t link,
	const char * src, const char * dst, enum vqp_gender src_gender);

vqp_msg_t vqp_msg_private_text(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * text, int is_action_text);

/* direct connection message funcs
 * (used for file transfers and private messages on vypress chat 2.0)
 *--------------------------------------------------------------------*/

vqp_dconn_t vqp_dconn_accept(vqp_link_t link);

vqp_dconn_t vqp_dconn_open(vqp_link_t link, vqp_addr_t remote_addr);
void vqp_dconn_close(vqp_dconn_t dconn);

int vqp_dconn_socket(vqp_dconn_t dconn);

int vqp_dconn_read(
	vqp_dconn_t dconn, const struct vqp_dconn_parse_funcs * parse_funcs,
	void * user_data);

int vqp_dconn_send_file_transfer_request(
	vqp_dconn_t dconn, size_t content_len, const char * file_name,
	const char * file_desc, int files_left, const vqp_uuid_t * p_uuid);

int vqp_dconn_send_file_transfer_ready(vqp_dconn_t dconn);
int vqp_dconn_send_file_transfer_deny(vqp_dconn_t dconn);
int vqp_dconn_send_file_transfer_done(vqp_dconn_t dconn);

int vqp_dconn_send_chat_request(
	vqp_dconn_t dconn, const vqp_uuid_t * p_uuid, const char * nickname,
	const char * public_key);
int vqp_dconn_send_chat_request_ok(
	vqp_dconn_t dconn, const vqp_uuid_t * p_uuid, const char * nickname);
int vqp_dconn_send_chat_typing(
	vqp_dconn_t dconn, const vqp_uuid_t * p_uuid, const char * nickname,
	int is_typing);
int vqp_dconn_send_chat_msg(
	vqp_dconn_t dconn, const vqp_uuid_t * p_uuid, const char * nickname,
	size_t content_length);
int vqp_dconn_send_chat_fin(
	vqp_dconn_t dconn, const vqp_uuid_t * p_uuid, const char * nickname);

int vqp_dconn_send(vqp_dconn_t dconn, const void * data, size_t data_sz);

#endif	/* #ifdef __VQPROTO_H */

