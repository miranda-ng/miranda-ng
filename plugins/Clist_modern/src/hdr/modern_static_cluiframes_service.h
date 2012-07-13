//////////////////////////////////////////////////////////////////////////
// WARNING
//////////////////////////////////////////////////////////////////////////
// TO BE INCLUDED ONLY TO modern_cluiframes.cpp
//////////////////////////////////////////////////////////////////////////

static int _us_DoSetFramePaintProc( WPARAM wParam,LPARAM lParam );
static int _us_DoAddFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoRemoveFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoSetFrameOptions( WPARAM wParam,LPARAM lParam );
static INT_PTR _us_DoGetFrameOptions( WPARAM wParam,LPARAM lParam );
static int _us_DoUpdateFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoShowHideFrameTitle( WPARAM wParam,LPARAM lParam );
static int _us_DoShowTitles( WPARAM wParam,LPARAM lParam );
static int _us_DoHideTitles( WPARAM wParam,LPARAM lParam );
static int _us_DoShowHideFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoShowAllFrames( WPARAM wParam,LPARAM lParam );
static int _us_DoLockFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoCollapseFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoSetFrameBorder( WPARAM wParam,LPARAM lParam );
static int _us_DoSetFrameAlign( WPARAM wParam,LPARAM lParam );
static int _us_DoMoveFrame( WPARAM wParam,LPARAM lParam );
static int _us_DoMoveFrameUp( WPARAM wParam,LPARAM lParam );
static int _us_DoMoveFrameDown( WPARAM wParam,LPARAM lParam );
static int _us_DoAlignFrameTop( WPARAM wParam,LPARAM lParam );
static int _us_DoAlignFrameClient( WPARAM wParam,LPARAM lParam );
static int _us_DoAlignFrameBottom( WPARAM wParam,LPARAM lParam );
static int _us_DoSetFrameFloat( WPARAM wParam,LPARAM lParam );

enum { 
    CFM_FIRST_MGS= WM_USER + 0x2FF,

    CFM_SETFRAMEPAINTPROC,
    CFM_ADDFRAME,
    CFM_REMOVEFRAME,
    CFM_SETFRAMEOPTIONS,
    CFM_GETFRAMEOPTIONS,
    CFM_UPDATEFRAME,
    CFM_SHOWHIDEFRAMETITLE,
    CFM_SHOWTITLES,
    CFM_HIDETITLES,
    CFM_SHOWHIDEFRAME,
    CFM_SHOWALL,
    CFM_LOCKFRAME,
    CFM_COLLAPSEFRAME,
    CFM_SETFRAMEBORDER,
    CFM_SETFRAMEALIGN,
    CFM_MOVEFRAME,
    CFM_MOVEFRAMEUP,
    CFM_MOVEFRAMEDOWN,
    CFM_ALIGNFRAMETOP,
    CFM_ALIGNFRAMEBOTTOM,
    CFM_ALIGNFRAMECLIENT,
    CFM_SETFRAMEFLOAT,

    CFM_LAST_MSG
};

#define CLM_PROCESS( msg, proc ) case msg: result = proc( wParam, lParam); break;

BOOL CALLBACK ProcessCLUIFrameInternalMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
    if ( msg <= CFM_FIRST_MGS || msg >= CFM_LAST_MSG ) return FALSE;

    switch ( msg )
    {
        CLM_PROCESS( CFM_SETFRAMEPAINTPROC,  _us_DoSetFramePaintProc    );
        CLM_PROCESS( CFM_ADDFRAME,           _us_DoAddFrame             );
        CLM_PROCESS( CFM_REMOVEFRAME,        _us_DoRemoveFrame          );
        CLM_PROCESS( CFM_SETFRAMEOPTIONS,    _us_DoSetFrameOptions      );
        CLM_PROCESS( CFM_GETFRAMEOPTIONS,    _us_DoGetFrameOptions      );
        CLM_PROCESS( CFM_UPDATEFRAME,        _us_DoUpdateFrame          );
        CLM_PROCESS( CFM_SHOWHIDEFRAMETITLE, _us_DoShowHideFrameTitle   );
        CLM_PROCESS( CFM_SHOWTITLES,         _us_DoShowTitles           );
        CLM_PROCESS( CFM_HIDETITLES,         _us_DoHideTitles           );
        CLM_PROCESS( CFM_SHOWHIDEFRAME,      _us_DoShowHideFrame        );
        CLM_PROCESS( CFM_SHOWALL,            _us_DoShowAllFrames        );
        CLM_PROCESS( CFM_LOCKFRAME,          _us_DoLockFrame            );
        CLM_PROCESS( CFM_COLLAPSEFRAME,      _us_DoCollapseFrame        );
        CLM_PROCESS( CFM_SETFRAMEBORDER,     _us_DoSetFrameBorder       );
        CLM_PROCESS( CFM_SETFRAMEALIGN,      _us_DoSetFrameAlign        );
        CLM_PROCESS( CFM_MOVEFRAME,          _us_DoMoveFrame            );
        CLM_PROCESS( CFM_MOVEFRAMEUP,        _us_DoMoveFrameUp          );
        CLM_PROCESS( CFM_MOVEFRAMEDOWN,      _us_DoMoveFrameDown        );
        CLM_PROCESS( CFM_ALIGNFRAMETOP,      _us_DoAlignFrameTop        );
        CLM_PROCESS( CFM_ALIGNFRAMEBOTTOM,   _us_DoAlignFrameClient     );
        CLM_PROCESS( CFM_ALIGNFRAMECLIENT,   _us_DoAlignFrameBottom     );
        CLM_PROCESS( CFM_SETFRAMEFLOAT,      _us_DoSetFrameFloat        );
    default:
        return FALSE;   // Not Handled
    }
    return TRUE;
}


