{}
const
{
  OldStatusText:array [0..15] of pWideChar =
    (nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,nil);
}
  OldXStatus:array [0..31] of byte =
    (255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
     255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255);
var
  UseListeningTo:cardinal;
  SimpleMode    :cardinal;
  UseStatus     :cardinal;
  UseExtStatus  :cardinal;
  UseMessages   :cardinal;
  KeepStatus    :cardinal;
  XIndepended   :cardinal;
  XStatusSet    :cardinal;
  ClearXStat    :cardinal;
  hINS          :THANDLE;
//  hLTo          :THANDLE;
  plStatusHook  :THANDLE;

const
  Changed:cardinal=0;
  LastStatus:integer=WAT_PLS_NOTFOUND;
