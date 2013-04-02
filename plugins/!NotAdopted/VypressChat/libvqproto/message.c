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
 * $Id: message.c,v 1.18 2005/03/08 16:53:22 bobas Exp $
 */

#include <stdarg.h>
#include <sys/types.h>

#include "vqproto.h"
#include "link.h"
#include "message.h"

#ifdef _WIN32
typedef unsigned int uint32_t;
#endif

/* static data
 */

/* static routines
 */
static char * 
vqp_msg_printf(size_t * p_res_sz, char * fmt, ...)
{
	char * buf, * str;
	size_t buf_sz, buf_alloc, len;
	va_list vargs;
	int ival;

#define BUF_ALLOC_CHUNK	256
#define BUF_ENSURE_FREE_SZ(size) \
	if(buf_sz + (size) > buf_alloc) {	\
		char * new_buf;			\
		buf_alloc = ((buf_sz + size + BUF_ALLOC_CHUNK - 1) / BUF_ALLOC_CHUNK)		\
									* BUF_ALLOC_CHUNK;	\
		new_buf = vqp_mmi_malloc(buf_alloc);						\
		if(!new_buf) {			\
			vqp_mmi_free(buf);	\
			va_end(vargs);		\
			return NULL;		\
		}				\
		memcpy(new_buf, buf, buf_sz);	\
		vqp_mmi_free(buf);		\
		buf = new_buf;			\
	}

	/* initialize vars */
	buf_sz = 0;
	buf_alloc = BUF_ALLOC_CHUNK;
	buf = vqp_mmi_malloc(buf_alloc);
	if(!buf) return NULL;

	/* append args */
	va_start(vargs, fmt);

	for(; *fmt; fmt ++) {
		if(*fmt == '%') {
			switch( *(++ fmt)) {
			case 'c':
				/* char */
				BUF_ENSURE_FREE_SZ(1);
				buf[buf_sz ++] = (char)va_arg(vargs, int);
				break;

			case 'l':
				/* long */
				BUF_ENSURE_FREE_SZ(sizeof(uint32_t));
				*((uint32_t *)(buf + buf_sz)) = (uint32_t)va_arg(vargs, int);
				buf_sz += sizeof(uint32_t);
				break;

			case 's': /* asciiz */
				len = strlen(str = va_arg(vargs, char *)) + 1;
				BUF_ENSURE_FREE_SZ(len);
				memcpy(buf + buf_sz, (str), len);
				buf_sz += len;
				break;

			case 'h':
				/* channel ('#" + asciiz) */
				len = strlen(str = va_arg(vargs, char *)) + 2;
				BUF_ENSURE_FREE_SZ(len);
				buf[buf_sz] = '#';
				memcpy(buf + buf_sz + 1, str, len - 1);
				buf_sz += len;
				break;

			case 'u':
				/* uuid */
				BUF_ENSURE_FREE_SZ(sizeof(vqp_uuid_t));
				memcpy(buf + buf_sz, (char*)va_arg(vargs, vqp_uuid_t *), sizeof(vqp_uuid_t));
				buf_sz += sizeof(vqp_uuid_t);
				break;

			case 'S': /* enum vqp_status */
				BUF_ENSURE_FREE_SZ(1);
				ival = va_arg(vargs, int);
				buf[buf_sz++] = ((enum vqp_status)ival == VQP_STATUS_NOTAVAILABLE)
					? '3'
					: ((enum vqp_status)ival == VQP_STATUS_AWAY)
						? '2'
						: ((enum vqp_status)ival == VQP_STATUS_DND)
							? '1': '0';
				break;

			case 'G': /* enum vqp_gender */
				BUF_ENSURE_FREE_SZ(1);
				buf[buf_sz ++] = ((enum vqp_gender)va_arg(vargs, int)
							== VQP_GENDER_MALE) ? '0': '1';
				break;

			case 'C': /* enum vqp_codepage */
				BUF_ENSURE_FREE_SZ(1);
				buf[buf_sz ++] = ((enum vqp_codepage)va_arg(vargs, int)
							== VQP_CODEPAGE_LOCALE) ? '0': '1';
				break;

			case 'A': /* enum vqp_active */
				BUF_ENSURE_FREE_SZ(1);
				buf[buf_sz ++] = ((enum vqp_active)va_arg(vargs, int)
							== VQP_ACTIVE_INACTIVE) ? '0': '1';
				break;
				
			case '0': /* '\0' */
				BUF_ENSURE_FREE_SZ(1);
				buf[buf_sz ++] = '\0';
				break;

			/* case '%': */
			default:
				/* '%' and the rest */
				BUF_ENSURE_FREE_SZ(1);
				buf[buf_sz ++] = *fmt;
				break;
			}
		} else {
			BUF_ENSURE_FREE_SZ(1);
			buf[buf_sz ++] = *fmt;
		}
	}

	va_end(vargs);

	*p_res_sz = buf_sz;
	return buf;
}

