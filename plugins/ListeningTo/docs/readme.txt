ListeningTo plugin
------------------

This plugins allows to set/remove your listening info to protocols that support it. It also has basic support for getting this info from the players.

This means that it set the user listening to information, not the info from your contacts. To see the info from your contacts you need a clist that supports it (only modern now) or a plugin like tipper (for a contact this info is stored inside <ProtocolName>/ListeningTo db key of the contact).

Any protocol that support some services (in m_proto_listeningto.h) can interface with this plugin. By now it means JGmail unicode version, MSN and Jabber.

This protocol also save the listening info for the user (you!) into <ProtocolName>/ListeningTo db key.

This plugin is disbled by default. To enable it you need to use the main menu: Listening to->Send to all protocols or use My Details (the option is in popup menu).

If you want this to implement some other player, I need a link to a page that explain how to do it, getting all the data, if possible without polling. (iTunes do it now but wont do in future). If you need more options, etc, use WATrack... This plugin is meant to be kept small.

One last thing: when reporting bugs, please post the name and version of the player you are using.

It also support Variables plugin to format the info, but it has to be the lastest version of the plugin. This version allows to use temporary variables in the replacement too, so you can use %artist%.

Todo:
- Better support for iTunes (it is not good now)

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=10912

This plugin requires at least miranda 0.6, but is fully functional only in miranda 0.7