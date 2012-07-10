Skins plugin
----------------

CAUTION: THIS IS AN ALPHA STAGE PLUGIN. IT CAN DO VERY BAD THINGS. USE AT YOUR OWN RISK.

This is a service plugin to allow using different skins. It uses javascript as the language the skins are written, thus allowing a lot of flexibility. 

Keep in mind that this is an initial version of the plugin. It is intended to grow a loot (its to do list is bigger than its features list :P ). Currently it allows only calculating the position of elements.

It works based in the consept of a skin beeing a group of small skins (one for each client). The skins are inside the dir <Miranda>\Skins\<Skin name> , and inside that folder, each client skin has the name <Client>.msk

Inside the zip there is also a version of MyDetails using this plugin.

Many thanks to the v8 team for the javascript engine implementation.

This needs Miranda 0.8 to work.

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?p=172392


TODO:
 - Allow changing skin without restart
 - Add support for handling skin packs
 - Add support for emoticons in text fields
 - Add code to draw elements on screen
 - Add service to create a new frame only based on the fields
 - Allow setting variables by the users of the plugin


KNOWN PROBLEMS:
 - It seems to have a memory leak (last time I debugged it, it was inside v8 - maybe I just don't understant its garbage collector?)