/* vqp_msg_scanf:
 *	reads message format (same as vqp_msg_printf)
 * returns:
 *	non-0 on failure,
 *	0, on success
 */
static int
vqp_msg_scanf(const struct vqp_message_struct * msg, char * fmt, ...)
{
	va_list vargs;
	char ** str_args[8], * str;
	const char * buf = msg->content, * scan;
	ssize_t buf_sz = msg->content_len;
	int n_str_args = 0, i;

	va_start(vargs, fmt);
	
	for(; *fmt; fmt++) {
		/* no more data */
		if(buf_sz <= 0)
			goto error_cleanup;

		if(*fmt == '%') {
			switch(* (++fmt)) {
			case 'c':
				*(va_arg(vargs, char *)) = *(buf++);
				buf_sz -= 1;
				break;

			case 'l':
				if(buf_sz < sizeof(uint32_t))
					goto error_cleanup;

				*(va_arg(vargs, uint32_t *)) = *(uint32_t *)buf;
				buf += sizeof(uint32_t);
				buf_sz -= sizeof(uint32_t);
				break;
				
			case 'u':
				if(buf_sz < sizeof(vqp_uuid_t))
					goto error_cleanup;

				memcpy((char*)va_arg(vargs, vqp_uuid_t *), buf, sizeof(vqp_uuid_t));
				buf += sizeof(vqp_uuid_t);
				buf_sz -= sizeof(vqp_uuid_t);
				break;
				
			case 's':
				/* get the length of this string */
				i = 0;
				for(scan = buf; *scan != '\0'; scan ++)
					if(++i == buf_sz) break;

				/* alloc & copy the string */
				str = vqp_mmi_malloc(i + 1);
				if(!str) goto error_cleanup;

				memcpy(str, buf, i);
				str[i] = '\0';
				buf += i + 1;
				buf_sz -= i + 1;

				/* store arg */
				str_args[n_str_args] = va_arg(vargs, char **);
				*str_args[n_str_args++] = str;
				break;
				
			case 'h':
				/* channel must begin with a '#' */
				if(*buf != '#')
					goto error_cleanup;

				/* get the length of channel name (minus the '#' char) */
				i = 0;
				for(scan = buf + 1; *scan != '\0'; scan ++)
					if(++i == buf_sz) break;

				/* the channel name must not be an empty string */
				if(!i) goto error_cleanup;

				/* alloc & copy the string */
				str = vqp_mmi_malloc(i + 1);
				if(!str) goto error_cleanup;

				/* copy the channel name and '\0' (minus the '#') */
				memcpy(str, buf + 1, i);
				str[i] = '\0';
				buf += i + 2;
				buf_sz -= i + 2;

				/* store arg */
				str_args[n_str_args] = va_arg(vargs, char **);
				*str_args[n_str_args++] = str;
				break;

			case 'S':
				*(va_arg(vargs, enum vqp_status *)) =
					(*buf == '3')
						? VQP_STATUS_NOTAVAILABLE
						: (*buf == '2')
							? VQP_STATUS_AWAY
							: (*buf == '1')
								? VQP_STATUS_DND
								: VQP_STATUS_AVAILABLE;
				buf ++;
				buf_sz --;
				break;

			case 'G':
				*(va_arg(vargs, enum vqp_gender *)) =
					(*buf == '0') ? VQP_GENDER_MALE: VQP_GENDER_FEMALE;
				buf ++;
				buf_sz --;
				break;

			case 'C':
				*(va_arg(vargs, enum vqp_codepage *)) =
					(*buf == '0') ? VQP_CODEPAGE_LOCALE: VQP_CODEPAGE_UTF8;
				buf ++;
				buf_sz --;
				break;

			case 'A':
				*(va_arg(vargs, enum vqp_active *)) =
					(*buf == '0') ? VQP_ACTIVE_INACTIVE: VQP_ACTIVE_ACTIVE;
				buf ++;
				buf_sz --;
				break;

			case '0':
			default:
				if(*buf != (*fmt == '0' ? '\0': *fmt))
					goto error_cleanup;
				buf ++;
				buf_sz --;
				break;
			}
		} else if(*fmt == '\0') {
			/* ignore the rest of buf string */
			goto success;
		} else {
			if(*fmt != *buf)
				goto error_cleanup;
			buf ++;
			buf_sz --;
		}
	}

success:
	/* everything was parsed ok */
	return 0;

error_cleanup:
	/* free the strings we've alloced so far
	 * and nullify their argument pointers
	 */
	for(i = 0; i < n_str_args; i++) {
		free(*str_args[i]);
		*str_args[i] = NULL;
	}

	va_end(vargs);
	return 1;
}

