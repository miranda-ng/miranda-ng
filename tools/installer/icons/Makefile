CC = ld
RC = windres

all:
	$(RC) --input-format rc --output-format coff -DICO_AIMHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_AIM.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_AIMLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_AIM.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_GGHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_GG.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_GGLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_GG.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_ICQHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_ICQ.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_ICQLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_ICQ.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_IRCHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_IRC.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_IRCLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_IRC.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_JABBERHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_JABBER.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_JABBERLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_JABBER.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_MSNHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_MSN.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_MSNLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_MSN.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_YAHOOHI -o resource.res -i resource.rc
	$(CC) -shared -o bin\hicolor\proto_YAHOO.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
	$(RC) --input-format rc --output-format coff -DICO_YAHOOLO -o resource.res -i resource.rc
	$(CC) -shared -o bin\locolor\proto_YAHOO.dll -T i386pe-ico.x -s -nostdlib resource.res
	del resource.res
