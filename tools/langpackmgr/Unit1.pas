unit Unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ExtDlgs, Vcl.StdCtrls, Vcl.Buttons;

type
  TForm1 = class(TForm)
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Edit1: TEdit;
    Edit2: TEdit;
    Edit3: TEdit;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    ListBox1: TListBox;
    OpenDialog1: TOpenDialog;
    C: TBitBtn;
    X: TBitBtn;
    B: TBitBtn;
    G: TBitBtn;
    procedure FormCreate(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure ListBox1Click(Sender: TObject);
    procedure CClick(Sender: TObject);
    procedure XClick(Sender: TObject);
    procedure refresh;
    procedure read;
    procedure enter(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure BClick(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  openDialog : TOpenDialog;
  full,ustring,tstring,fline,dupes: array [1..9999]  of string;
  notranslate: array [1..9999] of integer;
  translate: textfile;
  bom, bomd, line:string;
  ii,i,it,ie:integer;
  idx,idm:integer;
  cores:boolean;
implementation

{$R *.dfm}
// Открытие файла
procedure TForm1.BClick(Sender: TObject);
begin
 edit2.SelectAll;
 edit2.CopyToClipboard;
end;

procedure TForm1.Button1Click(Sender: TObject);
 begin
 chdir(ExtractFilePath(Application.ExeName));
 chdir('..');
 openDialog := TOpenDialog.Create(self);
 openDialog.Filter := 'Text files only|*.txt';
 openDialog.InitialDir :=GetCurrentDir ;
 openDialog.Options := [ofFileMustExist];
 if (openDialog.Execute)
 and(extractfilename(openDialog.filename)<>'=HEAD=.txt')
 and(extractfilename(openDialog.filename)<>'=DUPES=.txt')
then
  begin
  edit1.Text:=copy(openDialog.filename,
  length(ExtractFilePath(Application.ExeName))-4,
  length(openDialog.filename)-length(ExtractFilePath(Application.ExeName))+5);
  read;
  refresh;
  end;
end;

// Процедура сохранения обработанного файла
procedure TForm1.Button2Click(Sender: TObject);
begin
refresh;
assignfile(translate,opendialog.filename,CP_UTF8);
rewrite(translate);
writeLn(translate, bom);
 for i := 1 to ie do
  begin
  if copy(ustring[i],1,1)=';' then  writeln(translate,ustring[i]);
  if (copy(ustring[i],1,1)='[') and (tstring[i]<>'')
  then begin
        cores:=false;
        if Button3.visible=true then
        for idx := 1 to idm do
        if (copy(dupes[idx],1,1)='[')
        and (dupes[idx]=ustring[i])
        then cores:=true;
       if cores=false then
         begin
          writeln(translate,ustring[i]);
          writeln(translate,tstring[i]);
         end;
  end;
   end;
closefile(translate);

// Процедура пересохранения файла =DUPES=
 if (extractfilename(openDialog.filename)='=CORE=.txt')
 and  (FileExists(ExtractFilePath(opendialog.filename)+'=DUPES=.txt')=true)
 then
  begin
assignfile(translate,ExtractFilePath(opendialog.filename)+'=DUPES=.txt',CP_UTF8);
rewrite(translate);
writeLn(translate, bomd);
// Если строка отсутствует в =СORE=, она запишется в =DUPES=
for idx := 1 to idm do
if (copy(dupes[idx],1,1)='[') then
    begin cores:=false;
    for i := 1 to ie do
    if dupes[idx]=ustring[i] then cores:=true;
    if cores=false then  begin
     writeLn(translate, dupes[idx]);
     writeLn(translate, dupes[idx+1]);
                        end;
    end;
closefile(translate);
  end;
//
  read;
end;

// Выключение фильтра повторяющихся строк
procedure TForm1.Button3Click(Sender: TObject);
begin
if edit1.Text<>'' then
If Button3.Caption='DUPES OFF'
then Button3.Caption:='DUPES ON'
else Button3.Caption:='DUPES OFF';
refresh;read;refresh;
end;

procedure TForm1.CClick(Sender: TObject);
begin
  Edit3.text:=edit2.text;
end;

procedure TForm1.XClick(Sender: TObject);
begin
  Edit3.text:='';
  tstring[notranslate[strtoint(label2.caption)]]:='';
  refresh;
end;

procedure TForm1.enter(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
case key of
    vk_return,vk_down: if listbox1.ItemIndex<listbox1.Items.Count then
    listbox1.ItemIndex:=listbox1.ItemIndex+1;
    vk_up: if listbox1.ItemIndex>0 then
    listbox1.ItemIndex:=listbox1.ItemIndex-1;
end;
refresh;
end;

procedure TForm1.ListBox1Click(Sender: TObject);
begin
  refresh;
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
   KeyPreview := True;
end;

procedure tform1.refresh;
var n:integer;
begin
n:=strtoint(label2.caption);
line:=edit3.Text;
if length(line)<>0 then
tstring[notranslate[n]]:=line;
label2.Caption:=inttostr(ListBox1.ItemIndex+1);
n:=strtoint(label2.caption);
edit2.Text:=copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2);
edit3.Text:=tstring[notranslate[n]];
edit3.SetFocus;
end;



procedure tform1.read;
begin
if extractfilename(opendialog.filename)='=CORE=.txt'
then button3.Visible:=false
else button3.Visible:=true;
   listbox1.Items.Clear;
   line:='';
   edit3.Text:='';
 for i:= 1 to 9999 do
 begin
   full[i]:='';
   dupes[i]:='';
   notranslate[i]:=0;
   ustring[i]:='';
   tstring[i]:='';
  end;
// Считывание выбранного файла в full[it]
 assignfile(translate,opendialog.filename,CP_UTF8);
 reset(translate);
  it:=0;
 while not Eof(translate) do
 begin
  ReadLn(translate, line);
  if (copy(line,1,1)='[') and (not Eof(translate)) then
  begin
    it:=it+1;
    full[it]:=line;
    ReadLn(translate, line);
    if (copy(line,1,1)<>'')
    and(copy(line,1,1)<>';')
    and(copy(line,1,1)<>'[')
    then  begin
          it:=it+1;
          full[it]:=line;
          end
    else it:=it-1;
  end;
 end;
closefile(translate);

// Считывание файла повторяющихся строк в dupes[idm]
  idm:=0;
if FileExists(ExtractFilePath(opendialog.filename)+'=DUPES=.txt')=true then
begin
assignfile(translate,ExtractFilePath(opendialog.filename)+'=DUPES=.txt',CP_UTF8);
reset(translate);
  ReadLn(translate, bomd);
 while not Eof(translate) do
 begin
  ReadLn(translate, line);
   if (copy(line,1,1)='[') and (not Eof(translate)) then
  begin
    idm:=idm+1;
    dupes[idm]:=line;
    ReadLn(translate, line);
    if (copy(line,1,1)<>'')and(copy(line,1,1)<>';')and(copy(line,1,1)<>'[')then
    begin idm:=idm+1; dupes[idm]:=line; end
    else idm:=idm-1;
  end;
end;
closefile(translate);
end;
// то же, если =DUPES= находится на директорию выше
if FileExists(ExtractFilePath(opendialog.filename)+'\..\=DUPES=.txt')=true then
begin
assignfile(translate,ExtractFilePath(opendialog.filename)+'\..\=DUPES=.txt',CP_UTF8);
reset(translate);
 while not Eof(translate) do
 begin
  ReadLn(translate, line);
   if (copy(line,1,1)='[') and (not Eof(translate)) then
  begin
    idm:=idm+1;
    dupes[idm]:=line;
    ReadLn(translate, line);
    if (copy(line,1,1)<>'')and(copy(line,1,1)<>';')and(copy(line,1,1)<>'[')then
    begin idm:=idm+1; dupes[idm]:=line; end
    else idm:=idm-1;
  end;
end;
closefile(translate);
end;
if extractfilename(opendialog.filename)='=CORE=.txt' then
begin
assignfile(translate,ExtractFilePath(Application.ExeName)+
'\..\english\'+extractfilename(opendialog.filename),CP_UTF8);
end
else
begin
// если обрабатываемый файл - не =CORE=, повторяющиеся строки считываются и из него.
assignfile(translate,ExtractFilePath(opendialog.filename)+'\..\=CORE=.txt',CP_UTF8);
reset(translate);
 while not Eof(translate) do
 begin
  ReadLn(translate, line);
   if (copy(line,1,1)='[') and (not Eof(translate)) then
  begin
    idm:=idm+1;
    dupes[idm]:=line;
    ReadLn(translate, line);
    if (copy(line,1,1)<>'')and(copy(line,1,1)<>';')and(copy(line,1,1)<>'[')then
    begin
    idm:=idm+1;
    dupes[idm]:=line;
    end else idm:=idm-1;
     end;
 end;
closefile(translate);
assignfile(translate,ExtractFilePath(Application.ExeName)+
'\..\english\plugins\'+extractfilename(opendialog.filename),CP_UTF8);
end;
   reset(translate);
   ReadLn(translate, bom);
   ie:=0;
while not Eof(translate) do
     begin
       ReadLn(translate, line);
       ie:=ie+1;
       ustring[ie]:=line;
       end;
   closefile(translate);
for i := 1 to ie do
   for  ii:= 1 to it-1 do
        if ustring[i]=full[ii]
        then  tstring[i]:=full[ii+1];
for i := 1 to ie do
if (copy(ustring[i],1,1)='[') and (tstring[i]='') then
begin
if (extractfilename(opendialog.filename)='=CORE=.txt') and (idm>0) then
     for idx := 1 to idm do
       if dupes[idx]=ustring[i] then tstring[i]:=dupes[idx+1];
if (extractfilename(opendialog.filename)<>'=CORE=.txt') and (idm>0)
and (button3.Caption='DUPES ON') then
   for idx := 1 to idm do
       if dupes[idx]=ustring[i] then tstring[i]:=dupes[idx+1];
if tstring[i]='' then
begin
listbox1.Items.Add(ustring[i]);
notranslate[listbox1.Items.Count]:=i;
end;
end;
label3.caption:='Untranslate:'+inttostr(listbox1.Items.Count)+' lines.';
refresh;
end;
end.