/* exported routines
 */

const char *
vqp_msg_signature(const vqp_msg_t msg)
{
	return P_VQP_MESSAGE_STRUCT(msg)->sig;
}

vqp_link_t
vqp_msg_link(const vqp_msg_t msg)
{
	return (vqp_link_t)P_VQP_MESSAGE_STRUCT(msg)->link;
}

vqp_addr_t
vqp_msg_src_addr(const vqp_msg_t msg)
{
	return P_VQP_MESSAGE_STRUCT(msg)->src_addr;
}

/* vqp_msg_parse:
 *	parses the specified message and invokes the corresponding
 *	callback for the message type specified in `parse_func' struct
 * returns:
 *	non-0 on error
 *	0 on success
 */
void vqp_msg_parse(
	const struct vqp_parse_func_struct * parsers,
	const vqp_msg_t vqmsg,
	void * user_data)
{
	const struct vqp_message_struct * msg = P_VQP_MESSAGE_STRUCT(vqmsg);
	char * s_src = NULL, * s_dst = NULL, * s_text = NULL,
	     * s_timestamp = NULL, * s_floodsecs = NULL, * s_password = NULL,
	     * s_cmdline = NULL, * s_channel = NULL, * s_packetsig = NULL,
	     * s_workgroup = NULL;
	enum vqp_codepage codepage;
	enum vqp_status status;
	enum vqp_gender gender;
	enum vqp_active active;
	vqp_uuid_t uuid;
	unsigned int i_swversion, i_pref_color, i_reserved;

	if(msg->content_len < 2)
		return;

	switch(msg->content[0])
	{
	case '0': /* refresh req */
		if(!parsers->func_refresh_req)
			break;

		if(!vqp_msg_scanf(msg, "0%s%C", &s_src, &codepage)) {
			parsers->func_refresh_req(vqmsg, user_data, s_src, codepage);
		}
		else if(!vqp_msg_scanf(msg, "0%s", &s_src)) {
			parsers->func_refresh_req(vqmsg, user_data, s_src, VQP_CODEPAGE_LOCALE);
		}
		break;
		
	case '1': /* refresh ack */
		if(!parsers->func_refresh_ack)
			break;

		/* try to parse as a vypresschat 1.9+ version of ack */
		if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(
				msg, "1%s%s%S%A%0" "%l%G%u" "%0%l%C%l",
				&s_dst, &s_src, &status, &active,
				&i_swversion, &gender, &uuid,
				&i_reserved, &codepage, &i_pref_color))
		{
			parsers->func_refresh_ack(
				vqmsg, user_data,
				s_src, s_dst, status, active, gender, i_swversion, &uuid,
				codepage, i_pref_color);
		}
		else if(!vqp_msg_scanf(msg, "1%s%s%S%A", &s_dst, &s_src, &status, &active))
		{
			vqp_uuid_t empty_uuid;
			vqp_uuid_create_nil(&empty_uuid);

			parsers->func_refresh_ack(
				vqmsg, user_data,
				s_src, s_dst, status, active,
				VQP_COMPAT_GENDER, VQP_COMPAT_SWVERSION, &empty_uuid,
				VQP_COMPAT_CODEPAGE, VQP_COMPAT_PREF_COLOR);
		}
		else if(msg->link->protocol == VQP_PROTOCOL_QUICKCHAT
			&& !vqp_msg_scanf(msg, "1%s%s%S", &s_dst, &s_src, &status))
		{
			vqp_uuid_t empty_uuid;
			vqp_uuid_create_nil(&empty_uuid);

			parsers->func_refresh_ack(
				vqmsg, user_data,
				s_src, s_dst, status,
				VQP_COMPAT_ACTIVE, VQP_COMPAT_GENDER, VQP_COMPAT_SWVERSION,
				&empty_uuid, VQP_COMPAT_CODEPAGE, VQP_COMPAT_PREF_COLOR);
		}

		break;
	
	case 'P': /* ping / pong */
		if(msg->link->protocol != VQP_PROTOCOL_VYPRESSCHAT)
			break;
		
		if(!vqp_msg_scanf(msg, "P0%s%s%s", &s_dst, &s_src, &s_timestamp)) {
			if(parsers->func_ping)
				parsers->func_ping(vqmsg, user_data, s_src, s_dst, s_timestamp);
		}
		else if(!vqp_msg_scanf(msg, "P1%s%s%s", &s_dst, &s_src, &s_timestamp)) {
			if(parsers->func_ping)
				parsers->func_ping(vqmsg, user_data, s_src, s_dst, s_timestamp);
		}

		break;

	case 'Z': /* flood notification */
		if(msg->link->protocol != VQP_PROTOCOL_VYPRESSCHAT
				|| !parsers->func_flood_notification)
			break;

		if(!vqp_msg_scanf(msg, "Z%s%s%s", &s_dst, &s_src, &s_floodsecs)) {
			parsers->func_flood_notification(
				vqmsg, user_data, s_src, s_dst, s_floodsecs);
		}

		break;

	case '8': /* remote execution */
		if(!parsers->func_remote_exec)
			break;

		if(!vqp_msg_scanf(msg, "8%s%s%s%s", &s_src, &s_dst, &s_cmdline, s_password)) {
			parsers->func_remote_exec(
				vqmsg, user_data,
				s_src, s_dst, s_cmdline, s_password);
		}
		
		break;

	case '9': /* remote exec ack */
		if(!parsers->func_remote_exec_ack)
			break;

		if(!vqp_msg_scanf(msg, "9%s%s%s", &s_dst, &s_src, &s_text)) {
			parsers->func_remote_exec_ack(
				vqmsg, user_data,
				s_src, s_dst, s_text);
		}
		break;

	case '3': /* nick change */
		if(!parsers->func_nick_change)
			break;

		if(!vqp_msg_scanf(msg, "3%s%s%G", &s_src, &s_text, &gender)) {
			parsers->func_nick_change(vqmsg, user_data, s_src, s_text, gender);
		}
		else if(!vqp_msg_scanf(msg, "3%s%s", &s_src, &s_text)) {
			parsers->func_nick_change(
				vqmsg, user_data, s_src, s_text, VQP_COMPAT_GENDER);
		}
		
		break;

	case 'D': /* status change */
		if(!parsers->func_status_change)
			break;

		if(!vqp_msg_scanf(msg, "D%s%S%G%s", &s_src, &status, &gender, &s_text)) {
			parsers->func_status_change(
				vqmsg, user_data, s_src, status, gender, s_text);
		}

		break;

	case 'M': /* active change */
		if(!parsers->func_active_change)
			break;

		if(!vqp_msg_scanf(msg, "M%s%A", &s_src, &active)) {
			parsers->func_active_change(vqmsg, user_data, s_src, active);
		}

		break;

	case '4': /* channel join */
		if(!parsers->func_channel_join)
			break;
		
		if(msg->link->protocol==VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(
				msg, "4%s#" VQP_MAIN_CHANNEL "%0%S%G%0%l%u" "%0%C%l%0",
				&s_src, &status, &gender, &i_swversion, &uuid,
				&codepage, &i_pref_color))
		{
			parsers->func_channel_join(
				vqmsg, user_data,
				VQP_MAIN_CHANNEL, s_src, status, gender, i_swversion, &uuid,
				codepage, i_pref_color);
		}
		else if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(
				msg, "4%s%h%S%G%0%C%0",
				&s_src, &s_channel, &status, &gender, &codepage))
		{
			vqp_uuid_t nil_uuid;
			vqp_uuid_create_nil(&nil_uuid);

			parsers->func_channel_join(
				vqmsg, user_data,
				s_channel, s_src, status, gender, VQP_COMPAT_SWVERSION, &nil_uuid,
				codepage, VQP_COMPAT_PREF_COLOR);
		}
		else if(!vqp_msg_scanf(msg, "4%s%h%S%G", &s_src, &s_channel, &status, &gender)) {
			vqp_uuid_t nil_uuid;
			vqp_uuid_create_nil(&nil_uuid);

			parsers->func_channel_join(
				vqmsg, user_data,
				s_channel, s_src, status, gender, VQP_COMPAT_SWVERSION,
				&nil_uuid, VQP_COMPAT_CODEPAGE, VQP_COMPAT_PREF_COLOR);
		}

		break;

	case '5': /* channel leave */
		if(!parsers->func_channel_leave)
			break;

		if(!vqp_msg_scanf(msg, "5%s%h%G", &s_src, &s_channel, &gender)) {
			parsers->func_channel_leave(vqmsg, user_data, s_channel, s_src, gender);
		}

		break;

	case '2': /* channel text */
		if(!parsers->func_channel_text)
			break;

		if(!vqp_msg_scanf(msg, "2%h%s%s", &s_channel, &s_src, &s_text)) {
			parsers->func_channel_text(
				vqmsg, user_data, s_channel, s_src, s_text, 0);
		}

		break;

	case 'A': /* channel /me text */
		if(!parsers->func_channel_text)
			break;

		if(!vqp_msg_scanf(msg, "A%h%s%s", &s_channel, &s_src, &s_text)) {
			parsers->func_channel_text(
				vqmsg, user_data, s_channel, s_src, s_text, 1);
		}

		break;
		
	case 'I': /* channel sound request */
		if(!parsers->func_channel_sound)
			break;

		if(!vqp_msg_scanf(msg, "I%s%s%h", &s_src, &s_text, &s_channel)) {
			parsers->func_channel_sound(
				vqmsg, user_data, s_channel, s_src, s_text);
		}

		break;

	case 'B': /* topic change */
		if(!parsers->func_channel_topic_change)
			break;

		if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(msg, "B%h%s", &s_channel, &s_text))
		{
			parsers->func_channel_topic_change(
				vqmsg, user_data, s_channel, s_text);
		}
		else if(msg->link->protocol == VQP_PROTOCOL_QUICKCHAT
			&& !vqp_msg_scanf(msg, "B%s", &s_text))
		{
			parsers->func_channel_topic_change(
				vqmsg, user_data, VQP_MAIN_CHANNEL, s_text);
		}

		break;

	case 'C': /* current topic */
		if(!parsers->func_channel_current_topic)
			break;

		if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(msg, "C%s%h%s", &s_dst, &s_channel, &s_text))
		{
			parsers->func_channel_current_topic(
				vqmsg, user_data, s_dst, s_channel, s_text);
		}
		else if(msg->link->protocol == VQP_PROTOCOL_QUICKCHAT
			&& !vqp_msg_scanf(msg, "C%s%s", &s_dst, &s_text))
		{
			parsers->func_channel_current_topic(
				vqmsg, user_data, s_dst, VQP_MAIN_CHANNEL, s_text);
		}

		break;

	case 'L': /* whois is on this channel req */
		if(!parsers->func_channel_whohere_req)
			break;

		if(!vqp_msg_scanf(msg, "L%s%h", &s_src, &s_channel)) {
			parsers->func_channel_whohere_req(
				vqmsg, user_data, s_channel, s_src);
		}

		break;

	case 'K': /* whois is on this channel ack */
		if(!parsers->func_channel_whohere_ack)
			break;

		if(!vqp_msg_scanf(msg, "K%s%h%s%A", &s_dst, &s_channel, &s_src, &active)) {
			parsers->func_channel_whohere_ack(
					vqmsg, user_data, s_channel, s_src, s_dst, active);
		}

		break;
	
	case 'N': /* channel list req */
		if(!parsers->func_channel_list_req)
			break;

		if(!vqp_msg_scanf(msg, "N%s", &s_src)) {
			parsers->func_channel_list_req(vqmsg, user_data, s_src);
		}

		break;

	case 'O': /* channel list ack */
		if(!parsers->func_channel_list_ack)
			break;

		if(!vqp_msg_scanf(msg, "O%s%s", &s_dst, &s_text)) {
			parsers->func_channel_list_ack(vqmsg, user_data, s_dst, s_text);
		}

		break;

	case '6': /* message */
		if(!parsers->func_message)
			break;

		if(!vqp_msg_scanf(msg, "6%s%s%s", &s_src, &s_dst, &s_text)) {
			parsers->func_message(vqmsg, user_data, s_src, s_dst, s_text, 0);
		}

		break;

	case 'E': /* multiaddress message */
		if(!parsers->func_message)
			break;

		if(!vqp_msg_scanf(msg, "E%s%s%s", &s_src, &s_dst, &s_text)) {
			parsers->func_message(vqmsg, user_data, s_src, s_dst, s_text, 1);
		}

		break;

	case '7': /* message ack */
		if(!parsers->func_message_ack)
			break;

		if(msg->link->protocol==VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(
				msg, "7%S%s%s%G%s%s",
				&status, &s_dst, &s_src, &gender, &s_text, &s_packetsig))
		{
			parsers->func_message_ack(
				vqmsg, user_data,
				s_src, s_dst, status, gender, s_text, s_packetsig);
		}
		else if(!vqp_msg_scanf(
				msg, "7%S%s%s%G%s",
				&status, &s_dst, &s_src, &gender, &s_text))
		{
			parsers->func_message_ack(
				vqmsg, user_data,
				s_src, s_dst, status, gender, s_text, VQP_COMPAT_PACKET_SIG);
		}

		break;

	case 'F': /* info req */
		if(!parsers->func_info_req)
			break;

		if(!vqp_msg_scanf(msg, "F%s%s", &s_dst, &s_src)) {
			parsers->func_info_req(vqmsg, user_data, s_src, s_dst);
		}

		break;

	case 'G': /* info ack */
		if(!parsers->func_info_ack)
			break;

		if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(msg, "G%s%s%s%s" "%s%s%s" "%s%s%s",
				&s_dst, &s_src, &s_text, &s_packetsig,
				&s_timestamp, &s_channel, &s_password,
				&s_workgroup, &s_cmdline, &s_floodsecs))
		{
			parsers->func_info_ack(
				vqmsg, user_data,
				s_src, s_dst, s_text, s_packetsig,
				s_timestamp, s_channel, s_password, s_workgroup,
				s_cmdline, s_floodsecs);
		}
		else if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
			&& !vqp_msg_scanf(msg, "G%s%s%s%s" "%s%s%s",
				&s_dst, &s_src, &s_text, &s_packetsig,
				&s_timestamp, &s_channel, &s_cmdline))
		{
			parsers->func_info_ack(
				vqmsg, user_data,
				s_src, s_dst, s_text, s_packetsig,
				s_timestamp, s_channel, s_cmdline, VQP_COMPAT_NETGROUP,
				VQP_COMPAT_PLATFORM, VQP_COMPAT_SWNAME_VC
			);
		} 
		else if(msg->link->protocol == VQP_PROTOCOL_QUICKCHAT
			&& !vqp_msg_scanf(msg, "G%s%s%s%s" "%s%s%s%s",
				&s_dst, &s_src, &s_text, &s_packetsig,
				&s_timestamp, &s_password, &s_channel, &s_cmdline))
		{
			char * address_str = vqp_mmi_malloc(64);

			if(address_str) {
				if(msg->link->connection == VQP_PROTOCOL_CONN_UDP) {
					sprintf(address_str, "%u.%u.%u.%u:%u",
						(unsigned)((msg->src_addr.node.ip >> 24) & 0xff),
						(unsigned)((msg->src_addr.node.ip >> 16) & 0xff),
						(unsigned)((msg->src_addr.node.ip >> 8) & 0xff),
						(unsigned)(msg->src_addr.node.ip & 0xff),
						(unsigned)msg->link->port
					);
				} else {
					sprintf(address_str, "%x-%x-%x-%x-%x-%x:%u",
						(unsigned)(msg->src_addr.node.ipx[0]),
						(unsigned)(msg->src_addr.node.ipx[1]),
						(unsigned)(msg->src_addr.node.ipx[2]),
						(unsigned)(msg->src_addr.node.ipx[3]),
						(unsigned)(msg->src_addr.node.ipx[4]),
						(unsigned)(msg->src_addr.node.ipx[5]),
						(unsigned)msg->link->port
					);
				}
				
				parsers->func_info_ack(
					vqmsg, user_data,
					s_src, s_dst, s_text, s_packetsig,
					address_str, s_channel, s_cmdline, VQP_COMPAT_NETGROUP,
					VQP_COMPAT_PLATFORM, VQP_COMPAT_SWNAME_QC
				);
				vqp_mmi_free(address_str);
			}
		}

		break;
	
	case 'H': /* beep / beep-ack */
		if(msg->content[1] == '0' && parsers->func_beep_signal) {
			if(!vqp_msg_scanf(msg, "H0%s%s", &s_dst, &s_src)) {
				parsers->func_beep_signal(vqmsg, user_data, s_src, s_dst);
			}
		}
		else if(msg->content[1] == '1' && parsers->func_beep_ack) {
			if(!vqp_msg_scanf(msg, "H1%s%s%G", &s_dst, &s_src, &gender)) {
				parsers->func_beep_ack(vqmsg, user_data, s_src, s_dst, gender);
			}
		}

		break;

	case 'J': /* private open/close/text */
		if(msg->content[1] == '0' && parsers->func_private_open) {
			if(!vqp_msg_scanf(msg, "J0%s%s%G", &s_src, &s_dst, &gender)) {
				parsers->func_private_open(vqmsg, user_data, s_src, s_dst, gender);
			}
		}
		else if(msg->content[1] == '1' && parsers->func_private_close) {
			if(!vqp_msg_scanf(msg, "J1%s%s%G", &s_src, &s_dst, &gender)) {
				parsers->func_private_close(vqmsg, user_data, s_src, s_dst, gender);
			}
		}
		else if(msg->content[1] == '2' && parsers->func_private_text) {
			if(!vqp_msg_scanf(msg, "J2%s%s%s", &s_src, &s_dst, &s_text)) {
				parsers->func_private_text(
					vqmsg, user_data, s_src, s_dst, s_text, 0);
			}
		}
		else if(msg->content[1] == '3' && parsers->func_private_text) {
			if(!vqp_msg_scanf(msg, "J3%s%s%s", &s_src, &s_dst, &s_text)) {
				parsers->func_private_text(
					vqmsg, user_data, s_src, s_dst, s_text, 1);
			}
		}

		break;

	default:
		/* unknown message */
		break;
	}

	/* free strings we've parsed in */
	if(s_src) vqp_mmi_free(s_src);
	if(s_dst) vqp_mmi_free(s_dst);
	if(s_text) vqp_mmi_free(s_text);
	if(s_timestamp) vqp_mmi_free(s_timestamp);
	if(s_floodsecs) vqp_mmi_free(s_floodsecs);
	if(s_password) vqp_mmi_free(s_password);
	if(s_cmdline) vqp_mmi_free(s_cmdline);
	if(s_channel) vqp_mmi_free(s_channel);
	if(s_packetsig) vqp_mmi_free(s_packetsig);
	if(s_workgroup) vqp_mmi_free(s_workgroup);
}
	
