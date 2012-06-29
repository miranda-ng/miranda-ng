/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#define IDI_CHAT                        100
#define IDI_ITALIC                      101
#define IDI_BOLD                        102
#define IDI_UNDERLINE                   103
#define IDI_OPTIONS                     104
#define IDI_OWNER                       105
#define IDI_ADMIN                       106
#define IDI_REGISTERED                  107
#define IDI_R_MODERATED                 108
#define IDI_R_MEMBERS                   109
#define IDI_R_ANONYMOUS                 110
#define IDI_SEARCH                      111
#define IDI_PREV                        112
#define IDI_NEXT                        113
#define IDI_BLANK                       114
#define IDI_INVITE                      115
#define IDI_ADMINISTRATION				116
#define IDI_GLOBALOWNER                 117
#define IDI_SMILEY						118
#define IDI_MESSAGE						119

#define IDD_CHATMANAGER                 150
#define IDD_GROUPCHAT_LOG               151
#define IDD_HELPER_JOIN                 152
#define IDD_HELPER_INPUT                153
#define IDD_HELPER_INVITE               154
#define IDD_HELPER_INVITE_ACCEPT        155
#define IDD_HELPER_TOPIC                156
#define IDD_OPTIONS                     157
#define IDD_OPT_POPUPS                  158
#define IDD_TAB_CHATROOMS               159
#define IDD_TAB_MYROOMS                 160
#define IDD_TAB_MYNICKNAMES             161
#define IDD_USER_ADMIN					162
#define IDD_USER_KICK					163
#define IDD_USER_ROLE					164
#define IDD_USER_BROWSER				165
#define IDD_GROUPCHAT_CONTAINER			166

#define IDR_CHATOPTIONS                 200
#define IDR_CONTEXT						201

#define IDC_FONTLIST                    1000
#define IDC_COLORINPUT                  1001
#define IDC_COLORBKG                    1002
#define IDC_COLORLIST                   1003
#define IDC_COLORTXT                    1004
#define IDC_CHOOSEFONT                  1005
#define IDC_FONTSGB                     1006
#define IDC_OPTIONSGB                   1007
#define IDC_LOGLIMITSPIN                1008
#define IDC_LOGLIMIT                    1009
#define IDC_SENDONENTER                 1010
#define IDC_USETABS						1011
#define IDC_ENABLEIEVIEW				1012
#define IDC_REGISTER                    1020
#define IDC_NAME                        1021
#define IDC_FRAME                       1037
#define IDC_SIMPLE                      1041
#define IDC_PASSWORD                    1050
#define IDC_JID                         1051
#define IDC_COUNTRY                     1060
#define IDC_NICKNAME                    1062
#define IDC_MSG                         1084
#define IDC_TITLE                       1089
#define IDC_DESC                        1090
#define IDC_DELETE                      1092
#define IDC_LOAD                        1093
#define IDC_JOIN                        1123
#define IDC_ROOM                        1124
#define IDC_ROOM_LABEL                  1125
#define IDC_BROWSE                      1126
#define IDC_VSCROLL                     1128
#define IDC_NICK                        1129
#define IDC_NICK_LABEL                  1130
#define IDC_EDIT                        1131
#define IDC_TREELIST                    1132
#define IDC_LIST                        1133
#define IDC_HSPLIT                      1134
#define IDC_LOG                         1136
#define IDC_VSPLIT                      1137
#define IDC_SET                         1140
#define IDC_TABS                        1141
#define IDC_TOPIC                       1141
#define IDC_TOPIC_BUTTON                1142
#define IDC_FONT                        1143
#define IDC_ENTER                       1144
#define IDC_CTRLENTER                   1145
#define IDC_FLASH                       1148
#define IDC_TIME                        1156
#define IDC_DATE                        1157
#define IDC_REASON                      1171
#define IDC_USER                        1172
#define IDC_INVITE                      1173
#define IDC_ACCEPT                      1174
#define IDC_FROM                        1175
#define IDC_DELAY                       1180
#define IDC_PREVIEW                     1183
#define IDC_ENABLEPOPUP                 1184
#define IDC_DELAY_PERMANENT             1185
#define IDC_DELAY_CUSTOM                1186
#define IDC_DELAY_POPUP                 1187
#define IDC_GROUP                       1203
#define IDC_BOLD                        1204
#define IDC_SMILEY                      1205
#define IDC_UNDERLINE                   1205
#define IDC_ITALIC                      1206
#define IDC_COLOR                       1207
#define IDC_OPTIONS                     1208
#define IDC_SMILEYBTN                   1209
#define IDC_CHECK_PERMANENT             1210
#define IDC_CHECK_PUBLIC                1211
#define IDC_CHECK_NICKNAMES             1212
#define IDC_CHECK_MEMBERS               1213
#define IDC_NEXT                        1214
#define IDC_PREV                        1215
#define IDC_FONTSIZE                    1215
#define IDC_SEARCH                      1216
#define IDC_REFRESH                     1216
#define IDC_REMOVE                      1217
#define IDC_KICK_OPTIONS				1218
#define IDC_ROLE_OPTIONS				1219
#define IDC_KICK						1220
#define IDC_SET_ROLE					1221
#define IDC_UPDATE                      1313
#define IDC_STATUS                      1414
#define IDC_PLAN                        1415
#define IDC_SHOW						1416

