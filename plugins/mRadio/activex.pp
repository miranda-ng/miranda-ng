Unit ActiveX;

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: objidl.idl
//
//  Header translation by Marco van de Voort for Free Pascal Platform
//  SDK dl'ed January 2002
//
//--------------------------------------------------------------------------

{$Mode objfpc}

{$ifndef NO_SMART_LINK}
{$smartlink on}
{$endif}

Interface

Uses variants,Windows,ctypes,types;


type
{ extra types }
   TOleChar = Types.TOleChar;
   POleStr = Types.POleStr;
   PPOleStr = Types.PPOleStr;
   TBStr = POleStr;
   TBStrList = array[0..(high(integer) div sizeof(TBSTR))-1] of TBstr;
   PBStrList = ^TBStrList;
   POleStrList = ^TOleStrList;
   TOleStrList = array[0..(high(integer) div sizeof(POleStr))-1] of POleStr;

   PBStr = ^TBStr;
   TOleEnum = type LongWord;
   LargeInt = Types.LargeInt;
   LargeUInt = Types.LargeUInt;
   PLargeInt = Types.PLargeInt;
   PLargeUInt = Types.PLargeUInt;
   FMTID  =  TGUID;
   pFMTID = pGUID;

   { Glue types, should be linked to the proper windows unit types}
   Size_t              = DWord;       {??, probably, like Unix, typecastable to pointer?!?}
   OleChar             = WChar;
   LPOLESTR            = ^OLECHAR;
   POLECHAR            = LPOLESTR;
   PLPOLESTR	       = ^LPOLESTR;
   PROPID = ULONG;
   TPROPID= PROPID;
   PPROPID= ^PROPID;
   VARIANT_BOOL        = wordbool;
   _VARIANT_BOOL       = VARIANT_BOOL;
   PVARIANT_BOOL       = ^VARIANT_BOOL;
   CY		       = CURRENCY;
   DATE	               = DOUBLE;
   BSTR	               = POLESTR;
   TOleDate	       = DATE;
   POleDate	       = ^TOleDate;	
   OLE_HANDLE	       = UINT;
   LPOLE_HANDLE        = ^OLE_HANDLE;
   OLE_COLOR	       = DWORD;
   LPOLE_COLOR         = ^OLE_COLOR;
   TOleHandle          = OLE_HANDLE;
   POleHandle          = LPOLE_HANDLE;
   TOleColor           = OLE_COLOR;
   POleColor           = LPOle_Color;

CONST
   GUID_NULL  : TGUID =  '{00000000-0000-0000-0000-000000000000}';

     // bit flags for IExternalConnection
CONST
    EXTCONN_STRONG              = $0001;   // strong connection
    EXTCONN_WEAK                = $0002;  // weak connection (table, container)
    EXTCONN_CALLABLE            = $0004;  // table .vs. callable
     {Bind Flags}
    BIND_MAYBOTHERUSER          = 1;
    BIND_JUSTTESTEXISTENCE      = 2;


    MKSYS_NONE                  = 0;
    MKSYS_GENERICCOMPOSITE      = 1;
    MKSYS_FILEMONIKER           = 2;
    MKSYS_ANTIMONIKER           = 3;
    MKSYS_ITEMMONIKER           = 4;
    MKSYS_POINTERMONIKER        = 5;
//  MKSYS_URLMONIKER            = 6;
    MKSYS_CLASSMONIKER          = 7;
    MKSYS_OBJREFMONIKER         = 8;
    MKSYS_SESSIONMONIKER        = 9;

    // system moniker types; returned from IsSystemMoniker.

    MKRREDUCE_ONE               = 3 SHL 16;
    MKRREDUCE_TOUSER            = 2 SHL 16;
    MKRREDUCE_THROUGHUSER       = 1 SHL 16;
    MKRREDUCE_ALL               = 0;

    // Storage element types
    STGTY_STORAGE               = 1;
    STGTY_STREAM                = 2;
    STGTY_LOCKBYTES             = 3;
    STGTY_PROPERTY              = 4;

    STREAM_SEEK_SET             = 0;
    STREAM_SEEK_CUR             = 1;
    STREAM_SEEK_END             = 2;

    LOCK_WRITE                  = 1;
    LOCK_EXCLUSIVE              = 2;
    LOCK_ONLYONCE               = 4;

    //Advise Flags
    ADVF_NODATA                 = 1;
    ADVF_PRIMEFIRST             = 2;
    ADVF_ONLYONCE               = 4;
    ADVF_DATAONSTOP             = 64;
    ADVFCACHE_NOHANDLER         = 8;
    ADVFCACHE_FORCEBUILTIN      = 16;
    ADVFCACHE_ONSAVE            = 32;


//****************************************************************************
//*  Notification Interfaces
//****************************************************************************/

    TYMED_HGLOBAL               = 1;
    TYMED_FILE                  = 2;
    TYMED_ISTREAM               = 4;
    TYMED_ISTORAGE              = 8;
    TYMED_GDI                   = 16;
    TYMED_MFPICT                = 32;
    TYMED_ENHMF                 = 64;
    TYMED_NULL                  = 0;

// Object Definitions for EnumObjects()
    OBJ_PEN                     = 1;
    OBJ_BRUSH                   = 2;
    OBJ_DC                      = 3;
    OBJ_METADC                  = 4;
    OBJ_PAL                     = 5;
    OBJ_FONT                    = 6;
    OBJ_BITMAP                  = 7;
    OBJ_REGION                  = 8;
    OBJ_METAFILE                = 9;
    OBJ_MEMDC                   = 10;
    OBJ_EXTPEN                  = 11;
    OBJ_ENHMETADC               = 12;
    OBJ_ENHMETAFILE             = 13;
    DATADIR_GET                 = 1;
    DATADIR_SET                 = 2;

// call type used by IMessageFilter::HandleIncomingMessage

    CALLTYPE_TOPLEVEL           = 1;      // toplevel call - no outgoing call
    CALLTYPE_NESTED             = 2;      // callback on behalf of previous outgoing call - should always handle
    CALLTYPE_ASYNC              = 3;      // aysnchronous call - can NOT be rejected
    CALLTYPE_TOPLEVEL_CALLPENDING = 4;  // new toplevel call with new LID
    CALLTYPE_ASYNC_CALLPENDING  = 5;   // async call - can NOT be rejected

// status of server call - returned by IMessageFilter::HandleIncomingCall
// and passed to  IMessageFilter::RetryRejectedCall

    SERVERCALL_ISHANDLED        = 0;
    SERVERCALL_REJECTED         = 1;
    SERVERCALL_RETRYLATER       = 2;

// Pending type indicates the level of nesting

    PENDINGTYPE_TOPLEVEL        = 1; // toplevel call
    PENDINGTYPE_NESTED          = 2;  // nested call

// return values of MessagePending

    PENDINGMSG_CANCELCALL       = 0; // cancel the outgoing call
    PENDINGMSG_WAITNOPROCESS    = 1; // wait for the return and don't dispatch the message
    PENDINGMSG_WAITDEFPROCESS   = 2;  // wait and dispatch the message

    EOAC_NONE                   = $0;
    EOAC_MUTUAL_AUTH            = $1;
    EOAC_STATIC_CLOAKING        = $20;
    EOAC_DYNAMIC_CLOAKING       = $40;
    EOAC_ANY_AUTHORITY          = $80;
    EOAC_MAKE_FULLSIC           = $100;
    EOAC_DEFAULT                = $800;

    // These are only valid for CoInitializeSecurity
    EOAC_SECURE_REFS            = $2;
    EOAC_ACCESS_CONTROL         = $4;
    EOAC_APPID                  = $8;
    EOAC_DYNAMIC                = $10;
    EOAC_REQUIRE_FULLSIC        = $200;
    EOAC_AUTO_IMPERSONATE       = $400;
    EOAC_NO_CUSTOM_MARSHAL      = $2000;
    EOAC_DISABLE_AAA            = $1000;

//****************************************************************************
//* ICOMThreadingInfo and enums
//****************************************************************************/

    APTTYPE_CURRENT             = -1;
    APTTYPE_STA                 = 0;
    APTTYPE_MTA                 = 1;
    APTTYPE_NA                  = 2;
    APTTYPE_MAINSTA             = 3;

    THDTYPE_BLOCKMESSAGES       = 0;
    THDTYPE_PROCESSMESSAGES     = 1;

    DCOM_NONE                   = $0;
    DCOM_CALL_COMPLETE          = $1;
    DCOM_CALL_CANCELED          = $2;

    COMBND_RPCTIMEOUT           = $1;  // Rpc transport-specific timeout.

//************************* Misc Enums wtypes.h ***********************************/

// Common typdefs used in API paramaters, gleamed from compobj.h

// memory context values; passed to CoGetMalloc

Const
    MEMCTX_TASK                 = 1;          // task (private) memory
    MEMCTX_SHARED               = 2;          // shared memory (between processes)
    MEMCTX_MACSYSTEM            = 3;          // on the mac, the system heap
    // these are mostly for internal use...
    MEMCTX_UNKNOWN              = -1;         // unknown context (when asked about it)
    MEMCTX_SAME                 = -2;         // same context (as some other pointer)


// For IRunningObjectTable::Register
    ROTFLAGS_REGISTRATIONKEEPSALIVE  = $1;
    ROTFLAGS_ALLOWANYCLIENT          = $2;

// Maximum size of comparison buffer for IROTData::GetComparisonData
    ROT_COMPARE_MAX                  = 2048;


// class context: used to determine what scope and kind of class object to use
// NOTE: this is a bitwise enum

    CLSCTX_INPROC_SERVER        = $0001;     // server dll (runs in same process as caller)
    CLSCTX_INPROC_HANDLER       = $0002;     // handler dll (runs in same process as caller)
    CLSCTX_LOCAL_SERVER         = $0004;     // server exe (runs on same machine; diff proc)
    CLSCTX_INPROC_SERVER16      = $0008;     // 16-bit server dll (runs in same process as caller)
    CLSCTX_REMOTE_SERVER        = $0010;     // remote server exe (runs on different machine)
    CLSCTX_INPROC_HANDLER16     = $0020;     // 16-bit handler dll (runs in same process as caller)
    CLSCTX_INPROC_SERVERX86     = $0040;     // Wx86 server dll (runs in same process as caller)
    CLSCTX_INPROC_HANDLERX86    = $0080;     // Wx86 handler dll (runs in same process as caller)
    CLSCTX_ESERVER_HANDLER      = $0100;     // handler dll (runs in the server process)
    CLSCTX_RESERVED =$0200;                  // reserved
    CLSCTX_NO_CODE_DOWNLOAD     = $0400;     // disallow code download from the Directory Service (if any) or the internet   -rahulth
    CLSCTX_NO_WX86_TRANSLATION  = $0800;
    CLSCTX_NO_CUSTOM_MARSHAL    = $1000;
    CLSCTX_ENABLE_CODE_DOWNLOAD = $2000;     // allow code download from the Directory Service (if any) or the internet
    CLSCTX_NO_FAILURE_LOG       = $04000;    // do not log messages about activation failure (should one occur) to Event Log
    CLSCTX_DISABLE_AAA          = $08000;    // Disable EOAC_DISABLE_AAA capability for this activation only
    CLSCTX_ENABLE_AAA           = $10000;    // Enable EOAC_DISABLE_AAA capability for this activation only
    CLSCTX_FROM_DEFAULT_CONTEXT = $20000;    // Begin this activation from the default context of the current apartment
    CLSCTX_INPROC               = (CLSCTX_INPROC_SERVER OR CLSCTX_INPROC_HANDLER);
// With DCOM, CLSCTX_REMOTE_SERVER should be included
    CLSCTX_ALL                  = (CLSCTX_INPROC_SERVER OR CLSCTX_INPROC_HANDLER OR CLSCTX_LOCAL_SERVER {$ifdef Remote} OR CLSCTX_REMOTE_SERVER {$endif});
    CLSCTX_SERVER               = (CLSCTX_INPROC_SERVER OR CLSCTX_LOCAL_SERVER {$ifdef Remote} OR CLSCTX_REMOTE_SERVER {$endif});


// marshaling flags; passed to CoMarshalInterface
    MSHLFLAGS_NORMAL            = 0;   // normal marshaling via proxy/stub
    MSHLFLAGS_TABLESTRONG       = 1;   // keep object alive; must explicitly release
    MSHLFLAGS_TABLEWEAK         = 2;   // doesn't hold object alive; still must release
    MSHLFLAGS_NOPING            = 4;   // remote clients dont 'ping' to keep objects alive
    MSHLFLAGS_RESERVED1         = 8;   // reserved
    MSHLFLAGS_RESERVED2         = 16;  // reserved
    MSHLFLAGS_RESERVED3         = 32;  // reserved
    MSHLFLAGS_RESERVED4         = 64;  // reserved

// marshal context: determines the destination context of the marshal operation

    MSHCTX_LOCAL                = 0;   // unmarshal context is local (eg.shared memory)
    MSHCTX_NOSHAREDMEM          = 1;   // unmarshal context has no shared memory access
    MSHCTX_DIFFERENTMACHINE     = 2;   // unmarshal context is on a different machine
    MSHCTX_INPROC               = 3;   // unmarshal context is on different thread
    MSHCTX_CROSSCTX             = 4;   // unmarshal context is on different context

// #########################################################################
//
//  VARTYPE
//
// #########################################################################


{
    VARENUM usage key,

    * [V] - may appear in a VARIANT
    * [T] - may appear in a TYPEDESC
    * [P] - may appear in an OLE property set
    * [S] - may appear in a Safe Array


     VT_EMPTY            [V]   [P]     nothing
     VT_NULL             [V]   [P]     SQL style Null
     VT_I2               [V][T][P][S]  2 byte signed int
     VT_I4               [V][T][P][S]  4 byte signed int
     VT_R4               [V][T][P][S]  4 byte real
     VT_R8               [V][T][P][S]  8 byte real
     VT_CY               [V][T][P][S]  currency
     VT_DATE             [V][T][P][S]  date
     VT_BSTR             [V][T][P][S]  OLE Automation string
     VT_DISPATCH         [V][T]   [S]  IDispatch *
     VT_ERROR            [V][T][P][S]  SCODE
     VT_BOOL             [V][T][P][S]  True=-1, False=0
     VT_VARIANT          [V][T][P][S]  VARIANT *
     VT_UNKNOWN          [V][T]   [S]  IUnknown *
     VT_DECIMAL          [V][T]   [S]  16 byte fixed point
     VT_RECORD           [V]   [P][S]  user defined type
     VT_I1               [V][T][P][s]  signed char
     VT_UI1              [V][T][P][S]  unsigned char
     VT_UI2              [V][T][P][S]  unsigned short
     VT_UI4              [V][T][P][S]  unsigned long
     VT_I8                  [T][P]     signed 64-bit int
     VT_UI8                 [T][P]     unsigned 64-bit int
     VT_INT              [V][T][P][S]  signed machine int
     VT_UINT             [V][T]   [S]  unsigned machine int
     VT_INT_PTR             [T]        signed machine register size width
     VT_UINT_PTR            [T]        unsigned machine register size width
     VT_VOID                [T]        C style void
     VT_HRESULT             [T]        Standard return type
     VT_PTR                 [T]        pointer type
     VT_SAFEARRAY           [T]        (use VT_ARRAY in VARIANT)
     VT_CARRAY              [T]        C style array
     VT_USERDEFINED         [T]        user defined type
     VT_LPSTR               [T][P]     null terminated string
     VT_LPWSTR              [T][P]     wide null terminated string
     VT_FILETIME               [P]     FILETIME
     VT_BLOB                   [P]     Length prefixed bytes
     VT_STREAM                 [P]     Name of the stream follows
     VT_STORAGE                [P]     Name of the storage follows
     VT_STREAMED_OBJECT        [P]     Stream contains an object
     VT_STORED_OBJECT          [P]     Storage contains an object
     VT_VERSIONED_STREAM       [P]     Stream with a GUID version
     VT_BLOB_OBJECT            [P]     Blob contains an object
     VT_CF                     [P]     Clipboard format
     VT_CLSID                  [P]     A Class ID
     VT_VECTOR                 [P]     simple counted array
     VT_ARRAY            [V]           SAFEARRAY*
     VT_BYREF            [V]           void* for local use
     VT_BSTR_BLOB                      Reserved for system use
}

// VARENUM's

    VT_EMPTY                    = 0;
    VT_NULL                     = 1;
    VT_I2                       = 2;
    VT_I4                       = 3;
    VT_R4                       = 4;
    VT_R8                       = 5;
    VT_CY                       = 6;
    VT_DATE                     = 7;
    VT_BSTR                     = 8;
    VT_DISPATCH                 = 9;
    VT_ERROR                    = 10;
    VT_BOOL                     = 11;
    VT_VARIANT                  = 12;
    VT_UNKNOWN                  = 13;
    VT_DECIMAL                  = 14;
// VBA reserves 15 for future use
    VT_I1                       = 16;
    VT_UI1                      = 17;
    VT_UI2                      = 18;
    VT_UI4                      = 19;
    VT_I8                       = 20;
    VT_UI8                      = 21;
    VT_INT                      = 22;
    VT_UINT                     = 23;
    VT_VOID                     = 24;
    VT_HRESULT                  = 25;
    VT_PTR                      = 26;
    VT_SAFEARRAY                = 27;
    VT_CARRAY                   = 28;
    VT_USERDEFINED              = 29;
    VT_LPSTR                    = 30;
    VT_LPWSTR                   = 31;
// VBA reserves 32-35 for future use
    VT_RECORD                   = 36;
    VT_INT_PTR                  = 37;
    VT_UINT_PTR                 = 38;

    VT_FILETIME                 = 64;
    VT_BLOB                     = 65;
    VT_STREAM                   = 66;
    VT_STORAGE                  = 67;
    VT_STREAMED_OBJECT          = 68;
    VT_STORED_OBJECT            = 69;
    VT_BLOB_OBJECT              = 70;
    VT_CF                       = 71;
    VT_CLSID                    = 72;
    VT_VERSIONED_STREAM         = 73;

    VT_BSTR_BLOB                = $0fff;

    VT_VECTOR                   = $1000;
    VT_ARRAY                    = $2000;
    VT_BYREF                    = $4000;
    VT_RESERVED                 = $8000;

    VT_ILLEGAL                  = $ffff;
    VT_ILLEGALMASKED            = $0fff;
    VT_TYPEMASK                 = $0fff;



//
// Common typedefs for paramaters used in data view API's, gleamed
// from dvobj.h
//

// Data/View aspect; specifies the desired aspect of the object when
// drawing or getting data.

    DVASPECT_CONTENT            = 1;
    DVASPECT_THUMBNAIL          = 2;
    DVASPECT_ICON               = 4;
    DVASPECT_DOCPRINT           = 8;

//****** Storage types *************************************************


// Storage commit types

    STGC_DEFAULT                = 0;
    STGC_OVERWRITE              = 1;
    STGC_ONLYIFCURRENT          = 2;
    STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE = 4;
    STGC_CONSOLIDATE            = 8;

    STGMOVE_MOVE                = 0;
    STGMOVE_COPY                = 1;
    STGMOVE_SHALLOWCOPY         = 2;

    STATFLAG_DEFAULT            = 0;
    STATFLAG_NONAME             = 1;
    STATFLAG_NOOPEN             = 2;

// #########################################################################
//
//   Constants for the call context
//

    WDT_INPROC_CALL             = ULONG($48746457);
    WDT_REMOTE_CALL             = ULONG($52746457);
    WDT_INPROC64_CALL           = ULONG($50746457);

    COLE_DEFAULT_PRINCIPAL {: pOleStr?} = pOleStr(-1);
    COLE_DEFAULT_AUTHINFO  {: pointer?} = pointer(-1);

// DISPID reserved to indicate an \"unknown\" name
// only reserved for data members (properties); reused as a method dispid below

    DISPID_UNKNOWN              = -1;

// DISPID reserved for the \"value\" property

    DISPID_VALUE                = 0;

// The following DISPID is reserved to indicate the param")
// that is the right-hand-side (or \"put\" value) of a PropertyPut")

    DISPID_PROPERTYPUT          = -3;

// DISPID reserved for the standard \"NewEnum\" method

    DISPID_NEWENUM              = -4;

// DISPID reserved for the standard \"Evaluate\" method

    DISPID_EVALUATE             = -5;
    DISPID_CONSTRUCTOR          = -6;
    DISPID_DESTRUCTOR           = -7;
    DISPID_COLLECT              = -8;

    DISPATCH_METHOD         = $1;
    DISPATCH_PROPERTYGET    = $2;
    DISPATCH_PROPERTYPUT    = $4;
    DISPATCH_PROPERTYPUTREF = $8;

    USERCLASSTYPE_FULL      = 1;
    USERCLASSTYPE_SHORT     = 2;
    USERCLASSTYPE_APPNAME   = 3;

// The range -500 through -999 is reserved for Controls
// The range 0x80010000 through 0x8001FFFF is reserved for Controls
// The range -5000 through -5499 is reserved for ActiveX Accessability
// The range -2000 through -2499 is reserved for VB5
// The range -3900 through -3999 is reserved for Forms
// The range -5500 through -5550 is reserved for Forms
// The remainder of the negative DISPIDs are reserved for future use


    DESCKIND_NONE               = 0;
    DESCKIND_FUNCDESC           = 1;
    DESCKIND_VARDESC            = 2;
    DESCKIND_TYPECOMP           = 3;
    DESCKIND_IMPLICITAPPOBJ     = 4;
    DESCKIND_MAX                = 5;


    SYS_WIN16                   = 0;
    SYS_WIN32                   = 1;
    SYS_MAC                     = 2;
    SYS_WIN64                   = 3;

    REGKIND_DEFAULT		= 0;
    REGKIND_REGISTER		= 1;
    REGKIND_NONE		= 2;

    INTERFACESAFE_FOR_UNTRUSTED_CALLER = $00000001;	// Caller of interface may be untrusted
    INTERFACESAFE_FOR_UNTRUSTED_DATA   = $00000002;	// Data passed into interface may be untrusted
    INTERFACE_USES_DISPEX              = $00000004;	// Object knows to use IDispatchEx
    INTERFACE_USES_SECURITY_MANAGER    = $00000008;	// Object knows to use IInternetHostSecurityManager

    LIBFLAG_FRESTRICTED         = $01;
    LIBFLAG_FCONTROL            = $02;
    LIBFLAG_FHIDDEN             = $04;
    LIBFLAG_FHASDISKIMAGE       = $08;
    TYPEFLAG_FAPPOBJECT         = $01;
    TYPEFLAG_FCANCREATE         = $02;
    TYPEFLAG_FLICENSED          = $04;
    TYPEFLAG_FPREDECLID         = $08;
    TYPEFLAG_FHIDDEN            = $10;
    TYPEFLAG_FCONTROL           = $20;
    TYPEFLAG_FDUAL              = $40;
    TYPEFLAG_FNONEXTENSIBLE     = $80;
    TYPEFLAG_FOLEAUTOMATION     = $100;
    TYPEFLAG_FRESTRICTED        = $200;
    TYPEFLAG_FAGGREGATABLE      = $400;
    TYPEFLAG_FREPLACEABLE       = $800;
    TYPEFLAG_FDISPATCHABLE      = $1000;
    TYPEFLAG_FREVERSEBIND       = $2000;
    TYPEFLAG_FPROXY             = $4000;

    FUNCFLAG_FRESTRICTED        = $1;
    FUNCFLAG_FSOURCE            = $2;
    FUNCFLAG_FBINDABLE          = $4;
    FUNCFLAG_FREQUESTEDIT       = $8;
    FUNCFLAG_FDISPLAYBIND       = $10;
    FUNCFLAG_FDEFAULTBIND       = $20;
    FUNCFLAG_FHIDDEN            = $40;
    FUNCFLAG_FUSESGETLASTERROR  = $80;
    FUNCFLAG_FDEFAULTCOLLELEM   = $100;
    FUNCFLAG_FUIDEFAULT         = $200;
    FUNCFLAG_FNONBROWSABLE      = $400;
    FUNCFLAG_FREPLACEABLE       = $800;
    FUNCFLAG_FIMMEDIATEBIND     = $1000;

    VARFLAG_FREADONLY           = $1;
    VARFLAG_FSOURCE             = $2;
    VARFLAG_FBINDABLE           = $4;
    VARFLAG_FREQUESTEDIT        = $8;
    VARFLAG_FDISPLAYBIND        = $10;
    VARFLAG_FDEFAULTBIND        = $20;
    VARFLAG_FHIDDEN             = $40;
    VARFLAG_FRESTRICTED         = $80;
    VARFLAG_FDEFAULTCOLLELEM    = $100;
    VARFLAG_FUIDEFAULT          = $200;
    VARFLAG_FNONBROWSABLE       = $400;
    VARFLAG_FREPLACEABLE        = $800;
    VARFLAG_FIMMEDIATEBIND      = $1000;

    FADF_AUTO                   = USHORT($0001);  // array is allocated on the stack
    FADF_STATIC                 = USHORT($0002);  // array is staticly allocated
    FADF_EMBEDDED               = USHORT($0004);  // array is embedded in a structure
    FADF_FIXEDSIZE              = USHORT($0010);  // may not be resized or reallocated
    FADF_RECORD                 = USHORT($0020);  // an array of records
    FADF_HAVEIID                = USHORT($0040);  // with FADF_DISPATCH, FADF_UNKNOWN
                                                  // array has an IID for interfaces
    FADF_HAVEVARTYPE            = USHORT($0080);  // array has a VT type
    FADF_BSTR                   = USHORT($0100);  // an array of BSTRs
    FADF_UNKNOWN                = USHORT($0200);  // an array of IUnknown*
    FADF_DISPATCH               = USHORT($0400);  // an array of IDispatch*
    FADF_VARIANT                = USHORT($0800);  // an array of VARIANTs
    FADF_RESERVED               = USHORT($F008);  // reserved bits

// IMPLTYPE Flags

    IMPLTYPEFLAG_FDEFAULT       = USHORT($1);
    IMPLTYPEFLAG_FSOURCE        = USHORT($2);
    IMPLTYPEFLAG_FRESTRICTED    = USHORT($4);
    IMPLTYPEFLAG_FDEFAULTVTABLE = USHORT($8);

    PARAMFLAG_NONE              = USHORT($00);
    PARAMFLAG_FIN               = USHORT($01);
    PARAMFLAG_FOUT              = USHORT($02);
    PARAMFLAG_FLCID             = USHORT($04);
    PARAMFLAG_FRETVAL           = USHORT($08);
    PARAMFLAG_FOPT              = USHORT($10);
    PARAMFLAG_FHASDEFAULT       = USHORT($20);
    PARAMFLAG_FHASCUSTDATA      = USHORT($40);

    VAR_PERINSTANCE             = 0;
    VAR_STATIC                  = 1;
    VAR_CONST                   = 2;
    VAR_DISPATCH                = 3;

    // notification messages used by the dynamic typeinfo protocol.

    CHANGEKIND_ADDMEMBER        = 0;
    CHANGEKIND_DELETEMEMBER     = 1;
    CHANGEKIND_SETNAMES         = 2;
    CHANGEKIND_SETDOCUMENTATION = 3;
    CHANGEKIND_GENERAL          = 4;
    CHANGEKIND_INVALIDATE       = 5;
    CHANGEKIND_CHANGEFAILED     = 6;
    CHANGEKIND_MAX              = 7;

    INVOKE_FUNC                 = 1;
    INVOKE_PROPERTYGET          = 2;
    INVOKE_PROPERTYPUT          = 4;
    INVOKE_PROPERTYPUTREF       = 8;

    TKIND_ENUM                  = 0;
    TKIND_RECORD                = 1;
    TKIND_MODULE                = 2;
    TKIND_INTERFACE             = 3;
    TKIND_DISPATCH              = 4;
    TKIND_COCLASS               = 5;
    TKIND_ALIAS                 = 6;
    TKIND_UNION                 = 7;
    TKIND_MAX                   = 8;                 // end of enum marker

    SF_ERROR                    = VT_ERROR;
    SF_I1                       = VT_I1;
    SF_I2                       = VT_I2;
    SF_I4                       = VT_I4;
    SF_I8                       = VT_I8;
    SF_BSTR                     = VT_BSTR;
    SF_UNKNOWN                  = VT_UNKNOWN;
    SF_DISPATCH                 = VT_DISPATCH;
    SF_VARIANT                  = VT_VARIANT;
    SF_RECORD                   = VT_RECORD;
    SF_HAVEIID                  = VT_UNKNOWN OR VT_RESERVED;
    IDLFLAG_NONE                = PARAMFLAG_NONE;
    IDLFLAG_FIN                 = PARAMFLAG_FIN;
    IDLFLAG_FOUT                = PARAMFLAG_FOUT;
    IDLFLAG_FLCID               = PARAMFLAG_FLCID;
    IDLFLAG_FRETVAL             = PARAMFLAG_FRETVAL;

    CC_FASTCALL                 = 0;
    CC_CDECL                    = 1;
    CC_MSCPASCAL                = 2;
    CC_PASCAL                   = CC_MSCPASCAL;
    CC_MACPASCAL                = 3;
    CC_STDCALL                  = 4;
    CC_FPFASTCALL               = 5;
    CC_SYSCALL                  = 6;
    CC_MPWCDECL                 = 7;
    CC_MPWPASCAL                = 8;
    CC_MAX                      = 9;   // end of enum marker

    FUNC_VIRTUAL                = 0;
    FUNC_PUREVIRTUAL            = 1;
    FUNC_NONVIRTUAL             = 2;
    FUNC_STATIC                 = 3;
    FUNC_DISPATCH               = 4;

