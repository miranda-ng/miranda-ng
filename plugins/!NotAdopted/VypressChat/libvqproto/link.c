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
 * $Id: link.c,v 1.14 2005/03/08 17:21:23 bobas Exp $
 */

#include <time.h>
#include <stdlib.h>

#include "vqproto.h"
#include "link.h"
#include "message.h"

/* global data
 */
#ifndef MEMWATCH
void * (* vqp_mmi_malloc)(size_t) = NULL;
void (* vqp_mmi_free)(void *) = NULL;
#endif

/* static routines
 */
static __inline unsigned short
ushort2bcd(unsigned short ushort)
{
	return (ushort % 10) | (((ushort / 10) % 10) << 4)
		| (((ushort / 100) % 10) << 8) | (((ushort / 1000) % 10) << 12);
}

static void
vqp_make_msg_sig(struct vqp_message_struct * msg)
{
	static unsigned rand_num = 0;
	int i;

	/* hash message contents to get some initial random num */
	if(!rand_num) {
		for(i=0; i < msg->content_len; i++)
			rand_num += msg->content[i];
		rand_num %= 211;
	}

	/* make sure the rng is seeded correctly */
	srand((unsigned) time(NULL) + rand_num++);
	
	/* generate packet signature */
	for(i=0; i < VQP_LINK_SIG_LEN; i++)
		msg->sig[i] = (unsigned char)('0' + rand() % ('9' - '0' + 1));

	/* add '\0' at the end (not truly necessary, but it helps to have
	 * asciiz string instead of an unbounded char[]
	 */
	msg->sig[VQP_LINK_SIG_LEN] = '\0';
}

static int
vqp_is_seen_msg_sig(struct vqp_link_struct * link, char * sig)
{
	int i;
	for(i = 0; i < VQP_LINK_SEEN_SIGS_NUM; i++)
		if(!memcmp(link->seen_sigs[i], sig, VQP_LINK_SIG_LEN))
			return 1;
	return 0;
}

static void
vqp_add_seen_msg_sig(struct vqp_message_struct * msg)
{
	memcpy(msg->link->seen_sigs[msg->link->seen_sig_inc],
		msg->sig, VQP_LINK_SIG_LEN);
	msg->link->seen_sig_inc = (msg->link->seen_sig_inc + 1)
						% VQP_LINK_SEEN_SIGS_NUM;
}

static int
vqp_link_open_setup_udp_multicast(struct vqp_link_struct * link)
{
	struct ip_mreq mreq;
	const unsigned char opt_loop = 1;
	const unsigned char ttl = 32;

	/* set IP_MULTICAST_LOOP to 1, so our host receives
	 * the messages we send
	 */
	if(setsockopt(	link->tx_socket, IPPROTO_IP, IP_MULTICAST_LOOP,
			(void*)&opt_loop, sizeof(opt_loop)
		) != 0)
		return -1;
		
	/* set IP_MULTICAST_TTL to 32, that is the packets
	 * will go through 32 routers before getting scrapped
	 */
	if(setsockopt(	link->tx_socket, IPPROTO_IP, IP_MULTICAST_TTL,
			(void *)&ttl, sizeof(ttl)
		) != 0)
		return -1;


	/* set our group membership */
	mreq.imr_multiaddr.s_addr = htonl(link->conndata.udp.multicast_address);
	mreq.imr_interface.s_addr = INADDR_ANY;
	if(setsockopt(	link->rx_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(void *)&mreq, sizeof(mreq)
		) != 0)
		return -1;

	return 0;
}

static int
vqp_link_open_setup_tx_broadcast(struct vqp_link_struct * link)
{
	const int sock_opt = 1;
	return setsockopt(
			link->tx_socket, SOL_SOCKET, SO_BROADCAST,
			(void *)&sock_opt, sizeof(sock_opt));
}

