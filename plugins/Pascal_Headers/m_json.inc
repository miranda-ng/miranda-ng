{$IFNDEF MIM_LIBJSON}
{$DEFINE MIM_LIBJSON}

type
 PJSONNODE = ^JSONNODE;
 JSONNODE = pointer; 
 TJSONNODE_ITERATOR = pointer; //^^JSONNODE; // typedef JSONNODE** JSONNODE_ITERATOR;

// This function get the service interface for direct calling
// wParam=(WPARAM)(LPJSONSERVICEINTERFACE)
// lParam=0
// Return value ALWAYS 0

type
  PJSONSERVICEINTERFACE = ^TJSONSERVICEINTERFACE;
  TJSONSERVICEINTERFACE = record
    free              : procedure (str:pointer);cdecl;
    delete_           : procedure (node:PJSONNODE);cdecl;
    free_all          : procedure ;cdecl;
    delete_all        : procedure ;cdecl;
    parse             : function (json:PAnsiChar):PJSONNODE;cdecl;
    strip_white_space : function (json:PAnsiChar):PAnsiChar;cdecl;
    validate          : function (json:PAnsiChar):PJSONNODE;cdecl;

    new_a     : function (name:PAnsiChar; value:PAnsiChar):PJSONNODE;cdecl;
    new_i     : function (name:PAnsiChar; value:long):PJSONNODE;cdecl;
    new_f     : function (name:PAnsiChar; value:double):PJSONNODE;cdecl;
    new_b     : function (name:PAnsiChar; value:int):PJSONNODE;cdecl;
    new_      : function (_type:char):PJSONNODE;cdecl;

    copy      : function (orig:PJSONNODE):PJSONNODE;cdecl;
    duplicate : function (orig:PJSONNODE):PJSONNODE;cdecl;

    set_a     : procedure (node:PJSONNODE; value:PAnsiChar);cdecl;
    set_i     : procedure (node:PJSONNODE; value:long);cdecl;
    set_f     : procedure (node:PJSONNODE; value:double);cdecl;
    set_b     : procedure (node:PJSONNODE; value:int);cdecl;
    set_n     : procedure (node:PJSONNODE; orig:PJSONNODE);cdecl;

    _type       : function (node:PJSONNODE):char;cdecl;
    size        : function (node:PJSONNODE):uint;cdecl;
    empty       : function (node:PJSONNODE):int;cdecl;
    name        : function (node:PJSONNODE):PAnsiChar;cdecl;
    get_comment : function (node:PJSONNODE):PAnsiChar;cdecl;

    as_string : function (node:PJSONNODE):PAnsiChar;cdecl;
    as_int    : function (node:PJSONNODE):long;cdecl;
    as_float  : function (node:PJSONNODE):double;cdecl;
    as_bool   : function (node:PJSONNODE):int;cdecl;
    as_node   : function (node:PJSONNODE):PJSONNODE;cdecl;
    as_array  : function (node:PJSONNODE):PJSONNODE;cdecl;
    as_binary : function (node:PJSONNODE; size:Pulong):pointer;cdecl;

    write           : function (node:PJSONNODE):PAnsiChar;cdecl;
    write_formatted : function (node:PJSONNODE):PAnsiChar;cdecl;
    set_name        : procedure (node:PJSONNODE; name:PAnsiChar);cdecl;
    set_comment     : procedure (node:PJSONNODE; comment:PAnsiChar);cdecl;
    clear           : procedure (node:PJSONNODE);cdecl;
    nullify         : procedure (node:PJSONNODE);cdecl;
    swap            : procedure (node:PJSONNODE; node2:PJSONNODE);cdecl;
    merge           : procedure (node:PJSONNODE; node2:PJSONNODE);cdecl;
    preparse        : procedure (node:PJSONNODE);cdecl;
    set_binary      : procedure (node:PJSONNODE; data:pointer; length:ulong);cdecl;
    cast            : procedure (node:PJSONNODE; _type:AnsiChar);cdecl;
    reserve         : procedure (node:PJSONNODE; siz:uint);cdecl;
    at              : function (node:PJSONNODE; pos:uint):PJSONNODE;cdecl;
    get             : function (node:PJSONNODE; name:PAnsiChar):PJSONNODE;cdecl;
    get_nocase      : function (node:PJSONNODE; name:PAnsiChar):PJSONNODE;cdecl;
    pop_back_nocase : function (node:PJSONNODE; name:PAnsiChar):PJSONNODE;cdecl;
    push_back       : procedure (node:PJSONNODE; node2:PJSONNODE);cdecl;
    pop_back_at     : function (node:PJSONNODE; pos:uint):PJSONNODE;cdecl;
    pop_back        : function (node:PJSONNODE; name:PAnsiChar):PJSONNODE;cdecl;
    find            : function (node:PJSONNODE; name:PAnsiChar):TJSONNODE_ITERATOR;cdecl;
    find_nocase     : function (node:PJSONNODE; name:PAnsiChar):TJSONNODE_ITERATOR;cdecl;
    erase           : function (node:PJSONNODE; it:TJSONNODE_ITERATOR):TJSONNODE_ITERATOR;cdecl;
    erase_multi     : function (node:PJSONNODE; start:TJSONNODE_ITERATOR;
                                _end:TJSONNODE_ITERATOR):TJSONNODE_ITERATOR;cdecl;
    insert          : function (node:PJSONNODE; it:TJSONNODE_ITERATOR; node2:PJSONNODE):TJSONNODE_ITERATOR;cdecl;
    insert_multi    : function (node:PJSONNODE; it:TJSONNODE_ITERATOR; start:TJSONNODE_ITERATOR;
                                _end:TJSONNODE_ITERATOR):TJSONNODE_ITERATOR;cdecl;

    _begin : function (node:PJSONNODE):TJSONNODE_ITERATOR;cdecl;
    _end   : function (node:PJSONNODE):TJSONNODE_ITERATOR;cdecl;
    equal  : function (node:PJSONNODE; node2:PJSONNODE):int;cdecl;
  end;

const
  MS_JSON_GETINTERFACE:PAnsiChar = 'JSON/GetInterface';

{$ENDIF}
