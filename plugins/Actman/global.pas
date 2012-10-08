unit global;

interface

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