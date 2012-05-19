/*
 *  Off-the-Record Messaging Toolkit
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SESSKEYS_H__
#define __SESSKEYS_H__

/* Generate the session id and the two encryption keys from our private
 * DH key and their public DH key.  Also indicate in *high_endp if we
 * are the "high" end of the key exchange (set to 1) or the "low" end
 * (set to 0) */
void sesskeys_gen(unsigned char sessionid[20], unsigned char sendenc[16],
	unsigned char rcvenc[16], int *high_endp, gcry_mpi_t *our_yp,
	gcry_mpi_t our_x, gcry_mpi_t their_y);

/* Generate a MAC key from the corresponding encryption key */
void sesskeys_make_mac(unsigned char mackey[20], unsigned char enckey[16]);

#endif
