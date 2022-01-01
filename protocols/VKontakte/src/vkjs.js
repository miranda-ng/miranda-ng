// Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation version 2
// of the License.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// Instruction:
// 1. Go to https://vk.com/editapp?id=3917910&section=functions
// 2. Create a stored procedure with the appropriate name
// 3. Copy&Paste procedure code to code field. Save it.
// 4. Create additional version(s) stored procedure (if required)


// Stored procedure name: RetrieveUserInfo = Begin
// Arguments:
// Args.userid 
// Args.fields

var res = API.users.get({ "user_ids": Args.userid, "fields": Args.fields, "name_case": "nom" });
return { "freeoffline": 0, "norepeat": 1, "usercount": res.length, "users": res };
// Stored procedure name: RetrieveUserInfo = End

// Stored procedure name: RetrieveUsersInfo = Begin
// Arguments:
// Args.userids 
// Args.fields
// Args.norepeat 
// Args.setonline

// ver 1 
if (Args.setonline == 1)
    API.account.setOnline();
var US = [];
var res = API.users.get({ "user_ids": API.friends.getOnline(), "fields": Args.fields, "name_case": "nom" });
var t = 10;
while (t > 0) {
    US = API.users.get({ "user_ids": Args.userids, "fields": Args.fields, "name_case": "nom" });
    var index = US.length;
    while (index > 0) {
        index = index - 1;
        if (US[index].online != 0) {
            res.push(US[index]);
        };
    };
    t = t - 1;
    if (res.length > 0) t = 0;
};
return { "freeoffline": 1, "norepeat": parseInt(Args.norepeat), "usercount": res.length, "users": res, "requests": API.friends.getRequests({ "extended": 0, "need_mutual": 0, "out": 0 }) };

// ver 2
if (Args.setonline == 1)
    API.account.setOnline();
var res = API.users.get({ "user_ids": Args.userids, "fields": Args.fields, "name_case": "nom" });
return { "freeoffline": 0, "norepeat": parseInt(Args.norepeat), "usercount": res.length, "users": res, "requests": API.friends.getRequests({ "extended": 0, "need_mutual": 0, "out": 0 }) };
// Stored procedure name: RetrieveUsersInfo = End


// Stored procedure name: GetServerHistory = Begin
// Arguments:
// Args.userid
// Args.offset
// Args.reqcount
// Args.time
// Args.lastmid
// Args.once

var Hist = API.messages.getHistory({ "user_id": Args.userid, "count": Args.reqcount, "offset": Args.offset });
var ext = Hist.items.length;
var index = 0;
while (ext != 0) {
    if (Hist.items[index].date > Args.time) {
        if (Hist.items[index].id > Args.lastmid) {
            index = index + 1;
            ext = ext - 1;
        } else
            ext = 0;
    } else
        ext = 0;
};
if (index > 0) {
    var ret = Hist.items.slice(0, index);
    var FMsgs = ret@.fwd_messages;
    var Idx = 0;
    var Uids = [];
    while (Idx < FMsgs.length) {
        var Jdx = 0;
        var CFMsgs = parseInt(FMsgs[Idx].length);
        while (Jdx < CFMsgs) {
            Uids.unshift(FMsgs[Idx][Jdx].user_id);
            Jdx = Jdx + 1;
        };
        Idx = Idx + 1;
    };
    var FUsers = API.users.get({ "user_ids": Uids, "name_case": "gen" });
    return { "count": index, "datetime": parseInt(Args.time), "items": ret, "fwd_users": FUsers, "once": parseInt(Args.once), "rcount": parseInt(Args.reqcount) };
} else {
    return{"count":0,"datetime":parseInt(Args.time),"items":[],"fwd_users":[],"once":parseInt(Args.once),"rcount":parseInt(Args.reqcount)};    
};
// Stored procedure name: GetServerHistory = End


// Stored procedure name: GetServerConversationHistory = Begin
// Arguments:
// Args.userid
// Args.offset
// Args.reqcount
// Args.time
// Args.lastmid
// Args.once

var Hist = API.messages.getHistory({ "user_id": Args.userid, "count": Args.reqcount, "offset": Args.offset });
var ext = Hist.items.length;
var index = 0;
while (ext != 0) {
    if (Hist.items[index].date > Args.time) {
        if (Hist.items[index].id > Args.lastmid) {
            index = index + 1;
            ext = ext - 1;
        } else
            ext = 0;
    } else
        ext = 0;
};
if (index > 0) {
    var ret = Hist.items.slice(0, index);
    var FMsgs = ret@.fwd_messages;
    var Idx = 0;
    var Uids = [];
    while (Idx < FMsgs.length) {
        var Jdx = 0;
        var CFMsgs = parseInt(FMsgs[Idx].length);
        while (Jdx < CFMsgs) {
            Uids.unshift(FMsgs[Idx][Jdx].peer_id);
            Jdx = Jdx + 1;
        };
        Idx = Idx + 1;
    };
    var FUsers = API.users.get({ "user_ids": Uids, "name_case": "gen" });
    return { "count": index, "datetime": parseInt(Args.time), "items": ret, "fwd_users": FUsers, "once": parseInt(Args.once), "rcount": parseInt(Args.reqcount) };
} else {
    return{"count":0,"datetime":parseInt(Args.time),"items":[],"fwd_users":[],"once":parseInt(Args.once),"rcount":parseInt(Args.reqcount)};    
};
// Stored procedure name: GetServerConversationHistory = End