void vqp_msg_free(vqp_msg_t msg)
{
	if(P_VQP_MESSAGE_STRUCT(msg)->content)
		vqp_mmi_free(P_VQP_MESSAGE_STRUCT(msg)->content);

	vqp_mmi_free(msg);
}

void vqp_msg_set_dst_addr(vqp_msg_t msg, vqp_addr_t dst_addr)
{
	P_VQP_MESSAGE_STRUCT(msg)->dst_addr = dst_addr;
}

/* message contructor functions
 */

#define NEWMSG_INIT(vqlink) \
	struct vqp_message_struct * msg; \
	msg = vqp_mmi_malloc(sizeof(struct vqp_message_struct)); \
	if(!msg) return NULL;	\
	msg->link = (vqlink);	\
	vqp_addr_nil(vqlink, &msg->dst_addr);

#define NEWMSG_PRINTF(fmt, ...) \
	msg->content = vqp_msg_printf(&msg->content_len, fmt, __VA_ARGS__);	\
	if(!msg->content) { vqp_mmi_free(msg); return NULL; }

#define NEWMSG_RETURN() \
	return (vqp_msg_t)msg;

/* user detection messages
 */
vqp_msg_t vqp_msg_refresh_req(
	vqp_link_t link,
	const char * src, enum vqp_codepage src_codepage)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("0%s%C%0", src, src_codepage);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_refresh_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	enum vqp_status src_status, enum vqp_active src_active, enum vqp_gender src_gender,
	unsigned int src_swversion, const vqp_uuid_t * src_uuid,
	enum vqp_codepage src_codepage, unsigned int src_pref_color)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF(
		"1%s%s%S%A%0" "%l%G%u%0%l" "%C%l%0",
		dst, src, src_status, src_active,
		(unsigned int)src_swversion, src_gender, src_uuid, (unsigned int)0,
		src_codepage, (unsigned int)src_pref_color);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_ping(
	vqp_link_t link,
	const char * src, const char * dst, const char * time_stamp)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("P0%s%s%s", dst, src, time_stamp);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_pong(
	vqp_link_t link,
	const char * src, const char * dst, const char * orig_time_stamp)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("P1%s%s%s", dst, src, orig_time_stamp);
	NEWMSG_RETURN();
}

