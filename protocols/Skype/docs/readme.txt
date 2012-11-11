for compiling plugin with your own skipekit you need to create Original_key.h with defining like that
#define MY_KEY "MY_KEY"
then you need to pack your *.crt file with keypacker utility. Also utlity create key.h file with encoded key. Key.h file included into Skype project.
Names and pathes of directories and cert file you can see in source code.