// Stored procedure name: RetrieveMessagesByIds = Begin
// Arguments:
// Args.mids

var Msgs = API.messages.getById({ "message_ids": Args.mids });
var FMsgs = Msgs.items@.fwd_messages;
var Idx = 0;
var Uids = [];
while (Idx < FMsgs.length) {
    var Jdx = 0;
    var CFMsgs = parseInt(FMsgs[Idx].length);
    while (Jdx < CFMsgs) {
        Uids.unshift(FMsgs[Idx][Jdx].user_id);
        Jdx = Jdx + 1;
    };
    Idx = Idx + 1;
};
var FUsers = API.users.get({ "user_ids": Uids, "name_case": "gen" });
return { "Msgs": Msgs, "fwd_users": FUsers };
// Stored procedure name: RetrieveMessagesByIds = End

// Stored procedure name: RetrieveMessagesConversationByIds = Begin
// Arguments:
// Args.mids

var Msgs = API.messages.getById({ "message_ids": Args.mids });
var FMsgs = Msgs.items@.fwd_messages;
var ConvIds = Msgs.items@.peer_id;
var Idx = 0;
var Uids = [];
while (Idx < FMsgs.length) {
    var Jdx = 0;
    var CFMsgs = parseInt(FMsgs[Idx].length);
    while (Jdx < CFMsgs) {
        Uids.unshift(FMsgs[Idx][Jdx].from_id);
        Jdx = Jdx + 1;
    };
    Idx = Idx + 1;
};
var FUsers = API.users.get({ "user_ids": Uids, "name_case": "gen" });
var Conv = API.messages.getConversationsById({"peer_ids": ConvIds});
return { "Msgs": Msgs, "fwd_users": FUsers, "conv":Conv };
// Stored procedure name: RetrieveMessagesConversationByIds = End

// Stored procedure name: RetrieveUnreadMessages = Begin
// Arguments: no

var dlg = API.messages.getDialogs({ "count": 200 });
var users = API.friends.areFriends({ "user_ids": dlg.items@.message @.user_id});
var groups = API.groups.get();
return { "dialogs": dlg, "users": users, "groups": groups.items };
// Stored procedure name: RetrieveUnreadMessages = End

// Stored procedure name: RetrieveUnreadConversations = Begin
// Arguments: no

var dlg = API.messages.getConversations({ "count": 200 });
var users = API.friends.areFriends({ "user_ids": dlg.items@.conversation@.peer@.local_id});
var groups = API.groups.get();
return { "dialogs": dlg, "users": users, "groups": groups.items };
// Stored procedure name: RetrieveUnreadConversations = End


// Stored procedure name: RetrieveStatusMusic = Begin
// Arguments: 
// Args.statusmsg

// ver 1
var OldMsg = API.status.get();
API.status.set({ "text": "&#9835; " + Args.statusmsg });
return OldMsg;

// ver 2
var CntLmt = 100;
var OldMsg = API.status.get();
var Tracks = API.audio.search({ "q": Args.statusmsg, "count": CntLmt, "search_own": 1 });
var Cnt = Tracks.count;
if (Cnt > CntLmt) {
    Cnt = CntLmt;
}
if (Cnt == 0) {
    API.audio.setBroadcast();
} else {
    var i = 0;
    var j = 0;
    var Track = " ";
    while (i < Cnt) {
        Track = Tracks.items[i].artist + " - " + Tracks.items[i].title;
        if (Track == Args.statusmsg) {
            j = i;
        } i = i + 1;
    }
    Track = Tracks.items[j].owner_id + "_" + Tracks.items[j].id;
    API.audio.setBroadcast({ "audio": Track });
};
return OldMsg;

// ver 3
var CntLmt = 100;
var Track = " ";
var OldMsg = API.status.get();
var Tracks = API.audio.search({ "q": Args.statusmsg, "count": CntLmt, "search_own": 1 });
var Cnt = Tracks.count;
if (Cnt > CntLmt) {
    Cnt = CntLmt;
}
if (Cnt == 0) {
    Track = "&#9835; " + Args.statusmsg;
    API.status.set({ "text": Track });
} else {
    var i = 0;
    var j = -1;
    while (i < Cnt) {
        Track = Tracks.items[i].artist + " - " + Tracks.items[i].title;
        if (Track == Args.statusmsg) {
            j = i;
        }
        i = i + 1;
    }
    if (j == -1) {
        Track = "&#9835; " + Args.statusmsg;
        API.status.set({ "text": Track });
    } else {
        Track = Tracks.items[j].owner_id + "_" + Tracks.items[j].id;
    };
    API.audio.setBroadcast({ "audio": Track });
};
return OldMsg;
// Stored procedure name: RetrieveStatusMusic = End


