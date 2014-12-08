{ ---------------------------------------------------------------------------
                ICQ plugin for Miranda Instant Messenger
                ________________________________________

 Copyright c 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
 Copyright c 2001-2002 Jon Keating, Richard Hughes
 Copyright c 2002-2004 Martin ñberg, Sam Kothari, Robert Rainwater
 Copyright c 2004-2009 Joe Kucera

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

 -----------------------------------------------------------------------------

// File name      : $URL: https://svn.sourceforge.net/svnroot/miranda/trunk/miranda/include/m_icq.h $
// Revision       : $Revision: 9376 $
// Last change on : $Date: 2009-04-09 23:55:33 +0400 (êë', 09 ê¯êıë? 2009) $
// Last change by : $Author: jokusoftware $

 DESCRIPTION:

  Describe me here please...

 -----------------------------------------------------------------------------
 Note: In v0.3 the part before "/Servicename" is dynamic. It will be the name of the protocol.
 Example: If the plugin was loaded from ICQ.dll, the service name is "ICQ/Servicename", and if
 the dll was Icq2.dll, the service name will be "Icq2/Servicename". This behaviour is temporary
 until proper multiaccounts are implemented.
}

{$IFNDEF M_ICQ}
{$DEFINE M_ICQ}
// extended search result structure, used for all searches
type
  PICQSEARCHRESULT = ^TICQSEARCHRESULT;
  TICQSEARCHRESULT = record
    hdr : TPROTOSEARCHRESULT;
    uin : dword;
    auth: byte;
    gender       :byte;
    age          :byte;
    country      :dword;
    maritalStatus:byte;
  end;
{
  Request authorization
  wParam=(WPARAM)hContact
}
const
   MS_REQ_AUTH = '/ReqAuth';
{
  Grant authorization
  wParam=(WPARAM)hContact;
}
   MS_GRANT_AUTH = '/GrantAuth';
{
  Revoke authorization
  wParam=(WPARAM)hContact
}
   MS_REVOKE_AUTH = '/RevokeAuth';

{
  Open ICQ profile
  wParam=(WPARAM)hContact
}
   MS_OPEN_PROFILE = '/OpenProfile';

{
  Add contact to server-list
  wParam=(WPARAM)hContact
}
   MS_ICQ_ADDSERVCONTACT  = '/AddServerContact';
{
  Display XStatus detail (internal use only)
  wParam=(WPARAM)hContact;
}
   MS_XSTATUS_SHOWDETAILS = '/ShowXStatusDetails';

{Send an SMS via the ICQ network
wParam=(WPARAM)(const AnsiChar*)szPhoneNumber
lParam=(LPARAM)(const AnsiChar*)szMessage
Returns a HANDLE to the send on success, or NULL on failure
szPhoneNumber should be the full number with international code and preceeded
  by a +

When the server acks the send, an ack will be broadcast:
 type=ICQACKTYPE_SMS, result=ACKRESULT_SENTREQUEST, lParam=(LPARAM)(AnsiChar*)szInfo
At this point the message is queued to be delivered. szInfo contains the raw
XML data of the ack. Here's what I got when I tried:
"<sms_response><source>airbornww.com</source><deliverable>Yes</deliverable><network>BT Cellnet, United Kingdom</network><message_id>[my uin]-1-1955988055-[destination phone#, without +]</message_id><messages_left>0</messages_left></sms_response>\r\n"
Now the hProcess has been deleted. The only way to track which receipt
corresponds with which response is to parse the <message_id> field.
At a (possibly much) later time the SMS will have been delivered. An ack will
be broadcast:
 type=ICQACKTYPE_SMS, result=ACKRESULT_SUCCESS, hProcess=NULL, lParam=(LPARAM)(AnsiChar*)szInfo
Note that the result will always be success even if the send failed, just to
save needing to have an attempt at an XML parser in the ICQ module.
Here's the szInfo for a success:
"<sms_delivery_receipt><message_id>[my uin]-1--1461632229-[dest phone#, without +]</message_id><destination>[dest phone#, without +]</destination><delivered>Yes</delivered><text>[first 20 bytes of message]</text><submition_time>Tue, 30 Oct 2001 22:35:16 GMT</submition_time><delivery_time>Tue, 30 Oct 2001 22:34:00 GMT</delivery_time></sms_delivery_receipt>"
And here's a failure:
"<sms_delivery_receipt><message_id>[my uin]-1-1955988055-[destination phone#, without leading +]</message_id><destination>[destination phone#, without leading +]</destination><delivered>No</delivered><submition_time>Tue, 23 Oct 2001 23:17:02 GMT</submition_time><error_code>999999</error_code><error><id>15</id><params><param>0</param><param>Multiple message submittion failed</param></params></error></sms_delivery_receipt>"
SMSes received from phones come through this same ack, again to avoid having
an XML parser in the protocol module. Here's one I got:
"<sms_message><source>MTN</source><destination_UIN>[UIN of recipient, ie this account]</destination_UIN><sender>[sending phone number, without +]</sender><senders_network>[contains one space, because I sent from ICQ]</senders_network><text>[body of the message]</text><time>Fri, 16 Nov 2001 03:12:33 GMT</time></sms_message>"
}

