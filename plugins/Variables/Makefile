CC = winegcc
CPP = wineg++
WRC = wrc

CPPFLAGS =	-I/usr/local/include/wine/windows \
		-I/usr/local/include/wine/msvcrt \
		-I./include \
		-I./src/core

LNK_COMMON = -L/usr/local/lib/wine \
		-lwine \
		-lmsvcrt \
		-lgdi32 \
		-lshell32 \
		-lole32 \
		-lcomctl32 \
		-lcomdlg32 

LNK_MAINLIBS =	-lversion \
		-lws2_32 \
		-lwinmm \
		-loleaut32 \
		-luuid 

VARIABLESOBJS = ./plugins/Variables/main.c ./plugins/Variables/parse_miranda.c ./plugins/Variables/options.c ./plugins/Variables/parse_alias.c ./plugins/Variables/parse_regexp.c ./plugins/Variables/tokenregister.c ./plugins/Variables/contact.c ./plugins/Variables/parse_external.c ./plugins/Variables/parse_str.c ./plugins/Variables/variables.c ./plugins/Variables/parse_inet.c ./plugins/Variables/parse_system.c ./plugins/Variables/parse_logic.c ./plugins/Variables/parse_variables.c ./plugins/Variables/help.c ./plugins/Variables/parse_math.c ./plugins/helpers/db_helpers.c ./plugins/helpers/gen_helpers.c

all: plugin-variables

plugin-variables:  $(VARIABLESOBJS)
	cd ./plugins/Variables && $(WRC) -I. resource.rc
	$(CC) $(CPPFLAGS) -DWINE -shared ./plugins/Variables/variables.spec $(VARIABLESOBJS) ./plugins/Variables/resource.res $(LNK_COMMON) -o variables.dll

.c.o:
	$(CC) -c $(CPPFLAGS) -DHAVE_CONFIG_H -o $@ $<

.cpp.o:
	$(CPP) -c $(CPPFLAGS) -o $@ $<

clean:
	rm -f Variables.so *~ 
	find ./plugins/Variables -name *~  -exec rm -f {} \;
	find ./plugins/Variables -name *.o  -exec rm -f {} \;
	find ./plugins/Variables -name *.res  -exec rm -f {} \;
	find ./plugins/Variables -name *.orig  -exec rm -f {} \;
	find ./plugins/Variables -name *.rej  -exec rm -f {} \;