// Stored procedure name: DestroyChat = Begin
// Arguments: 
// Args.chatid
// Args.userid

API.messages.removeChatUser({ "chat_id": Args.chatid, "user_id": Args.userid });
var Hist = API.messages.getHistory({ "chat_id": Args.chatid, "count": 200 });
var countMsg = Hist.count;
var itemsMsg = Hist.items@.id;
while (countMsg > 0) {
    API.messages.delete({ "message_ids": itemsMsg });
    Hist = API.messages.getHistory({ "chat_id": Args.chatid, "count": 200 });
    countMsg = Hist.count;
    itemsMsg = Hist.items@.id;
};
return 1;
// Stored procedure name: DestroyChat = End

// Stored procedure name: RetrieveChatInfo = Begin
// Arguments: 
// Args.chatid

// ver 1
var Info = API.messages.getChat({ "chat_id": Args.chatid });
var ChatUsers = API.messages.getChatUsers({ "chat_id": Args.chatid, "fields": "id,first_name,last_name" });
return { "info": Info, "users": ChatUsers;};


// ver 2
var Info = API.messages.getChat({ "chat_id": Args.chatid });
var ChatUsers = API.messages.getChatUsers({ "chat_id": Args.chatid, "fields": "id,first_name,last_name" });
var ChatMsg = API.messages.getHistory({ "chat_id": Args.chatid, "count": 20, "rev": 0 });
var UR = parseInt(ChatMsg.unread);
if (UR > 20) {
    if (UR > 200)
        UR = 200;
    ChatMsg = API.messages.getHistory({ "chat_id": Args.chatid, "count": UR, "rev": 0 });
};
var FMsgs = ChatMsg.items@.fwd_messages;
var Idx = 0;
var Uids = [];
while (Idx < FMsgs.length) {
    var Jdx = 0;
    var CFMsgs = parseInt(FMsgs[Idx].length);
    while (Jdx < CFMsgs) {
        Uids.unshift(FMsgs[Idx][Jdx].user_id);
        Jdx = Jdx + 1;
    };
    Idx = Idx + 1;
};
var FUsers = API.users.get({ "user_ids": Uids, "name_case": "gen" });
var MsgUsers = API.users.get({ "user_ids": ChatMsg.items@.user_id, "fields":"id,first_name,last_name"});

return { "info": Info, "users": ChatUsers, "msgs": ChatMsg, "fwd_users": FUsers, "msgs_users": MsgUsers };


// ver 3
var Info = API.messages.getChat({ "chat_id": Args.chatid });
var ChatUsers = API.messages.getConversationMembers({ "peer_id": 2000000000 + parseInt(Args.chatid), "fields": "id,first_name,last_name" });
var ChatMsg = API.messages.getHistory({ "chat_id": Args.chatid, "count": 20, "rev": 0 });
var UR = parseInt(ChatMsg.unread);
if (UR > 20) {
    if (UR > 200)
        UR = 200;
    ChatMsg = API.messages.getHistory({ "chat_id": Args.chatid, "count": UR, "rev": 0 });
};
var FMsgs = ChatMsg.items@.fwd_messages;
var Idx = 0;
var Uids = [];
var GUids =[];
while (Idx < FMsgs.length) {
    var Jdx = 0;
    var CFMsgs = parseInt(FMsgs[Idx].length);
    while (Jdx < CFMsgs) {
        if (FMsgs[Idx][Jdx].from_id>0) {
   Uids.unshift(FMsgs[Idx][Jdx].from_id);
  } else {
   GUids.unshift(-1*FMsgs[Idx][Jdx].from_id);
  };
        Jdx = Jdx + 1;
    };
    Idx = Idx + 1;
};
var FUsers = API.users.get({ "user_ids": Uids, "name_case": "gen" });
var GUsers = [];
if(GUids.length>0){
	GUsers = API.groups.getById({ "group_ids": GUids });
};
var MsgUsers = API.users.get({ "user_ids": ChatMsg.items@.from_id, "fields":"id,first_name,last_name"});

return { "info": Info, "users": ChatUsers, "msgs": ChatMsg, "fwd_users": FUsers + GUsers, "msgs_users": MsgUsers};
// Stored procedure name: RetrieveChatInfo = End

// Stored procedure name: DestroyKickChat = Begin
// Arguments: 
// Args.chatid

var Hist = API.messages.getHistory({ "chat_id": Args.chatid, "count": 200 });
var countMsg = Hist.count;
var itemsMsg = Hist.items@.id;
while (countMsg > 0) {
    API.messages.delete({ "message_ids": itemsMsg });
    Hist = API.messages.getHistory({ "chat_id": Args.chatid, "count": 200 });
    countMsg = Hist.count;
    itemsMsg = Hist.items@.id;
};
return 1;
// Stored procedure name: DestroyKickChat = End