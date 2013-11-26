(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

unit HistoryControls_Design;

interface

procedure Register;

implementation

uses Classes, HistoryControls, HistoryGrid, DesignIntf, DesignEditors;

type

  THppFormCustomModule = class(TCustomModule)
  public
    class function DesignClass: TComponentClass; override;
  end;

procedure Register;
begin
  RegisterComponents('History++', [THppEdit]);
  RegisterComponents('History++', [THppToolBar]);
  RegisterComponents('History++', [THppToolButton]);
  RegisterComponents('History++', [THppSpeedButton]);
  RegisterComponents('History++', [THppGroupBox]);
  RegisterComponents('THistoryGrid', [THistoryGrid]);
  RegisterCustomModule(THppForm, THppFormCustomModule);
  {RegisterComponents('History++', [THppSaveDialog]);}
end;

{ THppFormCustomModule }

class function THppFormCustomModule.DesignClass: TComponentClass;
begin
  Result := THppForm;
end;

end.
