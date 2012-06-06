========================
= Base Filter for YAMN =
========================

Q: What???
A: YAMN filter to classify incoming email.

Q: How?
A: Finding occurency of defiend MIME header item and its value from blacklist file.

Q: Blacklist file?
A: Yes. It is created by yourself and located in Miranda directory with name 'basefilterdeny.txt'

Q: Created by myself?
A: Just create the file and write there your header MIME items and its values.

Q: What do you mean "header MIME items" and "its values"?
A: Every mail has header consisting of MIME items like "Subject" or "Return-Path".

Q: So I need to understand how the header looks like...
A: Yes, if you want to use this filter, you should. Header MIME is defined in RFC822 standard.

Q: Ok, I've just studied it. So how to set filter (write some rules to the blacklist file)?
A: Each line is one rule: write the exact item, press <tab>, press the substring of value needed to be found, press <tab>, define spamlevel and then press <Enter>.

Q: Spamlevel?
A: Yes.
   0=do not notify
   1=notify, display with another color in mailbrowser
   2=do not notify, display with another color in mailbrowser
   3=delete, display in mailbrowser about deleted mail
   4=delete, do not display (mail's quick death, hehe)

Q: So the rule has 3 parameters, that's it?
A: Yes. This is the example:
<------ start of file ------>
From	CrazyMail	1
X-Importance	low	0
Subject	LinuxMailList	0
Return-Path	cheapsoftware@junkmails.net	2
X-TextClassification	spam	3
<------ end of file ------->

Q: Wait while. Ok, but it does not work.
A: Check if you have this plugin listed in Miranda/Options/Plugins/YAMN item

Q: No, it is not listed in YAMN plugins.
A: Then check if the dll residents in Plugins/YAMN direcotry.

Q: This directory does not exists.
A: Create it and put the dll there. Restart Miranda.

Q: Hmmm, ok. But it is not still listed.
A: Your version of YAMN and filter does not match.

Q: And?
A: Try to look to http://www.majvan.host.sk/Projekty/YAMN for updates.

Q: Now, it is listed, but does not work anyway.
A: Try to download debug version from YAMN homepage, if you are not using it (the name of filter must contain the word "debug")

Q: What does debug version do?
A: It creates debug log file in Miranda home directory where you can browse how does filter mark mails.