static int
vqp_link_open_setup_udp(struct vqp_link_struct * link, int * p_error)
{
	int setup_result;
	const int sockopt_true = 1;
	struct sockaddr_in sin;

	/* setup tx socket */
	link->tx_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(link->tx_socket < 0) {
		if(p_error)
			*p_error = errno;
		return 1;
	}

	link->rx_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(link->rx_socket < 0) {
		if(p_error)
			*p_error = errno;

		closesocket(link->tx_socket);
		return 1;
	}
	
	/* on win32 we can setup the socket so we can
	 * use multiple clients on the same pc
	 */
#ifdef _WIN32
	setsockopt(
		link->rx_socket, SOL_SOCKET, SO_REUSEADDR,
		(void *)&sockopt_true, sizeof(sockopt_true));
#endif

	/* bind rx socket */
	sin.sin_family = PF_INET;
	sin.sin_addr.s_addr = htonl(link->conndata.udp.local_address
						? link->conndata.udp.local_address
						: INADDR_ANY);
	sin.sin_port = htons(link->port);
	if(bind(link->rx_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		if(p_error)
			*p_error = errno;

		closesocket(link->rx_socket);
		closesocket(link->tx_socket);
		return 1;
	}

	link->tx_socket = link->rx_socket;

	/* setup sockets for multicast or broadcast service
	 */
	setup_result = (link->options & VQP_PROTOCOL_OPT_MULTICAST)
		? vqp_link_open_setup_udp_multicast(link)
		: vqp_link_open_setup_tx_broadcast(link);

	if(setup_result < 0) {
		if(p_error)
			*p_error = errno;

		closesocket(link->rx_socket);
		closesocket(link->tx_socket);
		return 1;
	}

	/* success */
	return 0;
}

static int vqp_link_open_setup_ipx(struct vqp_link_struct * link, int * p_error)
{
	struct sockaddr_ipx six;

	/* setup tx socket */
	link->tx_socket = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if(link->tx_socket < 0) {
		if(p_error)
			*p_error = errno;
		return 1;
	}

	if(vqp_link_open_setup_tx_broadcast(link)) {
		if(p_error)
			*p_error = errno;
		
		closesocket(link->tx_socket);
		return 1;
	}

	/* setup rx socket */
	link->rx_socket = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if(link->rx_socket < 0) {
		if(p_error)
			*p_error = errno;
		closesocket(link->rx_socket);
		return 1;
	}

	/* bind rx socket */
	memset(&six, 0, sizeof(six));
	six.sa_family = AF_IPX;
	six.sa_socket = htons(ushort2bcd(link->port));

	if(bind(link->rx_socket, (struct sockaddr *)&six, sizeof(six))) {
		if(p_error)
			*p_error = errno;

		closesocket(link->rx_socket);
		closesocket(link->tx_socket);
		return 1;
	}

	/* save node and network number */
	memcpy(link->conndata.ipx.netnum, six.sa_netnum, 4);
	memcpy(link->conndata.ipx.nodenum, six.sa_nodenum, 6);

	/* success */
	return 0;
}

/* exported routines
 */
void vqp_init(
	void * (* mmi_malloc_func)(size_t),
	void (* mmi_free_func)(void *))
{
#ifndef MEMWATCH
	if(mmi_malloc_func)
		vqp_mmi_malloc = mmi_malloc_func;
	else	vqp_mmi_malloc = malloc;

	if(mmi_free_func)
		vqp_mmi_free = mmi_free_func;
	else	vqp_mmi_free = free;
#endif
}

void vqp_uninit()
{
}

vqp_link_t vqp_link_open(
	enum vqp_protocol_type protocol,
	enum vqp_protocol_options options,
	enum vqp_protocol_connection connection,
	unsigned long local_address,
	unsigned long * p_broadcast_addresses,	/* 0UL terminated list */
	unsigned long multicast_address,
	unsigned short port,
	int * p_error)
{
	struct vqp_link_struct * link;

	/* alloc and init link struct */
	link = vqp_mmi_malloc(sizeof(struct vqp_link_struct));
	if(!link) {
		if(p_error) *p_error = ENOMEM;
		return NULL;
	}

	link->rx_socket = 0;
	link->tx_socket = 0;
	link->protocol = protocol;
	link->options = options;
	link->connection = connection;
	link->port = port ? port: 8167;
	link->seen_sig_inc = 0;
	memset(link->seen_sigs, 0, VQP_LINK_SEEN_SIGS_NUM * VQP_LINK_SIG_LEN);

	if(connection == VQP_PROTOCOL_CONN_UDP) {
		/* UDP
		 */
		link->conndata.udp.local_address = local_address;
		link->conndata.udp.multicast_address = multicast_address;
		link->conndata.udp.p_broadcast_addresses = NULL;

		if(vqp_link_open_setup_udp(link, p_error)) {
			vqp_mmi_free(link);
			return NULL;
		}

		/* setup broadcast masks lists
		 */
		if(!(link->options & VQP_PROTOCOL_OPT_MULTICAST)) {
			/* standard broadcast
			 */
			int i;
			for(i = 0; p_broadcast_addresses[i]; i++) /* nothing */ ;

			if(!i) {
				/* no broadcast addresses defined:
				 * use 255.255.255.255 */
				link->conndata.udp.p_broadcast_addresses
						= vqp_mmi_malloc(sizeof(unsigned long) * 2);

				if(!link->conndata.udp.p_broadcast_addresses) {
					vqp_mmi_free(link);
					if(p_error)
						*p_error = ENOMEM;
					
					closesocket(link->rx_socket);
					closesocket(link->tx_socket);
					return NULL;
				}

				link->conndata.udp.p_broadcast_addresses[0] = 0xffffffffUL;
				link->conndata.udp.p_broadcast_addresses[1] = 0UL;
			} else {
				/* copy broadcast addresses */
				size_t listsz = sizeof(unsigned long) * (i + 1);
				link->conndata.udp.p_broadcast_addresses = vqp_mmi_malloc(listsz);

				if(link->conndata.udp.p_broadcast_addresses == NULL) {
					vqp_mmi_free(link);
					if(p_error)
						*p_error = ENOMEM;
					
					closesocket(link->rx_socket);
					closesocket(link->tx_socket);
					return NULL;
				}
				memcpy(link->conndata.udp.p_broadcast_addresses,
					p_broadcast_addresses, listsz);
			}
		}
	}
	else {
		/* IPX
		 */
		if(vqp_link_open_setup_ipx(link, p_error)) {
			vqp_mmi_free(link);
			return NULL;
		}
	}

	return (vqp_link_t)link;
}

int vqp_link_close(vqp_link_t vqlink)
{
	struct vqp_link_struct * link = P_VQP_LINK_STRUCT(vqlink);

	closesocket(link->tx_socket);
	closesocket(link->rx_socket);

	if(link->connection == VQP_PROTOCOL_CONN_UDP && link->conndata.udp.p_broadcast_addresses)
		vqp_mmi_free(link->conndata.udp.p_broadcast_addresses);
	
	vqp_mmi_free(link);

	return 0;
}

int vqp_link_rx_socket(vqp_link_t link)
{
	return P_VQP_LINK_STRUCT(link)->rx_socket;
}

enum vqp_protocol_type
vqp_link_protocol(vqp_link_t link)
{
	return P_VQP_LINK_STRUCT(link)->protocol;
}

int vqp_link_send(vqp_msg_t vqmsg)
{
	struct vqp_message_struct * msg = P_VQP_MESSAGE_STRUCT(vqmsg);
	char * packet;
	size_t packet_len;

	/* check that the message contains something */
	if(msg->content_len == 0)
		return EINVAL;

	/* check that we have the correct msg dst addr (if specified)
	 */

	if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT) {
		/* assign & register unique packet id for the message */
		vqp_make_msg_sig(msg);
		vqp_add_seen_msg_sig(msg);

		/* alloc real packet contents with signature */
		packet_len = 1 + VQP_LINK_SIG_LEN + msg->content_len;
		packet = vqp_mmi_malloc(packet_len);
		if(!packet)
			return ENOMEM;

		/* fill packet contents in */
		packet[0] = 'X';	/* vypress chat packet */
		memcpy(packet + 1, msg->sig, VQP_LINK_SIG_LEN);
		memcpy(packet + 1 + VQP_LINK_SIG_LEN,
					msg->content, msg->content_len);
	} else {
		/* there's no packet sig to add for quickchat packets */
		packet = msg->content;
		packet_len = msg->content_len;
	}
	
	if(msg->link->connection == VQP_PROTOCOL_CONN_UDP) {
		/* IP/UDP transport
		 */
		struct sockaddr_in sin;
		
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = PF_INET;
		sin.sin_port = htons(msg->link->port);

		/* send message to all the netmasks/multicasts specified */
		if(!vqp_addr_is_nil(&msg->dst_addr)) {
			/* send packet directly to specified address
			 */
			sin.sin_addr.s_addr = htonl(msg->dst_addr.node.ip);
			sendto(msg->link->tx_socket, packet, packet_len, 0,
					(struct sockaddr *)&sin, sizeof(sin));
		}
		else if(msg->link->protocol == VQP_PROTOCOL_VYPRESSCHAT
				&& (msg->link->options & VQP_PROTOCOL_OPT_MULTICAST))
		{
			/* send packet to multicast group
			 */
			sin.sin_addr.s_addr = htonl(msg->link->conndata.udp.multicast_address);
			sendto(msg->link->tx_socket, packet, packet_len, 0,
					(struct sockaddr *)&sin, sizeof(sin));
		}
		else {
			/* send packet to multiple broadcast addresses
			 */
			int n;
			for(n = 0; msg->link->conndata.udp.p_broadcast_addresses[n] != 0; n++) {
				sin.sin_addr.s_addr = htonl(
					msg->link->conndata.udp.p_broadcast_addresses[n]);

				sendto(msg->link->tx_socket, packet, packet_len, 0,
						(struct sockaddr *)&sin, sizeof(sin));
			}
		}
	}
	else if(msg->link->connection == VQP_PROTOCOL_CONN_IPX) {
		/* IPX transport
		 */
		struct sockaddr_ipx six;
		
		memset(&six, 0, sizeof(six));
		six.sa_family = AF_IPX;
		six.sa_socket = htons(ushort2bcd(msg->link->port));

		if(!vqp_addr_is_nil(&msg->dst_addr)) {
			/* send packet to specified address
			 */
			memcpy(six.sa_netnum, msg->link->conndata.ipx.netnum, 4);
			memcpy(six.sa_nodenum, msg->dst_addr.node.ipx, 6);
		}
		else {
			/* send packet to broadcast
			 */
			memset(six.sa_netnum, 0, 4);
			memset(six.sa_nodenum, 0xff, 6);
		}

		sendto(	msg->link->tx_socket, packet, packet_len, 0,
			(struct sockaddr *)&six, sizeof(six));
	}

	/* free packet data */
	if(packet != msg->content)
		vqp_mmi_free(packet);

	return 0;	/* packet sent ok */
}