/* flood notification */
vqp_msg_t vqp_msg_flood_notification(
	vqp_link_t link,
	const char * src, const char * dst, const char * secs_blocked)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("Z%s%s%s", dst, src, secs_blocked);
	NEWMSG_RETURN();
}

/* remote execution */
vqp_msg_t vqp_msg_remote_exec(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * cmdline, const char * password)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("8%s%s%s%s", dst, src, cmdline, password);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_remote_exec_ack(
	vqp_link_t link,
	const char * src, const char * dst, const char * exec_text)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("9%s%s%s", dst, src, exec_text);
	NEWMSG_RETURN();
}

/* user status */
vqp_msg_t vqp_msg_nick_change(
	vqp_link_t link,
	const char * src, const char * src_new_nick, enum vqp_gender src_gender)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("3%s%s%G", src, src_new_nick, src_gender);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_status_change(
	vqp_link_t link,
	const char * src, enum vqp_status src_status,
	enum vqp_gender src_gender, const char * src_autoanswer)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("D%s%S%G%s", src, src_status, src_gender, src_autoanswer);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_active_change(
	vqp_link_t link,
	const char * src, enum vqp_active src_is_active)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("M%s%A", src, src_is_active);
	NEWMSG_RETURN();
}

/* channels */
vqp_msg_t vqp_msg_channel_join(
	vqp_link_t link,
	const char * channel, const char * src,
	enum vqp_status src_status, enum vqp_gender src_gender,
	unsigned int src_swversion, const vqp_uuid_t * src_uuid,
	enum vqp_codepage src_codepage, unsigned int src_pref_color)
{
	NEWMSG_INIT(link);
	
