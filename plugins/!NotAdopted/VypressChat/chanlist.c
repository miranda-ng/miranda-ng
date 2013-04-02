/*
 * Miranda-IM Vypress Chat/quickChat plugins
 * Copyright (C) Saulius Menkevicius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: chanlist.c,v 1.10 2005/03/07 14:26:54 bobas Exp $
 */

#include "miranda.h"

#include "main.h"
#include "chanlist.h"

/* internal routines
 */

/* chanlist_find_channel:
 *	  Returns a pointer to channel in a channel list
 *	(or a NULL, if not found)
 * 	  *p_cl_len will be set to the length of channel list.
 *	  *p_ch_len will be set to the length of a channel without the '#'.
 */
static const char *
chanlist_find_channel(
	const char * chanlist, const char * channel,
	int * p_cl_len, int * p_ch_len)
{
	char * cl_channel;
	int cl_len, ch_len;

	if(!chanlist)
		return NULL;

	/* get chanlist and channel lengths */
	cl_len = strlen(chanlist);
	ASSERT_RETURNVALIFFAIL(cl_len >= 2 && chanlist[0]=='#', NULL);
	if(p_cl_len)
		*p_cl_len = cl_len;

	ch_len = strlen(channel);
	ASSERT_RETURNVALIFFAIL(ch_len!=0, NULL);
	if(p_ch_len)
		*p_ch_len = ch_len;

	/* the list doesn't contain a channel if it's smaller than
	 * the channel itself
	 */
	if(cl_len-1 < ch_len)
		return NULL;

	/* find the channel in the list */
	cl_channel = strstr(chanlist, channel);

	/* there must be a '#' before channel name */
	if(cl_channel==NULL || *(cl_channel - 1) != '#')
		return NULL;

	/* and it must be last on the channel list, or end up with '#' */
	if(cl_channel[ch_len]!='\0' && cl_channel[ch_len]!='#')
		return NULL;

	/* ok, the channel was found */
	return cl_channel;
}

/* exported routines
 */

/* chanlist_is_valid:
 *	returns if the chanlist is in valid format
 */
int chanlist_is_valid(const char * chanlist, int no_empty_channels)
{
	int cl_len;
	
	/* chanlist can be empty -- NULL */
	if(chanlist==NULL)
		return 1;

	/* non-empty chanlist must be with length >= 0,
	 * and must begin with a '#'
	 */
	cl_len = strlen(chanlist);
	if(cl_len < 2 || chanlist[0]!='#')
		return 0;

	if(no_empty_channels) {
		/* check that there are no 0-length channel names
		 * in the list
		 */
		const char * prev = chanlist, * next;
		do {
			next = strchr(prev + 1, '#');
			if(next && (next - prev) == 1)
				return 0;

			prev = next;
		} while(next);
	}

	return 1;	/* this chanlist is supposedly valid */
}

/* chanlist_add:
 *	  Creates a channel list (if chanlist==NULL) with a new channel
 *	or add a new one to the end.
 *	  Checks if the channel is already in the list.
 * 	  The result channel list string should look like:
 *		"#channel1#channel2...#channelN".
 *	  An empty channel list is a (char*)NULL.
 */
char * chanlist_add(char * chanlist, const char * channel)
{
	char * new_chanlist;
	int cl_len, ch_len;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), chanlist);

	if(chanlist_find_channel(chanlist, channel, NULL, NULL))
		return chanlist;

	/* the chanlist doesn't contain the new channel:
	 * append it to the end
	 */

	ch_len = strlen(channel);
	ASSERT_RETURNVALIFFAIL(ch_len!=0, chanlist);

	if(chanlist) {
		/* get the length of channel list, note that an empty chanlist
		 * MUST be (char*)NULL, and an non-empty chanlist must be at
		 * least 2 chars in length ("#a") and have '#' at the beginning
		 */
		cl_len = strlen(chanlist);
		ASSERT_RETURNVALIFFAIL(
			cl_len >= 2 && chanlist[0]=='#', chanlist);
	} else {
		cl_len = 0;
	}

	/* allocate space for a previous channel list, plus a # character
	 * and new channel, and a terminator
	 */
	new_chanlist = malloc((cl_len + ch_len + 1 + 1) * sizeof(char));
	ASSERT_RETURNVALIFFAIL(new_chanlist!=NULL, chanlist);

	if(chanlist) {
		/* strcpy(new_chanlist, chanlist); */
		memcpy(new_chanlist, chanlist, cl_len);
		free(chanlist);
	}

	new_chanlist[cl_len] = '#';	/* strcat(new_chanlist, "#"); */

	/* strcat(new_chanlist, channel); */
	memcpy(new_chanlist + cl_len + 1, channel, ch_len + 1); 

	return new_chanlist;
}

/* chanlist_remove:
 *	  Removes a channel from chanlist and frees the resulting
 *	chanlist, if it becomes empty (thus returning the (char*)NULL)
 */
char * chanlist_remove(char * chanlist, const char * channel)
{
	char * cl_channel;
	int cl_len, ch_len;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), chanlist);

	if(chanlist==NULL)
		return NULL;

	cl_channel = (char*)chanlist_find_channel(
				chanlist, channel, &cl_len, &ch_len);

	if(cl_channel == NULL)
		return chanlist;

	/* check if we need to free the list, (if it was the only channel) */
	if(cl_len == ch_len + 1) {
		free(chanlist);
		return NULL;
	}

	/* if the channel was the last on the list, we put a terminator '\0'
	 * and we're finished */
	if((cl_channel - chanlist) + ch_len == cl_len) {
		*(cl_channel - 1) = '\0'; /* put '\0' on channel's '#' char */
		return chanlist;
	}

	/* we need to move channels after cl_channel in the chanlist
	 * to the place of cl_channel (including the '\0' terminator) */
	memcpy(cl_channel, cl_channel + ch_len + 1,
		cl_len - (cl_channel - chanlist) - ch_len);

	return chanlist;
}