// objbase.h

    MARSHALINTERFACE_MIN        = 500; // minimum number of bytes for interface marshl

//
// Common typedefs for paramaters used in Storage API's, gleamed from storage.h
// Also contains Storage error codes, which should be moved into the storage
// idl files.
//


    CWCSTORAGENAME              = 32;

// Storage instantiation modes
    STGM_DIRECT                 = $00000000;
    STGM_TRANSACTED             = $00010000;
    STGM_SIMPLE                 = $08000000;
    STGM_READ                   = $00000000;
    STGM_WRITE                  = $00000001;
    STGM_READWRITE              = $00000002;
    STGM_SHARE_DENY_NONE        = $00000040;
    STGM_SHARE_DENY_READ        = $00000030;
    STGM_SHARE_DENY_WRITE       = $00000020;
    STGM_SHARE_EXCLUSIVE        = $00000010;
    STGM_PRIORITY               = $00040000;
    STGM_DELETEONRELEASE        = $04000000;
    STGM_NOSCRATCH              = $00100000;   {WINNT+}
    STGM_CREATE                 = $00001000;
    STGM_CONVERT                = $00020000;
    STGM_FAILIFTHERE            = $00000000;
    STGM_NOSNAPSHOT             = $00200000;
    STGM_DIRECT_SWMR            = $00400000;   { Win2000+}

//  flags for internet asyncronous and layout docfile
    ASYNC_MODE_COMPATIBILITY    = $00000001;
    ASYNC_MODE_DEFAULT          = $00000000;

    STGTY_REPEAT                = $00000100;
    STG_TOEND                   = $FFFFFFFF;

    STG_LAYOUT_SEQUENTIAL       = $00000000;
    STG_LAYOUT_INTERLEAVED      = $00000001;

    STGFMT_STORAGE              = 0;
    STGFMT_NATIVE               = 1;
    STGFMT_FILE                 = 3;
    STGFMT_ANY                  = 4;
    STGFMT_DOCFILE              = 5;

// This is a legacy define to allow old component to builds
    STGFMT_DOCUMENT             = 0;


// COM initialization flags; passed to CoInitialize.
    COINIT_APARTMENTTHREADED    = $2;      // Apartment model

  // These constants are only valid on Windows NT 4.0

    COINIT_MULTITHREADED        = $0;      // OLE calls objects on any thread.
    COINIT_DISABLE_OLE1DDE      = $4;      // Don't use DDE for Ole1 support.
    COINIT_SPEED_OVER_MEMORY    = $8;      // Trade memory for speed.

    SMEXF_SERVER                = $01;     // server side aggregated std marshaler
    SMEXF_HANDLER               = $02;     // client side (handler) agg std marshaler

    COWAIT_WAITALL              = 1;
    COWAIT_ALERTABLE            = 2;

    DOCMISC_CANCREATEMULTIPLEVIEWS     = 1;
    DOCMISC_SUPPORTCOMPLEXRECTANGLES   = 2;
    DOCMISC_CANTOPENEDIT               = 4;        // fails the IOleDocumentView::Open  method
    DOCMISC_NOFILESUPPORT              = 8;        //  does not support read/writing to a file

    PID_DICTIONARY         = PROPID($00000000);
    PID_CODEPAGE           = PROPID($00000001);
    PID_FIRST_USABLE       = PROPID($00000002);
    PID_FIRST_NAME_DEFAULT = PROPID($00000fff);
    PID_LOCALE             = PROPID($80000000);
    PID_MODIFY_TIME        = PROPID($80000001);
    PID_SECURITY           = PROPID($80000002);
    PID_BEHAVIOR           = PROPID($80000003);
    PID_ILLEGAL            = PROPID($ffffffff);
    PID_MIN_READONLY       = PROPID($80000000);
    PID_MAX_READONLY       = PROPID($bfffffff);
    PIDDI_THUMBNAIL        = DWORD($00000002);// VT_BLOB
    PIDSI_TITLE            = DWORD($00000002); // VT_LPSTR
    PIDSI_SUBJECT          = DWORD($00000003); // VT_LPSTR
    PIDSI_AUTHOR           = DWORD($00000004); // VT_LPSTR
    PIDSI_KEYWORDS         = DWORD($00000005); // VT_LPSTR
    PIDSI_COMMENTS         = DWORD($00000006); // VT_LPSTR
    PIDSI_TEMPLATE         = DWORD($00000007); // VT_LPSTR
    PIDSI_LASTAUTHOR       = DWORD($00000008); // VT_LPSTR
    PIDSI_REVNUMBER        = DWORD($00000009); // VT_LPSTR
    PIDSI_EDITTIME         = DWORD($0000000a); // VT_FILETIME
    PIDSI_LASTPRINTED      = DWORD($0000000b); // VT_FILETIME
    PIDSI_CREATE_DTM       = DWORD($0000000c); // VT_FILETIME
    PIDSI_LASTSAVE_DTM     = DWORD($0000000d); // VT_FILETIME
    PIDSI_PAGECOUNT        = DWORD($0000000e); // VT_I4
    PIDSI_WORDCOUNT        = DWORD($0000000f); // VT_I4
    PIDSI_CHARCOUNT        = DWORD($00000010); // VT_I4
    PIDSI_THUMBNAIL        = DWORD($00000011); // VT_CF
    PIDSI_APPNAME          = DWORD($00000012); // VT_LPSTR
    PIDSI_DOC_SECURITY     = DWORD($00000013); // VT_I4
    PIDDSI_CATEGORY        = $00000002; // VT_LPSTR
    PIDDSI_PRESFORMAT      = $00000003; // VT_LPSTR
    PIDDSI_BYTECOUNT       = $00000004; // VT_I4
    PIDDSI_LINECOUNT       = $00000005; // VT_I4
    PIDDSI_PARCOUNT        = $00000006; // VT_I4
    PIDDSI_SLIDECOUNT      = $00000007; // VT_I4
    PIDDSI_NOTECOUNT       = $00000008; // VT_I4
    PIDDSI_HIDDENCOUNT     = $00000009; // VT_I4
    PIDDSI_MMCLIPCOUNT     = $0000000A; // VT_I4
    PIDDSI_SCALE           = $0000000B; // VT_BOOL
    PIDDSI_HEADINGPAIR     = $0000000C; // VT_VARIANT |VT_VECTOR
    PIDDSI_DOCPARTS        = $0000000D; // VT_LPSTR |VT_VECTOR
    PIDDSI_MANAGER         = $0000000E; // VT_LPSTR
    PIDDSI_COMPANY         = $0000000F; // VT_LPSTR
    PIDDSI_LINKSDIRTY      = $00000010; // VT_BOOL

//  FMTID_MediaFileSummaryInfo - Property IDs

    PIDMSI_EDITOR          = DWord($00000002);  // VT_LPWSTR
    PIDMSI_SUPPLIER        = DWord($00000003);  // VT_LPWSTR
    PIDMSI_SOURCE          = DWord($00000004);  // VT_LPWSTR
    PIDMSI_SEQUENCE_NO     = DWord($00000005);  // VT_LPWSTR
    PIDMSI_PROJECT         = DWord($00000006);  // VT_LPWSTR
    PIDMSI_STATUS          = DWord($00000007);  // VT_UI4
    PIDMSI_OWNER           = DWord($00000008);  // VT_LPWSTR
    PIDMSI_RATING          = DWord($00000009);  // VT_LPWSTR
    PIDMSI_PRODUCTION      = DWord($0000000A);  // VT_FILETIME (UTC)
    PIDMSI_COPYRIGHT       = DWord($0000000B);  // VT_LPWSTR
    PRSPEC_INVALID = ULONG($ffffffff);
    PRSPEC_LPWSTR  = ULONG(0);
    PRSPEC_PROPID  = ULONG(1);
    PROPSETFLAG_DEFAULT   = DWORD(0);
    PROPSETFLAG_NONSIMPLE = DWORD(1);
    PROPSETFLAG_ANSI      = DWORD(2);

TYPE
    VARTYPE             = USHORT;

//TypeInfo stuff.

    DISPID              = Long ;
    SCODE               = Long;
    pSCODE              = ^SCODE;
    lpDISPID            = ^DISPID;
    MEMBERID            = DispId;
    HREFTYPE            = DWord;
    TResultList		= array[0..high(integer) div 4-50] of HResult;
    PResultList         = ^TResultList;

    PSYSINT = ^SYSINT;
    SYSINT = LongInt;
    PSYSUINT = ^SYSUINT;
    SYSUINT = LongWord;

// Enums
    VARKIND             = DWord;
    DESCKIND            = DWord;
    SYSKIND             = DWord;
    TSYSKIND		= SYSKIND;
    REGKIND		= DWord;
    TREGKIND		= REGKIND;
    FUNCKIND            = DWord;
    CHANGEKIND          = DWord;
    CALLCONV            = DWord;

   PIDMSI_STATUS_VALUE = (
    PIDMSI_STATUS_NORMAL  = 0,
    PIDMSI_STATUS_NEW,
    PIDMSI_STATUS_PRELIM,
    PIDMSI_STATUS_DRAFT,
    PIDMSI_STATUS_INPROGRESS,
    PIDMSI_STATUS_EDIT,
    PIDMSI_STATUS_REVIEW,
    PIDMSI_STATUS_PROOF,
    PIDMSI_STATUS_FINAL,
    PIDMSI_STATUS_OTHER   = $7FFF
    );
   TPIDMSI_STATUS_VALUE= PIDMSI_STATUS_Value;



    PCOAUTHIDENTITY    = ^TCOAUTHIDENTITY;
    _COAUTHIDENTITY    = Record
                          User           : PUSHORT;
                          UserLength     : ULONG;
                          Domain         : PUSHORT;
                          DomainLength   : ULong;
                          Password       : PUSHORT;
                          PassWordLength : ULong;
                          Flags          : ULong;
                          End;

   COAUTHIDENTITY      = _COAUTHIDENTITY;
   TCOAUTHIDENTITY     = _COAUTHIDENTITY;

   PCOAUTHINFO         = ^TCOAUTHINFO;
   COAuthInfo          = Record
                          AuthnSvc              : DWord;
                          AuthzSvc              : DWord;
                          ServerPrincName       : LPWSTR;
                          AuthnLevel            : DWord;
                          ImpersonationLevel    : DWord;
                          AuthIdentityData      : PCOAUTHIDENTITY;
                          Capabilities          : DWord;
                          END;
   TCOAUTHINFO         = COAUTHINFO;

   PCOSERVERINFO       = ^TCOSERVERINFO;
   _COSERVERINFO       = Record
                          dwReserved1 : DWord;
                          pwszName    : LPWSTR;
                          pAuthInfo   : PCoAuthInfo;
                          dwReserved2 : DWord;
                          end;
   TCOSERVERINFO       = _COSERVERINFO;
   PMultiQI            = ^Multi_QI;
   tagMULTI_QI         = Record
                          iid: piid;                   // pass this one in
                          itf: IUnknown;               // get these out (you must set to NULL before calling)
                          hr : HResult;
                          END;
   MULTI_QI            = TagMULTI_QI;
   PMulti_QI           = PMultiQI;
   TMultiQI						 = tagMULTI_QI;

   PMultiQIArray = ^TMultiQIArray;
   TMultiQIArray = array[0..65535] of TMultiQI;


   HContext            = Pointer;
   ApartmentID         = DWord;


//****** Critical Section Wrappers ***********************************

//   LCID                    = WORD;
//   LANGID                  = USHORT;

// #######################################################################
//
//  User marshal support for Windows data types.

//
//  Frequently used helpers: sized blobs
//
//      Never put [user_marshal] or [wire_marshal] on the helpers directly.
//

// Flagged blobs.

   _FLAGGED_BYTE_BLOB           = Record
                                   fFlags :  ULong;
                                   clSize :  ULong;
                                   abdata :  array[0..0] of byte;
                                   End;
   FLAGGED_BYTE_BLOB            = _FLAGGED_BYTE_BLOB;
   UP_FLAGGED_BYTE_BLOB         = ^FLAGGED_BYTE_BLOB;

   _FLAGGED_WORD_BLOB           = Record
                                   fFlags :  ULong;
                                   clSize :  ULong;
                                   abdata :  array[0..0] of USHORT;
                                   End;
   FLAGGED_WORD_BLOB            = _FLAGGED_WORD_BLOB;
   UP_FLAGGED_WORD_BLOB         = ^FLAGGED_WORD_BLOB;

   _FLAGGED_DWORD_BLOB          = Record
                                   fFlags :  ULong;
                                   clSize :  ULong;
                                   abdata :  array[0..0] of ULONG;
                                   End;
   FLAGGED_DWORD_BLOB           = _FLAGGED_DWORD_BLOB;
   FLAGGED_UP_DWORD_BLOB        = ^FLAGGED_DWORD_BLOB;

// Simple blobs.

   _BYTE_BLOB                   = Record
                                   clSize :  ULong;
                                   abdata :  array[0..0] of byte;
                                   End;
   BYTE_BLOB                    = _BYTE_BLOB;
   UP_BYTE_BLOB                 = ^BYTE_BLOB;

   _WORD_BLOB                   = Record
                                   clSize :  ULong;
                                   abdata :  array[0..0] of USHORT;
                                   End;
   WORD_BLOB                    = _WORD_BLOB;
   UP_WORD_BLOB                 = ^WORD_BLOB;

   _DWORD_BLOB                  = Record
                                   clSize :  ULong;
                                   abdata :  array[0..0] of ULONG;
                                   End;
   DWORD_BLOB                   = _DWORD_BLOB;
   UP_DWORD_BLOB                = ^DWORD_BLOB;

// Frequently used helpers with sized pointers.

   _BYTE_SIZEDARR               = Record
                                   clsize : ULong;
                                   Data   : PByte;
                                   End;
   BYTE_SIZEDARR                = _BYTE_SIZEDARR;

   _SHORT_SIZEDARR              = Record
                                   clsize : ULong;
                                   Data   : PSHORT;
                                   End;
   SHORT_SIZEDARR               = _SHORT_SIZEDARR;

   _LONG_SIZEDARR               = Record
                                   clsize : ULong;
                                   Data   : PLONG;
                                   End;
   LONG_SIZEDARR                = _LONG_SIZEDARR;
   HYPER                        = LONGLONG;
   PHYPER                       = ^HYPER;
   _HYPER_SIZEDARR              = Record
                                   clsize : ULong;
                                   Data   : PHYPER;
                                   End;
   HYPER_SIZEDARR               = _HYPER_SIZEDARR;


// #########################################################################
//
//  CLIPFORMAT
//

   userCLIPFORMAT               = Record
                                    FContext : Long;
                                    CASE INTEGER OF
                                      0 : (dwvalue : DWORD);
                                      1 : (szName : poleStr);
                                      End;

   wireCLIPFORMAT               = ^userCLIPFORMAT;


// #########################################################################
//
//  Good for most of the gdi handles.

   _GDI_NONREMOTE               = Record
                                    FContext : Long;
                                    Case Integer Of
                                     0 : (HInProc : Long);
                                     1 : (HRemote : DWORD_BLOB);
                                     END;
   GDI_NONREMOTE                = _GDI_NONREMOTE;

// #########################################################################
//
//  HGLOBAL
//
// A global may be Null or may be non-NULL with 0 length.

   _userHGLOBAL                 = Record
                                   FContext : Long;
                                   CASE Integer OF
                                      0 : (hInproc : Long);
                                      1 : (hRemote : UP_FLAGGED_BYTE_BLOB);
                                      2 : (hInproc64: int64);
                                      End;
   userHGlobal                  = _userHGLOBAL;
   wireHGLOBAL                  = ^userHGLOBAL;

// #########################################################################
//
//  HMETAFILE
//
   _userHMETAFILE               = Record
                                   fContext : Long;
                                   Case Integer OF
                                      0 : (hInproc   : Long);
                                      1 : (hRemote   : up_byte_blob);
                                      2 : (hInProc64 : Int64);
                                      End;
   userHMETAFILE                = _userHMETAFILE;
   puserHMETAFILE               = ^userHMETAFILE;

// #########################################################################
//
//  HMETAFILEPICT
//

   _remoteMETAFILEPICT          = Record
                                    mm   : Long;
                                    xExt : Long;
                                    yExt : Long;
                                    mgf  : puserHMETAFILE;
                                    End;

   remoteMETAFILEPICT           = _remoteMETAFILEPICT;
   premoteMETAFILEPICT          = ^remoteMETAFILEPICT;

   _userHMETAFILEPICT           = Record
                                    fContext : Long;
                                    Case Integer OF
                                      0 : (hInproc   : Long);
                                      1 : (hRemote   : premoteMETAFILEPICT);
                                      2 : (hInProc64 : Int64);
                                      End;
   userHMETAFILEPICT            = _userHMETAFILEPICT;


// #########################################################################
//
//  HENHMETAFILE
//

   _userHENHMETAFILE            = Record
                                    fContext : Long;
                                    Case Integer OF
                                      0 : (hInproc   : Long);
                                      1 : (hRemote   : up_byte_blob);
                                      2 : (hInProc64 : Int64);
                                      End;
   userHENHMETAFILE             = _userHENHMETAFILE;
   puserHENHMETAFILE            = ^userHENHMETAFILE;

// #########################################################################
//
//  HBITMAP
//

// RemHBITMAP was just a byte blob, but the whole bitmap structure was copied
// at the beginning of the buffer.

// So, we take BITMAP fields from wingdi.x


   _userBITMAP                  = Record
                                   bmType,
                                   bmWidth,
                                   bmHeight,
                                   bmWidthBytes : Long;
                                   bmPlanes,
                                   bmBitsPixel  : Word;
                                   cvsize       : ULONG;
                                   buffer       : pbyte;
                                   End;

   userBITMAP                   = _userBITMAP;
   puserBITMAP                  = ^userBITMAP;

   _userHBITMAP                 = Record
                                    fContext : Long;
                                    Case Integer OF
                                      0 : (hInproc   : Long);
                                      1 : (hRemote   : puserBITMAP);
                                      2 : (hInProc64 : Int64);
                                      End;
   userHBITMAP                  = _userHBITMAP;
   puserHBITMAP                 = ^userHBITMAP;


// #########################################################################
//
//  HPALETTE
//

// PALETTEENTRY is in wingdi.x, it is a struct with 4 bytes.
// LOGPALETTE   is in wingdi.x, it is a conf struct with paletteentries and
//                                    a version field

   _userHpalette                = Record
                                    fContext : Long;
                                    Case Integer OF
                                      0 : (hInproc   : Long);
                                      1 : (hRemote   : logpalette);
                                      2 : (hInProc64 : Int64);
                                  End;
   userHpalette                 = _userHpalette;
   puserHpalette                = ^userHpalette;

// #########################################################################
//
//  Handles passed locally as longs.
//

   _RemotableHandle             = Record
                                    fContext : Long;
                                    Case Integer OF
                                      0 : (hInproc   : Long);
                                      1 : (hRemote   : Long);
                                      End;
   RemotableHandle              = _RemotableHandle;


   wireHWND                     = ^RemotableHandle;
   wireHMENU                    = ^RemotableHandle;
   wireHACCEL                   = ^RemotableHandle;
   wireHBRUSH                   = ^RemotableHandle;
   wireHFONT                    = ^RemotableHandle;
   wireHDC                      = ^RemotableHandle;
   wireHICON                    = ^RemotableHandle;
   HCursor                      = HICON;


   tagTEXTMETRICW               = Record
                                    tmHeight,
                                    tmAscent,
                                    tmDescent,
                                    tmInternalLeading,
                                    tmExternalLeading,
                                    tmAveCharWidth,
                                    tmMaxCharWidth,
                                    tmWeight,
                                    tmOverhang,
                                    tmDigitizedAspectX,
                                    tmDigitizedAspectY   : Long;
                                    tmFirstChar,
                                    tmLastChar,
                                    tmDefaultChar,
                                    tmBreakChar          : WCHAR;
                                    tmItalic,
                                    tmUnderlined,
                                    tmStruckOut,
                                    tmPitchAndFamily,
                                    tmCharSet            : BYTE;
                                  End;

   TEXTMETRICW                  = tagTEXTMETRICW;
   PTEXTMETRICW                 = ^TEXTMETRICW;
   LPTEXTMETRICW                = PTEXTMETRICW;
   wireHBITMAP                  = ^userHBITMAP;
   wireHPALETTE                 = ^userHPALETTE;
   wireHENHMETAFILE             = ^userHENHMETAFILE;
   wireHMETAFILE                = ^userHMETAFILE;
   wireHMETAFILEPICT            = ^userHMETAFILEPICT;
   HMetaFilePict                = Pointer;
   HLOCAL                       = HGLOBAL;
//  Date              = Double;

{****************************************************************************
 *  Binding Interfaces
 ****************************************************************************}

   tagBIND_OPTS                 = Record
                                    cvStruct,          //  sizeof(BIND_OPTS)
                                    grfFlags,
                                    grfMode,
                                    dwTickCountDeadline : DWord;
                                    End;
   TBind_Opts                   = tagBIND_OPTS;
   PBind_Opts                   = ^TBind_Opts;
   TBindOpts		        = tagBIND_OPTS;
   PBindOpts			= ^TBindOpts;
   Bind_Opts			= tagBind_opts;

   tagBIND_OPTS2_CPP            = Record
                                    dwTrackFlags,
                                    dwClassContext  : Dword;
                                    Locale          : LCID;
                                    ServerInfo      : pCoServerInfo;
                                    End;

   TBind_Opts2_CPP              = tagBIND_OPTS2_CPP;
   PBind_Opts2_CPP              = ^TBind_Opts2_CPP;


   tagBind_OPTS2                = Record
                                    cvStruct,          //  sizeof(BIND_OPTS)
                                    grfFlags,
                                    grfMode,
                                    dwTickCountDeadline : DWord;
                                    dwTrackFlags,
                                    dwClassContext      : DWord;
                                    Locale              : LCID;
                                    ServerInfo          : pCoServerInfo;
                                    End;

   TBind_Opts2                  = tagBIND_OPTS2;
   PBind_Opts2                  = ^TBind_Opts2;

// ****************************************************************************
// *  Structured Storage Interfaces
// ****************************************************************************



   tagSTATSTG                   = types.tagSTATSTG;

   TStatStg                     = tagSTATSTG;
   PStatStg                     = types.PStatStg;
   STATSTG                      = TStatStg;

{    TagRemSNB = Record
                   ulCntStr  : ULong;
                   ulCntChar : ULong;
                   [size_is(ulCntChar)] OLECHAR rgString[];
                 End;
        RemSNB=TagRemSNB
        WireSNB=^RemSNB}
   SNB                          = ^PoleStr;
   tagDVTARGETDEVICE            = Record
                                    tdSize                     : DWord;
                                    tdDriverNameOffset,
                                    tdDeviceNameOffset,
                                    tdPortNameOffset,
                                    tdExtDevmodeOffset         : Word;
                                    Data                       : Record End;
                                    End;

   DVTARGETDEVICE               = TagDVTARGETDEVICE;
   PDVTARGETDEVICE              = ^tagDVTARGETDEVICE;
   LPCLIPFORMAT                 = ^TCLIPFORMAT;
   TCLIPFORMAT                  = Word;
   CLIPFORMAT                   = TCLIPFORMAT;
   PClipFormat                                                                  = LPCLIPFORMAT;

   tagFORMATETC                 = Record
                                    CfFormat :  Word {TCLIPFORMAT};
                                    Ptd      : PDVTARGETDEVICE;
                                    dwAspect : DWORD;
                                    lindex   : Long;
                                    tymed    : DWORD;
                                    End;
   FORMATETC                    = TagFORMATETC;
   TFORMATETC                   = FORMATETC;
   LPFORMATETC                  = ^FORMATETC;
   PFormatEtc                   = LPFORMATETC;

    // Stats for data; used by several enumerations and by at least one
    // implementation of IDataAdviseHolder; if a field is not used, it
    // will be NULL.


   tagRemSTGMEDIUM              = Record
                                    tymed                     : DWord;
                                    dwHandleType              : DWord;
                                    pData,
                                    pUnkForRelease,
                                    cbData                    : ULong;
                                    Data                      : Record end;
                                    End;

   RemSTGMEDIUM                 = TagRemSTGMedium;

   TagSTGMEDIUM                 = Record
                                    Tymed : DWord;
                                    Case Integer Of
                                      0 : (HBITMAP             : hBitmap;       PUnkForRelease :  Pointer {IUnknown});
                                      1 : (HMETAFILEPICT       : hMetaFilePict );
                                      2 : (HENHMETAFILE        : hEnhMetaFile  );
                                      3 : (HGLOBAL             : hGlobal       );
                                      4 : (lpszFileName        : LPOLESTR    );
                                      5 : (pstm                : Pointer{IStream}  );
                                      6 : (pstg                : Pointer{IStorage} );
                                      End;
   USTGMEDIUM                   = TagSTGMEDIUM;
   STGMEDIUM                    = USTGMEDIUM;
   TStgMedium                                                                           = TagSTGMEDIUM;
   PStgMedium                   = ^TStgMedium;

//
//  wireSTGMEDIUM
//
// These flags are #defined (not enumerated) in wingdi.
// We need to repeat #defines to avoid conflict in the generated file.
//

   _GDI_OBJECT                  = Record
                                    ObjectType : DWord;
                                    Case Integer Of
                                      0 : (HBitmap : WireHBITMAP);
                                      1 : (hPalette: wireHPALETTE);
                                      2 : (hGeneric: wireHGLOBAL);
                                      END;
   GDI_OBJECT                   = _GDI_OBJECT;


   _userSTGMEDIUM               = Record
                                    tymed : DWORD;
                                    Case Integer OF
                                      0 : (hMetaFilePict : wireHMETAFILEPICT;punkforrelease:Pointer {IUnknown});
                                      1 : (hHEnhMetaFile : wireHENHMETAFILE);
                                      2 : (hGdiHandle    : ^GDI_OBJECT);
                                      3 : (HGlobal       : wireHGLOBAL);
                                      4 : (lpszFileName  : LPOLESTR);
                                      5 : (pstm          : ^BYTE_BLOB);
                                      6 : (pstg          : ^BYTE_BLOB);
                                      END;

   userSTGMEDIUM                = _userSTGMEDIUM;


   LPSTGMEDIUM                  = ^STGMEDIUM;

   _userFLAG_STGMEDIUM          = Record
                                    ContextFlags,
                                    fPassOwnership  : Long;
                                    stgmed          : userSTGMEDIUM;
                                    End;

   userFLAG_STGMEDIUM           = _userFLAG_STGMEDIUM;

   wireFLAG_STGMEDIUM           = ^userFLAG_STGMEDIUM;


   _FLAG_STGMEDIUM              = Record
                                    ContextFlags,
                                    fPassOwnership   : Long;
                                    Stgmed           : STGMEDIUM;
                                    End;
   FLAG_STGMEDIUM               = _FLAG_STGMEDIUM;


   VARIANTARG                   = VARIANT;
   LPVARIANT                    = ^VARIANT;
   LPVARIANTARG                 = ^VARIANT;