	if(!strcmp(channel, VQP_MAIN_CHANNEL)) {
		NEWMSG_PRINTF(
			"4%s%h%S%G%0" "%l%u%0%C%l%0%0",
			src, channel, src_status, src_gender,
			src_swversion, src_uuid, src_codepage, src_pref_color);
	} else {
		NEWMSG_PRINTF(
			"4%s%h%S%G%C%0",
			src, channel, src_status, src_gender, src_codepage);
	}

	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_leave(
	vqp_link_t link,
	const char * channel, const char * src,
	enum vqp_gender src_gender)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("5%s%h%G", src, channel, src_gender);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_text(
	vqp_link_t link,
	const char * channel, const char * src,
	const char * text, int is_action_text)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF(is_action_text ? "A%h%s%s": "2%h%s%s", channel, src, text);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_sound(
	vqp_link_t link,
	const char * channel, const char * src,
	const char * sound_filename)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("I%s%s%h", src, sound_filename, channel);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_topic_change(
	vqp_link_t link,
	const char * channel, const char * topic_text)
{
	NEWMSG_INIT(link);
	if(P_VQP_LINK_STRUCT(link)->protocol == VQP_PROTOCOL_VYPRESSCHAT) {
		NEWMSG_PRINTF("B%h%s", channel, topic_text);
	} else {
		NEWMSG_PRINTF("B%s", topic_text);
	}
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_current_topic(
	vqp_link_t link,
	const char * channel, const char * dst, const char * topic_text)
{
	NEWMSG_INIT(link);
	if(P_VQP_LINK_STRUCT(link)->protocol == VQP_PROTOCOL_VYPRESSCHAT) {
		NEWMSG_PRINTF("C%s%h%s", dst, channel, topic_text);
	} else {
		NEWMSG_PRINTF("C%s%s", dst, topic_text);
	}
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_whohere_req(
	vqp_link_t link,
	const char * channel, const char * src)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("L%s%h", src, channel);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_whohere_ack(
	vqp_link_t link,
	const char * channel, const char * src, const char * dst,
	enum vqp_active src_is_active)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("K%s%h%s%A", dst, channel, src, src_is_active);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_list_req(
	vqp_link_t link,
	const char * src)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("N%s", src);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_channel_list_ack(
	vqp_link_t link,
	const char * dst, const char * channel_list)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("O%s%s", dst, channel_list);
	NEWMSG_RETURN();
}

/* message delivery/ack */
vqp_msg_t vqp_msg_message(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * text, int is_multiaddress_msg)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF(is_multiaddress_msg ? "E%s%s%s": "6%s%s%s", src, dst, text);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_message_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	enum vqp_status src_status, enum vqp_gender src_gender,
	const char * src_autoanswer, const char * orig_packetsig)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("7%S%s%s%G%s%s",
		src_status, dst, src, src_gender, src_autoanswer, orig_packetsig);
	NEWMSG_RETURN();
}