/* chanlist_shift:
 *	removes the first channel from the list and returns
 *	the chanlist without the channel
 *
 * params:
 *	@p_chanlist - chanlist to shift the channel off
 * returns:
 *	NULL, if the chanlist is empty
 *	malloc'ed channel name shifted from the chanlist
 */
char * chanlist_shift(char ** p_chanlist)
{
	char * next, * new_chanlist, * channel;
	
	/* check that the channel is valid */
	ASSERT_RETURNVALIFFAIL(VALIDPTR(p_chanlist), NULL);
	if(!chanlist_is_valid(*p_chanlist, 0))
		return NULL;

	/* check if chanlist is empty */
	if(*p_chanlist==NULL) return NULL;

	/* get pointer to the next channel in the list */
	next = strchr(*p_chanlist + 1, '#');

	/* make a copy of the rest as new chanlist */
	new_chanlist = chanlist_copy(next);
	
	/* finish channel name with a '\0' */
	if(next) *next = '\0';
	channel = *p_chanlist;
	memmove(channel, channel + 1, strlen(channel));

	*p_chanlist = new_chanlist;
	return channel;
}

/* chanlist_merge:
 *	  Merges two chanlists, the result adds to chanlist_dst
 *	and it is what returns.
 *
 *	  Note that chanlist_src might get modified in the process
 *	thus it is non-const, but it is kept unmodified after return.
 */
static int
chanlist_merge_enum_fn(const char * channel, void * enum_data)
{
	char ** p_dst_chanlist = (char **)enum_data;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), 0);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(p_dst_chanlist), 0);

	*p_dst_chanlist = chanlist_add(*p_dst_chanlist, channel);

	return 1;	/* keep enumerating */
}

char * chanlist_merge(char * chanlist_dst, const char * chanlist_src)
{
	chanlist_enum(chanlist_src, chanlist_merge_enum_fn, &chanlist_dst);

	return chanlist_dst;
}

/* chanlist_contains:
 *	  Returns non-0, if the chanlist contains the specified channel.
 */
int chanlist_contains(const char * chanlist, const char * channel)
{
	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), 0);

	return chanlist_find_channel(chanlist, channel, NULL, NULL) != NULL;
}

/* chanlist_enum:
 *	  Enumerates chanlist.
 *	The enum function should return non-0 to keep enumeration going,
 *	or 0 to stop enumeration.
 */
void chanlist_enum(
	const char * orig_chanlist, chanlist_enum_fn enum_fn,
	void * enum_data)
{
	char * cl_channel, * cl_next_block;
	int cl_len;
	char * chanlist = chanlist_copy(orig_chanlist);
		
	ASSERT_RETURNIFFAIL(enum_fn!=NULL);

	/* do no enumeration if chanlist is empty */
	if(chanlist == NULL)
		return;

	/* get chanlist length */
	cl_len = strlen(chanlist);

	/* the length must be at least 2 chars ("#a"), and begin with a '#' */
	ASSERT_RETURNIFFAIL(cl_len >= 2 && chanlist[0]=='#');

	/* ok, walk the channel list.. */
	cl_channel = chanlist + 1;
	do {
		/* get address of the next channel's '#' character */
		cl_next_block = strchr(cl_channel, '#');
		if(cl_next_block) {
			/* temporary add a terminator and invoke the callback */
			*cl_next_block = '\0';
			enum_fn(cl_channel, enum_data);

			/* remove the terminator */
			*cl_next_block = '#';
		} else {
			/* the channel is the last on the list: invoke cb */
			enum_fn(cl_channel, enum_data);
		}

		/* skip to next channel */
		if(cl_next_block)
			cl_channel = cl_next_block + 1;
		else	cl_channel = NULL;

	} while(cl_channel);

	/* free the chanlist copy */
	chanlist_free(chanlist);
}

/* chanlist_make_vqp_chanlist:
 *	allocates a vqp chanlist (the same as chanlist, only with '#' at the end)
 *	and always non-NULL
 */
char * chanlist_make_vqp_chanlist(const char * chanlist)
{
	int cl_len;
	char * vqp_chanlist;

	cl_len = chanlist ? strlen(chanlist): 0;
	
	vqp_chanlist = malloc(cl_len + 2);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(vqp_chanlist), NULL);

	if(chanlist)
		memcpy(vqp_chanlist, chanlist, cl_len);

	/* append the '#' and '\0' terminator at the end */
	vqp_chanlist[cl_len] = '#';
	vqp_chanlist[cl_len + 1] = '\0';

	return vqp_chanlist;
}

/* chanlist_parse_vqp_chanlist:
 *	makes a chanlist from vqp chanlist format
 */
char * chanlist_parse_vqp_chanlist(const char * vqp_chanlist)
{
	int vqp_cl_len;
	char * chanlist;
	
	ASSERT_RETURNVALIFFAIL(VALIDPTR(vqp_chanlist), NULL);

	vqp_cl_len = strlen(vqp_chanlist);
	ASSERT_RETURNVALIFFAIL(vqp_cl_len != 0, NULL);

	/* vqp_chanlist must begin and end with '#' */
	ASSERT_RETURNVALIFFAIL(
		vqp_chanlist[0]=='#' && vqp_chanlist[vqp_cl_len-1]=='#', NULL);

	/* make the chanlist (copy everything, except the last '#') */
	chanlist = malloc(vqp_cl_len);
	memcpy(chanlist, vqp_chanlist, vqp_cl_len - 1);
	chanlist[vqp_cl_len - 1] ='\0';

	return chanlist;
}