static INT_PTR CLUIFrames_SetFramePaintProc( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SETFRAMEPAINTPROC, wParam,lParam ) : 0; }

static INT_PTR CLUIFrames_AddFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_ADDFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_RemoveFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_REMOVEFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_SetFrameOptions( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SETFRAMEOPTIONS, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_GetFrameOptions( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_GETFRAMEOPTIONS, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_UpdateFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_UPDATEFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_ShowHideFrameTitle( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SHOWHIDEFRAMETITLE, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_ShowTitles( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SHOWTITLES, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_HideTitles( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_HIDETITLES, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_ShowHideFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SHOWHIDEFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_ShowAllFrames( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SHOWALL, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_LockFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_LOCKFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_CollapseFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_COLLAPSEFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_SetFrameBorder( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SETFRAMEBORDER, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_SetFrameAlign( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SETFRAMEALIGN, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_MoveFrame( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_MOVEFRAME, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_MoveFrameUp( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_MOVEFRAMEUP, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_MoveFrameDown( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_MOVEFRAMEDOWN, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_AlignFrameTop( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_ALIGNFRAMETOP, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_AlignFrameClient( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_ALIGNFRAMEBOTTOM, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_AlignFrameBottom( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_ALIGNFRAMECLIENT, wParam, lParam ) : 0; }

static INT_PTR CLUIFrames_SetFrameFloat( WPARAM wParam,LPARAM lParam )
{  return ( pcli->hwndContactList ) ? SendMessage( pcli->hwndContactList, CFM_SETFRAMEFLOAT, wParam, lParam ) : 0; }

static void CreateCluiFramesServices()
{
    CreateServiceFunction( MS_SKINENG_REGISTERPAINTSUB,	    CLUIFrames_SetFramePaintProc );
    CreateServiceFunction( MS_CLIST_FRAMES_ADDFRAME,	    CLUIFrames_AddFrame );
    CreateServiceFunction( MS_CLIST_FRAMES_REMOVEFRAME,     CLUIFrames_RemoveFrame );

    CreateServiceFunction( MS_CLIST_FRAMES_SETFRAMEOPTIONS,	CLUIFrames_SetFrameOptions );
    CreateServiceFunction( MS_CLIST_FRAMES_GETFRAMEOPTIONS,	CLUIFrames_GetFrameOptions );
    CreateServiceFunction( MS_CLIST_FRAMES_UPDATEFRAME,		CLUIFrames_UpdateFrame );

    CreateServiceFunction( MS_CLIST_FRAMES_SHFRAMETITLEBAR,	CLUIFrames_ShowHideFrameTitle );
    CreateServiceFunction( MS_CLIST_FRAMES_SHOWALLFRAMESTB,	CLUIFrames_ShowTitles );
    CreateServiceFunction( MS_CLIST_FRAMES_HIDEALLFRAMESTB,	CLUIFrames_HideTitles );
    CreateServiceFunction( MS_CLIST_FRAMES_SHFRAME,			CLUIFrames_ShowHideFrame );
    CreateServiceFunction( MS_CLIST_FRAMES_SHOWALLFRAMES,	CLUIFrames_ShowAllFrames );

    CreateServiceFunction( MS_CLIST_FRAMES_ULFRAME,			CLUIFrames_LockFrame );
    CreateServiceFunction( MS_CLIST_FRAMES_UCOLLFRAME,		CLUIFrames_CollapseFrame );
    CreateServiceFunction( MS_CLIST_FRAMES_SETUNBORDER,		CLUIFrames_SetFrameBorder );

    CreateServiceFunction( CLUIFRAMESSETALIGN,			    CLUIFrames_SetFrameAlign );
    CreateServiceFunction( CLUIFRAMESMOVEUPDOWN,		    CLUIFrames_MoveFrame );
    CreateServiceFunction( CLUIFRAMESMOVEUP,			    CLUIFrames_MoveFrameUp );
    CreateServiceFunction( CLUIFRAMESMOVEDOWN,			    CLUIFrames_MoveFrameDown );

    CreateServiceFunction( CLUIFRAMESSETALIGNALTOP,		    CLUIFrames_AlignFrameTop );
    CreateServiceFunction( CLUIFRAMESSETALIGNALCLIENT,	    CLUIFrames_AlignFrameClient );
    CreateServiceFunction( CLUIFRAMESSETALIGNALBOTTOM,	    CLUIFrames_AlignFrameBottom );

    CreateServiceFunction( CLUIFRAMESSETFLOATING,		    CLUIFrames_SetFrameFloat );
}