// parameter description

   tagPARAMDESCEX               = Record
                                    cBytes         : ULong;      // size of this structure
                                    varDefaultValue: VariantARG; // default value of this parameter
                                    End;

   PARAMDESCEX                  = tagPARAMDESCEX;
   LPPARAMDESCEX                = ^PARAMDESCEX;

   tagPARAMDESC                 = Record
                                    pparamdescex: LPPARAMDESCEX ;  // valid if PARAMFLAG_FHASDEFAULT bit is set
                                    wParamFlags : UShort ;         // IN, OUT, etc
                                    End;

   PARAMDESC                    = tagPARAMDESC;
   LPPARAMDESC                  = ^PARAMDESC;


   tagSAFEARRAYBOUND            = Record
                                     cElements : ULong;
                                     lLbound   : Long;
                                     End;
   SAFEARRAYBOUND               = tagSAFEARRAYBOUND;
   LPSAFEARRAYBOUND             = ^SAFEARRAYBOUND;

   tagSAFEARRAY = record
     cDims: USHORT;
     fFeatures: USHORT;
     cbElements: ULONG;
     cLocks: ULONG;
     pvData: PVOID;
     rgsabound: array[0..0] of SAFEARRAYBOUND;
   end;
   TSafeArray = tagSAFEARRAY;
   SAFEARRAY = TSafeArray;
   PSafeArray = ^TSafeArray;

// additional interface information about the incoming call
   tagINTERFACEINFO             = Record
                                    Unk     : Pointer {IUnknown};   // the pointer to the object
                                    IID     : Tiid;       // interface id
                                    wMethod : WORD;        // interface method
                                    End;

   INTERFACEINFO                = tagINTERFACEINFO;
   LPINTERFACEINFO              = ^INTERFACEINFO;
   RPCOLEDATAREP                = ULong;
   tagRPCOLEMESSAGE             = Record
                                    Reserved1          : Pointer;
                                    DataRepresentation : RPCOLEDATAREP;
                                    Buffer             : Pointer;
                                    cbBuffer,
                                    IMethod            : ULong;
                                    Reserved2          : Array[0..4] Of Pointer;
                                    rpcFlags           : ULong;
                                    End;

   RPCOLEMESSAGE                = tagRPCOLEMESSAGE;
   PRPCOLEMESSAGE               = ^RPCOLEMESSAGE;

   tagStorageLayout             = Record
                                    LayoutType       : Dword;
                                    pwcsElementName  : POleStr;
                                    cOffset,
                                    cBytes           : Large_Integer;
                                    End;

   StorageLayout                = tagStorageLayout;

   tagSTATDATA                  = Record
                                                                // field used by:
                                    FORMATETC   : Tformatetc;   // EnumAdvise, EnumData (cache), EnumFormats
                                    advf        : DWord;        // EnumAdvise, EnumData (cache)
                                    padvSink    : Pointer {IAdviseSink};  // EnumAdvise
                                    dwConnection: DWord;        // EnumAdvise
                                    End;
   STATDATA                     = TagStatData;
   LPStatData                   = ^StatData;

   pARRAYDESC                   = ^ARRAYDESC;
   pTYPEDESC                    = ^TYPEDESC;
   tagTYPEKIND                  = Dword;
   TYPEKIND                     = tagTYPEKIND;
   TTYPEKIND			= TYPEKIND;
   INVOKEKIND                   = Dword;
   tagTYPEDESC                  = Record
                                    Case Integer OF
                                      VT_PTR,
                                      VT_SAFEARRAY   :  (lptdesc : PTYPEDESC;vt : VARTYPE);
                                      VT_CARRAY      :  (lpadesc : PARRAYDESC);
                                      VT_USERDEFINED :  (hreftype : HREFTYPE);
                                      End;
   TYPEDESC                     = tagTYPEDESC;




   tagARRAYDESC                 = Record
                                     tdescElem   : TYPEDESC;                       // element type
                                     cDims       : USHORT;
                                     rgbounds    : ARRAY [0..0] OF SAFEARRAYBOUND; // dimension count
                                     End;

   ARRAYDESC                    = tagARRAYDESC;

   tagIDLDESC                   = Record
                                    dwReserved     : pULONG;
                                    wIDLFlags      : USHORT;           // IN, OUT, etc
                                    End;
   IDLDESC                      = tagIDLDESC;
   LPIDLDESC                    = ^IDLDESC;


   tagELEMDESC                  = Record
                                    tdesc : TYPEDESC;
                                    case Integer Of
                                      0 : (idldesc    : IDLDESC);
                                      1 : (paramdesc  : PARAMDESC);
                                      END;

   ELEMDESC                     = tagELEMDESC;
   LPELEMDESC                   = ^ELEMDESC;
   tagVARDESC                   = Record
                                    memId               : MEMBERID;
                                    lpstrSchema         : pOleStr;
                                    CASE Integer OF
                                    VAR_PERINSTANCE,
                                    VAR_DISPATCH,
                                    VAR_STATIC       : (oInst   : ULong;                          // offset of variable within the instance
                                                        ElemdescVar : ELEMDESC;
                                                        wVarFlags   : WORD;
                                                        varkind     : VARKIND);
                                    VAR_CONST        : (lpvarValue : PVARIANT);                       // the value of the constant
                                    End;
  VARDESC                       = tagVARDESC;
  LPVARDESC                     = ^VARDESC;
  pVARDESC			= LPVARDESC;
  tagDISPPARAMS                 = Record
                                   rgvarg            : lpVARIANTARG;
                                   rgdispidNamedArgs : lpDISPID;
                                   cArgs,
                                   cNamedArgs        : UINT;
                                   End;
  DISPPARAMS                    = tagDISPPARAMS;
  TDispParams                   = tagDISPPARAMS;
  PDispParams                   = ^TDispParams;

  PExcepInfo                    = ^TExcepInfo;
  TFNDeferredFillIn             = function(info : PExcepInfo): HRESULT;stdcall;
  tagEXCEPINFO                  = Record
                                    wCode,                         // An error code describing the error.
                                    wReserved      : Word;
                                    Source,                        // A source of the exception
                                    Description,                   // A description of the error
                                    HelpFile       : WideString;   // Fully qualified drive, path, and file name
                                    dwHelpContext  : ULONG;        // help context of topic within the help file
                                                                   // We can use ULONG_PTR here, because EXCEPINFO is marshalled by RPC
                                                                   // RPC will marshal pfnDeferredFillIn.
                                    pvReserved     : pointer;
                                    pfnDeferredFillIn : TFNDeferredFillIn;
                                    SCODE          : scode;
                                    End;

  EXCEPINFO                     =  tagEXCEPINFO;
  TExcepInfo                    =  tagEXCEPINFO;

  tagTYPEATTR                   = Record
                                   GUID            : Tguid;       // the GUID of the TypeInfo
                                   LCID            : lcid;        // locale of member names and doc strings
                                   dwReserved      : DWord;
                                   memidConstructor,              // ID of constructor, MEMBERID_NIL if none
                                   memidDestructor : MemberID;    // ID of destructor, MEMBERID_NIL if none
                                   lpstrSchema     : pOleStr;

                                   cbSizeInstance  : ULong;       // the size of an instance of this type
                                   typekind        : TYPEKIND;    // the kind of type this typeinfo describes
                                   cFuncs,                        // number of functions
                                   cVars,                         // number of variables / data members
                                   cImplTypes,                    // number of implemented interfaces
                                   cbSizeVft,                     // the size of this types virtual func table
                                   cbAlignment,                   { specifies the alignment requirements for
                                                                    an instance of this type,
                                                                      0 = align on 64k boundary
                                                                      1 = byte align
                                                                      2 = word align
                                                                      4 = dword align... }
                                   wTypeFlags,
                                   wMajorVerNum,                  // major version number
                                   wMinorVerNum    : Word;        // minor version number
                                   tdescAlias      : TYPEDESC;    { if typekind == TKIND_ALIAS this field
                                                                    specifies the type for which this type
                                                                    is an alias }
                                   idldescType     : IDLDESC;     // IDL attributes of the described type
                                   END;
  TYPEATTR                       = tagTYPEATTR;

  LPTYPEATTR                     = ^TYPEATTR;
  PTYPEAttr			 = LPTYPEATTR;

  tagTLIBATTR                    = Record
                                     GUID        : guid;
                                     LCID        : lcid;
                                     SYSKIND     : syskind;
                                     wMajorVerNum,
                                     wMinorVerNum,
                                     wLibFlags   : Word
                                     End;

  TLIBATTR                       = tagTLIBATTR;
  LPTLIBATTR                     = ^tagTLIBATTR;
  PTLIBAttr			 = LPTLIBATTR;

  LPFUNCDESC                     = ^FUNCDESC;
  PFUNCDESC			 = LPFUNCDESC;

  tagFUNCDESC                    = Record
                                     memid             : MEMBERID;
                                     lprgscode         : PResultList;
                                     lprgelemdescParam : lpELEMDESC;    // array of param types
                                     FUNCKIND          : funckind;
                                     invkind           : INVOKEKIND;
                                     callconv          : CALLCONV;
                                     cParams,
                                     cParamsOpt,
                                     oVft,
                                     cScodes           : SHORT;
                                     elemdescFunc      : ELEMDESC;
                                     wFuncFlags        : WORD;
                                     End;
  FUNCDESC                       = tagFUNCDESC;


  tagBINDPTR                     = Record
                                     case integer Of
                                      0 : (lpfuncdesc : LPFUNCDESC);
                                      1 : (lpvardesc  : LPVARDESC);
                                      2 : (lptcomp    : Pointer {ITypeComp} );
                                      End;
  BINDPTR                        = tagBINDPTR;
  LPBINDPTR                      = ^BINDPTR;

  tagCUSTDATAITEM                = Record
                                     GUID         : TGuid;           // guid identifying this custom data item
                                     varValue     : VARIANTARG;      // value of this custom data item
                                     End;

  CUSTDATAITEM                   = tagCUSTDATAITEM;

  LPCUSTDATAITEM                 = ^CUSTDATAITEM;

  tagCUSTDATA                    = Record
                                     cCustData   : DWord;             // number of custom data items in rgCustData
                                     prgCustData : LPCUSTDATAITEM;    // array of custom data items
                                     End;

  CUSTDATA                       = tagCUSTDATA;
  LPCUSTDATA                     = ^CUSTDATA;

  PPROPVARIANT		         = ^TPROPVARIANT;



  tagPROPSPEC = record
                  ulKind : ULONG ;
                  case boolean of
                    false : ( propid:propid);
                    true  :  (lpwstr: LPOLEStr);	
                    end;

  PROPSPEC= tagPROPSPEC;
  TPROPSPEC = PROPSPEC;
  PPROPSPEC = ^TPROPSPEC;

  tagSTATPROPSTG = record
                    lpwstrName : LPOLESTR ;
                    propid:PROPID ;
                    vt : VARTYPE;
                    end;
  STATPROPSTG = tagSTATPROPSTG;
  TSTATPROPSTG = STATPROPSTG;
  PSTATPROPSTG = ^TSTATPROPSTG;

      tagSTATPROPSETSTG = record
          fmtid : FMTID;
          clsid : CLSID;
          grfFlags : DWORD;
          mtime : FILETIME;
          ctime : FILETIME;
          atime : FILETIME;
          dwOSVersion : DWORD;
       end;
  STATPROPSETSTG = tagSTATPROPSETSTG;
  TSTATPROPSETSTG = STATPROPSETSTG;
  PSTATPROPSETSTG = ^STATPROPSETSTG;

   tagVersionedStream = record
        guidVersion : TGUID;
        pStream : pointer; {IStream}
     end;
   VERSIONEDSTREAM = tagVersionedStream;
   TVERSIONEDSTREAM = tagVersionedStream;
   LPVERSIONEDSTREAM = tagVersionedStream;
   PVERSIONEDSTREAM = ^TagVersionedStream;


   LPSAFEARRAY = ^SAFEARRAY;
   tagDEC = record //  simpler remoting variant without nested unions. see wtypes.h
         wReserved : ushort;
         scale,
         sign : byte;
         hi32 : ULONG;
         lo64 : ULONGLONG;
         end;
   TDECIMAL=tagDEC;
   PDecimal=^TDECIMAL;

   tagCAC = record
        cElems : ULONG;
        pElems : pCHAR;
     end;
   CAC = tagCAC;
   TCAC = tagCAC;
   tagCAUB = record
        cElems : ULONG;
        pElems : pUCHAR;
     end;
   CAUB = tagCAUB;
   TCAUB = tagCAUB;
   tagCAI = record
        cElems : ULONG;
        pElems : pSHORT;
     end;
   CAI = tagCAI;
   TCAI = tagCAI;
   tagCAUI = record
        cElems : ULONG;
        pElems : pUSHORT;
     end;
   CAUI = tagCAUI;
   TCAUI = tagCAUI;
   tagCAL = record
        cElems : ULONG;
        pElems : pLONG;
     end;
   CAL = tagCAL;
   TCAL = tagCAL;
   tagCAUL = record
        cElems : ULONG;
        pElems : pULONG;
     end;
   CAUL = tagCAUL;
   TCAUL = tagCAUL;
   tagCAFLT = record
        cElems : ULONG;
        pElems : pSingle;
     end;
   CAFLT = tagCAFLT;
   TCAFLT = tagCAFLT;
   tagCADBL = record
        cElems : ULONG;
        pElems : ^DOUBLE;
     end;
   CADBL = tagCADBL;
   TCADBL = tagCADBL;
   tagCACY = record
        cElems : ULONG;
        pElems : ^CY;
     end;
   CACY = tagCACY;
   TCACY = tagCACY;
   tagCADATE = record
        cElems : ULONG;
        pElems : ^DATE;
     end;
   CADATE = tagCADATE;
   TCADATE = tagCADATE;
   tagCABSTR = record
        cElems : ULONG;
        pElems : ^BSTR;
     end;
   CABSTR = tagCABSTR;
   TCABSTR = tagCABSTR;
   tagCABSTRBLOB = record
        cElems : ULONG;
        pElems : ^BSTRBLOB;
     end;
   CABSTRBLOB = tagCABSTRBLOB;
   TCABSTRBLOB = tagCABSTRBLOB;
   tagCABOOL = record
        cElems : ULONG;
        pElems : ^VARIANT_BOOL;
     end;
   CABOOL = tagCABOOL;
   TCABOOL = tagCABOOL;
   tagCASCODE = record
        cElems : ULONG;
        pElems : ^SCODE;
     end;
   CASCODE = tagCASCODE;
   TCASCODE = tagCASCODE;
   tagCAPROPVARIANT = record
        cElems : ULONG;
        pElems : ^PROPVARIANT;
     end;
   CAPROPVARIANT = tagCAPROPVARIANT;
   TCAPROPVARIANT = tagCAPROPVARIANT;
   tagCAH = record
        cElems : ULONG;
        pElems : ^LARGE_INTEGER;
     end;
   CAH = tagCAH;
   TCAH = tagCAH;
   tagCAUH = record
        cElems : ULONG;
        pElems : ^ULARGE_INTEGER;
     end;
   CAUH = tagCAUH;
   TCAUH = tagCAUH;
   tagCALPSTR = record
        cElems : ULONG;
        pElems : ^LPSTR;
     end;
   CALPSTR = tagCALPSTR;
   TCALPSTR = tagCALPSTR;
   tagCALPWSTR = record
        cElems : ULONG;
        pElems : ^LPWSTR;
     end;
   CALPWSTR = tagCALPWSTR;
   TCALPWSTR = tagCALPWSTR;
   tagCAFILETIME = record
        cElems : ULONG;
        pElems : ^FILETIME;
     end;
   CAFILETIME = tagCAFILETIME;
   TCAFILETIME = tagCAFILETIME;
   tagCACLIPDATA = record
        cElems : ULONG;
        pElems : ^CLIPDATA;
     end;
   CACLIPDATA = tagCACLIPDATA;
   TCACLIPDATA = tagCACLIPDATA;
   tagCACLSID = record
        cElems : ULONG;
        pElems : ^CLSID;
     end;
   CACLSID = tagCACLSID;
   TCACLSID = tagCACLSID;

   PROPVAR_PAD1 = WORD;
   PROPVAR_PAD2 = WORD;
   PROPVAR_PAD3 = WORD;

