Steam mobile app protocol
=========================

This is the data structure of the steam mobile app's protocl

This could be useful for implementing the protcol in another messaging
program.

Inside the proxy folder there are commands to record the traffic from
the phone.

Structure
---------

There seems to be three (maybe more) main groups of functions.

 - `ISteamOAuth2`: Main authentication functions
 - `ISteamWebAPIUtil`: Useless stuff but might be useful for testing
 - `ISteamUserOAuth`: Querying for friends list
 - ISteamWebUserPresenceOAuth: Used for chatting

Structure of calls
------------------

Each method is either a get or post method which are used as in http
(i.e. get methods just return data and wont update anything)

The path is 
`https://api.steampowered.com/<main group>/<method>/<version string (always v0001)>`

Data is passed for get as a standard query string and for post it is the
same (the actual client adds a ? on the front of the post data despite
this actually not being required)

All methods return json

### Common fields ###

 - steamid: this is the users account can be retrieved with
`ISteamOAuth2/GetTokenWithCredentials`
 - access_token: authentication token retrieved with
 `ISteamOAuth2/GetTokenWithCredentials`
 - umqid: as far as I can tell this is just a random number to
 differenciate between clients just pass to
 `ISteamWebUserPresenceoAuth/Logon`
 - message: used for polling the server for new messages
 - messages: list of messages

### Message format ###

 - `type` - "saytext" or "typing". saytext sends a message while typing is
		 just for an indication they are typing (probably more)
 - `timestamp`
 - `steamid_from` - who sent it
 - `secure_message_id` - as the PollStatus can be accessed over http when
 a message comes in the client will then fetch the message over https
 from Poll
 - `text` - actual text

Calls
=====

ISteamOAuth2
------------

### `GET /ISteamOAuth2/GetTokenWithCredentials` ###

To login gets access_token

#### Parameters ####

 - `client_id` - use `DE45CD61`
 - `grant_type` - use password
 - `username` - the username
 - `password` - the password
 - `x_emailauthcode` - if the user is using two factor authentication you
 will have to try once without this and then again with this after they
 receive a code in an email
 - `scope` - use 'read_profile write_profile read_client write_client'

#### Returns ####

 - `access_token`
 - `x_webcookie` - presumably used to access steam webpages without logging
 in again
 - `x_steamid`

ISteamWebAPIUtil
----------------

### `GET /ISteamWebAPIUtil/GetServerInfo/v0001` ###

Gives you the time

#### Parameters ####

none

#### Returns ####

 - `servertime`
 - `servertimestring`

ISteamUserOAuth
---------------

### `GET /ISteamUserOAuth/GetFriendList/v0001` ###

Gets your friend list

#### Parameters ####

 - `access_token`
 - `steamid`

#### Returns ####

 - `friends` - a list of:
    - `steamid`
    - `relationship` - "friend"? 
    - `friend_since`

#### Example #####

    GET
    /ISteamUserOAuth/GetFriendList/v0001?access_token=c1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX&steamid=XXXXXXXXXXXXXXX79
    HTTP/1.1
    User-Agent: Steam App / Android / 1.0 / 1297579
    Host: api.steampowered.com:443
    Connection: Keep-Alive

    HTTP/1.1 200 OK
    content-type: application/json; charset=UTF-8
    content-length: 1642

    {
	"friends": [
	{
		"steamid": "XXXXXXXXXXXXXXX77",
			"relationship": "friend",
			"friend_since": 0
	},
	{
		"steamid": "XXXXXXXXXXXXXXX52",
		"relationship": "friend",
		"friend_since": XXXXXXXX73
	}
	]

    }

### `GET /ISteamUserOAuth/GetGroupList/v0001` ###



### `GET /ISteamUserOAuth/GetUserSummaries/v0001` ###


Gets extra information on users

#### Parameters ####

 - `access_token`
 - `steamids` - comma seperated list of the ids to look up

#### Returns ####

 - `players` - list of 
   - `steamid`
   - `communityvisibilitystate` - always 3?
   - `profilestate` - always 1?
   - `personaname` - seems to be the name set
   - `profileurl`
   - `avatar`
   - `avatarmedium`
   - `avatarfull`
   - `personastate`
   - `primaryclanid`
   - `timecreated`
   - some extra attributes sometimes...

