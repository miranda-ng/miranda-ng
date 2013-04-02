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
 * $Id: skin.h,v 1.1 2005/03/09 14:44:22 bobas Exp $
 */

#ifndef __SKIN_H
#define __SKIN_H

void skin_init();
void skin_uninit();
void skin_hook_modules_loaded();

/* use SkinPlaySound() to play it */
#define SKIN_SOUND_ALERT_BEEP	VQCHAT_PROTO "/AlertBeep"

#endif