int vqp_link_recv(vqp_link_t vqlink, vqp_msg_t * p_in_msg)
{
	struct vqp_link_struct * link = P_VQP_LINK_STRUCT(vqlink);
	struct vqp_message_struct * msg;
	struct sockaddr_in sin;
	struct sockaddr_ipx six;
	socklen_t sa_len;
	char * buf;
	ssize_t buf_data_len;

	/* receive the msg */
	buf = vqp_mmi_malloc(VQP_MAX_PACKET_SIZE);

	if(link->connection == VQP_PROTOCOL_CONN_UDP) {
		sa_len = sizeof(sin);
		buf_data_len = recvfrom(
			link->rx_socket, (void*)buf, VQP_MAX_PACKET_SIZE, 0,
			(struct sockaddr *) &sin, &sa_len);
	} else {
		sa_len = sizeof(six);
		buf_data_len = recvfrom(
			link->rx_socket, (void *)buf, VQP_MAX_PACKET_SIZE, 0,
			(struct sockaddr *) &six, &sa_len);
	}

	if(buf_data_len <= 1) {
		vqp_mmi_free(buf);
		return errno;
	}

	if(link->protocol == VQP_PROTOCOL_VYPRESSCHAT) {
		/* check that the packets begins with 'X' and contains
		 * a signature */
		if(buf[0] != 'X' || buf_data_len < (1 + VQP_LINK_SIG_LEN + 1)) {
			vqp_mmi_free(buf);
			return 1;
		}

		/* check that the signature is not already seen */
		if(vqp_is_seen_msg_sig(link, buf + 1)) {
			vqp_mmi_free(buf);
			return 1;
		}
	}

	/* alloc message */
	msg = vqp_mmi_malloc(sizeof(struct vqp_message_struct));
	if(!msg) return 1;

	msg->link = link;
	msg->src_addr.conn = link->connection;
	if(link->connection == VQP_PROTOCOL_CONN_UDP) {
		msg->src_addr.node.ip = ntohl(sin.sin_addr.s_addr);
	} else {
		memcpy(msg->src_addr.node.ipx, six.sa_nodenum, 6);
	}

	/* copy contents */
	msg->content_len = (link->protocol == VQP_PROTOCOL_VYPRESSCHAT)
		? buf_data_len - 1 - VQP_LINK_SIG_LEN
		: buf_data_len;

	msg->content = vqp_mmi_malloc(msg->content_len);
	if(!msg->content) {
		vqp_mmi_free(buf);
		vqp_mmi_free(msg);
		return 1;
	}

	if(link->protocol == VQP_PROTOCOL_VYPRESSCHAT) {
		/* copy signature */
		memcpy(msg->sig, buf + 1, VQP_LINK_SIG_LEN);
		msg->sig[VQP_LINK_SIG_LEN] = '\0';

		/* copy contents */
		memcpy(msg->content, buf + 1 + VQP_LINK_SIG_LEN, msg->content_len);
	} else {
		memcpy(msg->content, buf, msg->content_len);
	}

	/* free packet buffer */
	vqp_mmi_free(buf);

	/* return the msg */
	*p_in_msg = msg;
	return 0;
}

void vqp_addr_nil(vqp_link_t link, vqp_addr_t * p_addr)
{
	p_addr->conn = P_VQP_LINK_STRUCT(link)->connection;
	memset(&p_addr->node, 0, sizeof(union vqp_addr_node_union));
}

int vqp_addr_is_nil(vqp_addr_t * p_addr)
{
	int is_nil;
	
	if(p_addr->conn == VQP_PROTOCOL_CONN_UDP) {
		is_nil = (p_addr->node.ip == 0);	
	} else {
		char nil_ipx[6] = { 0, 0, 0, 0, 0, 0 };
		is_nil = memcmp(nil_ipx, p_addr->node.ipx, 6) == 0;
	}
	return is_nil;
}

