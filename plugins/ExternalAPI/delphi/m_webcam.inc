{
  WebCam Video plugin by Sergei Polishchuk, SoftCab Inc
  http://www.softcab.com
  pserge@softcab.com
}

{$IFNDEF M_WEBCAM}
{$DEFINE M_WEBCAM}

const
  MS_WEBCAM_OPEN = 'WebCam/Open';
{
  This opens webcamera
  wParam, and lParam must be zero.
  Returns HANDLE to web camera.
  For Example:
    HANDLE hWebcamera = CallService(MS_WEBCAM_OPEN, 0, 0);
}

  MS_WEBCAM_ISREADY = 'WebCam/IsReady';
{
  This zero if camera is ready for use, and non-zero if camera is still initializing.
  It's useful to user this function after asynchronous opening of camera
  wParam must be zero.
  lParam = (LPARAM)(HANDLE)hCamera - camera handle
  For Example:
    HANDLE hWebcamera = CallService(MS_WEBCAM_ISREADY, 0, 0);
}

  MS_WEBCAM_CLOSE = 'WebCam/Close';
{
  This will close web camera.
  wParam must be zero
  lParam = (LPARAM)(HANDLE)hWebcamera - a handle returned by MS_WEBCAM_OPEN
  Return value is undefined.
  For Example:
    CallService(MS_WEBCAM_CLOSE, 0, (LPARAM)hWebcamera);
}

  MS_WEBCAM_SHOWWND = 'WebCam/Show';
{
  This will show or hide web camera window
  wParam = 1 to show window, or zero to hide one
  lParam = (LPARAM)(HANDLE)hWebcamera - handle to camera
  Return value is undefined.
  For Example, this will show the window:
    CallService(MS_WEBCAM_SHOWWND, 1, (LPARAM)hWebcamera);
}

 MS_WEBCAM_FREE = 'WebCam/Free';
{
  This will free WEBCAM_QUERY fields.
  wParam = sizeof(WEBCAM_QUERY)
  lParam = (LPARAM)(WEBCAM_QUERY*)&Query
  Return value is undefined
  For Example:
    CallService(MS_WEBCAM_FREE, sizeof(Query), (LPARAM)&Query);
}

  MS_WEBCAM_QUERY = 'WebCam/Query';
  WANT_PICTURE = pointer(-1);
{
  This will query web camera for data.
  wParam = sizeof(WEBCAM_QUERY)
  lParam = (LPARAM)(WEBCAM_QUERY*)&Query
  Returns zero in case of success, or non-zero in case of any error
  Before queryng camera, you need to setup some WEBCAM_QUERY structure fields.
}

(*
WEBCAM_QUERY Query;
memset(&Query, 0, sizeof(Query));
Query.hCamera = hWebcamera;
Query.Jpeg    = WANT_PICTURE; // we want to get .JPG image
Query.Bitmap  = NULL; // we do not need .BMP image
int ret = CallService(MS_WEBCAM_QUERY, sizeof(Query), (LPARAM)&Query);
if(!ret)
{	if(Query.Jpeg != NULL)
	{	// do something with JPG picture. For example, you may save it to .JPG file.
	}
	// now let's release the memory
	CallService(MS_WEBCAM_FREE, sizeof(Query), (LPARAM)&Query);
}
*)

  MS_WEBCAM_SCREENSHOT = 'WebCam/ScreenShot';
{
  This will return window screenshot
  wParam = sizeof(WEBCAM_QUERY)
  lParam = (LPARAM)(WEBCAM_QUERY*)&Query
  Returns zero in case of success, or non-zero in case of any error
  WEBCAMBUF->hCamera specifies window handle.
  It's not required to open webcamera in order to run this service function.
}

(*

WEBCAM_QUERY Query;
memset(&Query, 0, sizeof(Query));
Query.hCamera = (HANDLE)GetDesktopWindow(); // getting whole desktop picture.
Query.Jpeg    = WANT_PICTURE; // we want to get .JPG image
Query.Bitmap  = NULL; // we do not need .BMP image
int ret = CallService(MS_WEBCAM_SCREENSHOT, sizeof(Query), (LPARAM)&Query);
if(!ret)
{	if(Query.Jpeg != NULL)
	{	// do something with JPG picture. For example, you may save it to .JPG file.
	}
	// now let's release the memory
	CallService(MS_WEBCAM_FREE, sizeof(Query), (LPARAM)&Query);
}

*)

  ME_WEBCAM_SNAPSHOTRECEIVED = 'WebCam/SnapshotRecv';
{
	This event will be fired right after receiving snapshot from remote contact.
	wParam=(WPARAM)(HANDLE)hContact - a contact handle
	lParam=(LPARAM)(WEBCAMBUF*)buffer - a buffer that contains JPEG image
	IMPORTANT: you should not modify the buffer. It's read-only.
}

type
  ptag_WEBCAMBUF = ^tag_WEBCAMBUF;
  tag_WEBCAMBUF = record
    Size:dword; // size of Data buffer in bytes
    Data:array [0..0] of byte;
  end;
  PWEBCAMBUF = ^WEBCAMBUF;
  WEBCAMBUF = tag_WEBCAMBUF;

type
  ptag_WEBCAM_QUERY = ^tag_WEBCAM_QUERY;
  tag_WEBCAM_QUERY = record
    hCamera:THANDLE;    // [in]     HANDLE to web camera
    cx,cy  :word;       // [out]    camera picture size
    Jpeg   :PWEBCAMBUF; // [in,out] points to .JPG file content in memory
    Bitmap :PWEBCAMBUF; // [in,out] points to .BMP file content in memory
  end;
  PWEBCAM_QUERY = ^WEBCAM_QUERY;
  WEBCAM_QUERY = tag_WEBCAM_QUERY;

{$ENDIF}