// Forward interfaces.

   IStream             = Types.IStream;
   IMoniker            = Interface;
   IEnumMoniker        = Interface;
   IEnumString         = Interface;
   IRunningObjectTable = Interface;
   IStorage            = Interface;
   IEnumSTATSTG        = Interface;
   IAdviseSink         = Interface;
   IBindCtx            = Interface;
   IAsyncManager       = Interface;
   ICallFactory        = Interface;
   ISynchronize        = Interface;
   ITypeLib            = Interface;
   IPropertyStorage    = Interface;
   IEnumSTATPROPSETSTG = interface;

   TPROPVARIANT = record
          vt : VARTYPE;
          wReserved1 : PROPVAR_PAD1;
          wReserved2 : PROPVAR_PAD2;
          wReserved3 : PROPVAR_PAD3;
          case longint of
                 0 : ( cVal : CHAR );
                 1 : ( bVal : UCHAR );
                 2 : ( iVal : SHORT );
                 3 : ( uiVal : USHORT );
                 4 : ( lVal : LONG );
                 5 : ( ulVal : ULONG );
                 6 : ( intVal : longINT );
                 7 : ( uintVal : UINT );
                 8 : ( hVal : LARGE_INTEGER );
                 9 : ( uhVal : ULARGE_INTEGER );
                 10 : ( fltVal : SINGLE );
                 11 : ( dblVal : DOUBLE );
                 12 : ( boolVal : VARIANT_BOOL );
                 13 : ( bool : _VARIANT_BOOL );
                 14 : ( scode : SCODE );
                 15 : ( cyVal : CY );
                 16 : ( date : DATE );
                 17 : ( filetime : FILETIME );
                 18 : ( puuid : ^CLSID );
                 19 : ( pclipdata : ^CLIPDATA );
                 20 : ( bstrVal : BSTR );
                 21 : ( bstrblobVal : BSTRBLOB );
                 22 : ( blob : BLOB );
                 23 : ( pszVal : LPSTR );
                 24 : ( pwszVal : LPWSTR );
                 25 : ( punkVal : pointer; { IUnknown to avoid Data types which require initialization/finalization can't be used in variant records});
                 26 : ( pdispVal : pointer; {IDispatch} );
                 27 : ( pStream : pointer {IStream} );
                 28 : ( pStorage : pointer{IStorage} );
                 29 : ( pVersionedStream : LPVERSIONEDSTREAM );
                 30 : ( parray : LPSAFEARRAY );
                 31 : ( cac : CAC );
                 32 : ( caub : CAUB );
                 33 : ( cai : CAI );
                 34 : ( caui : CAUI );
                 35 : ( cal : CAL );
                 36 : ( caul : CAUL );
                 37 : ( cah : CAH );
                 38 : ( cauh : CAUH );
                 39 : ( caflt : CAFLT );
                 40 : ( cadbl : CADBL );
                 41 : ( cabool : CABOOL );
                 42 : ( cascode : CASCODE );
                 43 : ( cacy : CACY );
                 44 : ( cadate : CADATE );
                 45 : ( cafiletime : CAFILETIME );
                 46 : ( cauuid : CACLSID );
                 47 : ( caclipdata : CACLIPDATA );
                 48 : ( cabstr : CABSTR );
                 49 : ( cabstrblob : CABSTRBLOB );
                 50 : ( calpstr : CALPSTR );
                 51 : ( calpwstr : CALPWSTR );
                 52 : ( capropvar : CAPROPVARIANT );
                 53 : ( pcVal : pCHAR );
                 54 : ( pbVal : pUCHAR );
                 55 : ( piVal : pSHORT );
                 56 : ( puiVal : pUSHORT );
                 57 : ( plVal : pLONG );
                 58 : ( pulVal : pULONG );
                 59 : ( pintVal : plongint );
                 60 : ( puintVal : pUINT );
                 61 : ( pfltVal : psingle );
                 62 : ( pdblVal : pDOUBLE );
                 63 : ( pboolVal : ^VARIANT_BOOL );
                 64 : ( pdecVal : pDECIMAL );
                 65 : ( pscode : ^SCODE );
                 66 : ( pcyVal : ^CY );
                 67 : ( pdate : ^DATE );
                 68 : ( pbstrVal : ^TBSTR );
                 69 : ( ppunkVal : ^IUnknown );
                 70 : ( ppdispVal : ^IDispatch );
                 71 : ( pparray : ^LPSAFEARRAY );
                 72 : ( pvarVal : ^PROPVARIANT );
             end;
     PROPVARIANT=TPROPVARIANT;
     TagPROPVARIANT = TPROPVARIANT;
// Unknwn.idl

// IUnknown is in classesh.inc



   AsyncIUnknown = Interface(IUnknown)
      ['{000e0000-0000-0000-C000-000000000046}']
        Function Begin_QueryInterface(Const riid : TIID): HResult; StdCall;
        Function Finish_QueryInterface(Out ppvObject : Pointer):HResult;StdCall;
        Function Begin_AddRef:HResult;StdCall;
        Function Finish_AddRef:ULong;StdCall;
        Function Begin_Release:HResult;StdCall;
        Function Finish_Release:ULong;StdCall;
        End;

   IClassFactory = Interface(IUnknown)
      ['{00000001-0000-0000-C000-000000000046}']
      Function CreateInstance(Const unkOuter:IUnknown;Const riid : TIID;Out vObject):HResult; StdCall;
      Function LockServer(fLock : Bool):HResult;StdCall;
      End;

    PLicInfo = ^TLicInfo;
    tagLICINFO = record
      cbLicInfo : ULONG;
      fRuntimeKeyAvail : BOOL;
      fLicVerified : BOOL;
    end;
    TLicInfo = tagLICINFO;
    LICINFO = TLicInfo;

    IClassFactory2 = interface(IClassFactory)
      ['{B196B28F-BAB4-101A-B69C-00AA00341D07}']
      function GetLicInfo(var licInfo: TLicInfo): HResult; stdcall;
      function RequestLicKey(dwResrved: DWORD; out bstrKey: WideString): HResult; stdcall;
      function CreateInstanceLic(const unkOuter: IUnknown; const unkReserved: IUnknown;
        const iid: TIID; const bstrKey: WideString; out vObject): HResult; stdcall;
    end;

// objidl.idl

{****************************************************************************
 *  Component Object Interfaces
 ****************************************************************************}

     IMarshal = Interface(IUnknown)
        ['{00000003-0000-0000-C000-000000000046}']
        Function GetUnmarshalClass ( Const riid: TIID; pv:Pointer; Const dwDestContext:DWord;
                    pvDestContext:Pointer; Const mshlflags:DWORD;out LCid : TCLSID ):HResult;Stdcall;
        Function GetMarshalSizeMax ( Const Riid: TIID; {in, unique} pv:Pointer; Const dwDestContext : DWord;
                   {in, unique} pvDestContext:Pointer; Const mshlflags : DWord; out pSize : PDWord ): HResult;Stdcall;
        Function MarshalInterface ( Const {in, unique} pStm: IStream; Const riid: TIID; {in, unique} pv:Pointer;
                   Const dwDestContext:DWord; {in, unique} pvDestContext:Pointer; Const mshlflags:DWord ): HRESULT;Stdcall;
        Function UnmarshalInterface ( {[in, unique]} Const pStm:IStream; Const riid: TIID;
                   out ppv ): HResult;Stdcall;
        Function ReleaseMarshalData ( {[in, unique]} Const Strm: IStream ):HResult;Stdcall;
        Function DisconnectObject ( Const dwReserved:DWord ):HRESULT;Stdcall;
        END;


     IMarshal2 = Interface(IMarshal)
        ['{000001cf-0000-0000-C000-000000000046}']
        End;

     IMalloc   = Interface(IUnknown)
        ['{00000002-0000-0000-C000-000000000046}']
        Function  Alloc(cb :size_t):Pointer; Stdcall;
        Function  Realloc (pv :pointer;cb:size_t):Pointer;stdcall;
        Procedure Free({[in]} pv: pointer); Stdcall;
        Function  GetSize(pv:pointer):size_t;stdcall;
        Function  DidAlloc(pv:pointer):Longint;stdcall;
        procedure HeapMinimize; stdcall;
        End;

     IMallocSpy = Interface(IUnknown)
        ['{0000001d-0000-0000-C000-000000000046}']

        Function  PreAlloc(cbrequest:Size_t):Longint; StdCall;
        function  PostAlloc(Pactual:Pointer):Pointer;StdCall;
        Function  PreFree(pRequest:Pointer;fSpyed:bool):pointer;StdCall;
        Procedure PostFree(fspyed:Bool);Stdcall;
        Function  PreRealloc(pRequest:Pointer;cbRequest:Size_t;Out ppNewRequest:Pointer;
                        fSpyed:Bool):Size_t;Stdcall;
        Function  PostRealloc(pactual:Pointer;fspyed:Bool):pointer;Stdcall;
        Function  PreGetSize(pRequest:pointer;fSpyed:Bool):Pointer;StdCall;
        Function  PostGetSize(cbactual:Size_t;fSpyed:Bool):Size_t;StdCall;
        Function  PreDidAlloc(pRequest:pointer;fSpyed:Bool):pointer;stdcall;
        Function  PostDidAlloc(pRequest:pointer;fSpyed:Bool;Factual:Longint):pointer;stdcall;
        Procedure PreHeapMinimize;StdCall;
        Procedure PostHeapMinimize;StdCall;
        End;

     IStdMarshalInfo = Interface(IUnknown)
       ['{00000018-0000-0000-C000-000000000046}']
       Function GetClassForHandler (dwDestContext : DWord;pvDestContext:pointer;out Clsid : Pclsid ):HResult;Stdcall;
       End;


     IExternalConnection = Interface(IUnknown)
       ['{00000019-0000-0000-C000-000000000046}']
       Function AddConnection    (ExtConn: DWord; Reserved: DWord):DWord;Stdcall;
       Function ReleaseConnection(extconn: DWord; Reserved: Dword;FLastReleaseCloses: Bool):DWord;StdCall;
      End;


      IMultiQI = Interface(IUnknown)
        ['{00000020-0000-0000-C000-000000000046}']
//if (__midl >= 500)
//    async_uuid(000e0020-0000-0000-C000-000000000046)
//endif
        Function QueryMultipleInterfaces(cMQIs:Ulong;pMQIs:pMultiQI):HResult;StdCall;
        END;

     IInternalUnknown=Interface(IUnknown)
        ['{00000021-0000-0000-C000-000000000046}']
        Function QueryInternalInterface(const riid:TIID;Out ppv:Pointer):HResult;StdCall;
        END;


     IEnumUnknown = Interface(IUnknown)
        ['{00000100-0000-0000-C000-000000000046}']
        //    pointer_default(unique)
     Function Next(Celt:Ulong;out rgelt;out pCeltFetched:pulong):HRESULT;StdCall;
//    HRESULT RemoteNext(        [in] ULONG celt,        [out, size_is(celt), length_is( *pceltFetched)]        IUnknown **rgelt,        [out] ULONG *pceltFetched);
     Function Skip(Celt:Ulong):HResult;StdCall;
     Function Reset():HResult;
     Function Close(Out ppenum: IEnumUnknown):HResult;
     END;


    IBindCtx = Interface (IUnknown)
       ['{0000000e-0000-0000-C000-000000000046}']
       Function RegisterObjectBound(Const punk:IUnknown):HResult; stdCall;
       Function RevokeObjectBound (Const Punk:IUnknown):HResult;  stdCall;
       Function ReleaseBoundObjects :HResult;  StdCall;
       Function SetBindOptions(Const bindOpts:TBind_Opts):HResult;  stdCall;
//       Function RemoteSetBindOptions(Const bind_opts: TBind_Opts2):HResult;StdCall;
       Function GetBindOptions(var BindOpts:TBind_Opts):HResult;  stdCall;
//       Function RemoteGetBindOptions(Var bind_opts: TBind_Opts2):HResult;StdCall;
       Function GetRunningObjectTable(Out rot : IRunningObjectTable):Hresult; StdCall;
       Function RegisterObjectParam(Const pszkey:LPOleStr;const punk:IUnknown):HResult;
       Function GetObjectParam(Const pszkey:LPOleStr; out punk: IUnknown):HResult; StdCall;
       Function EnumObjectParam (out enum:IEnumString):Hresult;StdCall;
       Function RevokeObjectParam(pszKey:LPOleStr):HResult;StdCall;
       End;


    IEnumMoniker = Interface (IUnknown)
       ['{00000102-0000-0000-C000-000000000046}']
       Function Next(celt:ULong; out Elt;out celftfetched: ULong):HResult; StdCall;
//     Function RemoteNext(Celt:ULong; Out rgelt;out celtfetched :ULong):Hresult; StdCall;
       Function Skip(celt:Ulong):HResult; StdCall;
       Function Reset:HResult; StdCall;
       Function Close(out penum:IEnumMoniker):HResult;StdCall;
       End;


    IRunnableObject = Interface(IUnknown)
    ['{00000126-0000-0000-C000-000000000046}']
       Function GetRunningClass(Out clsid:Tclsid):Hresult; StdCall;
       Function Run(Const pb: IBindCtx):HResult; StdCall;
       Function IsRunning:Bool; StdCall;
//     Function RemoteIsRunning:Bool; StdCall;
       Function LockRunning(FLock,FLastUnlockClose:BOOL):HResult; StdCall;
       Function SetContainedObject(fContained:Bool):Hresult;Stdcall;
       End;

    IRunningObjectTable = Interface (IUnknown)
       ['{00000010-0000-0000-C000-000000000046}']
       Function Register  (grfFlags :DWord;const unkobject:IUnknown;Const mkObjectName:IMoniker;Out dwregister:DWord):HResult;StdCall;
       Function Revoke    (dwRegister:DWord):HResult; StdCall;
       Function IsRunning (Const mkObjectName: IMoniker):HResult;StdCall;
       Function GetObject (Const mkObjectName: IMoniker; Out punkObject:IUnknown):HResult; StdCall;
       Function NoteChangeTime(dwRegister :DWord;Const FileTime: TFileTime):HResult;StdCall;
       Function GetTimeOfLastChange(Const mkObjectName:IMoniker;Out filetime:TFileTime):HResult; StdCall;
       Function EnumRunning (Out enumMoniker: IEnumMoniker):HResult; StdCall;
       End;

    IPersist = Interface (IUnknown)
       ['{0000010c-0000-0000-C000-000000000046}']
       Function GetClassId(out clsid:TClsId):HResult; StdCall;
       End;

    IPersistStream = Interface(IPersist)
       ['{00000109-0000-0000-C000-000000000046}']
       Function IsDirty:HResult; StdCall;
       Function Load(Const stm: IStream):HResult; StdCall;
       Function Save(Const stm: IStream;fClearDirty:Bool):HResult;StdCall;
       Function GetSizeMax(Out cbSize:ULarge_Integer):HResult; StdCall;
       End;

    PIMoniker = ^IMoniker;
    IMoniker = Interface (IPersistStream)
      ['{0000000f-0000-0000-C000-000000000046}']
      Function BindToObject (const pbc:IBindCtx;const mktoleft:IMoniker; const RiidResult:TIID;Out vresult):HResult;StdCall;
//    Function RemoteBindToObject (const pbc:IBindCtx;const mktoleft:IMoniker;const RiidResult:TIID;Out vresult):HResult;StdCall;
      Function BindToStorage(Const Pbc:IBindCtx;Const mktoLeft:IMoniker; const Riid:TIID;Out vobj):HResult; StdCall;
//    Function RemoteBindToStorage(Const Pbc:IBindCtx;Const mktoLeft:IMoniker;const Riid:TIID;Out vobj):HResult; StdCall;
      Function Reduce (const pbc:IBindCtx; dwReduceHowFar:DWord; mktoLeft: PIMoniker; Out mkReduced:IMoniker):HResult; StdCall;
      Function ComposeWith(Const MkRight:IMoniker;fOnlyIfNotGeneric:BOOL; OUT mkComposite:IMoniker):HResult; StdCall;
      Function Enum(fForward:Bool;Out enumMoniker:IEnumMoniker):HResult;StdCall;
      Function IsEqual(Const mkOtherMoniker:IMoniker):HResult;StdCall;
      Function Hash   (Out dwHash:Dword):HResult;StdCall;
      Function IsRunning(Const bc:IBindCtx;Const MkToLeft:IMoniker;Const mknewlyRunning:IMoniker):HResult;StdCall;
      Function GetTimeOfLastChange(Const bc:IBindCtx;Const mkToLeft:IMoniker; out ft : FileTime):HResult; StdCall;
      Function Inverse(out mk : IMoniker):HResult; StdCall;
      Function CommonPrefixWith (Const mkOther:IMoniker):HResult; StdCall;
      Function RelativePathTo(Const mkother:IMoniker; Out mkRelPath : IMoniker):HResult;StdCall;
      Function GetDisplayName(Const bc:IBindCtx;const mktoleft:IMoniker;Out szDisplayName: pOleStr):HResult; StdCall;
      Function ParseDisplayName(Const bc:IBindCtx;Const mkToLeft:IMoniker;szDisplayName:POleStr;out cheaten:ULong;out mkOut:IMoniker):HResult; StdCall;
      Function IsSystemMoniker(Out dwMkSys:DWord):HResult;StdCall;
      End;

    IROTData = Interface (IUnknown)
       ['{f29f6bc0-5021-11ce-aa15-00006901293f}']
       Function GetComparisonData(out data; cbMax:ULong;out cbData:ULong):HResult;StdCall;
       End;


    IEnumString = Interface (IUnknown)
       ['{00000101-0000-0000-C000-000000000046}']
       Function Next(Celt:ULong;Out xcelt;Out Celtfetched:ULong):HResult; StdCall;
//     Function RemoteNext(Celt:ULong; Out celt;Out Celtfetched:ULong):HResult; StdCall;
       Function Skip (Celt:ULong):Hresult;StdCall;
       Function Reset:HResult;StdCall;
       Function Clone(Out penum:IEnumString):HResult;StdCall;
       End;

    ISequentialStream = Types.ISequentialStream;
    (*interface(IUnknown)
       ['{0c733a30-2a1c-11ce-ade5-00aa0044773d}']
       function Read(pv : Pointer;cb : ULONG;pcbRead : PULONG) : HRESULT;stdcall;
       function Write(pv : Pointer;cb : ULONG;pcbWritten : PULONG): HRESULT;stdcall;
     end;
    *)

    (* defined above by pulling it in from types IStream = interface(ISequentialStream)
       ['{0000000C-0000-0000-C000-000000000046}']
       function Seek(dlibMove : LargeInt; dwOrigin: Longint;
            out libNewPosition : LargeInt): HResult; stdcall;
       function SetSize(libNewSize : LargeInt) : HRESULT;stdcall;
       function CopyTo(stm: IStream;cb : LargeInt;out cbRead : LargeInt;
            out cbWritten: LargeInt) : HRESULT;stdcall;
       function Commit(grfCommitFlags : Longint) : HRESULT; stdcall;
       function Revert : HRESULT; stdcall;
       function LockRegion(libOffset : LargeInt;cb : LargeInt;
            dwLockType: Longint) : HRESULT;stdcall;
       function UnlockRegion(libOffset: LargeInt;cb: LargeInt;
            dwLockType: Longint) : HRESULT;stdcall;
       Function Stat(out statstg : TStatStg; grfStatFlag: Longint): HRESULT;stdcall;
       function Clone(out stm : IStream) : HRESULT; stdcall;
     end;
    *)
    IEnumSTATSTG = Interface (IUnknown)
       ['{0000000d-0000-0000-C000-000000000046}']
        Function Next (Celt:ULong;Out xcelt;pceltfetched : PUlong):HResult; StdCall;
//     Function RemoteNext(Celt:Ulong; Out Celt;pceltfetched : PUlong);
       Function Skip(Celt:ULong):HResult; StdCall;
       Function Reset:HResult; StdCall;
       Function Clone(Out penum:IEnumStatSTG):HResult; StdCall;
       End;

    IStorage = Interface (IUnknown)
       ['{0000000b-0000-0000-C000-000000000046}']
       Function CreateStream(pwcsname:POleStr;GrfMode,Reserved1,Reserved2 : DWord; Out stm : IStream):HResult; StdCall;
       Function OpenStream(pwcsname:POleStr;Reserved1:Pointer;GrfMode,Reserved2 : DWord; Out stm : IStream):HResult; StdCall;
//       Function RemouteOpenStream(pwcsname:POleStr;cbReserved1:ULong;reserved1:pbyte;GrfMode,Reserved2 : DWord; Out stm : IStream):HResult; StdCall;
       Function CreateStorage(pwcsname:POleStr;GrfMode,Reserved1,Reserved2 : DWord; Out stm : IStorage):HResult; StdCall;
       Function OpenStorage(pwcsname:POleStr;Const stgPriority:IStorage;grfmode : DWord;Const SNBExclude :SNB;reserved:DWord;Out stm : IStorage):HResult; StdCall;
       Function CopyTo(ciidExclude:DWord; rgiidexclude:piid; const snbexclude:SNB;const pstg : IStorage):HResult;StdCall;
       Function MoveElementTo(wcsName:POleStr;Const pstgDest : IStorage;
                                wcvsNewName:POleStr; GrfFlags:DWord):Hresult; StdCall;
       Function Commit(grfCommitFlags:Dword):Hresult; StdCall;
       Function Revert:HResult; StdCall;
       Function EnumElements(Reserved1 :Dword;Reserved2:Pointer;Reserved3:DWord;Out penum:IEnumStatStg):HResult;StdCall;
       Function RemoteEnumElements(Reserved1 :Dword;cbReserved2:ULong;Reserved2:pbyte;reserved3:DWord;Out penum:IEnumStatStg):HResult;StdCall;
       Function DestroyElement(wcsName: POleStr):HResult;StdCall;
       Function RenameElement(wcsoldName: POleStr;wcsnewName: POleStr):HResult;StdCall;
       Function SetElementTimes(wcsName:POleStr; Const pctime,patime,pmtime : FileTime):HResult;StdCall;
       Function SetClass(Const ClasId: TClsID):HResult;StdCall;
       Function SetStateBits(grfStateBits:DWord;grfMask:DWord):HResult;StdCall;
       Function Stat(Out pStatStg:StatStg;grfStatFlag:DWord):HResult;StdCall;
       End;

    IPersistFile = Interface (IPersist)
       ['{0000010b-0000-0000-C000-000000000046}']
       Function IsDirty:HResult;StdCall;
       Function Load(FileName:POleStr;dwMode:DWord):HResult;StdCall;
       Function Save(FileName:POleStr;fremember:Bool):HResult;StdCall;
       Function SaveCompleted(FileName:POleStr):HResult;StdCall;
       Function GetCurFIle(Out FileName:POleStr):HResult;StdCall;
       End;


    IPersistStorage = Interface (IPersist)
       ['{0000010a-0000-0000-C000-000000000046}']
       Function IsDirty:HResult;StdCall;
       Function InitNew(const pstg:IStorage):HResult;StdCall;
       Function Load(const pstg:IStorage):HResult;StdCall;
       Function Save(const pstg:IStorage;FSameAsLoad:Boolean):HResult;StdCall;
       Function SaveCompleted(const pstg:IStorage):HResult;StdCall;
       Function HandsOffStorage:HResult;StdCall;
       End;

    ILockBytes = Interface (IUnknown)
       ['{0000000a-0000-0000-C000-000000000046}']
       Function ReadAt(ulOffset:ULarge_Integer;pv:Pointer;cb:Ulong; Out pcbRead:ULong):HResult; StdCall;
//       Function RemoteReadAt(ulOffset:ULarge_Integer;pv:Pointer;cb:Ulong; Out pcbRead:ULong):HResult; StdCall;
       Function WriteAt(ulOffset:ULarge_Integer;pv:Pointer;cb:Ulong; Out pcbWritten:ULong):HResult; StdCall;
//       Function RemoteWriteAt(ulOffset:ULarge_Integer;pv:Pointer;cb:Ulong; Out pcbWritten:ULong):HResult; StdCall;
       Function Flush:HResult;StdCall;
       Function SetSize(cb:ULarge_Integer):HResult;StdCall;
       Function LockRegion(LibOffSet:ULarge_Integer;cb:ULarge_Integer;dwLockType:DWord):HResult;StdCall;
       Function UnlockRegion(LibOffSet:ULarge_Integer;cb:ULarge_Integer;dwLockType:DWord):HResult;StdCall;
       Function Stat(Out pstatstg:STATSTG;grfstatFlag:DWord):HResult;StdCall;
       End;


   IEnumFORMATETC = Interface (IUnknown)
     ['{00000103-0000-0000-C000-000000000046}']
     Function Next(Celt:ULong;Out Rgelt:FormatEtc;pceltFetched:pULong=nil):HResult; StdCall;
//     Function RemoteNext(Celt:ULong;Out Rgelt:FormatEtc; pceltFetched:pULong=nil):HResult; StdCall;
     Function Skip(Celt:ULong):HResult;StdCall;
     Function Reset:HResult;StdCall;
     Function Clone(out penum:IEnumFORMATETC):HResult;StdCall;
     End;

    IEnumSTATDATA = Interface (IUnknown)
        ['{00000105-0000-0000-C000-000000000046}']
        Function Next(Celt:ULong;Out Rgelt:statdata; pceltFetched:pULong=nil):HResult; StdCall;
//      Function RemoteNext(Celt:ULong;Out Rgelt:statdata;Out pceltFetched:ULong):HResult; StdCall;
        Function Skip(Celt:ULong):HResult;StdCall;
        Function Reset:HResult;StdCall;
        Function Clone(out penum:IEnumstatdata):HResult;StdCall;
        End;



    IRootStorage = Interface (IUnknown)
        ['{00000012-0000-0000-C000-000000000046}']
        Function SwitchToFile(pszfile:PoleStr):HResult;StdCall;
        End;



    IAdviseSink = Interface (IUnknown)
        ['{0000010f-0000-0000-C000-000000000046}']
    {$ifdef midl500} ['{00000150-0000-0000-C000-000000000046}'] {$endif}
        Procedure OnDataChange (Const pformatetc : Formatetc;const pstgmed : STGMEDIUM); StdCall;
        Procedure OnViewChange (dwAspect : DWord; lindex : Long); StdCall;
        Procedure OnRename (Const pmk : IMoniker); StdCall;
        Procedure OnSave; StdCall;
        Procedure OnClose; StdCall;
        End;

    IAdviseSink2 = Interface (IAdviseSink)
       ['{00000125-0000-0000-C000-000000000046}']
        Procedure OnLinkSrcChange(Const Pmk: IMoniker); StdCall;
        End;


    IDataObject = Interface (IUnknown)
       ['{0000010e-0000-0000-C000-000000000046}']
       Function GetData(Const formatetcIn : FORMATETC;Out medium : STGMEDIUM):HRESULT; STDCALL;
       Function GetDataHere(CONST pformatetc : FormatETC; Out medium : STGMEDIUM):HRESULT; STDCALL;
       Function QueryGetData(const pformatetc : FORMATETC):HRESULT; STDCALL;
       Function GetCanonicalFormatEtc(const pformatetcIn : FORMATETC;Out pformatetcOut : FORMATETC):HResult; STDCALl;
       Function SetData (Const pformatetc : FORMATETC;const medium:STGMEDIUM;FRelease : BOOL):HRESULT; StdCall;
       Function EnumFormatEtc(dwDirection : DWord; OUT enumformatetcpara : IENUMFORMATETC):HRESULT; StdCall;
       Function DAdvise(const formatetc : FORMATETC;advf :DWORD; CONST AdvSink : IAdviseSink;OUT dwConnection:DWORD):HRESULT;StdCall;
       Function DUnadvise(dwconnection :DWord) :HRESULT;StdCall;
       Function EnumDAdvise(Out enumAdvise : IEnumStatData):HResult;StdCall;
       End;

    IDataAdviseHolder = Interface (IUnknown)
       ['{00000110-0000-0000-C000-000000000046}']
       Function Advise    (CONST pdataObject : IDataObject;CONST fetc:FORMATETC;advf : DWORD;Const pAdvise:IAdviseSink;Out DwConnection:DWord):HResult; StdCall;
       Function Unadvise  (dwConnection:Dword):HResult; StdCall;
       Function EnumAdvise(out penumAdvise : IEnumStatData):HResult;StdCall;
       Function SendOnDataChange(const pDataObject :IDataObject;DwReserved,advf : DWord):HResult; StdCall;
       End;




    IMessageFilter = Interface (IUnknown)
       ['{00000016-0000-0000-C000-000000000046}']
       Function HandleInComingCall(dwCallType :DWord;htaskCaller : HTASK; dwTickCount: DWORD;CONST sinterfaceinfo:InterfaceInfo):DWord; StdCall;
       Function RetryRejectedCall (htaskCallee:HTASK; dwTickCount : DWord; dwRejectType : Dword):DWord; StdCall;
       Function MessagePending    (htaskCallee:HTASK; dwTickCount : DWord; dwPendingType : Dword):DWord; StdCall;
       End;

//****************************************************************************
//*  Object Remoting Interfaces
//****************************************************************************



    IRpcChannelBuffer = Interface (IUnknown)
       ['{D5F56B60-593B-101A-B569-08002B2DBF7A}']
       Function GetBuffer (Const pMesasge : RPCOLEMESSAGE;Const riid :TIId):HResult; StdCall;
       Function SendReceive(Var pMessage : RPCOLEMESSAGE; Out PStatus : ULong):HResult; StdCall;
       Function FreeBuffer(Const pMessage : RPCOLEMESSAGE):HResult; StdCall;
       Function GetDestCTX(Out dwDestContext : DWord;Out pvDestContext : Pointer):HResult; StdCall;
       Function IsConnected:HResult; StdCall;
       End;

    IRpcChannelBuffer2 = Interface (IRpcChannelBuffer)
       ['{594f31d0-7f19-11d0-b194-00a0c90dc8bf}']
       Function GetProtocolVersion(Var dwVersion : DWord):HResult; StdCall;
       End;


    IAsyncRpcChannelBuffer = Interface (IRpcChannelBuffer2)
       ['{a5029fb6-3c34-11d1-9c99-00c04fb998aa}']
       Function Send(Var Msg: RPCOLEMESSAGE;Const pSync : ISynchronize;Out PulStatus : ULong):HResult; StdCall;
       Function Receive(Var Msg: RPCOLEMESSAGE;Out PulStatus : ULong):HResult; StdCall;
       Function GetDestCTXEx(Out MSG : RPCOLEMESSAGE;Out vDestContext : DWord;Out pvDestContext : Pointer ):HResult;StdCall;
       End;

    IRpcChannelBuffer3 = Interface (IRpcChannelBuffer2)
       ['{25B15600-0115-11d0-BF0D-00AA00B8DFD2}']
       Function Send(Var msg : RPCOLEMESSAGE;Out ulStatus : ULONG):HResult; StdCall;
       Function Receive(Var msg : RPCOLEMESSAGE;ulSize : ULong;Out ulStatus : ULONG):HResult; StdCall;
       Function Cancel (Const msg : RPCOLEMESSAGE):HResult; StdCall;
       Function GetCallContext(Const msg : RPCOLEMESSAGE; Const riid : TIID; Out pInterface : Pointer):HResult; StdCall;
       Function GetDestCTXEx(Const Msg : RPCOLEMESSAGE;Out vDestContext : DWord;Out pvDestContext : Pointer ):HResult;StdCall;
       Function GetState(Const Msg : RPCOLEMESSAGE;Out State: DWord):HResult;StdCall;
       Function RegisterAsync(Const Msg : RPCOLEMESSAGE;Const asyncmgr : IAsyncManager):HResult;StdCall;
       End;

    IRpcSyntaxNegotiate = Interface (IUnknown)
       ['{58a08519-24c8-4935-b482-3fd823333a4f}']
       Function NegotiateSyntax ( Var msg :  RPCOLEMESSAGE):HResult; StdCall;
       End;




    IRpcProxyBuffer = Interface (IUnknown)
       ['{D5F56A34-593B-101A-B569-08002B2DBF7A}']
       Function Connect(Const rpcchannelbuffer : IRpcChannelBuffer):HResult; StdCall;
       Procedure Disconnect;
       End;

    IRpcStubBuffer = Interface (IUnknown)
       ['{D5F56AFC-593B-101A-B569-08002B2DBF7A}']
       Function COnnect ( Const UnkServer : IUnknown):HResult; StdCall;
       Procedure Disconnect; StdCall;
       Function Invoke(Const rpcmsg : RPCOLEMESSAGE;Const RpcChanBuf : IRpcChannelBuffer):HResult; StdCall;
       Function IsIIDSupported (Const riid : TIID):Pointer {IRpcStubBuffer}; StdCall;
       Function CountRefs :ULong; StdCall;
       Function DebugServerQueryInterface(CONST pv : Pointer):HResult; StdCall;
       Procedure DebugServerRelease (pv : Pointer); StdCall;
       End;

    IPSFactoryBuffer = Interface (IUnknown)
       ['{D5F569D0-593B-101A-B569-08002B2DBF7A}']
       Function CreateProxy(Const UnkOuter : IUnknown;const riid : TIID; Out proxy: IRpcProxyBuffer; Out Pv :Pointer):HResult; StdCall;
       Function CreateStub (Const riid : TIID; Const UnkServer : IUnknown; Out pstub : IRpcStubBuffer):HResult; StdCall;
       End;

{$ifdef NT4_greater_Or_DCOM}
// This interface is only valid on Windows NT 4.0

// This structure contains additional data for hooks.  As a backward
// compatability hack, the entire structure is passed in place of the
// RIID parameter on all hook methods.  Thus the IID must be the first
// parameter.  As a forward compatability hack the second field is the
// current size of the structure.

    SChannelHookCallInfo= Record;
                            IID               : iid;
                            cbSize            : Dword;
                            uCausality        : GUID;
                            dwServerPid,
                            iMethod           : DWord;
                            pObject           : Pointer;
                            End;



  IChannelHook = Interface (IUnknown)
     ['{1008c4a0-7613-11cf-9af1-0020af6e72f4}']
     Procedure ClientGetSize(Const uExtent : TGuid; CONST riid : TIID; Out datasize :ULong); StdCall;
     Procedure ClientFillBuffer(Const uExtent : TGuid; CONST riid : TIID; Var datasize :ULong;Buffer :Pointer); StdCall;
     Procedure ClientNotify(Const uExtent : TGuid; CONST riid : TIID; datasize :ULong;Buffer :Pointer;hrfault:HResult); StdCall;
     Procedure ServerNotify(Const uExtent : TGuid; CONST riid : TIID; datasize :ULong;Buffer :Pointer;DataRep:DWord); StdCall;
     Procedure ServerGetSize(Const uExtent : TGuid; CONST riid : TIID;hrFault :HResult; Out datasize :ULong); StdCall;
     Procedure ServerFillBuffer(Const uExtent : TGuid; CONST riid : TIID; Var datasize :ULong;Buffer :Pointer;HrFault:HResult); StdCall;
     End;
{$Endif}


// Well-known Property Set Format IDs
//FMTID_SummaryInformation              = {CONST} FMTID;
//FMTID_DocSummaryInformation           = {CONST} FMTID;
//FMTID_UserDefinedProperties           = {CONST} FMTID;
//FMTID_DiscardableInformation          = {CONST} FMTID;
//FMTID_ImageSummaryInformation         = {CONST} FMTID;
//FMTID_AudioSummaryInformation         = {CONST} FMTID;
//FMTID_VideoSummaryInformation         = {CONST} FMTID;
//FMTID_MediaFileSummaryInformation     = {CONST} FMTID;


//****************************************************************************
// *  Connection Point Interfaces
// ****************************************************************************/

//#ifdef __INCLUDE_CPIFS
     IConnectionPointContainer = Interface;
//interface IConnectionPoint;
//interface IEnumConnections;
     IEnumConnectionPoints = Interface;
     IEnumConnections      = Interface;


    IConnectionPoint = Interface (IUnknown)
       ['{B196B286-BAB4-101A-B69C-00AA00341D07}']
       Function GetConnectionInterface(out piid : TIID):HResult;StdCall;
       Function GetConnectionPointContainer(CPC : IConnectionPointContainer):HResult;StdCall;
       Function Advise(unkSink : IUnknown;Out dwCookie : DWord):HResult;StdCall;
       Function UnAdvise(dwCookie : DWord):HResult;StdCall;
       Function EnumConnection(out pEnum : IEnumConnections):HResult;stdCall;
      End;

    IConnectionPointContainer = Interface (IUnknown)
       ['{B196B284-BAB4-101A-B69C-00AA00341D07}']
       Function EnumConnectionPoints(out pEnum : IEnumConnectionPoints):HResult;StdCall;
       Function FindConnectionPoint(Const RIID : TIID;Out ppcp : IConnectionPoint):HResult;StdCall;
       End;

    tagCONNECTDATA = Record
                      unk        : Pointer; {IUnknown}
                      dwCookie   : DWord;
                      End;
    ConnectData    = tagCONNECTDATA;

    IEnumConnections = Interface (IUnknown)
       ['{B196B287-BAB4-101A-B69C-00AA00341D07}']
       Function Next(cConnections : ULong; Out rgcd : ConnectData; lpcFetched : pULong=nil):HResult;StdCall;
       Function Skip(cConnections : ULong):HResult;StdCall;
       Function Reset:HResult;StdCall;
       Function Clone(Out pEnum : IEnumConnections):HResult; StdCall;
       End;


    IEnumConnectionPoints = Interface (IUnknown)
       ['{B196B285-BAB4-101A-B69C-00AA00341D07}']
       Function Next(cConnections : ULong; Out rgpcm : IConnectionPoint; lpcFetched : pULong=nil):HResult;StdCall;
       Function Skip(cConnections : ULong):HResult;StdCall;
       Function Reset:HResult;StdCall;
       Function Clone(Out pEnum : IEnumConnectionPoints):HResult;StdCall;
       End;



    tagSOLE_AUTHENTICATION_SERVICE = Record
                                       dwAuthnSvc,
                                       dwAuthzSvc     : DWord;
                                       pPrincipalName : POleStr;
                                       hr             : HResult;
                                       End;
    SOLE_AUTHENTICATION_SERVICE   = tagSOLE_AUTHENTICATION_SERVICE;
    PSOLE_AUTHENTICATION_SERVICE  = ^SOLE_AUTHENTICATION_SERVICE;

    tagSOLE_AUTHENTICATION_INFO   = Record
                                       dwAuthnSvc,
                                       dwAuthzSvc     : DWord;
                                       AuthInfo       : Pointer;
                                       End;
    SOLE_AUTHENTICATION_INFO      = tagSOLE_AUTHENTICATION_INFO;
    PSOLE_AUTHENTICATION_INFO     = ^SOLE_AUTHENTICATION_INFO;

    tagSOLE_AUTHENTICATION_LIST   = Record
                                       cAuthInfo      : DWord;
                                       AuthInfo       : PSOLE_AUTHENTICATION_INFO;
                                       End;
    SOLE_AUTHENTICATION_LIST      = tagSOLE_AUTHENTICATION_LIST;
    PSOLE_AUTHENTICATION_LIST     = ^SOLE_AUTHENTICATION_LIST;

{$ifdef WINNT_DCOM}

    IClientSecurity = Interface (IUnknown)
        ['{0000013D-0000-0000-C000-000000000046}']
        Function QueryBlanket (Proxy : IUnknown;Out AuthnSvc,AuthzSvc : Dword;Out ServerPrincName:pOleStr;Out AuthnLevel,ImpLevel:Dword; Out AuthInfo : Pointer; Out Capabilities : Dword):HResult;StdCall;
        Function SetBlanket   (Proxy : IUnknown;AuthnSvc,AuthzSvc : Dword;ServerPrincName:pOleStr;AuthnLevel,ImpLevel:Dword;AuthInfo : Pointer;Capabilities : Dword):HResult;StdCall;
        Function CopyProxy    (Proxy : IUnknown;Out pcopy:IUnknown):HResult;StdCall;
        End;

    IServerSecurity = Interface (IUnknown)
       ['{0000013E-0000-0000-C000-000000000046}']
       Function QueryBlanket ( out authnSvc,AuthzSvc : DWord; Out pServerPrincName : pOleStr; Out AuthnLevel, ImpLevel; :DWord; out Privs : Pointer; Var Capabilities :DWord):HResult;StdCall;
       Function ImpersonateClient:HResult;StdCall;
       Function RevertToSelf:HResult;StdCall;
       Function IsImpersonating:Bool;StdCall;
    End;

    IClassActivator = Interface (IUnknown)
       ['{00000140-0000-0000-C000-000000000046}']
       Function GetClassObject(Const rclsif : TClsID; ClassContext : DWord; locale : LCID; Const ridd : TIID; Out pv : Pointer):HResult;StdCall;
       End;


    IRpcOptions = Interface (IUnknown)
       ['{00000144-0000-0000-C000-000000000046}']
       Function xSet (prx : IUnknown;dwProperty : DWord; dwValue:ULONG_PTR):HResult; StdCall;
       Function Query (prx : IUnknown;dwProperty:Dword; dwValue:ULONG_PTR):HResult; StdCall;
       End;

{$endif} {DCOM}

    IFillLockBytes = Interface (IUnknown)
       ['{99caf010-415e-11cf-8814-00aa00b569f5}']
       Function FillAppend(const pv : Pointer;cb:ULong; Out PcbWritten : ULong):HResult;StdCall;
       Function FillAt(ulOffset : ULarge_INTEGER;Const pv : Pointer;cb :ULong; Out pcbWritten:ULong):HResult;StdCall;
       Function SetFillSize ( ulSize :ULarge_Integer):HResult;StdCall;
       Function Terminate (bCanceled :Bool):HResult;StdCall;
    End;

    IProgressNotify = Interface (IUnknown)
       ['{a9d758a0-4617-11cf-95fc-00aa00680db4}']
       Function OnProgress (ProgressCurrent,ProgressMaximum :Dword; FAccurate,Fowner : Bool):HResult;StdCall;
       End;

    ILayoutStorage = Interface (IUnknown)
       ['{0e6d4d90-6738-11cf-9608-00aa00680db4}']
       {The methods in this interface all had "__stdcall" as modifier, while the other classes don't. ?!?!?}
       Function LayoutScript ( xStorageLayout : StorageLayout;nEntries,glfInterleaveFlag : Dword) :HResult; StdCall;
       Function BeginMonitor:HResult;StdCall;
       Function EndMonitor:HResult;StdCall;
       Function ReLayourDocFile(pwcsNewDFName :pOleStr):HResult;StdCall;
       Function ReLayoutDocfileOnILockBytes(LockBytes : ILockBytes):Hresult;StdCall;
       End;

    IBlockingLock = Interface (IUnknown)
       ['{30f3d47a-6447-11d1-8e3c-00c04fb9386d}']
       Function Lock (dwTimeOut : DWord) : HResult;Stdcall;
       Function Unlock : HResult;Stdcall;
       End;

    ITimeAndNoticeControl = Interface (IUnknown)
       ['{bc0bf6ae-8878-11d1-83e9-00c04fc2c6d4}']
       Function SuppressChanges(res1,res2 : Dword):HResult;StdCall;
       End;

    IOplockStorage = Interface (IUnknown)
       ['{8d19c834-8879-11d1-83e9-00c04fc2c6d4}']
       Function CreateStorageEx(wcsName : LPCWSTR;grfMode,StgFmt,GrfAtrrs :Dword;Const riid :Tiid; Out ppstgOpen : Pointer):HResult;StdCall;
       Function OpenStorageEx(wcsName : LPCWSTR;grfMode,StgFmt,GrfAtrrs :Dword;Const riid :Tiid; Out ppstgOpen : Pointer):HResult;StdCall;
       End;

    ISurrogate = Interface (IUnknown)
       ['{00000022-0000-0000-C000-000000000046}']
       Function LoadDllServer (Const ClsId : TClsId):HResult;StdCall;
       Function FreeSurrogate:HResult;StdCall;
       End;

    IGlobalInterfaceTable = Interface (IUnknown)
       ['{00000146-0000-0000-C000-000000000046}']
       Function RegisterInterfaceInGlobal(unk :IUnknown;Const riid : TIID; Out dwcookie :DWord):HResult;StdCall;
       Function RevokeInterfaceFromGlobal (dwCookie :DWord):HResult;StdCall;
       Function GetInterfaceFromGlobal (dwCookie :DWord;Const riid : TIID;out pv : Pointer):HResult;StdCall;
       End;

    IDirectWriterLock = Interface (IUnknown)
       ['{0e6d4d92-6738-11cf-9608-00aa00680db4}']
       Function WaitForWriteAccess (dwTimeOut : DWORD):HResult;StdCall;
       Function ReleaseWriteAccess:HResult;StdCall;
       Function HaveWriteAccess:HResult;StdCall;
       End;

    ISynchronize = Interface (IUnknown)
       ['{00000030-0000-0000-C000-000000000046}']
       Function Wait (dwFlags : DWord; dwMilliSeconds : DWord):HResult;StdCall;
       Function Signal : HResult;StdCall;
       Function Reset : HResult;StdCall;
       End;

    ISynchronizeHandle = Interface (IUnknown)
       ['{00000031-0000-0000-C000-000000000046}']
       Function GetHandle(Out ph : Handle):HResult;StdCall;
       End;

    ISynchronizeEvent = Interface (ISynchronizeHandle)
       ['{00000032-0000-0000-C000-000000000046}']
       Function SetEventHandle (Const ph : Handle):HResult; StdCall;
       End;

    ISynchronizeContainer = Interface (IUnknown)
       ['{00000033-0000-0000-C000-000000000046}']
       Function AddSynchronize(pSync : ISynchronize):HResult; StdCall;
       Function WaitMultiple(dwFlags : Dword; dwTimeOut : Dword; Out pSync : ISynchronize):HResult;StdCall;
       End;

    ISynchronizeMutex = Interface (ISynchronize)
       ['{00000025-0000-0000-C000-000000000046}']
       Function ReleaseMutex:HResult; StdCall;
       End;

    ICancelMethodCalls = Interface (IUnknown)
       ['{00000029-0000-0000-C000-000000000046}']
       Function Cancel(ulSeconds : ULong):HResult; StdCall;
       Function TestCancel:HResult;StdCall;
       End;

    IAsyncManager = Interface (IUnknown)
       ['{0000002A-0000-0000-C000-000000000046}']
       Function CompleteCall (xResult : HResult):HResult;StdCall;
       Function GetCallContext(Const iid :TIID; Out pInterface : Pointer):HResult;StdCall;
       Function GetState(Out pulStateFlags : ULong):HResult;StdCall;
       End;

    ICallFactory = Interface (IUnknown)
       ['{1c733a30-2a1c-11ce-ade5-00aa0044773d}']
       Function CreateCall(Const riid:TIID;CtrUnk : IUnknown;Const Riid2:TIID;Out Unknown : IUnknown):HResult;StdCall;
       End;

    IRpcHelper = Interface (IUnknown)
       ['{00000149-0000-0000-C000-000000000046}']
       Function GetDCOMProtocolVersion(Out ComVersion :DWord):HResult;StdCall;
       Function GettIIDFromOBJREF(ObjRef : Pointer;Out xIID : piid):HResult;StdCall;
       End;

    IReleaseMarshalBuffers = Interface (IUnknown)
       ['{eb0cb9e8-7996-11d2-872e-0000f8080859}']
       Function ReleaseMarshalBuffer(const pnsg : RPCOLEMESSAGE;dwFlags:DWord;Const pchn : IUnknown):HResult; StdCall;
       End;

    IWaitMultiple = Interface (IUnknown)
       ['{0000002B-0000-0000-C000-000000000046}']
       Function WaitMulitple(TImeout :DWord;out psync : ISynchronize):HResult; StdCall;
       Function AddSynchronize (const psync : ISynchronize):HResult;StdCall;
       End;

    IUrlMon = Interface (IUnknown)
       ['{00000026-0000-0000-C000-000000000046}']
       Function AsyncGetClassBits(CONST rclsif : TClsID; psztype,pzext : lpcwstr; dwfileversionMS,dwFileVersionLS : DWord; pzcodebase : LPCWSTR; Const pbc : IBindCTX; dwclasscontext : DWord; const Riid:TIID; flags :DWORD):HResult; StdCall;
       End;

    IForegroundTransfer = Interface (IUnknown)
       ['{00000145-0000-0000-C000-000000000046}']
       Function AllowForegroundTransfer(lpvReserved:Pointer):HResult; StdCall;
       End;

    IAddrTrackingControl = Interface (IUnknown)
       ['{00000147-0000-0000-C000-000000000046}']
       Function EnableCOMDynamicAddrTracking:HResult; StdCall;
       Function DisableCOMDynamicAddrTracking:HResult; StdCall;
       End;

    IAddrExclusionControl = Interface (IUnknown)
       ['{00000148-0000-0000-C000-000000000046}']
       Function GetCurrentAddrExclusionList(Const riid : TIID;out Enumerator : Pointer):HResult;StdCall;
       Function UpdateAddrExclusionList(Enumerator : IUnknown):HResult;StdCall;
       End;

//****************************************************************************
//* Pipe interfaces
//****************************************************************************/

// Doesn't look translatable. See objidl.idl

//****************************************************************************
//* Thumbnail generator interface
//****************************************************************************/

   IThumbnailExtractor = Interface (IUnknown)
      ['{969dc708-5c76-11d1-8d86-0000f804b057}']
       Function ExtractThumbnail (pStg : IStorage; uLength,UHeight : ULong; Out uloutputlength,Height :ULong; Out OutputBitmap : HBITMAP): HResult; StdCall;
       Function OnFileUpdated (pStg : IStorage):HResult;
       End;

//****************************************************************************
//* Dummy Interface to force inclusion of HICON and HDC in proxy/stub code....
//****************************************************************************/

    IDummyHICONIncluder = Interface (IUnknown)
       ['{947990de-cc28-11d2-a0f7-00805f858fb1}']
       Function Dummy (h1 : HICON; H2 :HDC):HResult;
       End;

    IComThreadingInfo = Interface (IUnknown)
       ['{000001ce-0000-0000-C000-000000000046}']
       Function GetCurrentApartmentType(out pAptType : DWord {APTTTYPE}):HResult;
       Function GetCurrentThreadType(Out ThreadType : Dword {THDTTYPE}):HResult;StdCall;
       Function GetCurrentLogicalThreadID(Out guidlogicalThreadId : TGUID):HResult;StdCall;
       Function SetCurrentLogicalThreadID(Const guidlogicalThreadId : TGUID):HResult;StdCall;
       End;

   IProcessInitControl = Interface (IUnknown)
       ['{72380d55-8d2b-43a3-8513-2b6ef31434e9}']
       Function ResetInitializerTimeout(dwSecondsRemaining:DWord):HResult; StdCall;
       End;


// Interfaces from OAIDL.IDL

   ITypeInfo = Interface;

   ICreateTypeInfo = Interface (IUnknown)
     ['{00020405-0000-0000-C000-000000000046}']
     Function  SetGuid(CONST guid: TGUID):HResult;StdCall;
     Function  SetTypeFlags(uTypeFlags: UINT):HResult;StdCall;
     Function  SetDocString(pStrDoc: pOleStr):HResult;StdCall;
     Function  SetHelpContext(dwHelpContext: DWORD):HResult;StdCall;
     Function  SetVersion(wMajorVerNum: WORD; wMinorVerNum: WORD):HResult;StdCall;
     Function  AddRefTypeInfo(CONST pTInfo: ITypeInfo; CONST phRefType: HREFTYPE):HResult;StdCall;
     Function  AddFuncDesc(index: UINT; CONST pFuncDesc: FUNCDESC):HResult;StdCall;
     Function  AddImplType(index: UINT; hRefType: HREFTYPE):HResult;StdCall;
     Function  SetImplTypeFlags(index: UINT; implTypeFlags: WINT):HResult;StdCall;
     Function  SetAlignment(cbAlignment: WORD):HResult;StdCall;
     Function  SetSchema(pStrSchema: pOleStr):HResult;StdCall;
     Function  AddVarDesc(index: UINT; CONST pVarDesc: VARDESC):HResult;StdCall;
     Function  SetFuncAndParamNames(index: UINT; CONST rgszNames: pOleStr; cNames: UINT):HResult;StdCall;
     Function  SetVarName(index: UINT; szName: pOleStr):HResult;StdCall;
     Function  SetTypeDescAlias(CONST pTDescAlias: TYPEDESC):HResult;StdCall;
     Function  DefineFuncAsDllEntry(index: UINT; szDllName: pOleStr; szProcName: pOleStr):HResult;StdCall;
     Function  SetFuncDocString(index: UINT; szDocString: pOleStr):HResult;StdCall;
     Function  SetVarDocString(index: UINT; szDocString: pOleStr):HResult;StdCall;
     Function  SetFuncHelpContext(index: UINT; dwHelpContext: DWORD):HResult;StdCall;
     Function  SetVarHelpContext(index: UINT; dwHelpContext: DWORD):HResult;StdCall;
     Function  SetMops(index: UINT; Const bstrMops: WideString):HResult;StdCall;
     Function  SetTypeIdldesc(CONST pIdlDesc: IDLDESC):HResult;StdCall;
     Function  LayOut():HResult;StdCall;
     End;

   ICreateTypeInfo2 = Interface (ICreateTypeInfo)
     ['{0002040E-0000-0000-C000-000000000046}']
     Function  DeleteFuncDesc(index: UINT):HResult;StdCall;
     Function  DeleteFuncDescByMemId(memid: MEMBERID; invKind: INVOKEKIND):HResult;StdCall;
     Function  DeleteVarDesc(index: UINT):HResult;StdCall;
     Function  DeleteVarDescByMemId(memid: MEMBERID):HResult;StdCall;
     Function  DeleteImplType(index: UINT):HResult;StdCall;
     Function  SetCustData(CONST guid: TGUID; CONST pVarVal: VARIANT):HResult;StdCall;
     Function  SetFuncCustData(index: UINT; CONST guid: TGUID; CONST pVarVal: VARIANT):HResult;StdCall;
     Function  SetParamCustData(indexFunc: UINT; indexParam: UINT; CONST guid: TGUID; CONST pVarVal: VARIANT):HResult;StdCall;
     Function  SetVarCustData(index: UINT; CONST guid: TGUID; CONST pVarVal: VARIANT):HResult;StdCall;
     Function  SetImplTypeCustData(index: UINT; CONST guid: TGUID; CONST pVarVal: VARIANT):HResult;StdCall;
     Function  SetHelpStringContext(dwHelpStringContext: ULONG):HResult;StdCall;
     Function  SetFuncHelpStringContext(index: UINT; dwHelpStringContext: ULONG):HResult;StdCall;
     Function  SetVarHelpStringContext(index: UINT; dwHelpStringContext: ULONG):HResult;StdCall;
     Function  Invalidate():HResult;StdCall;
     Function  SetName(szName: pOleStr):HResult;StdCall;
     End;

   ICreateTypeLib = Interface (IUnknown)
     ['{00020406-0000-0000-C000-000000000046}']
     Function  CreateTypeInfo(szName: pOleStr; tkind: TYPEKIND; OUT ppCTInfo: ICreateTypeInfo):HResult;StdCall;
     Function  SetName(szName: pOleStr):HResult;StdCall;
     Function  SetVersion(wMajorVerNum: WORD; wMinorVerNum: WORD):HResult;StdCall;
     Function  SetGuid(CONST guid: TGUID):HResult;StdCall;
     Function  SetDocString(szDoc: pOleStr):HResult;StdCall;
     Function  SetHelpFileName(szHelpFileName: pOleStr):HResult;StdCall;
     Function  SetHelpContext(dwHelpContext: DWORD):HResult;StdCall;
     Function  SetLcid(lcid: LCID):HResult;StdCall;
     Function  SetLibFlags(uLibFlags: UINT):HResult;StdCall;
     Function  SaveAllChanges():HResult;StdCall;
     End;

   ICreateTypeLib2 = Interface (ICreateTypeLib)
    ['{0002040F-0000-0000-C000-000000000046}']
     Function  DeleteTypeInfo(szName: pOleStr):HResult;StdCall;
     Function  SetCustData(CONST guid: TGUID; CONST pVarVal: VARIANT):HResult;StdCall;
     Function  SetHelpStringContext(dwHelpStringContext: ULONG):HResult;StdCall;
     Function  SetHelpStringDll(szFileName: pOleStr):HResult;StdCall;
     End;

   IEnumVARIANT = Interface (IUnknown)
     ['{00020404-0000-0000-C000-000000000046}']
     {$ifndef Call_as}
      Function  Next(celt: ULONG; OUT rgVar: VARIANT;  pCeltFetched: pULONG=nil):HResult;StdCall;
     {$else}
      Function  Next(celt: ULONG; OUT rgVar: VARIANT;  pCeltFetched: pULONG=nil):HResult;StdCall;
     {$endif}
     Function  Skip(celt: ULONG):HResult;StdCall;
     Function  Reset():HResult;StdCall;
     Function  Clone(OUT ppEnum: IEnumVARIANT):HResult;StdCall;
     End;

   ITypeComp = Interface (IUnknown)
     ['{00020403-0000-0000-C000-000000000046}']
     {$ifndef Call_as}
      Function  Bind(szName: pOleStr; lHashVal: ULONG; wFlags: WORD; OUT ppTInfo: ITypeInfo; OUT pDescKind: DESCKIND; OUT pBindPtr: BINDPTR):HResult;StdCall;
      Function  BindType(szName: pOleStr; lHashVal: ULONG; OUT ppTInfo: ITypeInfo; OUT ppTComp: ITypeComp):HResult;StdCall;
     {$else}
      Function  Bind(szName: pOleStr; lHashVal: ULONG; wFlags: WORD; OUT ppTInfo: ITypeInfo; OUT pDescKind: DESCKIND; OUT ppFuncDesc: LPFUNCDESC; OUT ppVarDesc: LPVARDESC; O
      Function  BindType(szName: pOleStr; lHashVal: ULONG; OUT ppTInfo: ITypeInfo):HResult;StdCall;
     {$endif}
     End;

   ITypeInfo = Interface (IUnknown)
     ['{00020401-0000-0000-C000-000000000046}']
     {$ifndef Call_as}
      Function  GetTypeAttr(OUT ppTypeAttr: lpTYPEATTR):HResult;StdCall;
     {$else}
      Function  GetTypeAttr(OUT ppTypeAttr: LPTYPEATTR; OUT pDummy: CLEANLOCALSTORAGE):HResult;StdCall;
     {$endif}
      Function  GetTypeComp(OUT ppTComp: ITypeComp):HResult;StdCall;
     {$ifndef Call_as}
      Function  GetFuncDesc(index: UINT; OUT ppFuncDesc: lpFUNCDESC):HResult;StdCall;
      Function  GetVarDesc(index: UINT; OUT ppVarDesc: lpVARDESC):HResult;StdCall;
      Function  GetNames(memid: MEMBERID;  rgBstrNames: PBStrList; cMaxNames: UINT; OUT pcNames: UINT):HResult;StdCall;
     {$else}
      Function  GetFuncDesc(index: UINT; OUT ppFuncDesc: LPFUNCDESC; OUT pDummy: CLEANLOCALSTORAGE):HResult;StdCall;
      Function  GetVarDesc(index: UINT; OUT ppVarDesc: LPVARDESC; OUT pDummy: CLEANLOCALSTORAGE):HResult;StdCall;
      Function  GetNames(memid: MEMBERID;  rgBstrNames: PBStrList; cMaxNames: UINT; OUT pcNames: UINT):HResult;StdCall;
     {$endif}
     Function  GetRefTypeOfImplType(index: UINT; OUT pRefType: HREFTYPE):HResult;StdCall;
     Function  GetImplTypeFlags(index: UINT; OUT pImplTypeFlags: WINT):HResult;StdCall;
     {$ifndef Call_as}
      Function  GetIDsOfNames(CONST rgszNames: pOleStr; cNames: UINT; OUT pMemId: MEMBERID):HResult;StdCall;
     {$else}
      Function  LocalGetIDsOfNames():HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
     Function  Invoke(pvInstance: Pointer; memid: MEMBERID; wFlags: WORD; VAR pDispParams: DISPPARAMS; OUT pVarResult: VARIANT; OUT pExcepInfo: EXCEPINFO; OUT puArgErr: UINT):HResult;StdCall;
     {$else}
     Function  LocalInvoke ():HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
     //Function  GetDocumentation(memid: MEMBERID; OUT pBstrName: WideString; OUT pBstrDocString: WideString; OUT pdwHelpContext: DWORD; OUT pBstrHelpFile: WideString):HResult;StdCall;
	 Function  GetDocumentation(memid: MEMBERID; pBstrName: PWideString; pBstrDocString: PWideString; pdwHelpContext: PDWORD; pBstrHelpFile: PWideString):HResult;StdCall;
     {$else}
	 Function  GetDocumentation(memid: MEMBERID; refPtrFlags: DWORD; OUT pBstrName: WideString; OUT pBstrDocString: WideString; OUT pdwHelpContext: DWORD; OUT pBstrHelpFile: WideString):HResult;StdCall;
     {$endif}

     {$ifndef Call_as}
     Function  GetDllEntry(memid: MEMBERID; invKind: INVOKEKIND; OUT pBstrDllName: WideString; OUT pBstrName: WideString; OUT pwOrdinal: WORD):HResult;StdCall;
     {$else}
     Function  GetDllEntry(memid: MEMBERID; invKind: INVOKEKIND; refPtrFlags: DWORD; OUT pBstrDllName: WideString; OUT pBstrName: WideString; OUT pwOrdinal: WORD):HResult;StdCall;
     {$endif}

     Function  GetRefTypeInfo(hRefType: HREFTYPE; OUT ppTInfo: ITypeInfo):HResult;StdCall;

     {$ifndef Call_as}
      Function  AddressOfMember(memid: MEMBERID; invKind: INVOKEKIND; OUT ppv: Pointer):HResult;StdCall;
     {$else}
      Function  LocalAddressOfMember():HResult;StdCall;
     {$endif}

     {$ifndef Call_as}
      Function  CreateInstance(CONST pUnkOuter: IUnknown; CONST riid: TIID; OUT ppvObj: Pointer):HResult;StdCall;
     {$else}
      Function  CreateInstance(CONST riid: TIID; OUT ppvObj: pIUnknown):HResult;StdCall;
     {$endif}
     Function  GetMops(memid: MEMBERID; OUT pBstrMops: WideString):HResult;StdCall;
     {$ifndef Call_as}
     Function  GetContainingTypeLib(OUT ppTLib: ITypeLib; OUT pIndex: UINT):HResult;StdCall;
     {$else}
     Function  GetContainingTypeLib(OUT ppTLib: ITypeLib; OUT pIndex: UINT):HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
      Procedure ReleaseTypeAttr( pTypeAttr: pTypeAttr); StdCall;
     {$else}
      Function  ReleaseTypeAttr():HResult;StdCall;
     {$endif}

     {$ifndef Call_as}
      Procedure ReleaseFuncDesc( pFuncDesc : lpFUNCDESC); StdCall;
     {$else}
      Function  LocalReleaseFuncDesc():HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
      Procedure ReleaseVarDesc( pVarDesc : lpVarDesc); stdcall;
     {$else}
      Function  LocalReleaseVarDesc():HResult;StdCall;
     {$endif}
     End;

   ITypeInfo2 = Interface (ITypeInfo)
     ['{00020412-0000-0000-C000-000000000046}']
     Function  GetTypeKind(OUT xpTypeKind: TYPEKIND):HResult;StdCall;
     Function  GetTypeFlags(OUT pTypeFlags: ULONG):HResult;StdCall;
     Function  GetFuncIndexOfMemId(memid: MEMBERID; invKind: INVOKEKIND; OUT pFuncIndex: UINT):HResult;StdCall;
     Function  GetVarIndexOfMemId(memid: MEMBERID; OUT pVarIndex: UINT):HResult;StdCall;
     Function  GetCustData(CONST guid: TGUID; OUT pVarVal: VARIANT):HResult;StdCall;
     Function  GetFuncCustData(index: UINT; CONST guid: TGUID; OUT pVarVal: VARIANT):HResult;StdCall;
     Function  GetParamCustData(indexFunc: UINT; indexParam: UINT; CONST guid: TGUID; OUT pVarVal: VARIANT):HResult;StdCall;
     Function  GetVarCustData(index: UINT; CONST guid: TGUID; OUT pVarVal: VARIANT):HResult;StdCall;
     Function  GetImplTypeCustData(index: UINT; CONST guid: TGUID; OUT pVarVal: VARIANT):HResult;StdCall;
     {$ifndef Call_as}
      Function  GetDocumentation2(memid: MEMBERID; lcid: LCID;  pbstrHelpString: PWideString; pdwHelpStringContext: PDWORD; pbstrHelpStringDll: PWideString):HResult;StdCall;
     {$else}
      Function  GetDocumentation2(memid: MEMBERID; lcid: LCID; refPtrFlags: DWORD; pbstrHelpString: PWideString;  pdwHelpStringContext: PDWORD; pbstrHelpStringDll: PWideString):HResult;StdCall;
     {$endif}
     Function  GetAllCustData(OUT pCustData: CUSTDATA):HResult;StdCall;
     Function  GetAllFuncCustData(index: UINT; OUT pCustData: CUSTDATA):HResult;StdCall;
     Function  GetAllParamCustData(indexFunc: UINT; indexParam: UINT; OUT pCustData: CUSTDATA):HResult;StdCall;
     Function  GetAllVarCustData(index: UINT; OUT pCustData: CUSTDATA):HResult;StdCall;
     Function  GetAllImplTypeCustData(index: UINT; OUT pCustData: CUSTDATA):HResult;StdCall;
     End;

   ITypeLib = Interface (IUnknown)
     ['{00020402-0000-0000-C000-000000000046}']
     {$ifndef Call_as}
      Function GetTypeInfoCount:UINT; StdCall;
     {$else}
      Function GetTypeInfoCount(OUT pcTInfo: UINT):HResult;StdCall;
     {$endif}
     Function  GetTypeInfo(index: UINT; OUT ppTInfo: ITypeInfo):HResult;StdCall;
     Function  GetTypeInfoType(index: UINT; OUT pTKind: TYPEKIND):HResult;StdCall;
     Function  GetTypeInfoOfGuid(CONST guid: TGUID; OUT ppTinfo: ITypeInfo):HResult;StdCall;
     {$ifndef Call_as}
     Function  GetLibAttr(OUT ppTLibAttr: lpTLIBATTR):HResult;StdCall;
     {$else}
     Function  GetLibAttr(OUT ppTLibAttr: LPTLIBATTR; OUT pDummy: CLEANLOCALSTORAGE):HResult;StdCall;
     {$endif}

     Function  GetTypeComp(OUT ppTComp: ITypeComp):HResult;StdCall;
     {$ifndef Call_as}
     Function  GetDocumentation(index: WINT; pBstrName: PWideString; pBstrDocString: PWideString; pdwHelpContext: PDWORD; pBstrHelpFile: PWideString):HResult;StdCall;
     {$else}
     Function  GetDocumentation(index: WINT; refPtrFlags: DWORD; pBstrName: PWideString; pBstrDocString: PWideString; pdwHelpContext: PDWORD; pBstrHelpFile: PWideString):HResult;StdCall;
     {$endif}

     {$ifndef Call_as}
     Function  IsName(szNameBuf: pOleStr; lHashVal: ULONG; OUT pfName: BOOL):HResult;StdCall;
     {$else}
     Function  IsName(szNameBuf: pOleStr; lHashVal: ULONG; OUT pfName: BOOL; OUT pBstrLibName: WideString):HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
     Function  FindName(szNameBuf: pOleStr; lHashVal: ULONG; OUT ppTInfo: ITypeInfo; OUT rgMemId: MEMBERID; VAR pcFound: USHORT):HResult;StdCall;
     {$else}
     Function  FindName(szNameBuf: pOleStr; lHashVal: ULONG; OUT ppTInfo: ITypeInfo; OUT rgMemId: MEMBERID; VAR pcFound: USHORT; OUT pBstrLibName: WideString):HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
      Procedure ReleaseTLibAttr( pTLibAttr : LPTLIBATTR); StdCall;
      {$else}
     Function  LocalReleaseTLibAttr:HResult;StdCall;
     {$endif}
     End;

   ITypeLib2 = Interface (ITypeLib)
     ['{00020411-0000-0000-C000-000000000046}']
     Function  GetCustData(CONST guid: TGUID; OUT pVarVal: VARIANT):HResult;StdCall;
     {$ifndef Call_as}
     Function  GetLibStatistics(OUT pcUniqueNames: ULONG; OUT pcchUniqueNames: ULONG):HResult;StdCall;
     {$else}
     Function  GetLibStatistics(OUT pcUniqueNames: ULONG; OUT pcchUniqueNames: ULONG):HResult;StdCall;
     {$endif}
     {$ifndef Call_as}
     Function  GetDocumentation2(index: WINT; lcid: LCID;  pbstrHelpString: PWideString;  pdwHelpStringContext: PDWORD;  pbstrHelpStringDll: PWideString):HResult;StdCall;
     {$else}
     Function  GetDocumentation2(index: WINT; lcid: LCID; refPtrFlags: DWORD; pbstrHelpString: PWideString; pdwHelpStringContext: PDWORD; pbstrHelpStringDll: PWideString):HResult;StdCall;
     {$endif}
     Function  GetAllCustData(OUT pCustData: CUSTDATA):HResult;StdCall;
     End;

   ITypeChangeEvents= Interface (IUnknown)
     ['{00020410-0000-0000-C000-000000000046}']
     Function  RequestTypeChange(changeKind: CHANGEKIND; CONST pTInfoBefore: ITypeInfo; pStrName: pOleStr; OUT pfCancel: WINT):HResult;StdCall;
     Function  AfterTypeChange(changeKind: CHANGEKIND; CONST pTInfoAfter: ITypeInfo; pStrName: pOleStr):HResult;StdCall;
     End;

   IErrorInfo= Interface (IUnknown)
     ['{1CF2B120-547D-101B-8E65-08002B2BD119}']
     Function  GetGUID(OUT pGUID: TGUID):HResult;StdCall;
     Function  GetSource(OUT pBstrSource: WideString):HResult;StdCall;
     Function  GetDescription(OUT pBstrDescription: WideString):HResult;StdCall;
     Function  GetHelpFile(OUT pBstrHelpFile: WideString):HResult;StdCall;
     Function  GetHelpContext(OUT pdwHelpContext: DWORD):HResult;StdCall;
     End;

   ICreateErrorInfo= Interface (IUnknown)
     ['{22F03340-547D-101B-8E65-08002B2BD119}']
     Function  SetGUID(CONST rguid: TGUID):HResult;StdCall;
     Function  SetSource(szSource: pOleStr):HResult;StdCall;
     Function  SetDescription(szDescription: pOleStr):HResult;StdCall;
     Function  SetHelpFile(szHelpFile: pOleStr):HResult;StdCall;
     Function  SetHelpContext(dwHelpContext: DWORD):HResult;StdCall;
     End;

   ISupportErrorInfo= Interface (IUnknown)
     ['{DF0B3D60-548F-101B-8E65-08002B2BD119}']
     Function  InterfaceSupportsErrorInfo(CONST riid: TIID):HResult;StdCall;
     End;

   ITypeFactory = Interface (IUnknown)
     ['{0000002E-0000-0000-C000-000000000046}']
     Function  CreateFromTypeInfo(CONST pTypeInfo: ITypeInfo; CONST riid: TIID; OUT ppv: IUnknown):HResult;StdCall;
     End;

   ITypeMarshal = Interface (IUnknown)
     ['{0000002D-0000-0000-C000-000000000046}']
     Function  Size(pvType: Pointer; dwDestContext: DWORD; pvDestContext: Pointer; OUT pSize: ULONG):HResult;StdCall;
     Function  Marshal(pvType: Pointer; dwDestContext: DWORD; pvDestContext: Pointer; cbBufferLength: ULONG; OUT pBuffer: BYTE; OUT pcbWritten: ULONG):HResult;StdCall;
     Function  Unmarshal(pvType: Pointer; dwFlags: DWORD; cbBufferLength: ULONG; CONST pBuffer: BYTE; OUT pcbRead: ULONG):HResult;StdCall;
     Function  Free(pvType: Pointer):HResult;StdCall;
     End;

   IRecordInfo = Interface(IUnknown)
     ['{0000002F-0000-0000-C000-000000000046}']
     Function  RecordInit(pvNew: Pointer):HResult;StdCall;
     Function  RecordClear(pvExisting: Pointer):HResult;StdCall;
     Function  RecordCopy(pvExisting: Pointer; pvNew: Pointer):HResult;StdCall;
     Function  GetGuid(OUT pguid: TGUID):HResult;StdCall;
     Function  GetName(OUT pbstrName: WideString):HResult;StdCall;
     Function  GetSize(OUT pcbSize: ULONG):HResult;StdCall;
     Function  GetTypeInfo(OUT ppTypeInfo: ITypeInfo):HResult;StdCall;
     Function  GetField(pvData: Pointer; szFieldName: pOleStr; OUT pvarField: VARIANT):HResult;StdCall;
     Function  GetFieldNoCopy(pvData: Pointer; szFieldName: pOleStr; OUT pvarField: VARIANT; OUT ppvDataCArray: Pointer):HResult;StdCall;
     Function  PutField(wFlags: ULONG; pvData: Pointer; szFieldName: pOleStr; CONST pvarField: VARIANT):HResult;StdCall;
     Function  PutFieldNoCopy(wFlags: ULONG; pvData: Pointer; szFieldName: pOleStr; CONST pvarField: VARIANT):HResult;StdCall;
     Function  GetFieldNames(VAR pcNames: ULONG; OUT rgBstrNames: WideString):HResult;StdCall;
     Function  IsMatchingType(CONST pRecordInfo : IRecordInfo):Bool;StdCall;
     Function  RecordCreate : Pointer; StdCall;
     Function  RecordCreateCopy(pvSource: Pointer; OUT ppvDest: Pointer):HResult;StdCall;
     Function  RecordDestroy(pvRecord: Pointer):HResult;StdCall;
     End;

   IErrorLog = Interface (IUnknown)
     ['{3127CA40-446E-11CE-8135-00AA004BB851}']
     Function  AddError(pszPropName: pOleStr; CONST pExcepInfo: EXCEPINFO):HResult;StdCall;
     End;


   IPropertyBag = Interface (IUnknown)
     ['{55272A00-42CB-11CE-8135-00AA004BB851}']
     {$ifndef Call_as}
      Function  Read(pszPropName: pOleStr; VAR pVar: VARIANT; CONST pErrorLog: IErrorLog):HResult;StdCall;
     {$else}
      Function  Read(pszPropName: pOleStr; OUT pVar: VARIANT; CONST pErrorLog: IErrorLog; varType: DWORD; CONST pUnkObj: IUnknown):HResult;StdCall;
     {$endif}
     Function  Write(pszPropName: pOleStr; CONST pVar: VARIANT):HResult;StdCall;
     End;

   IEnumGUID = interface(IUnknown)
     ['{0002E000-0000-0000-C000-000000000046}']
     Function Next(celt: UINT; OUT rgelt: TGUID;  pceltFetched: pUINT=nil):HResult;StdCall;
     Function Skip(celt:UINT):HResult;StdCall;
     Function Reset: HResult;StdCall;
     Function Clone(out ppenum: IEnumGUID):HResult;StdCall;
     End;

   IBindHost = interface(IUnknown)
     ['{FC4801A1-2BA9-11CF-A229-00AA003D7352}']
     End;

   IServiceProvider = interface(IUnknown)
     ['{6D5140C1-7436-11CE-8034-00AA006009FA}']
     Function QueryService(CONST rsid, iid: TGuid; OUT Obj):HResult;StdCall;
     End;

   PServiceProvider = ^IServiceProvider;

   IParseDisplayName = interface(IUnknown)
     ['{0000011A-0000-0000-C000-000000000046}']
     Function ParseDisplayName(CONST bc: IBindCtx; pszDisplayName: POleStr;OUT chEaten: Longint; OUT mkOut: IMoniker): HResult;StdCall;
     End;

   IOleContainer = interface(IParseDisplayName)
     ['{0000011B-0000-0000-C000-000000000046}']
     Function EnumObjects(grfFlags: Longint; OUT Enum: IEnumUnknown):HResult;StdCall;
     Function LockContainer(fLock: BOOL):HResult;StdCall;
     End;

   IOleClientSite = interface(IUnknown)
     ['{00000118-0000-0000-C000-000000000046}']
     Function SaveObject: HResult;StdCall;
     Function GetMoniker(dwAssign: Longint; dwWhichMoniker: Longint;OUT mk: IMoniker):HResult;StdCall;
     Function GetContainer(OUT container: IOleContainer):HResult;StdCall;
     Function ShowObject:HResult;StdCall;
     Function OnShowWindow(fShow: BOOL):HResult;StdCall;
     Function RequestNewObjectLayout:HResult;StdCall;
     End;

  IOleWindow = interface(IUnknown)
    ['{00000114-0000-0000-C000-000000000046}']
    function GetWindow(out wnd: HWnd): HResult; stdcall;
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
  end;


  tagOleMenuGroupWidths = record
    width : array[0..5] Of LONG;
  end;
  OLEMENUGROUPWIDTHS = tagOleMenuGroupWidths;
  TOleMenuGroupWidths = tagOleMenuGroupWidths;
  LPOLEMENUGROUPWIDTHS = ^OLEMENUGROUPWIDTHS;
  POleMenuGroupWidths = LPOLEMENUGROUPWIDTHS;


  IProvideClassInfo = Interface (IUnknown)
    ['{B196B283-BAB4-101A-B69C-00AA00341D07}']
    function GetClassInfo(out pptti : ITypeInfo):HResult; StdCall;
  end;


  IProvideClassInfo2 = Interface (IProvideClassInfo)
    ['{A6BC3AC0-DBAA-11CE-9DE3-00AA004BB851}']
    function GetGUID(dwguid:DWord;out pguid:TGUID):HResult; StdCall;
  end;

{ ******************************************************************************************************************
                                                          stuff from objbase.h
  ****************************************************************************************************************** }

  tagOIFI = record
    cb: UINT;
    fMDIApp: BOOL;
    hwndFrame: HWND;
    haccel: HAccel;
    cAccelEntries: UINT;
  end;
  TOleInPlaceFrameInfo = tagOIFI;
  POleInPlaceFrameInfo = ^TOleInPlaceFrameInfo;
  LPOleInPlaceFrameInfo = POleInPlaceFrameInfo;
  OLEINPLACEFRAMEINFO = tagOIFI;


{ redefinitions }
  function CoCreateGuid(out _para1:TGUID):HRESULT;stdcall;external 'ole32.dll' name 'CoCreateGuid';

{ additional definitions }
{$ifndef wince}
  function IsEqualGUID(const guid1,guid2 : TGUID) : Boolean;stdcall;external 'ole32.dll' name 'IsEqualGUID';
  function IsEqualIID(const iid1,iid2 : TIID) : Boolean;stdcall;external 'ole32.dll' name 'IsEqualGUID';
  function IsEqualCLSID(const clsid1,clsid2 : TCLSID) : Boolean;stdcall;external 'ole32.dll' name 'IsEqualGUID';
{$endif wince}

{ OleIdl.h }
type
  IOleInPlaceActiveObject = interface;

  IOleAdviseHolder = interface(IUnknown)
    ['{00000111-0000-0000-C000-000000000046}']
    function Advise(const advise: IAdviseSink; out dwConnection: DWORD): HResult;StdCall;
    function Unadvise(dwConnection: DWORD): HResult;StdCall;
    function EnumAdvise(out enumAdvise: IEnumStatData): HResult;StdCall;
    function SendOnRename(const mk: IMoniker): HResult;StdCall;
    function SendOnSave: HResult;StdCall;
    function SendOnClose: HResult;StdCall;
  end;

  IEnumOLEVERB = interface(IUnknown)
    ['{00000104-0000-0000-C000-000000000046}']
    function Next(celt: ULONG; out elt; pceltFetched: PULONG=nil): HResult;StdCall;
    function Skip(celt: ULONG): HResult;StdCall;
    function Reset: HResult;StdCall;
    function Clone(out ppenum: IEnumOLEVERB): HResult;StdCall;
  end;

  IDropSource = interface(IUnknown)
    ['{00000121-0000-0000-C000-000000000046}']
    function QueryContinueDrag(fEscapePressed: BOOL;
      grfKeyState: Longint):HResult;StdCall;
    function GiveFeedback(dwEffect: Longint): HResult;StdCall;
  end;

  IOleObject = interface(IUnknown)
    ['{00000112-0000-0000-C000-000000000046}']
    function SetClientSite(const clientSite: IOleClientSite): HResult;StdCall;
    function GetClientSite(out clientSite: IOleClientSite): HResult;StdCall;
    function SetHostNames(szContainerApp: POleStr; szContainerObj: POleStr): HResult;StdCall;
    function Close(dwSaveOption: DWORD): HResult;StdCall;
    function SetMoniker(dwWhichMoniker: DWORD; const mk: IMoniker): HResult;StdCall;
    function GetMoniker(dwAssign: DWORD; dwWhichMoniker: DWORD; out mk: IMoniker): HResult;StdCall;
    function InitFromData(const dataObject: IDataObject; fCreation: BOOL; dwReserved: DWORD): HResult;StdCall;
    function GetClipboardData(dwReserved: DWORD; out dataObject: IDataObject): HResult;StdCall;
    function DoVerb(iVerb: LONG; msg: PMsg; const activeSite: IOleClientSite; lindex: LONG; hwndParent: HWND; const posRect: TRect): HResult;StdCall;
    function EnumVerbs(out enumOleVerb: IEnumOleVerb): HResult;StdCall;
    function Update: HResult;StdCall;
    function IsUpToDate: HResult;StdCall;
    function GetUserClassID(out clsid: TCLSID): HResult;StdCall;
    function GetUserType(dwFormOfType: DWORD; out pszUserType: POleStr): HResult;StdCall;
    function SetExtent(dwDrawAspect: DWORD; const size: TPoint): HResult;StdCall;
    function GetExtent(dwDrawAspect: DWORD; out size: TPoint): HResult;StdCall;
    function Advise(const advSink: IAdviseSink; out dwConnection: Longint): HResult;StdCall;
    function Unadvise(dwConnection: DWORD): HResult;StdCall;
    function EnumAdvise(out enumAdvise: IEnumStatData): HResult;StdCall;
    function GetMiscStatus(dwAspect: DWORD; out dwStatus: DWORD): HResult;StdCall;
    function SetColorScheme(const logpal: TLogPalette): HResult;StdCall;
  end;

  IDropTarget = interface(IUnknown)
    ['{00000122-0000-0000-C000-000000000046}']
    function DragEnter(const dataObj: IDataObject; grfKeyState: DWORD; pt: TPoint; var dwEffect: DWORD): HResult;StdCall;
    function DragOver(grfKeyState: DWORD; pt: TPoint; var dwEffect: DWORD): HResult;StdCall;
    function DragLeave: HResult;StdCall;
    function Drop(const dataObj: IDataObject; grfKeyState: DWORD; pt: TPoint; var dwEffect: DWORD):HResult;StdCall;
  end;

  IOleInPlaceUIWindow = interface(IOleWindow)
    ['{00000115-0000-0000-C000-000000000046}']
    function GetBorder(out rectBorder: TRect):HResult;StdCall;
    function RequestBorderSpace(const borderwidths: TRect):HResult;StdCall;
    function SetBorderSpace(const borderwidths: TRect):HResult;StdCall;
    function SetActiveObject(const activeObject: IOleInPlaceActiveObject;pszObjName: POleStr):HResult;StdCall;
  end;

  IOleInPlaceActiveObject = interface(IOleWindow)
    ['{00000117-0000-0000-C000-000000000046}']
    function TranslateAccelerator(var msg: TMsg):HResult;StdCall;
    function OnFrameWindowActivate(fActivate: BOOL):HResult;StdCall;
    function OnDocWindowActivate(fActivate: BOOL):HResult;StdCall;
    function ResizeBorder(const rcBorder: TRect; const uiWindow: IOleInPlaceUIWindow; fFrameWindow: BOOL):HResult;StdCall;
    function EnableModeless(fEnable: BOOL):HResult;StdCall;
  end;

  IOleInPlaceFrame = interface(IOleInPlaceUIWindow)
    ['{00000116-0000-0000-C000-000000000046}']
    function InsertMenus(hmenuShared: HMenu; var menuWidths: TOleMenuGroupWidths): HResult;StdCall;
    function SetMenu(hmenuShared: HMenu; holemenu: HMenu; hwndActiveObject: HWnd): HResult;StdCall;
    function RemoveMenus(hmenuShared: HMenu): HResult;StdCall;
    function SetStatusText(pszStatusText: POleStr): HResult;StdCall;
    function EnableModeless(fEnable: BOOL): HResult;StdCall;
    function TranslateAccelerator(var msg: TMsg; wID: Word): HResult;StdCall;
  end;

  IOleLink = interface(IUnknown)
     ['{0000011d-0000-0000-C000-000000000046}']
    function SetUpdateOptions(dwupdateopt:dword):HResult; stdcall;
    function GetUpdateOptions(dwupdateopt:pdword):HResult; stdcall;
    function SetSourceMoniker(pmk : IMoniker;const clsid: TCLSID):HRESULT; stdcall;
    function GetSourceMoniker(out pmk : IMoniker):HRESULT; stdcall;
    function SetSourceDisplayName(ppszDisplayName:lpolestr):HResult; stdcall;
    function GetSourceDisplayName(out ppszDisplayName:lpolestr):HResult; stdcall;
    function BindToSource(bindflags:DWord;pbc: IBindCTX):HResult; stdcall;
    function BindIfRunning:HResult; stdcall;
    function GetBoundSource(out ppunk: IUnKnown):HResult; stdcall;
    function UnbindSource:HResult; stdcall;
    function Update(pbc:IBindCtx):HResult; stdcall;
    end;

   IOleInPlaceSite = interface(IOleWindow)
      ['{00000119-0000-0000-C000-000000000046}']
      function CanInPlaceActivate : HResult;
      function OnInPlaceActivate : HResult;
      function OnUIActivate : HResult;
      function GetWindowContext(out ppframe:IOleInPlaceFrame;out ppdoc:IOleInPlaceUIWindow;lprcposrect:LPRECT;lprccliprect:LPRECT;lpframeinfo:LPOLEINPLACEFRAMEINFO):hresult; stdcall;
      function Scroll(scrollExtant:TSIZE):hresult; stdcall;
      function OnUIDeactivate(fUndoable:BOOL):hresult; stdcall;
      function OnInPlaceDeactivate :hresult; stdcall;
      function DiscardUndoState :hresult; stdcall;
      function DeactivateAndUndo :hresult; stdcall;
      function OnPosRectChange(lprcPosRect:LPRect):hresult; stdcall;
      end;

    IOleInPlaceObject = interface(IOleWindow)
      ['{00000113-0000-0000-C000-000000000046}']
      function InPlaceDeactivate : HResult;
      function UIDeactivate : HResult;
      function SetObjectRects(lprcPosRect:LPRect;lprcClipRect:LPRect):hresult; stdcall;
      function ReactivateAndUndo : HResult;
     end;

    IOleDocumentView = interface(IUnknown)
        ['{b722bcc6-4e68-101b-a2bc-00aa00404770}']
        function SetInPlaceSite(ppipsite:IOleInPlaceSite):hresult; stdcall;
        function GetInPlaceSite(out ppipsite:IOleInPlaceSite):hresult; stdcall;
        function GetDocument(out ppipsite:Iunknown):hresult; stdcall;
        function SetRect(prcview:LPRect):hresult; stdcall;
        function Getrect(prcView:LPRect):hresult; stdcall;
        function SetRectComplex(prcview:LPRect;prcHScroll:LPRect;prcVScroll:LPRect;prcSizeBox:LPRect):hresult; stdcall;
        function Show(fshow:Bool) :hresult; stdcall;
        function UIActivate(fUIActive :BOOL): HResult;
        function Open :hresult; stdcall;
        function Closeview(dwreserved:DWORD):hresult; stdcall;
        function SaveViewState(pstm:IStream):hresult; stdcall;
        function ApplyViewState(pstm:IStream):hresult; stdcall;
        function Clone(pipsitenew: IOleInPlaceSite;out ppviewNew:IOleDocumentView):HResult;
        end;

    IEnumOleDocumentViews = Interface(IUnknown)
        ['{b722bcc8-4e68-101b-a2bc-00aa00404770}']
        function Next (CViews:ULONG; out rgpview:IOleDocumentView;pcfetched:pulong):hresult; stdcall;
        function Skip (CViews:ULong):hresult; stdcall;
        function Reset:HResult; stdcall;
        function Clone (out ppenum :IEnumOleDocumentViews)  :HResult; stdcall;
       end;

    IOleDocument = interface(IUnknown)
      ['{b722bcc5-4e68-101b-a2bc-00aa00404770}']
        function CreateView(pipsite:IOleInPlaceSite;pstm:IStream;dwReserved:DWord;out ppview : IOleDocumentView):hresult; stdcall;
        function GetDocMiscStatus(pdwstatus:PDWord):hresult; stdcall;
        function EnumViews(out ppenum:IEnumOleDocumentViews;out ppview:IOleDocumentView):hresult; stdcall;
       end;

    IOleDocumentSite = interface(IUnknown)
       ['{b722bcc7-4e68-101b-a2bc-00aa00404770}']
       function ActivateMe(pviewtoactivate:IOleDocumentView):hresult; stdcall;
       end;

    IContinueCallback = interface(IUnknown)
       ['{b722bcca-4e68-101b-a2bc-00aa00404770}']
        function FContinue:HResult;Stdcall;
        function FContinuePrinting( nCntPrinted:LONG;nCurPage:Long;pwzprintstatus:polestr):HResult;Stdcall;
      end;


{ ObjSafe.idl}
  IObjectSafety = interface(IUnknown)
    ['{CB5BDC81-93C1-11cf-8F20-00805F2CD064}']
    function GetInterfaceSafetyOptions(const riid:Tiid; out pdwsupportedoptions: dword;out pdwenabledoptions: dword):HRESULT; stdcall;
    function SetInterfaceSafetyOptions(const riid:Tiid; const dwoptionsetmask: dword;const dwenabledoptions : dword):HRESULT; stdcall;
    end;

  TContinueCallback = function (dwcontinue:ULONG_PTR):BOOL; stdcall;


  IViewObject = interface(IUnknown)
    ['{0000010d-0000-0000-C000-000000000046}']
    function Draw(dwDrawAspect:DWord;LIndex:Long;pvaspect:pointer;ptd:PDVTARGETDEVICE;hdcTargetDev:HDC; hdcDraw:HDC;lprcBounds:PRECTL;lprcWBounds:PRECTL;pfncontinue:TContinueCallback;dwcontinue:ULONG_PTR):HResult; stdcall;
    function GetColorSet(wDrawAspect:DWord;LIndex:Long;pvaspect:pointer;ptd:PDVTARGETDEVICE;hdcTargetDev:HDC;var ppcolorset:PLogPalette):HREsult; stdcall;
    function Freeze(dwDrawAspect:DWord;LIndex:Long;pvaspect:pointer;pdwfreeze:pdword):HResult;stdcall;
    function Unfreeze(dwfreeze:dword):HResult; stdcall;
    function SetAdvise(aspects:DWORD;advf:DWORD;padvSink:IAdviseSink):HRESULT;stdcall;
    function Getadvise(paspects:pdword;padvf:pdword;out ppadvsink: IADviseSink):HRESULT;stdcall;
    end;

  IViewObject2 = interface(IViewObject)
    ['{00000127-0000-0000-C000-000000000046}']
    function GetExtent(dwDrawAspect:dword;lindex:DWord;ptd:pDVTARGETDEVICE;lpsizel:LPSIZEL):HRESULT;stdcall;
    end;



{ COMCAT}

Const CATDESC_MAX = 128;

Type
    CATID  = TGUID;
    TCATID = TGUID;
    PCATID = PGUID;
    tagCATEGORYINFO = packed record
	catid : CATID;
	LCID  : lcid;
        szDescription : array[0..CATDESC_MAX-1] of WideChar;
        end;

   CATEGORYINFO   = tagCATEGORYINFO;
   TCATEGORYINFO  = tagCATEGORYINFO;
   LPCATEGORYINFO = ^tagCATEGORYINFO;
   PCATEGORYINFO  = LPCATEGORYINFO;

   IEnumCLSID = IEnumGUID;
   IEnumCategoryInfo = interface(IUnknown)
          ['{0002E011-0000-0000-C000-000000000046}']
          function Next(celt: ULONG; out rgelt: TCategoryInfo; out pceltFetched: ULONG): HResult; stdcall;
          function Skip(celt:ULONG):HResult; StdCall;
          function Reset:HResult; StdCall;
          function CLone(Out ppenum : IEnumCategoryInfo):HResult;StdCall;
          end;

   ICatRegister = interface (IUnknown)
     ['{0002E012-0000-0000-C000-000000000046}']
     function RegisterCategories   (cCategories:ULONG;rgCategoryInfo : PCategoryInfo):HResult;Stdcall;
     function UnRegisterCategories (cCategories:ULONG;PCatid :PCATID):HResult;Stdcall;
     function RegisterClassImplCategories (const rclsid:TGUID;cCategories:ULONG; rgCatid :PCATID):HResult;Stdcall;
     function UnRegisterClassImplCategories (const rclsid:TGUID;cCategories:ULONG; rgCatid :PCATID):HResult;Stdcall;
     function RegisterClassReqCategories  (const rclsid:TGUID;cCategories:ULONG; rgCatid :PCATID):HResult;Stdcall;
     function UnRegisterClassReqCategories  (const rclsid:TGUID;cCategories:ULONG; rgCatid :PCATID):HResult;Stdcall;
    end;

   ICatInformation = interface(IUnknown)
     ['{0002E013-0000-0000-C000-000000000046}']
     function EnumCategories(lcid:lcid;out ppenumCategoryInfo : IEnumCategoryInfo):HResult; StdCall;
     function GetCategoryDesc(rcatid:PCATID;lcid:LCID;out pszdesc:lpwstr):HResult; StdCall;
     function EnumClassesOfCategories(cImplemented : ULong; rgcatidImpl:PCATID;cRequired:ULong; rgcatidreq:PCATID; out ppenumclsid : IEnumClsID):HResult; StdCall;
     function ISClassOfCategories(rclsid:pclsid;cImplemented:ULong;rgcatidimpl:PCATID;CRequired:ULONG;rgcatidreq : pcatid):HResult; StdCall;
     function EnumImplCategoriesOfClass(rclsid:pclsid;out ppenumclsid : IEnumClsID):HResult; StdCall;
     function EnumReqCategoriesOfClass(rclsid:pclsid;out ppenumclsid : IEnumClsID):HResult; StdCall;
     end;

    IPropertySetStorage = Interface(IUnknown)
     ['{0000013A-0000-0000-C000-000000000046}']
     function Create(const rfmtid:FMTID; const pclsid:CLSID; grfFlags:DWORD; grfMode:DWORD; out ppprstg:IPropertyStorage):HRESULT; StdCall;
     function Open(const fmtid:FMTID; grfMode:DWORD; out ppprstg:IPropertyStorage):HRESULT; StdCall;
     function Delete(const rfmtid:FMTID):HRESULT; StdCall;
     function Enum(out ppenum:IEnumSTATPROPSETSTG):HRESULT; StdCall;
     end;

   IEnumSTATPROPSTG = interface( IUnknown)
     ['{00000139-0000-0000-C000-000000000046}']
     function Next(celt:ULONG; var rgelt:STATPROPSTG; pceltFetched:pULONG):HRESULT; StdCall;
     function Skip(celt:ULONG):HRESULT; StdCall;
     function Reset:HRESULT; StdCall;
     function Clone(out ppenum:IEnumSTATPROPSTG):HRESULT; StdCall;
     end;

   IEnumSTATPROPSETSTG = interface( IUnknown)
     ['{0000013B-0000-0000-C000-000000000046}']
     function Next(celt:ULONG; var rgelt:STATPROPSETSTG; pceltFetched:pULONG):HRESULT; StdCall;
     function Skip(celt:ULONG):HRESULT; StdCall;
     function Reset:HRESULT; StdCall;
     function Clone(out ppenum:IEnumSTATPROPSETSTG):HRESULT; StdCall;
     end;

   IPropertyStorage = interface(IUnknown)
     ['{00000138-0000-0000-C000-000000000046}']
     function ReadMultiple(cpspec:ULONG; rgpspec:pPROPSPEC; rgpropvar:pPROPVARIANT):HRESULT; StdCall;
     function WriteMultiple(cpspec:ULONG; rgpspec:pPROPSPEC; rgpropvar:pPROPVARIANT; propidNameFirst:PROPID):HRESULT; StdCall;
     function DeleteMultiple(cpspec:ULONG; rgpspec:pPROPSPEC):HRESULT; StdCall;
     function ReadPropertyNames(cpspec:ULONG; rgpropid:pPROPID; rgpropvar:plpolestr):HRESULT; StdCall;
     function WritePropertyNames(cpspec:ULONG; rgpspec:pPROPID; rgpropvar:plpolestr):HRESULT; StdCall;
     function DeletePropertyNames(cpspec:ULONG; rgpspec:pPROPid):HRESULT; StdCall;
     function Commit(grfCommitFlags:DWORD):HRESULT; StdCall;
     function Revert:HRESULT; StdCall;
     function Enum(out ppenum:IEnumSTATPROPSTG):HRESULT; StdCall;
     function SetTimes(pctime:PFILETIME; patime:PFILETIME; pmtime:PFILETIME):HRESULT; StdCall;
     function SetClass(clsid:pCLSID):HRESULT; StdCall;
     function Stat(pstatpsstg:pSTATPROPSETSTG):HRESULT; StdCall;
     end;

{ ole2.h }

  type
    WINOLEAPI = HResult;
    TLCID = DWORD; // is this needed (duplicate from windows?)

  const
     OLEIVERB_PRIMARY = 0;
     OLEIVERB_SHOW = -(1);
     OLEIVERB_OPEN = -(2);
     OLEIVERB_HIDE = -(3);
     OLEIVERB_UIACTIVATE = -(4);
     OLEIVERB_INPLACEACTIVATE = -(5);
     OLEIVERB_DISCARDUNDOSTATE = -(6);
  { for OleCreateEmbeddingHelper flags; roles low word; options high word }
     EMBDHLP_INPROC_HANDLER = $0000;
     EMBDHLP_INPROC_SERVER = $0001;
     EMBDHLP_CREATENOW = $00000000;
     EMBDHLP_DELAYCREATE = $00010000;
  { extended create function flags  }
     OLECREATE_LEAVERUNNING = $00000001;
  { pull the MIDL generated header  }
{$ifndef wince}
  function OleBuildVersion:DWORD;stdcall;external 'ole32.dll' name 'OleBuildVersion';
{$endif wince}
  { helper functions  }
  function ReadClassStg(pStg:IStorage; pclsid:PCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'ReadClassStg';
  function WriteClassStg(pStg:IStorage;const rclsid:TCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'WriteClassStg';
  function ReadClassStm(pStm:IStream; pclsid:PCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'ReadClassStm';
  function WriteClassStm(pStm:IStream;const rclsid:TCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'WriteClassStm';
{$ifndef wince}
  function WriteFmtUserTypeStg(pstg:IStorage; cf:CLIPFORMAT; lpszUserType:LPOLESTR):WINOLEAPI;stdcall;external 'ole32.dll' name 'WriteFmtUserTypeStg';
  function ReadFmtUserTypeStg(pstg:IStorage; pcf:PCLIPFORMAT;out lplpszUserType:POleStr):WINOLEAPI;stdcall;external 'ole32.dll' name 'ReadFmtUserTypeStg';

  { init/term  }
  function OleInitialize(pvReserved:LPVOID):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleInitialize';
  procedure OleUninitialize;stdcall;external 'ole32.dll' name 'OleUninitialize';

  { APIs to query whether (Embedded/Linked) object can be created from
     the data object  }
  function OleQueryLinkFromData(pSrcDataObject:IDataObject):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleQueryLinkFromData';
  function OleQueryCreateFromData(pSrcDataObject:IDataObject):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleQueryCreateFromData';
{$endif wince}
  { Object creation APIs  }
  function OleCreate(const rclsid:TCLSID; const riid:TIID;
             renderopt:DWORD; pFormatEtc:LPFORMATETC; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name
             'OleCreate';
{$ifndef wince}
  function OleCreateEx(const rclsid:TCLSID; const riid:TIID; dwFlags:DWORD; renderopt:DWORD; cFormats:ULONG;
             rgAdvf:PDWORD; rgFormatEtc:LPFORMATETC; lpAdviseSink:IAdviseSink; rgdwConnection:PDWORD; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateEx';

  function OleCreateFromData(pSrcDataObj:IDataObject; const riid:TIID; renderopt:DWORD; pFormatEtc:LPFORMATETC; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateFromData';

  function OleCreateFromDataEx(pSrcDataObj:IDataObject; const riid:TIID; dwFlags:DWORD; renderopt:DWORD; cFormats:ULONG;
             rgAdvf:PDWORD; rgFormatEtc:LPFORMATETC; lpAdviseSink:IAdviseSink; rgdwConnection:PDWORD; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateFromDataEx';

  function OleCreateLinkFromData(pSrcDataObj:IDataObject; const riid:TIID; renderopt:DWORD; pFormatEtc:LPFORMATETC; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateLinkFromData';

  function OleCreateLinkFromDataEx(pSrcDataObj:IDataObject; const riid:TIID; dwFlags:DWORD; renderopt:DWORD; cFormats:ULONG;
             rgAdvf:PDWORD; rgFormatEtc:LPFORMATETC; lpAdviseSink:IAdviseSink; rgdwConnection:PDWORD; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateLinkFromDataEx';

  function OleCreateStaticFromData(pSrcDataObj:IDataObject; const iid:TIID; renderopt:DWORD; pFormatEtc:LPFORMATETC; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateStaticFromData';

  function OleCreateLink(pmkLinkSrc:IMoniker; const riid:TIID; renderopt:DWORD; lpFormatEtc:LPFORMATETC; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateLink';

  function OleCreateLinkEx(pmkLinkSrc:IMoniker; const riid:TIID; dwFlags:DWORD; renderopt:DWORD; cFormats:ULONG;
             rgAdvf:PDWORD; rgFormatEtc:LPFORMATETC; lpAdviseSink:IAdviseSink; rgdwConnection:PDWORD; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateLinkEx';

  function OleCreateLinkToFile(lpszFileName:POleStr; const riid:TIID; renderopt:DWORD; lpFormatEtc:LPFORMATETC; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateLinkToFile';

  function OleCreateLinkToFileEx(lpszFileName:POleStr; const riid:TIID; dwFlags:DWORD; renderopt:DWORD; cFormats:ULONG;
             rgAdvf:PDWORD; rgFormatEtc:LPFORMATETC; lpAdviseSink:IAdviseSink; rgdwConnection:PDWORD; pClientSite:IOleClientSite;
             pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateLinkToFileEx';

  function OleCreateFromFile(const rclsid:TCLSID; lpszFileName:POleStr; const riid:TIID; renderopt:DWORD; lpFormatEtc:LPFORMATETC;
             pClientSite:IOleClientSite; pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateFromFile';

  function OleCreateFromFileEx(const rclsid:TCLSID; lpszFileName:POleStr; const riid:TIID; dwFlags:DWORD; renderopt:DWORD;
             cFormats:ULONG; rgAdvf:PDWORD; rgFormatEtc:LPFORMATETC; lpAdviseSink:IAdviseSink; rgdwConnection:PDWORD;
             pClientSite:IOleClientSite; pStg:IStorage; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateFromFileEx';

  function OleLoad(pStg:IStorage; const riid:TIID; pClientSite:IOleClientSite; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleLoad';
  function OleLoadFromStream(pStm:IStream; const iidInterface:TIID; out ppvObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleLoadFromStream';
  function OleSaveToStream(pPStm:IPersistStream; pStm:IStream):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleSaveToStream';
  function OleNoteObjectVisible(pUnknown:IUnknown; fVisible:BOOL):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleNoteObjectVisible';
{$endif wince}
  function OleSave(pPS:IPersistStorage; pStg:IStorage; fSameAsLoad:BOOL):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleSave';
  function OleSetContainedObject(pUnknown:IUnknown; fContained:BOOL):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleSetContainedObject';

  { Drag/Drop APIs  }
{$ifndef wince}
  function RegisterDragDrop(hwnd:HWND; pDropTarget:IDropTarget):WINOLEAPI;stdcall;external 'ole32.dll' name 'RegisterDragDrop';
  function RevokeDragDrop(hwnd:HWND):WINOLEAPI;stdcall;external 'ole32.dll' name 'RevokeDragDrop';
  function DoDragDrop(pDataObj:IDataObject; pDropSource:IDropSource; dwOKEffects:DWORD; pdwEffect:LPDWORD):WINOLEAPI;stdcall;external 'ole32.dll' name 'DoDragDrop';

  { Clipboard APIs  }
  function OleSetClipboard(pDataObj:IDataObject):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleSetClipboard';
  function OleGetClipboard(out ppDataObj:IDataObject):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleGetClipboard';
  function OleFlushClipboard:WINOLEAPI;stdcall;external 'ole32.dll' name 'OleFlushClipboard';
  function OleIsCurrentClipboard(pDataObj:IDataObject):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleIsCurrentClipboard';
{$endif wince}

type
  HOLEMENU = HMenu;

  { InPlace Editing APIs  }
{$ifndef wince}
  function OleCreateMenuDescriptor(hmenuCombined:HMENU; lpMenuWidths:LPOLEMENUGROUPWIDTHS):HOLEMENU;stdcall;external 'ole32.dll' name 'OleCreateMenuDescriptor';
  function OleDestroyMenuDescriptor(holemenu:HOLEMENU):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleDestroyMenuDescriptor';
  function OleTranslateAccelerator(lpFrame:IOleInPlaceFrame; lpFrameInfo:TOleInPlaceFrameInfo; lpmsg:LPMSG):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleTranslateAccelerator';
{$endif wince}
  function OleSetMenuDescriptor(holemenu:HOLEMENU; hwndFrame:HWND; hwndActiveObject:HWND; lpFrame:IOleInPlaceFrame; lpActiveObj:IOleInPlaceActiveObject):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleSetMenuDescriptor';

  { Helper APIs  }
{$ifndef wince}
  function OleDuplicateData(hSrc:HANDLE; cfFormat:CLIPFORMAT; uiFlags:UINT):HANDLE;stdcall;external 'ole32.dll' name 'OleDuplicateData';
  function OleLockRunning(pUnknown:IUnknown; fLock:BOOL; fLastUnlockCloses:BOOL):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleLockRunning';
  function OleCreateDefaultHandler(const clsid:TCLSID; pUnkOuter:IUnknown; const riid:TIID; out lplpObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateDefaultHandler';
  function OleCreateEmbeddingHelper(const clsid:TCLSID; pUnkOuter:IUnknown; flags:DWORD; pCF:IClassFactory; const riid:TIID;
             out lplpObj):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleCreateEmbeddingHelper';
  function IsAccelerator(hAccel:HACCEL; cAccelEntries:longint; lpMsg:LPMSG; lpwCmd:PWORD):BOOL;stdcall;external 'ole32.dll' name 'IsAccelerator';
{$endif wince}
  function OleDraw(pUnknown:IUnknown; dwAspect:DWORD; hdcDraw:HDC;const lprcBounds:TRect):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleDraw';
  function OleRun(pUnknown:IUnknown):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleRun';
  function OleIsRunning(pObject:IOleObject):BOOL;stdcall;external 'ole32.dll' name 'OleIsRunning';

  procedure ReleaseStgMedium(var _para1:STGMEDIUM);stdcall;external 'ole32.dll' name 'ReleaseStgMedium';
  procedure ReleaseStgMedium(_para1:LPSTGMEDIUM);stdcall;external 'ole32.dll' name 'ReleaseStgMedium';
  function CreateOleAdviseHolder(out ppOAHolder:IOleAdviseHolder):WINOLEAPI;stdcall;external 'ole32.dll' name 'CreateOleAdviseHolder';

  { Icon extraction Helper APIs  }
{$ifndef wince}
  function OleGetIconOfFile(lpszPath:LPOLESTR; fUseFileAsLabel:BOOL):HGLOBAL;stdcall;external 'ole32.dll' name 'OleGetIconOfFile';
  function OleGetIconOfClass(const rclsid:TCLSID; lpszLabel:LPOLESTR; fUseTypeAsLabel:BOOL):HGLOBAL;stdcall;external 'ole32.dll' name 'OleGetIconOfClass';
  function OleMetafilePictFromIconAndLabel(hIcon:HICON; lpszLabel:LPOLESTR; lpszSourceFile:LPOLESTR; iIconIndex:UINT):HGLOBAL;stdcall;external 'ole32.dll' name 'OleMetafilePictFromIconAndLabel';
{$endif wince}

  { Registration Database Helper APIs  }
{$ifndef wince}
  function OleRegGetUserType(const clsid:TCLSID; dwFormOfType:DWORD;out pszUserType:POleStr):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleRegGetUserType';
  function OleRegGetMiscStatus(const clsid:TCLSID; dwAspect:DWORD; pdwStatus:PDWORD):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleRegGetMiscStatus';
  function OleRegEnumFormatEtc(const clsid:TCLSID; dwDirection:DWORD;out ppenum:IEnumFormatEtc):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleRegEnumFormatEtc';
  function OleRegEnumVerbs(const clsid:TCLSID;out ppenum:IEnumOLEVERB):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleRegEnumVerbs';
{$endif wince}

{$ifdef _MAC}
  { WlmOLE helper APIs  }

  function WlmOleCheckoutMacInterface(pUnk:IUnknown; out ppv):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleCheckoutMacInterface';

  function WlmOleCheckinMacInterface(pUnk:IUnknown):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleCheckinMacInterface';

  function WlmOleWrapMacInterface(pUnk:IUnknown; const riid:TIID; out ppv):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleWrapMacInterface';

  function WlmOleUnwrapMacInterface(pv:LPVOID):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleUnwrapMacInterface';

  function WlmOleCheckoutWinInterface(pUnk:LPVOID; ppv:PIUnknown):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleCheckoutWinInterface';

  function WlmOleCheckinWinInterface(pUnk:LPVOID):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleCheckinWinInterface';

  function WlmOleWrapWinInterface(pUnk:LPVOID; const riid:TIID; ppv:PIUnknown):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleWrapWinInterface';

  function WlmOleUnwrapWinInterface(pv:LPVOID):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleUnwrapWinInterface';

  procedure WlmOleVersion;stdcall;external 'ole32.dll' name 'WlmOleVersion';

  procedure WlmOleSetInPlaceWindow(hwnd:HWND);stdcall;external 'ole32.dll' name 'WlmOleSetInPlaceWindow';

  { typedef HRESULT (STDAPICALLTYPE* OLEWRAPPROC) (TIID riid, LPVOID* ppvWin, LPVOID* ppvMac); }
  function WlmOleRegisterUserWrap(procNew:OLEWRAPPROC; pprocOld:POLEWRAPPROC):WINOLEAPI;stdcall;external 'ole32.dll' name 'WlmOleRegisterUserWrap';

{$endif}
  { OLE 1.0 conversion APIS  }
  {**** OLE 1.0 OLESTREAM declarations ************************************ }

  type
     LPOLESTREAM = ^_OLESTREAM;
     _OLESTREAMVTBL = record
       Get : function (p : POleStr;out o;dw : DWORD) : DWORD;
       Put : function (p : POleStr;const o;dw : DWORD) : DWORD;
     end;
     OLESTREAMVTBL =  _OLESTREAMVTBL;

     LPOLESTREAMVTBL = OLESTREAMVTBL;

     _OLESTREAM = record
          lpstbl : LPOLESTREAMVTBL;
       end;
     OLESTREAM = _OLESTREAM;
(* Const before type ignored *)

{$ifndef wince}
  function OleConvertOLESTREAMToIStorage(_lpolestream:LPOLESTREAM; pstg:IStorage; ptd:PDVTARGETDEVICE):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleConvertOLESTREAMToIStorage';
  function OleConvertIStorageToOLESTREAM(pstg:IStorage; lpolestream:LPOLESTREAM):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleConvertIStorageToOLESTREAM';
{$endif wince}

  { Storage Utility APIs  }
  function GetHGlobalFromILockBytes(plkbyt:ILockBytes;out phglobal:HGLOBAL):WINOLEAPI;stdcall;external 'ole32.dll' name 'GetHGlobalFromILockBytes';
  function CreateStreamOnHGlobal(hGlobal:HGLOBAL; fDeleteOnRelease:BOOL;out stm:IStream):WINOLEAPI;stdcall;external 'ole32.dll' name 'CreateStreamOnHGlobal';
{$ifndef wince}
  function CreateILockBytesOnHGlobal(hGlobal:HGLOBAL; fDeleteOnRelease:BOOL;out pplkbyt:ILockBytes):WINOLEAPI;stdcall;external 'ole32.dll' name 'CreateILockBytesOnHGlobal';
  function GetHGlobalFromStream(pstm:IStream;out phglobal:HGLOBAL):WINOLEAPI;stdcall;external 'ole32.dll' name 'GetHGlobalFromStream';
{$endif wince}

  { ConvertTo APIS  }
{$ifndef wince}
  function OleDoAutoConvert(pStg:IStorage; pClsidNew:LPCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleDoAutoConvert';
  function OleGetAutoConvert(const clsidOld:TCLSID; pClsidNew:LPCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleGetAutoConvert';
  function OleSetAutoConvert(const clsidOld:TCLSID; clsidNew:TCLSID):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleSetAutoConvert';
  function GetConvertStg(pStg:IStorage):WINOLEAPI;stdcall;external 'ole32.dll' name 'GetConvertStg';
  function SetConvertStg(pStg:IStorage; fConvert:BOOL):WINOLEAPI;stdcall;external 'ole32.dll' name 'SetConvertStg';

  { Presentation data to OLESTREAM }
  {      format }
  {      width }
  {      height }
  {      size bytes }
  {      bits }
  function OleConvertIStorageToOLESTREAMEx(pstg:IStorage; cfFormat:CLIPFORMAT; lWidth:LONG; lHeight:LONG; dwSize:DWORD;
             pmedium:LPSTGMEDIUM; polestm:LPOLESTREAM):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleConvertIStorageToOLESTREAMEx';

  { Presentation data from OLESTREAM }
  {      format }
  {      width }
  {      height }
  {      size bytes }
  function OleConvertOLESTREAMToIStorageEx(polestm:LPOLESTREAM; pstg:IStorage; pcfFormat:PCLIPFORMAT; plwWidth:PLONG; plHeight:PLONG;
             pdwSize:PDWORD; pmedium:LPSTGMEDIUM):WINOLEAPI;stdcall;external 'ole32.dll' name 'OleConvertOLESTREAMToIStorageEx';
{$endif wince}

const
  DROPEFFECT_NONE   = 0;
  DROPEFFECT_COPY   = 1;
  DROPEFFECT_MOVE   = 2;
  DROPEFFECT_LINK   = 4;
  DROPEFFECT_SCROLL = dword($80000000);


type
  BORDERWIDTHS = TRect;
  LPBORDERWIDTHS = PRect;
  LPCBORDERWIDTHS = PRect;

  TBorderWidths = TRect;
  PBorderWidths = PRect;

  function CoInitializeEx(_para1:LPVOID; _para2:DWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoInitializeEx';
  procedure CoUninitialize;stdcall; external  'ole32.dll' name 'CoUninitialize';
  function CoGetClassObject(const _para1:TCLSID; _para2:DWORD; _para3:PVOID; const _para4:TIID; out _para5):HRESULT;stdcall; external  'ole32.dll' name 'CoGetClassObject';
  function CoLoadLibrary(_para1:LPOLESTR; _para2:BOOL):THandle;stdcall; external  'ole32.dll' name 'CoLoadLibrary';
  procedure CoFreeLibrary(_para1:THandle);stdcall; external  'ole32.dll' name 'CoFreeLibrary';
  procedure CoFreeUnusedLibraries;stdcall; external  'ole32.dll' name 'CoFreeUnusedLibraries';
  function CoCreateInstance(const _para1:TCLSID; _para2:IUnknown; _para3:DWORD;const _para4:TIID;out _para5):HRESULT;stdcall; external  'ole32.dll' name 'CoCreateInstance';
  function StringFromCLSID(const _para1:TCLSID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'StringFromCLSID';
  function CLSIDFromString(_para1:LPOLESTR; _para2:LPCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CLSIDFromString';
  function StringFromIID(const _para1:TIID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'StringFromIID';
  function ProgIDFromCLSID(para:PCLSID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'ProgIDFromCLSID';
  function ProgIDFromCLSID(const _para1:TCLSID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'ProgIDFromCLSID';
  function CLSIDFromProgID(_para1:POLESTR; _para2:LPCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CLSIDFromProgID';
  function CLSIDFromProgID(_para1:POLESTR; out _para2:TCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CLSIDFromProgID';
  function StringFromGUID2(const _para1:TGUID; _para2:LPOLESTR; _para3:longint):longint;stdcall; external  'ole32.dll' name 'StringFromGUID2';
  function CoCreateGuid(_para1:PGUID):HRESULT;stdcall; external  'ole32.dll' name 'CoCreateGuid';
{$ifndef wince}
  function CoBuildVersion:DWORD;stdcall; external  'ole32.dll' name 'CoBuildVersion';
  function CoInitialize(_para1:PVOID):HRESULT;stdcall; external  'ole32.dll' name 'CoInitialize';
  function CoGetMalloc(_para1:DWORD; out _para2:IMalloc):HRESULT;stdcall; external  'ole32.dll' name 'CoGetMalloc';
  function CoGetCurrentProcess:DWORD;stdcall; external  'ole32.dll' name 'CoGetCurrentProcess';
  function CoRegisterMallocSpy(_para1:IMallocSpy):HRESULT;stdcall; external  'ole32.dll' name 'CoRegisterMallocSpy';
  function CoRevokeMallocSpy:HRESULT;stdcall; external  'ole32.dll' name 'CoRevokeMallocSpy';
  function CoCreateStandardMalloc(_para1:DWORD; out _para2:IMalloc):HRESULT;stdcall; external  'ole32.dll' name 'CoGetMalloc';
  function CoRegisterClassObject(const _para1:TCLSID; _para2:IUnknown; _para3:DWORD; _para4:DWORD; _para5:PDWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoRegisterClassObject';
  function CoRevokeClassObject(_para1:DWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoRevokeClassObject';
  function CoGetMarshalSizeMax(_para1:PULONG;const _para2:TIID; _para3:IUnknown; _para4:DWORD; _para5:PVOID;
             _para6:DWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoGetMarshalSizeMax';
  function CoMarshalInterface(_para1:IStream;const _para2:TIID; _para3:IUnknown; _para4:DWORD; _para5:PVOID;
             _para6:DWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoMarshalInterface';
  function CoUnmarshalInterface(_para1:IStream;const _para2:TIID; out _para3):HRESULT;stdcall; external  'ole32.dll' name 'CoUnmarshalInterface';
  function CoMarshalHresult(_para1:IStream; _para2:HRESULT):HRESULT;stdcall; external  'ole32.dll' name 'CoMarshalHresult';
  function CoUnmarshalHresult(_para1:IStream; _para2:HRESULT):HRESULT;stdcall; external  'ole32.dll' name 'CoUnmarshalHresult';
  function CoReleaseMarshalData(_para1:IStream):HRESULT;stdcall; external  'ole32.dll' name 'CoReleaseMarshalData';
  function CoDisconnectObject(_para1:IUnknown; _para2:DWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoDisconnectObject';
  function CoLockObjectExternal(_para1:IUnknown; _para2:BOOL; _para3:BOOL):HRESULT;stdcall; external  'ole32.dll' name 'CoLockObjectExternal';
  function CoGetStandardMarshal(const _para1:TIID; _para2:IUnknown; _para3:DWORD; _para4:PVOID; _para5:DWORD;
             out _para6:IMarshal):HRESULT;stdcall; external  'ole32.dll' name 'CoGetStandardMarshal';
  function CoGetStdMarshalEx(_para1:IUnknown; _para2:DWORD; out _para3:IUnknown):HRESULT;stdcall; external  'ole32.dll' name 'CoGetStdMarshalEx';
  function CoIsHandlerConnected(_para1:IUnknown):BOOL;stdcall; external  'ole32.dll' name 'CoIsHandlerConnected';
  function CoHasStrongExternalConnections(_para1:IUnknown):BOOL;stdcall; external  'ole32.dll' name 'CoHasStrongExternalConnections';
  function CoMarshalInterThreadInterfaceInStream(const _para1:TIID; _para2:IUnknown; out _para3:IStream):HRESULT;stdcall; external  'ole32.dll' name 'CoMarshalInterThreadInterfaceInStream';
  function CoGetInterfaceAndReleaseStream(_para1:IStream;const _para2:TIID; out _para3):HRESULT;stdcall; external  'ole32.dll' name 'CoGetInterfaceAndReleaseStream';
  function CoCreateFreeThreadedMarshaler(_para1:IUnknown; out _para2:IUnknown):HRESULT;stdcall; external  'ole32.dll' name 'CoCreateFreeThreadedMarshaler';
  procedure CoFreeAllLibraries;stdcall; external  'ole32.dll' name 'CoFreeAllLibraries';
  function CoCreateInstanceEx(const _para1:TCLSID; _para2:IUnknown; _para3:DWORD; _para4:PCOSERVERINFO; _para5:DWORD;
             _para6:PMULTI_QI):HRESULT;stdcall; external  'ole32.dll' name 'CoCreateInstanceEx';
  function IIDFromString(_para1:LPOLESTR; out _para2:TIID):HRESULT;stdcall; external  'ole32.dll' name 'IIDFromString';
  function CoIsOle1Class(const _para1:TCLSID):BOOL;stdcall; external  'ole32.dll' name 'CoIsOle1Class';
  function CoFileTimeToDosDateTime(_para1:PFILETIME; _para2:LPWORD; _para3:LPWORD):BOOL;stdcall; external  'ole32.dll' name 'CoFileTimeToDosDateTime';
  function CoDosDateTimeToFileTime(_para1:WORD; _para2:WORD; _para3:PFILETIME):BOOL;stdcall; external  'ole32.dll' name 'CoDosDateTimeToFileTime';
  function CoFileTimeNow(_para1:PFILETIME):HRESULT;stdcall; external  'ole32.dll' name 'CoFileTimeNow';
  function CoRegisterMessageFilter(_para1:IMessageFilter;out _para2:IMessageFilter):HRESULT;stdcall; external  'ole32.dll' name 'CoRegisterMessageFilter';
  function CoGetTreatAsClass(const _para1:TCLSID; _para2:LPCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CoGetTreatAsClass';
  function CoTreatAsClass(const _para1:TCLSID; const _para2:TCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CoTreatAsClass';
{$endif wince}

  type
    LPFNGETCLASSOBJECT = function (const _para1:TCLSID; const _para2:TIID;out _para3):HRESULT;stdcall;
    LPFNCANUNLOADNOW = function:HRESULT;stdcall;

{$ifndef wince}
  function DllGetClassObject(const _para1:TCLSID; const _para2:TIID; out _para3):HRESULT;stdcall; external  'ole32.dll' name 'DllGetClassObject';
  function DllCanUnloadNow:HRESULT;stdcall; external  'ole32.dll' name 'DllCanUnloadNow';
{$endif wince}
  function CoTaskMemAlloc(_para1:ULONG):PVOID;stdcall; external  'ole32.dll' name 'CoTaskMemAlloc';
  function CoTaskMemRealloc(_para1:PVOID; _para2:ULONG):PVOID;stdcall; external  'ole32.dll' name 'CoTaskMemRealloc';
  procedure CoTaskMemFree(_para1:PVOID);stdcall; external  'ole32.dll' name 'CoTaskMemFree';

{$ifndef wince}
  function CreateDataAdviseHolder(_para1:IDataAdviseHolder):HRESULT;stdcall; external  'ole32.dll' name 'CreateDataAdviseHolder';
  function CreateDataCache(_para1:IUnknown; const _para2:TCLSID; const _para3:TIID; out _para4):HRESULT;stdcall; external  'ole32.dll' name 'CreateDataCache';
{$endif wince}

(* Const before type ignored *)
  function StgCreateDocfile(_para1:POLESTR; _para2:DWORD; _para3:DWORD; out _para4:IStorage):HRESULT;stdcall; external  'ole32.dll' name 'StgCreateDocfile';
  function StgCreateDocfileOnILockBytes(_para1:ILockBytes; _para2:DWORD; _para3:DWORD; out _para4:IStorage):HRESULT;stdcall; external  'ole32.dll' name 'StgCreateDocfileOnILockBytes';

(* Const before type ignored *)
  function StgOpenStorage(_para1:POLESTR; _para2:IStorage; _para3:DWORD; _para4:SNB; _para5:DWORD;
             out _para6:IStorage):HRESULT;stdcall; external  'ole32.dll' name 'StgOpenStorage';
  function StgOpenStorageOnILockBytes(_para1:ILockBytes; _para2:IStorage; _para3:DWORD; _para4:SNB; _para5:DWORD;
             out _para6:IStorage):HRESULT;stdcall; external  'ole32.dll' name 'StgOpenStorageOnILockBytes';
{$ifndef wince}
  function StgIsStorageFile(_para1:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'StgIsStorageFile';
  function StgIsStorageILockBytes(_para1:ILockBytes):HRESULT;stdcall; external  'ole32.dll' name 'StgIsStorageILockBytes';
  function StgSetTimes(_para1:POLESTR; _para2:PFILETIME; _para3:PFILETIME; _para4:PFILETIME):HRESULT;stdcall; external  'ole32.dll' name 'StgSetTimes';
  function CoGetObject(pszname:lpwstr; bndop:PBind_Opts; const riid:TIID; out ppv):HRESULT; stdcall; external  'ole32.dll' name 'CoGetObject';
  function BindMoniker(_para1:IMoniker; _para2:DWORD; const _para3:TIID; out _para4):HRESULT;stdcall; external  'ole32.dll' name 'BindMoniker';
  function MkParseDisplayName(_para1:IBindCtx; _para2:POLESTR; out _para3:PULONG; out _para4:IMoniker):HRESULT;stdcall; external  'ole32.dll' name 'MkParseDisplayName';
  function MonikerRelativePathTo(_para1:IMoniker; _para2:IMoniker; out _para3:IMoniker; _para4:BOOL):HRESULT;stdcall; external  'ole32.dll' name 'MonikerRelativePathTo';
  function MonikerCommonPrefixWith(_para1:IMoniker; _para2:IMoniker; _para3:PIMoniker):HRESULT;stdcall; external  'ole32.dll' name 'MonikerCommonPrefixWith';
{$endif wince}
  function CreateBindCtx(_para1:DWORD;out _para2:IBindCtx):HRESULT;stdcall; external  'ole32.dll' name 'CreateBindCtx';
  function GetClassFile(_para1:POLESTR; out _para2:TCLSID):HRESULT;stdcall; external  'ole32.dll' name 'GetClassFile';
{$ifndef wince}
  function CreateGenericComposite(_para1:IMoniker; _para2:IMoniker; out _para3:IMoniker):HRESULT;stdcall; external  'ole32.dll' name 'CreateGenericComposite';
  function CreateFileMoniker(_para1:POLESTR; out _para2:IMoniker):HRESULT;stdcall; external  'ole32.dll' name 'CreateFileMoniker';
  function CreateItemMoniker(_para1:POLESTR; _para2:POLESTR;out _para3:IMoniker):HRESULT;stdcall; external  'ole32.dll' name 'CreateItemMoniker';
  function CreateAntiMoniker(_para1:PIMoniker):HRESULT;stdcall; external  'ole32.dll' name 'CreateAntiMoniker';
  function CreatePointerMoniker(_para1:IUnknown; out _para2:IMoniker):HRESULT;stdcall; external  'ole32.dll' name 'CreatePointerMoniker';
  function GetRunningObjectTable(_para1:DWORD; _para2:IRunningObjectTable):HRESULT;stdcall; external  'ole32.dll' name 'GetRunningObjectTable';
  function CoInitializeSecurity(_para1:PSECURITY_DESCRIPTOR; _para2:LONG; _para3:PSOLE_AUTHENTICATION_SERVICE; _para4:pointer; _para5:DWORD;
             _para6:DWORD; _para7:pointer; _para8:DWORD; _para9:pointer):HRESULT;stdcall; external  'ole32.dll' name 'CoInitializeSecurity';
  function CoGetCallContext(const _para1:TIID; _para2:Ppointer):HRESULT;stdcall; external  'ole32.dll' name 'CoGetCallContext';
  function CoQueryProxyBlanket(_para1:IUnknown; _para2:PDWORD; _para3:PDWORD; _para4:POLESTR; _para5:PDWORD;
             _para6:PDWORD; _para7:Pointer; _para8:PDWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoQueryProxyBlanket';
  function CoSetProxyBlanket(_para1:IUnknown; _para2:DWORD; _para3:DWORD; _para4:POLESTR; _para5:DWORD;
             _para6:DWORD; _para7:pointer; _para8:DWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoSetProxyBlanket';
  function CoCopyProxy(_para1:IUnknown; var _para2:IUnknown):HRESULT;stdcall; external  'ole32.dll' name 'CoCopyProxy';
  function CoQueryClientBlanket(_para1:PDWORD; _para2:PDWORD; _para3:POLESTR; _para4:PDWORD; _para5:PDWORD;
             _para6:pointer; _para7:PDWORD):HRESULT;stdcall; external  'ole32.dll' name 'CoQueryClientBlanket';
  function CoImpersonateClient:HRESULT;stdcall; external  'ole32.dll' name 'CoImpersonateClient';
  function CoRevertToSelf:HRESULT;stdcall; external  'ole32.dll' name 'CoRevertToSelf';
  function CoQueryAuthenticationServices(_para1:PDWORD; _para2:PSOLE_AUTHENTICATION_SERVICE):HRESULT;stdcall; external  'ole32.dll' name 'CoQueryAuthenticationServices';
  function CoSwitchCallContext(_para1:IUnknown; var _para2:IUnknown):HRESULT;stdcall; external  'ole32.dll' name 'CoSwitchCallContext';
  function CoGetInstanceFromFile(_para1:PCOSERVERINFO; _para2:PCLSID; _para3:IUnknown; _para4:DWORD; _para5:DWORD;
             _para6:POLESTR; _para7:DWORD; _para8:PMULTI_QI):HRESULT;stdcall; external  'ole32.dll' name 'CoGetInstanceFromFile';
  function CoGetInstanceFromIStorage(_para1:PCOSERVERINFO; _para2:PCLSID; _para3:IUnknown; _para4:DWORD; _para5:IStorage;
             _para6:DWORD; _para7:PMULTI_QI):HRESULT;stdcall; external  'ole32.dll' name 'CoGetInstanceFromIStorage';
{$endif wince}

  type
    TDispID = DISPID;

    TDispIDList = array[0..65535] of TDispID;
    PDispIDList = ^TDispIDList;

    REFIID = TIID;
    TREFIID = TIID;

{$ifndef wince}
  function SetErrorInfo(dwReserved:ULONG;errinfo:IErrorInfo):HResult;stdcall; external 'ole32.dll' name 'SetErrorInfo';
  function GetErrorInfo(dwReserved:ULONG;out errinfo:IErrorInfo):HResult;stdcall; external 'ole32.dll' name 'GetErrorInfo';
  function CreateErrorInfo(out errinfo:ICreateErrorInfo):HResult;stdcall; external 'ole32.dll' name 'CreateErrorInfo';
{$endif wince}

  const
    oleaut32dll   = 'oleaut32.dll';

  function  SysAllocString(psz: pointer): TBStr; stdcall; external oleaut32dll name 'SysAllocString';
  function  SysAllocStringLen(psz: pointer; len:dword): Integer; stdcall; external oleaut32dll name 'SysAllocStringLen';
  procedure SysFreeString(bstr:pointer); stdcall; external oleaut32dll name 'SysFreeString';
  function  SysStringLen(bstr:pointer):UINT; stdcall; external oleaut32dll name 'SysStringLen';
  function  SysStringByteLen(bstr:pointer):UINT; stdcall; external oleaut32dll name 'SysStringByteLen';
  function  SysReAllocString(var bstr:pointer;psz: pointer): Integer; stdcall; external oleaut32dll name 'SysReAllocString';
  function  SysReAllocStringLen(var bstr:pointer;psz: pointer; len:dword): Integer; stdcall; external oleaut32dll name 'SysReAllocStringLen';

	{ Active object registration API }
{$ifndef wince}
	const
	  ACTIVEOBJECT_STRONG = 0;
	  ACTIVEOBJECT_WEAK = 1;
	
	function RegisterActiveObject(unk: IUnknown; const clsid: TCLSID; dwFlags: DWORD; out dwRegister: culong): HResult; stdcall; external oleaut32dll name 'RegisterActiveObject';
	function RevokeActiveObject(dwRegister: culong; pvReserved: Pointer) : HResult; stdcall; external oleaut32dll name 'RevokeActiveObject';
	function GetActiveObject(const clsid: TCLSID; pvReserved: Pointer; out unk: IUnknown) : HResult; stdcall; external oleaut32dll name 'GetActiveObject';
{$endif wince}

function Succeeded(Res: HResult) : Boolean;inline;
function Failed(Res: HResult) : Boolean;inline;
function ResultCode(Res: HResult) : Longint;inline;
function ResultFacility(Res: HResult): Longint;inline;
function ResultSeverity(Res: HResult): Longint;inline;
function MakeResult(Severity, Facility, Code: Longint): HResult;inline;

function LoadTypeLib(szfile : lpolestr; var pptlib: ITypelib):HResult; stdcall; external oleaut32dll name 'LoadTypeLib';
function LoadRegTypeLib(const rguid:TGUID;wVerMajor:ushort;wVerMinor:ushort;_lcid:lcid;out pptlib:ITypeLib):HResult; stdcall; external oleaut32dll name 'LoadRegTypeLib';
function RegisterTypeLib(const ptrlib :ITypeLib;szfullpath:lpolestr;szhelpdir:lpolestr):HResult; stdcall; external oleaut32dll name 'RegisterTypeLib';
function CreateTypeLib2(sysk:TSysKind;szfile:lpolestr;out ppctlib:ICreateTypeLib2):HResult; stdcall; external oleaut32dll name 'CreateTypeLib2';
function DispInvoke(this:pointer;const ptinfo: ITypeInfo;dispidMember:TDISPID;wflags:ushort;pparams:pDISPParams;var pvarresult:OLEVARIANT;pexcepinfo:EXCEPINFO;puArgErr:puint):HRESULT; stdcall; external oleaut32dll name 'CreateTypeLib2';
{$ifndef wince}
function LoadTypeLibEx(szfile : lpolestr; regk:tregkind; var pptlib: ITypelib):HResult; stdcall; external oleaut32dll name 'LoadTypeLibEx';
function QueryPathOfRegTypeLib(const guid:TGUID;wVerMajor:ushort;wVerMinor:ushort;_lcid:lcid;lpbstr:LPolestr):HResult; stdcall; external oleaut32dll name 'QueryPathOfRegTypeLib';
function UnRegisterTypeLib(const libid:TGUID; wVerMajor:ushort;wVerMinor:ushort;_lcid:lcid;sysk:TSysKind):HResult; stdcall; external oleaut32dll name 'UnRegisterTypeLib';
function CreateTypeLib(sysk:TSysKind;szfile:lpolestr;out ppctlib:ICreateTypeLib):HResult; stdcall; external oleaut32dll name 'CreateTypeLib';

function DosDateTimeToVariantTime( wDosDate: ushort; wDosTime:ushort;pvtime:pdouble):longint; stdcall; external oleaut32dll name 'DosDateTimeToVariantTime';
function VariantTimeToDosDateTime( vtime:DOUBLE;pwdosdate:PUSHORT;pwDosTime:PUSHORT):longint; stdcall; external oleaut32dll name 'VariantTimeToDosDateTime';
{$endif wince}

function SystemTimeToVariantTime(var lpsystemtime:TSystemTime;out pvtime: TOleDate):LONGINT; stdcall; external oleaut32dll name 'SystemTimeToVariantTime';
function VariantTimeToSystemTime(vtime:TOleDate; out lpsystemtime: TSystemTime):LONGINT; stdcall; external oleaut32dll name 'VariantTimeToSystemTime';


{--------------------------------------------------------------------- }
{                     VARTYPE Coercion API                             }
{--------------------------------------------------------------------- }
{ Note: The routines that convert *from* a string are defined
 * to take a OLECHAR* rather than a BSTR because no allocation is
 * required, and this makes the routines a bit more generic.
 * They may of course still be passed a BSTR as the strIn param.
  }

function VarUI1FromI2(sIn:SHORT; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromI2';
function VarUI1FromI4(lIn:LONG; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromI4';
function VarUI1FromI8(i64In:LONG64; pbOut:PBYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromI8';
function VarUI1FromR4(fltIn:Single; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromR4';
function VarUI1FromR8(dblIn:DOUBLE; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromR8';
function VarUI1FromCy(cyIn:CY; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromCy';
function VarUI1FromDate(dateIn:DATE; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromDate';
function VarUI1FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromStr';
function VarUI1FromDisp(pdispIn:IDispatch; lcid:LCID; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromDisp';
function VarUI1FromBool(boolIn:VARIANT_BOOL; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromBool';
function VarUI1FromI1(cIn:CHAR; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromI1';
function VarUI1FromUI2(uiIn:USHORT; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromUI2';
function VarUI1FromUI4(ulIn:ULONG; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromUI4';
function VarUI1FromUI8(ui64In:ULONG64; pbOut:PBYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromUI8';
function VarUI1FromDec(var pdecIn:TDecimal; var pbOut:BYTE):HResult;stdcall;external oleaut32dll name 'VarUI1FromDec';
function VarI2FromUI1(bIn:BYTE; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromUI1';
function VarI2FromI4(lIn:LONG; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromI4';

function VarI2FromI8(i64In:LONG64; psOut:PSHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromI8';
function VarI2FromR4(fltIn:Single; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromR4';
function VarI2FromR8(dblIn:DOUBLE; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromR8';
function VarI2FromCy(cyIn:CY; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromCy';
function VarI2FromDate(dateIn:DATE; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromDate';
function VarI2FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromStr';
function VarI2FromDisp(pdispIn:IDispatch; lcid:LCID; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromDisp';
function VarI2FromBool(boolIn:VARIANT_BOOL; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromBool';
function VarI2FromI1(cIn:CHAR; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromI1';
function VarI2FromUI2(uiIn:USHORT; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromUI2';
function VarI2FromUI4(ulIn:ULONG; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromUI4';

function VarI2FromUI8(ui64In:ULONG64; psOut:PSHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromUI8';
function VarI2FromDec(var pdecIn:TDecimal; var psOut:SHORT):HResult;stdcall;external oleaut32dll name 'VarI2FromDec';
function VarI4FromUI1(bIn:BYTE; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromUI1';
function VarI4FromI2(sIn:SHORT; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromI2';

function VarI4FromI8(i64In:LONG64; plOut:PLONG):HResult;stdcall;external oleaut32dll name 'VarI4FromI8';
function VarI4FromR4(fltIn:Single; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromR4';
function VarI4FromR8(dblIn:DOUBLE; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromR8';
function VarI4FromCy(cyIn:CY; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromCy';
function VarI4FromDate(dateIn:DATE; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromDate';
function VarI4FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromStr';
function VarI4FromDisp(dispIn:IDispatch; lcid:LCID; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromDisp';
function VarI4FromBool(boolIn:VARIANT_BOOL; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromBool';
function VarI4FromI1(cIn:CHAR; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromI1';
function VarI4FromUI2(uiIn:USHORT; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromUI2';
function VarI4FromUI4(ulIn:ULONG; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromUI4';

function VarI4FromUI8(ui64In:ULONG64; plOut:PLONG):HResult;stdcall;external oleaut32dll name 'VarI4FromUI8';
function VarI4FromDec(var pdecIn:TDecimal; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromDec';
function VarI4FromInt(intIn:cint; var plOut:LONG):HResult;stdcall;external oleaut32dll name 'VarI4FromInt';

function VarI8FromUI1(bIn:BYTE; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromUI1';
function VarI8FromI2(sIn:SHORT; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromI2';
function VarI8FromI4(lIn:LONG; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromI4';
function VarI8FromR4(fltIn:Single; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromR4';
function VarI8FromR8(dblIn:DOUBLE; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromR8';
function VarI8FromCy(cyIn:CY; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromCy';
function VarI8FromDate(dateIn:DATE; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromDate';
function VarI8FromStr(strIn:POLECHAR; lcid:LCID; dwFlags:dword; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromStr';
function VarI8FromDisp(pdispIn:IDispatch; lcid:LCID; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromDisp';
function VarI8FromBool(boolIn:VARIANT_BOOL; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromBool';
function VarI8FromI1(cIn:CHAR; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromI1';

function VarI8FromUI2(uiIn:USHORT; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromUI2';
function VarI8FromUI4(ulIn:ULONG; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromUI4';
function VarI8FromUI8(ui64In:ULONG64; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromUI8';
function VarI8FromDec(var pdecIn:TDecimal; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromDec';
function VarI8FromInt(intIn:cint; pi64Out:PLONG64):HResult;stdcall;external oleaut32dll name 'VarI8FromInt';

{******************* }
function VarR4FromUI1(bIn:BYTE; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromUI1';
function VarR4FromI2(sIn:SHORT; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromI2';
function VarR4FromI4(lIn:LONG; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromI4';

function VarR4FromI8(i64In:LONG64; pfltOut:PSingle):HResult;stdcall;external oleaut32dll name 'VarR4FromI8';
function VarR4FromR8(dblIn:DOUBLE; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromR8';
function VarR4FromCy(cyIn:CY; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromCy';
function VarR4FromDate(dateIn:DATE; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromDate';
function VarR4FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromStr';
function VarR4FromDisp(pdispIn:IDispatch; lcid:LCID; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromDisp';
function VarR4FromBool(boolIn:VARIANT_BOOL; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromBool';
function VarR4FromI1(cIn:CHAR; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromI1';
function VarR4FromUI2(uiIn:USHORT; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromUI2';
function VarR4FromUI4(ulIn:ULONG; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromUI4';

function VarR4FromUI8(ui64In:ULONG64; pfltOut:PSingle):HResult;stdcall;external oleaut32dll name 'VarR4FromUI8';
function VarR4FromDec(var pdecIn:TDecimal; var pfltOut:Single):HResult;stdcall;external oleaut32dll name 'VarR4FromDec';
function VarR8FromUI1(bIn:BYTE; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromUI1';
function VarR8FromI2(sIn:SHORT; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromI2';
function VarR8FromI4(lIn:LONG; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromI4';

function VarR8FromI8(i64In:LONG64; pdblOut:PDOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromI8';
function VarR8FromR4(fltIn:Single; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromR4';
function VarR8FromCy(cyIn:CY; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromCy';
function VarR8FromDate(dateIn:DATE; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromDate';
function VarR8FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromStr';
function VarR8FromDisp(pdispIn:IDispatch; lcid:LCID; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromDisp';
function VarR8FromBool(boolIn:VARIANT_BOOL; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromBool';
function VarR8FromI1(cIn:CHAR; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromI1';
function VarR8FromUI2(uiIn:USHORT; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromUI2';
function VarR8FromUI4(ulIn:ULONG; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromUI4';

function VarR8FromUI8(ui64In:ULONG64; pdblOut:PDOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromUI8';
function VarR8FromDec(var pdecIn:TDecimal; var pdblOut:DOUBLE):HResult;stdcall;external oleaut32dll name 'VarR8FromDec';
function VarDateFromUI1(bIn:BYTE; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromUI1';
function VarDateFromI2(sIn:SHORT; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromI2';
function VarDateFromI4(lIn:LONG; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromI4';

function VarDateFromI8(i64In:LONG64; pdateOut:PDATE):HResult;stdcall;external oleaut32dll name 'VarDateFromI8';
function VarDateFromR4(fltIn:Single; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromR4';
function VarDateFromR8(dblIn:DOUBLE; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromR8';
function VarDateFromCy(cyIn:CY; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromCy';
function VarDateFromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromStr';

function VarDateFromDisp(pdispIn:IDispatch; lcid:LCID; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromDisp';
function VarDateFromBool(boolIn:VARIANT_BOOL; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromBool';
function VarDateFromI1(cIn:CHAR; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromI1';
function VarDateFromUI2(uiIn:USHORT; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromUI2';
function VarDateFromUI4(ulIn:ULONG; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromUI4';

function VarDateFromUI8(ui64In:ULONG64; pdateOut:PDATE):HResult;stdcall;external oleaut32dll name 'VarDateFromUI8';
function VarDateFromDec(var pdecIn:TDecimal; var pdateOut:DATE):HResult;stdcall;external oleaut32dll name 'VarDateFromDec';
function VarCyFromUI1(bIn:BYTE; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromUI1';
function VarCyFromI2(sIn:SHORT; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromI2';
function VarCyFromI4(lIn:LONG; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromI4';

function VarCyFromI8(i64In:LONG64; pcyOut:PCurrency):HResult;stdcall;external oleaut32dll name 'VarCyFromI8';
function VarCyFromR4(fltIn:Single; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromR4';
function VarCyFromR8(dblIn:DOUBLE; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromR8';
function VarCyFromDate(dateIn:DATE; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromDate';
function VarCyFromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromStr';
function VarCyFromDisp(pdispIn:IDispatch; lcid:LCID; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromDisp';
function VarCyFromBool(boolIn:VARIANT_BOOL; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromBool';
function VarCyFromI1(cIn:CHAR; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromI1';
function VarCyFromUI2(uiIn:USHORT; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromUI2';
function VarCyFromUI4(ulIn:ULONG; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromUI4';

function VarCyFromUI8(ui64In:ULONG64; pcyOut:PCurrency):HResult;stdcall;external oleaut32dll name 'VarCyFromUI8';
function VarCyFromDec(var pdecIn:TDecimal; var pcyOut:CY):HResult;stdcall;external oleaut32dll name 'VarCyFromDec';
function VarBstrFromUI1(bVal:BYTE; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromUI1';
function VarBstrFromI2(iVal:SHORT; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromI2';
function VarBstrFromI4(lIn:LONG; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromI4';

function VarBstrFromI8(i64In:LONG64; lcid:LCID; dwFlags:dword; pbstrOut:PBSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromI8';
function VarBstrFromR4(fltIn:Single; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromR4';
function VarBstrFromR8(dblIn:DOUBLE; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromR8';
function VarBstrFromCy(cyIn:CY; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromCy';
function VarBstrFromDate(dateIn:DATE; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromDate';
function VarBstrFromDisp(pdispIn:IDispatch; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromDisp';
function VarBstrFromBool(boolIn:VARIANT_BOOL; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromBool';
function VarBstrFromI1(cIn:CHAR; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromI1';
function VarBstrFromUI2(uiIn:USHORT; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromUI2';
function VarBstrFromUI4(ulIn:ULONG; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromUI4';

function VarBstrFromUI8(ui64In:ULONG64; lcid:LCID; dwFlags:dword; pbstrOut:PBSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromUI8';
function VarBstrFromDec(var pdecIn:TDecimal; lcid:LCID; dwFlags:ULONG; var pbstrOut:BSTR):HResult;stdcall;external oleaut32dll name 'VarBstrFromDec';
function VarBoolFromUI1(bIn:BYTE; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromUI1';
function VarBoolFromI2(sIn:SHORT; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromI2';
function VarBoolFromI4(lIn:LONG; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromI4';

function VarBoolFromI8(i64In:LONG64; pboolOut:PVARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromI8';
function VarBoolFromR4(fltIn:Single; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromR4';
function VarBoolFromR8(dblIn:DOUBLE; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromR8';
function VarBoolFromDate(dateIn:DATE; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromDate';
function VarBoolFromCy(cyIn:CY; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromCy';
function VarBoolFromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromStr';
function VarBoolFromDisp(pdispIn:IDispatch; lcid:LCID; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromDisp';
function VarBoolFromI1(cIn:CHAR; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromI1';
function VarBoolFromUI2(uiIn:USHORT; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromUI2';
function VarBoolFromUI4(ulIn:ULONG; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromUI4';

function VarBoolFromUI8(i64In:ULONG64; pboolOut:PVARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromUI8';
function VarBoolFromDec(var pdecIn:TDecimal; var pboolOut:VARIANT_BOOL):HResult;stdcall;external oleaut32dll name 'VarBoolFromDec';
function VarI1FromUI1(bIn:BYTE; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromUI1';
function VarI1FromI2(uiIn:SHORT; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromI2';
function VarI1FromI4(lIn:LONG; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromI4';
function VarI1FromI8(i64In:LONG64; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromI8';
function VarI1FromR4(fltIn:Single; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromR4';
function VarI1FromR8(dblIn:DOUBLE; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromR8';
function VarI1FromDate(dateIn:DATE; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromDate';
function VarI1FromCy(cyIn:CY; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromCy';
function VarI1FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromStr';
function VarI1FromDisp(pdispIn:IDispatch; lcid:LCID; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromDisp';
function VarI1FromBool(boolIn:VARIANT_BOOL; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromBool';
function VarI1FromUI2(uiIn:USHORT; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromUI2';
function VarI1FromUI4(ulIn:ULONG; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromUI4';
function VarI1FromUI8(i64In:ULONG64; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromUI8';
function VarI1FromDec(var pdecIn:TDecimal; pcOut:pCHAR):HResult;stdcall;external oleaut32dll name 'VarI1FromDec';
function VarUI2FromUI1(bIn:BYTE; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromUI1';
function VarUI2FromI2(uiIn:SHORT; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromI2';
function VarUI2FromI4(lIn:LONG; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromI4';

function VarUI2FromI8(i64In:LONG64; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromI8';
function VarUI2FromR4(fltIn:Single; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromR4';
function VarUI2FromR8(dblIn:DOUBLE; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromR8';
function VarUI2FromDate(dateIn:DATE; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromDate';
function VarUI2FromCy(cyIn:CY; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromCy';
function VarUI2FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromStr';
function VarUI2FromDisp(pdispIn:IDispatch; lcid:LCID; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromDisp';
function VarUI2FromBool(boolIn:VARIANT_BOOL; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromBool';
function VarUI2FromI1(cIn:CHAR; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromI1';
function VarUI2FromUI4(ulIn:ULONG; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromUI4';
function VarUI2FromUI8(i64In:ULONG64; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromUI8';
function VarUI2FromDec(var pdecIn:TDecimal; var puiOut:USHORT):HResult;stdcall;external oleaut32dll name 'VarUI2FromDec';
function VarUI4FromUI1(bIn:BYTE; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromUI1';
function VarUI4FromI2(uiIn:SHORT; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromI2';
function VarUI4FromI4(lIn:LONG; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromI4';
function VarUI4FromI8(i64In:LONG64; var plOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromI8';
function VarUI4FromR4(fltIn:Single; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromR4';
function VarUI4FromR8(dblIn:DOUBLE; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromR8';
function VarUI4FromDate(dateIn:DATE; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromDate';
function VarUI4FromCy(cyIn:CY; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromCy';
function VarUI4FromStr(strIn:pOLECHAR; lcid:LCID; dwFlags:ULONG; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromStr';
function VarUI4FromDisp(pdispIn:IDispatch; lcid:LCID; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromDisp';
function VarUI4FromBool(boolIn:VARIANT_BOOL; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromBool';
function VarUI4FromI1(cIn:CHAR; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromI1';
function VarUI4FromUI2(uiIn:USHORT; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromUI2';
function VarUI4FromUI8(ui64In:ULONG64; var plOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromUI8';
function VarUI4FromDec(var pdecIn:TDecimal; var pulOut:ULONG):HResult;stdcall;external oleaut32dll name 'VarUI4FromDec';

{**************************************** }
function VarUI8FromUI1(bIn:BYTE; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromUI1';
function VarUI8FromI2(sIn:SHORT; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromI2';
function VarUI8FromI4(lIn:LONG; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromI4';
function VarUI8FromI8(ui64In:LONG64; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromI8';
function VarUI8FromR4(fltIn:Single; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromR4';
function VarUI8FromR8(dblIn:DOUBLE; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromR8';
function VarUI8FromCy(cyIn:CY; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromCy';
function VarUI8FromDate(dateIn:DATE; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromDate';
function VarUI8FromStr(strIn:POLECHAR; lcid:LCID; dwFlags:dword; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromStr';
function VarUI8FromDisp(pdispIn:IDispatch; lcid:LCID; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromDisp';
function VarUI8FromBool(boolIn:VARIANT_BOOL; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromBool';
function VarUI8FromI1(cIn:CHAR; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromI1';
function VarUI8FromUI2(uiIn:USHORT; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromUI2';
function VarUI8FromUI4(ulIn:ULONG; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromUI4';
function VarUI8FromDec(var pdecIn:TDecimal; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromDec';
function VarUI8FromInt(intIn:cint; pi64Out:PULONG64):HResult;stdcall;external oleaut32dll name 'VarUI8FromInt';

implementation

function Succeeded(Res: HResult) : Boolean;inline;
  begin
    Result := Res and $80000000 = 0;
  end;


function Failed(Res: HResult) : Boolean;inline;
  begin
    Result := Res and $80000000 <> 0;
  end;


function ResultCode(Res: HResult) : Longint;inline;
  begin
    Result := Res and $0000FFFF;
  end;


function ResultFacility(Res: HResult): Longint;inline;
  begin
    Result := (Res shr 16) and $00001FFF;
  end;


function ResultSeverity(Res: HResult): Longint;inline;
  begin
    Result := Res shr 31;
  end;


function MakeResult(Severity, Facility, Code: Longint): HResult;inline;
  begin
    Result := (Severity shl 31) or (Facility shl 16) or Code;
  end;


end.