#define IDC_PERSONALGROUP               1434
#define IDC_EXTRAGROUP                  1436

#define ID_OPTIONMENU_SHOWNICKNAMES     40001
#define ID_OPTIONMENU_MSGINNEWLINE      40002
#define ID_OPTIONMENU_SHOWTIMESTAMP     40011
#define ID_OPTIONMENU_SHOWDATE          40012
#define ID_OPTIONMENU_SHOWSECONDS       40013
#define ID_OPTIONMENU_USELONGDATE       40014
#define ID_OPTIONMENU_FORMATFONT        40021
#define ID_OPTIONMENU_FORMATSIZE        40022
#define ID_OPTIONMENU_FORMATCOLOR       40023
#define ID_OPTIONMENU_FORMATSTYLE       40024
#define ID_OPTIONMENU_LOGMESSAGES       40031
#define ID_OPTIONMENU_LOGJOINED         40032
#define ID_OPTIONMENU_LOGLEFT           40033
#define ID_OPTIONMENU_LOGTOPIC          40034
#define ID_OPTIONMENU_FLASHMESSAGES     40035
#define ID_OPTIONMENU_FLASHJOINED       40036
#define ID_OPTIONMENU_FLASHLEFT         40037
#define ID_OPTIONMENU_FLASHTOPIC        40038
#define ID_OPTIONMENU_SAVEDEFAULT       40040
#define ID_USERMENU_MESSAGE             40050
#define ID_USERMENU_KICK_POPUP          40051
#define ID_USERMENU_KICK_NO_BAN         40052
#define ID_USERMENU_KICK_BAN_1_MIN      40053
#define ID_USERMENU_KICK_BAN_5_MIN      40054
#define ID_USERMENU_KICK_BAN_15_MIN     40055
#define ID_USERMENU_KICK_BAN_30_MIN     40056
#define ID_USERMENU_KICK_BAN_1_H        40057
#define ID_USERMENU_KICK_BAN_6_H        40058
#define ID_USERMENU_KICK_BAN_1_D        40059
#define ID_USERMENU_KICK_BAN_3_D        40060
#define ID_USERMENU_KICK_BAN_1_W        40061
#define ID_USERMENU_KICK_BAN_2_W        40062
#define ID_USERMENU_KICK_BAN_4_W        40063
#define ID_USERMENU_RIGHTS_NO           40065
#define ID_USERMENU_RIGHTS_MEMBER       40066
#define ID_USERMENU_RIGHTS_ADMIN        40067
#define ID_USERMENU_ADMINISTRATION		40070
#define ID_USERMENU_UNBAN				40071
#define ID_ADMINMENU_DESTROY			40080
#define ID_ADMINMENU_ADMIN				40081
#define ID_ADMINMENU_BROWSE				40082
#define ID_ADMINMENU_SAVELOG			40083

#define IDM_CUT                         40000
#define IDM_COPY                        40001
#define IDM_PASTE                       40002
#define IDM_UNDO                        40003
#define IDM_DELETE                      40004
#define IDM_REDO                        40005
#define IDM_COPYALL                     40011
#define IDM_SELECTALL                   40012
#define IDM_CLEAR                       40013
#define IDM_OPENNEW                     40014
#define IDM_OPENEXISTING                40015
#define IDM_COPYLINK                    40016

// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        205
#define _APS_NEXT_COMMAND_VALUE         40090
#define _APS_NEXT_CONTROL_VALUE         1217
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
