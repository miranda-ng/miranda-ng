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
 * $Id: Resource.h,v 1.15 2005/04/11 20:23:58 bobas Exp $
 */

/* icons
 */
#define IDI_VQCHAT_PROTO		1001
#define IDI_VQCHAT_PROTO_LARGE		1002
#define IDI_VQCHAT_PROTO_ONLINE		1010
#define IDI_VQCHAT_PROTO_OFFLINE	1011
#define IDI_CONTACT_BELL		1100
#define IDI_CHATROOM			1200

/* user configuration dialog
 */
#define IDD_USER		2000
#define IDC_USER_EDIT_NICKNAME	2010
#define IDC_USER_COMBO_GENDER	2020
#define IDC_USER_EDIT_UUID	2030
#define IDC_USER_MSGONALERTBEEP	2040
#define IDC_USER_NICKNAMEONTOPIC 2050
#define IDC_USER_NEWPREFERMSG	2060

/* network connection configuration dialog
 */
#define IDD_CONN		3000
#define IDC_CONN_EDIT_REFRESH	3010
#define IDC_CONN_EDIT_PORT	3020
#define IDC_CONN_BTN_DEF_PORT	3030
#define IDC_CONN_EDIT_SCOPE	3040
#define IDC_CONN_BTN_DEF_MULTICAST 3050
#define IDC_CONN_IP_MULTICAST	3060
#define IDC_CONN_CHECK_UTF8	3070
#define IDC_CONN_RADIO_MULTICAST 3080
#define IDC_CONN_RADIO_BROADCAST 3090
#define IDC_CONN_RADIO_IPX	3100
#define IDC_CONN_BCAST_LIST	3110
#define IDC_CONN_BCAST_INPUT	3120
#define IDC_CONN_BCAST_ADD	3130
#define IDC_CONN_BCAST_REMOVE	3140
#define IDC_CONN_LABEL_UTF8	3200

/* channel settings dialog
 */
#define IDD_CS			4000
#define IDC_CS_EDIT_NAME	4001
#define IDC_CS_EDIT_TOPIC	4002
#define IDC_CS_BTN_SET		4004

/* join channel dialog
 */
#define IDD_JC			5000
#define IDC_JC_COMBO_CHANNEL	5001

/* set nickname dialog
 */
#define IDD_SN			6000
#define IDC_SN_EDIT_NICKNAME	6001

#define IDC_STATIC	-1

/* "user details: basic info" property page
 */
#define IDD_USERINFO		7000
#define IDC_USERINFO_COMPUTER	7010
#define IDC_USERINFO_USER	7020
#define IDC_USERINFO_NODE	7030
#define IDC_USERINFO_PLATFORM	7040
#define IDC_USERINFO_WORKGROUP	7050
#define IDC_USERINFO_SOFTWARE	7060
#define IDC_USERINFO_CHANNELS	7070

/* "user details: <protocol> options" property page
 */
#define IDD_USEROPT		8000
#define IDC_USEROPT_NICKNAME	8010
#define IDC_USEROPT_SET		8020
#define IDC_USEROPT_PREFERS	8030
#define IDC_USEROPT_LOCKNICK	8040

