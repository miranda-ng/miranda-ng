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
    Button1: TButton;
    Button2: TButton;
    ListBox1: TListBox;
    OpenDialog1: TOpenDialog;
    X: TBitBtn;
    Memo1: TMemo;
    Memo2: TMemo;
    procedure FormCreate(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
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
//
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
          writeln(translate,ustring[i]);
          writeln(translate,tstring[i]);
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


procedure TForm1.CClick(Sender: TObject);
begin
//
end;

procedure TForm1.XClick(Sender: TObject);
begin
  memo2.Lines.Clear;
  tstring[notranslate[strtoint(label2.caption)]]:='';
  refresh;
end;

procedure TForm1.enter(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
case key of
    vk_down: if listbox1.ItemIndex<listbox1.Items.Count then
    begin listbox1.ItemIndex:=listbox1.ItemIndex+1; refresh; end;
    vk_up: if listbox1.ItemIndex>0 then
    begin listbox1.ItemIndex:=listbox1.ItemIndex-1; refresh; end;
end;
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
var m,n:integer;
begin
memo1.Lines.Clear;
n:=strtoint(label2.caption);
//
if memo2.Lines.Count=memo1.Lines.Count then
tstring[notranslate[n]]:='';
for I := 0 to memo2.Lines.Count-1 do
begin
tstring[notranslate[n]]:=tstring[notranslate[n]]+memo2.Lines[i];
if i<memo2.Lines.Count-1 then
tstring[notranslate[n]]:=tstring[notranslate[n]]+'\n';
end;


//
label2.Caption:=inttostr(ListBox1.ItemIndex+1);
n:=strtoint(label2.caption);
memo2.Lines.Clear;
//
if tstring[notranslate[n]]<>'' then
begin
m:=1;
for i := 1 to length(copy(tstring[notranslate[n]],2,length(tstring[notranslate[n]])-2))-1 do
begin
  if copy(copy(tstring[notranslate[n]],2,length(tstring[notranslate[n]])-2),i,2)='\n' then
  begin
  memo2.Lines.Add(copy(copy(tstring[notranslate[n]],2,length(tstring[notranslate[n]])-2),m,i-m));
  m:=i+2;
  end;
end;
  if m=1 then   memo2.Lines.Add(copy(tstring[notranslate[n]],2,length(tstring[notranslate[n]])-2))
  else memo2.Lines.Add(copy(copy(tstring[notranslate[n]],2,length(tstring[notranslate[n]])-2),
  m,length(copy(tstring[notranslate[n]],2,length(tstring[notranslate[n]])-2))-m+1));
end;
//
m:=1;
for i := 1 to length(copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2))-1 do
begin
  if copy(copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2),i,2)='\n' then
  begin
  memo1.Lines.Add(copy(copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2),m,i-m));
  m:=i+2;
  end;
end;
  if m=1 then   memo1.Lines.Add(copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2))
  else memo1.Lines.Add(copy(copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2),
  m,length(copy(ustring[notranslate[n]],2,length(ustring[notranslate[n]])-2))-m+1));
//

memo2.SetFocus;
end;



procedure tform1.read;
begin
   listbox1.Items.Clear;
   line:='';
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
then for idx := 1 to idm do
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