/* info req-ack */
vqp_msg_t vqp_msg_info_req(
	vqp_link_t link,
	const char * src, const char * dst)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("F%s%s", dst, src);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_info_ack(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * src_computer_name, const char * src_user_name,
	const char * src_ip_address, const char * src_channel_list,
	const char * src_autoanswer, const char * src_workgroup,
	const char * src_platform, const char * src_software)
{
	NEWMSG_INIT(link);

	if(P_VQP_LINK_STRUCT(link)->protocol == VQP_PROTOCOL_VYPRESSCHAT) {
		NEWMSG_PRINTF("G%s%s%s%s" "%s%s%s" "%s%s%s%0",
			dst, src, src_computer_name, src_user_name,
			src_ip_address, src_channel_list, src_autoanswer,
			src_workgroup, src_platform, src_software);
	} else {
		NEWMSG_PRINTF("G%s%s%s%s0 %%%00 Kb%0%s%s",
			dst, src, src_computer_name, src_user_name,
			src_channel_list, src_autoanswer);
	}
	NEWMSG_RETURN();
}

/* beep's */
vqp_msg_t vqp_msg_beep_signal(
	vqp_link_t link,
	const char * src, const char * dst)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("H0%s%s", dst, src);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_beep_ack(
	vqp_link_t link,
	const char * src, const char * dst, enum vqp_gender src_gender)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("H1%s%s%G", dst, src, src_gender);
	NEWMSG_RETURN();
}

/* privates */
vqp_msg_t vqp_msg_private_open(
	vqp_link_t link,
	const char * src, const char * dst, enum vqp_gender src_gender)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("J0%s%s%G", src, dst, src_gender);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_private_close(
	vqp_link_t link,
	const char * src, const char * dst, enum vqp_gender src_gender)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF("J1%s%s%G", src, dst, src_gender);
	NEWMSG_RETURN();
}

vqp_msg_t vqp_msg_private_text(
	vqp_link_t link,
	const char * src, const char * dst,
	const char * text, int is_action_text)
{
	NEWMSG_INIT(link);
	NEWMSG_PRINTF(
		is_action_text ? "J3%s%s%s": "J2%s%s%s",
		src, dst, text);
	NEWMSG_RETURN();
}