#### Example ####

    GET
    /ISteamUserOAuth/GetUserSummaries/v0001?access_token=c1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX&steamids=XXXXXXXXXXXXXXX79,XXXXXXXXXXXXXXX55
    HTTP/1.1
    User-Agent: Steam App / Android / 1.0 / 1297579
    Host: api.steampowered.com:443
    Connection: Keep-Alive

    HTTP/1.1 200 OK
    Content-Type: application/json; charset=UTF-8
    Content-Length: 13433
 
    {
 	"players": [
	{
		"steamid": "XXXXXXXXXXXXXXX79",
			"communityvisibilitystate": 3,
			"profilestate": 1,
			"personaname": "XXXXXX",
			"lastlogoff": XXXXXXXX60,
			"profileurl":
				"http:\/\/steamcommunity.com\/profiles\/XXXXXXXXXXXXXXX79\/",
			"avatar":
				"http:\/\/media.steampowered.com\/steamcommunity\/public\/images\/avatars\/fe\/feXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXb.jpg",
			"avatarmedium":
				"http:\/\/media.steampowered.com\/steamcommunity\/public\/images\/avatars\/fe\/feXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXb_medium.jpg",
			"avatarfull":
				"http:\/\/media.steampowered.com\/steamcommunity\/public\/images\/avatars\/fe\/feXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXb_full.jpg",
			"personastate": 1,
			"primaryclanid": "XXXXXXXXXXXXXXXX69",
			"timecreated": XXXXXXXX79
	},
	{
		"steamid": "XXXXXXXXXXXXXXX55",
		"communityvisibilitystate": 3,
		"profilestate": 1,
		"personaname": "XXXXXX",
		"lastlogoff": XXXXXXXX48,
		"profileurl":
			"http:\/\/steamcommunity.com\/id\/XXXXXX\/",
		"avatar":
			"http:\/\/media.steampowered.com\/steamcommunity\/public\/images\/avatars\/26\/26XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXe.jpg",
		"avatarmedium":
			"http:\/\/media.steampowered.com\/steamcommunity\/public\/images\/avatars\/26\/26XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXe_medium.jpg",
		"avatarfull":
			"http:\/\/media.steampowered.com\/steamcommunity\/public\/images\/avatars\/26\/26XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXe_full.jpg",
		"personastate": 0,
		"realname": "XXXXXX",
		"primaryclanid": "XXXXXXXXXXXXXXXX87",
		"timecreated": XXXXXXXX33,
		"loccountrycode": "US",
		"locstatecode": "CA",
		"loccityid": XXX
	}
	]

    }




ISteamWebUserPresenceOAuth
--------------------------

### `POST /ISteamWebUserPresenceOAuth/Logon/v0001` ###

Connects to chat

#### Parameters ####

 - `access_token`
 - `umqid` - just make up a number for this

#### Returns ####
 
 - `steamid`
 - `umqid`
 - `timestamp`
 - `message` - last message use for polling
 - `push` - ??

### `POST /ISteamWebUserPresenceOAuth/PollStatus/v0001` ###

To check for new messages (can be used over http and when a message
		comes in it can be fetched over https)

#### Parameters ####

 - `steamid`
 - `umqid`
 - `message` - id of last message recieved used to check for newer mesages

#### Returns ####
 
 - `pollid` - always 0?
 - `messages` - a list of new messages (only if new messages)
 - `messagelast` - last message id used to poll for new messages (only if
		 new mesages)
 - `messagebase` - message id you sent (only if new messages)
 - `timestamp` - (only if new messages)
 - `sectimeout` - 20 if no new messages less otherwise

 - `error` - "OK" for new messages otherwise "Timeout"

#### Example ####

    POST /ISteamWebUserPresenceOAuth/PollStatus/v0001 HTTP/1.1
    Content-Type: application/x-www-form-urlencoded
    User-Agent: Steam App / Android / 1.0 / 1297579
    Content-Length: 64
    Host: api.steampowered.com:80
    Connection: Keep-Alive
    
    ?steamid=XXXXXXXXXXXXXXX79&umqid=XXXXXXXXXXXXXXXXX27&message=334
    
    HTTP/1.1 200 OK
    
    {
    	"pollid": 0,
    		"messages": [
    		{
    			"type": "typing",
    			"timestamp": XXXXXXX38,
    			"steamid_from": "XXXXXXXXXXXXXXX70",
    			"text": ""
    		}
    	]
    		,
    		"messagelast": 335,
    		"timestamp": XXXXXXX38,
    		"messagebase": 334,
    		"sectimeout": 11,
    		"error": "OK"
    }


### `POST /ISteamWebUserPresenceOAuth/Poll/v0001` ###

Recieves messages

Same as previous just with access_token and over https

### `POST /ISteamWebUserPresenceOAuth/Message/v0001` ###

Sends a message

#### Parameters ####

 - `access_token`
 - `umqid`
 - `type` - see message format
 - `text` - (optional) see message format
 - `steamid_dst`


- https://github.com/canoon/steam_mobile_protocol/blob/master/README.md
- cameron@shptech.com