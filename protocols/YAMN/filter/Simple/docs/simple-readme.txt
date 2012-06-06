==========================
= Simple Filter for YAMN =
==========================

Q: What???
A: YAMN filter to classify incoming email.

Q: How?
A: Regarding what the email is from and finding it in the blacklist email file.

Q: Blacklist email file?
A: Yes. It is created by yourself and located in Miranda directory with name 'simplefilterdeny.txt'

Q: Created by myself?
A: Just create the file and write there your blacklist mails in every line.

Q: That's all?
A: Yes and no. You can specify spamlevel for each mail.

Q: Spamlevel?
A: Yes.
   1=notify, display with another color in mailbrowser
   2=do not notify, display with another color in mailbrowser
   3=delete, display in mailbrowser about deleted mail
   4=delete, do not display (mail's quick death, hehe)

Q: How to specify it?
A: After email press <tab> and write number 1-4. Note this is optional. If not defined, level 2 is default.

Q: Ok, that's easy.
A: Yes, this is the example:
<------ start of file ------>
nigeria@spamserver.com	2
cheapsoftware@junkmails.net	3
learnenglish@commercial.org
<------ end of file ------->

Q: Wait while. Ok, but it does not work.
A: Check if you have this plugin listed in Miranda/Options/Plugins/YAMN item as YAMN plugin.

Q: No, it is not listed in YAMN plugins.
A: Then check if the dll residents in Plugins/YAMN direcotry.

Q: This directory does not exists.
A: Create it and put the dll there. Restart Miranda.

Q: Hmmm, ok. But it is not still listed.
A: Your version of YAMN and filter does not match.

Q: And?
A: Try to look to http://www.majvan.host.sk/Projekty/YAMN for updates.