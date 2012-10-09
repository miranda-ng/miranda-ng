{$IFNDEF M_MAGNETICWINDOWS}
{$DEFINE M_MAGNETICWINDOWS}

const
// For other Plugins to start snapping for their windows 
// wparam: hwnd of window
// lparam: 0
// return: 0 on success, 1 on error
  MS_MW_ADDWINDOW = 'Utils/MagneticWindows/Add';

// For other Plugins to stop snapping for their windows
// wparam: hwnd of window
// lparam: 0
// return: 0 on success, 1 on error
  MS_MW_REMWINDOW = 'Utils/MagneticWindows/Rem';

//decide where to align on the list:
  MS_MW_STL_List_Left   = $00000001;  //Snaps the window to the left   border of the list
  MS_MW_STL_List_Top    = $00000002;  //Snaps the window to the top    border of the list
  MS_MW_STL_List_Right  = $00000004;  //Snaps the window to the right  border of the list
  MS_MW_STL_List_Bottom = $00000008;  //Snaps the window to the bottom border of the list
//decide with what side (of the window you want to snap) to snap to the list
  MS_MW_STL_Wnd_Left    = $00000010;  //Snaps the window with the left   border to the left/right side of the list
  MS_MW_STL_Wnd_Top     = $00000020;  //Snaps the window with the top    border to the top/bottom side of the list
  MS_MW_STL_Wnd_Right   = $00000040;  //Snaps the window with the right  border to the left/right side of the list
  MS_MW_STL_Wnd_Bottom  = $00000080;  //Snaps the window with the bottom border to the top/bottom side of the list

  MS_MW_STL_Wnd_FullWidth = (MS_MW_STL_Wnd_Left or MS_MW_STL_Wnd_Right);
											//Snaps to the top/bottom of the list and spans over the full width

  MS_MW_STL_Wnd_FullHeight = (MS_MW_STL_Wnd_Top or  MS_MW_STL_Wnd_Bottom);
											//Snaps to the left/right of the list and spans over the full height

// to place the window in the list combine f.e.
// MS_MW_STL_List_Left | MS_MW_STL_Wnd_Right | *vetical alignment*

//For other Plugins to snap a window to the list for other Plugins
// wparam: hwnd of window
// lparam: combination of the above constants MS_MW_STL_*
// return: 0 on success, 1 on error
  MS_MW_SNAPTOLIST  = 'Utils/MagneticWindows/SnapToList';

// Helper functions
{$IFNDEF MW_NO_HELPPER_FUNCTIONS}

function MagneticWindows_AddWindow(hWnd:HWND):integer;
begin
  if ServiceExists(MS_MW_ADDWINDOW)<>0 then 
    result:=CallService(MS_MW_ADDWINDOW,hWnd,0);
  else
    result:=-1;
end;

function MagneticWindows_RemoveWindow(hWnd:HWND):integer;
begin
  if ServiceExists(MS_MW_REMWINDOW)<>0 then
    result:=CallService(MS_MW_REMWINDOW,hWnd,0);
  else
    result:=-1;
end;

function MagneticWindows_SnapWindowToList(hWnd:HWND;MS_MW_STL_Options:integer):integer;
begin
  if (ServiceExists(MS_MW_SNAPTOLIST))
    result:=CallService(MS_MW_SNAPTOLIST,hWnd,MS_MW_STL_Options);
  else
    result:=-1;
end;

{$ENDIF}

{$ENDIF}
