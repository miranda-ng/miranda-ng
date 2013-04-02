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
 * $Id: uuid.c,v 1.2 2005/03/02 18:29:09 bobas Exp $
 */

#include "vqproto.h"
#include "link.h"
#include "uuid.h"

#ifdef _WIN32
# include <rpc.h>
#endif

/* static data
 */

/* static routines
 */

/* exported routines
 */

void vqp_uuid_create(vqp_uuid_t * p_uuid)
{
#ifdef _WIN32
	UuidCreate((UUID *) p_uuid);
#else
	/* XXX */
#endif
}

void vqp_uuid_create_nil(vqp_uuid_t * p_uuid)
{
#ifdef _WIN32
	UuidCreateNil((UUID *) p_uuid);
#else
	/* XXX */
#endif
}

int vqp_uuid_is_nil(const vqp_uuid_t * p_uuid)
{
#ifdef _WIN32
	RPC_STATUS rpc_status;
	return UuidIsNil((UUID *) p_uuid, &rpc_status);
#else
	/* XXX */
	return 0;
#endif
}

int vqp_uuid_is_equal(const vqp_uuid_t * p_uuid_a, const vqp_uuid_t * p_uuid_b)
{
#ifdef _WIN32
	RPC_STATUS rpc_status;
	return UuidCompare((UUID *) p_uuid_a, (UUID *) p_uuid_b, &rpc_status) == 0;
#else
	/* XXX */
	return 0;
#endif
}

char * vqp_uuid_to_string(const vqp_uuid_t * p_uuid)
{
#ifdef _WIN32
	unsigned char * rpc_str;
	char * str;
	int len;

	UuidToString((UUID *) p_uuid, &rpc_str);
	len = strlen(rpc_str) + 1;
	str = vqp_mmi_malloc(len);
	memcpy(str, rpc_str, len);
	RpcStringFree(&rpc_str);

	return str;
#else
	/* XXX */
	return NULL;
#endif
}

int vqp_uuid_from_string(vqp_uuid_t * p_uuid, const char * str)
{
#ifdef _WIN32
	return UuidFromString((unsigned char *)str, (UUID *) p_uuid) != RPC_S_OK;
#else
	/* XXX */
	return 1;
#endif
}

