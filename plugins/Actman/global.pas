unit global;

interface

const
  DBBranch = 'ActMan';
const
  ACF_SELECTED = $08000000;
  ACF_EXPORT   = ACF_SELECTED;
  ACF_IMPORT   = ACF_SELECTED;
  ACF_OVERLOAD = $01000000;

type
  tAddOption = function(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
type
  pActionLink=^tActionLink;
  tActionLink=record
    Next     :pActionLink;
    Init     :procedure;
    DeInit   :procedure;
    AddOption:tAddOption;
  end;

const
  ActionLink:pActionLink=nil;

implementation

end.