{
  e-mail express
  db event added to NULL contact
  blob format is:
  ASCIIZ    text, usually of the form "Subject: %s\r\n%s"
  ASCIIZ    from name
  ASCIIZ    from e-mail
}
  ICQEVENTTYPE_EMAILEXPRESS = 2002; // database event type
{
  www pager
  db event added to NULL contact
  blob format is:
  ASCIIZ    text, usually "Sender IP: xxx.xxx.xxx.xxx\r\n%s"
  ASCIIZ    from name
  ASCIIZ    from e-mail
}
  ICQEVENTTYPE_WEBPAGER = 2003; // database event type
{
  missed message notification
  db event added to contact's history
  blob format is:
  word      error code
}
  ICQEVENTTYPE_MISSEDMESSAGE = 2004; //database event type
{
  for server-side lists, used internally only
  hProcess=dwSequence
  lParam=server's error code, 0 for success
}
  ICQACKTYPE_SERVERCLIST = 1003;
{
  for rate warning distribution (mainly upload dlg)
  hProcess=Rate class ID
  lParam=server's status code
}
  ICQACKTYPE_RATEWARNING = 1004;
{
  received Xtraz Notify response
  hProcess=dwSequence
  lParam=contents of RES node
}
  ICQACKTYPE_XTRAZNOTIFY_RESPONSE = 1005;
{
  received Custom Status details response
  hProcess=dwSequence
  lParam=0
}
  ICQACKTYPE_XSTATUS_RESPONSE = 1006;

//Update user details on server
//Permited operation types:
  CIXT_BASIC      = $0001;
  CIXT_MORE       = $0002;
  CIXT_WORK       = $0004;
  CIXT_CONTACT    = $0008;
  CIXT_LOCATION   = $0010;
  CIXT_BACKGROUND = $0020;
  CIXT_EDUCATION  = $0040;
  CIXT_EXTRA      = $0080;
  CIXT_FULL       = $00FF;

//wParam=operationType
  PS_CHANGEINFOEX = '/ChangeInfoEx';

{
  miranda/icqoscar/statusmsgreq event
  called when our status message is requested
  wParam=(byte)msgType
  lParam=(dword)uin
  msgType is one of the ICQ_MSGTYPE_GET###MSG constants in icq_constants.h
  uin is the UIN of the contact requesting our status message
}
  ME_ICQ_STATUSMSGREQ = '/StatusMsgReq';

{
  Request Custom status details (messages) for specified contact
  wParam = hContact  // request custom status details for this contact
  lParam = 0
  return = (int)dwSequence   // if successful it is sequence for ICQACKTYPE_XSTATUS_RESPONSE
  0 failed to request (e.g. auto-request enabled)
  -1 delayed (rate control) - sequence unknown
}
  PS_ICQ_REQUESTCUSTOMSTATUS = '/RequestXStatusDetails';

const
  MAX_CAPNAME = 64;

type
  TICQ_CUSTOMCAP = record
    cbSize: int;
    caps  : array [0..15] of AnsiChar;
    hIcon : THANDLE;
    name  : array [0..MAX_CAPNAME-1] of AnsiChar;
  end;

const
{
  Add a custom icq capability.
  wParam = 0;
  lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
}
  PS_ICQ_ADDCAPABILITY = '/IcqAddCapability';

{
  Check if capability is supportes. Only icqCustomCap.caps does matter.
  wParam = (WPARAM)(HANDLE)hContact;
  lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
  returns non-zero if capability is supported
}
  PS_ICQ_CHECKCAPABILITY = '/IcqCheckCapability';

{$ENDIF}
