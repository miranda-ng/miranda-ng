unit Unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ExtDlgs, Vcl.StdCtrls, Vcl.Buttons, ShellApi;

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
    C: TBitBtn;
    G: TBitBtn;
    Memo1: TMemo;
    Memo2: TMemo;
    T: TBitBtn;
    procedure FormCreate(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure ListBox1Click(Sender: TObject);
    procedure CClick(Sender: TObject);
    procedure XClick(Sender: TObject);
    procedure refresh;
    procedure read;
    procedure enter(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure GClick(Sender: TObject);
    procedure TClick(Sender: TObject);


  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  openDialog : TOpenDialog;
  full,ustring,tstring,fline: array [1..9999]  of string;
  notranslate: array [1..9999] of integer;
  translate: textfile;
  bom, line:string;
  ii,i,it,ie:integer;
implementation

{$R *.dfm}
// Открытие файла






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
memo2.Lines.Clear;
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
//
  read;
end;


procedure TForm1.CClick(Sender: TObject);
begin
memo2.Lines:=memo1.Lines;
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

procedure TForm1.GClick(Sender: TObject);
var str:string;
 begin
 str:='http://translate.google.com/?hl=&ie=&langpair=en&text=';
for I := 0 to memo1.Lines.Count-1 do
begin
str:=str+memo1.Lines[i];
if i<memo1.Lines.Count-1 then
str:=str+'+%0A+';
end;
   ShellExecute(0, 'open',PChar(str), nil, nil, SW_SHOW);
end;

procedure tform1.refresh;
var m,n:integer;
begin
n:=strtoint(label2.caption);
//
if memo2.Lines.Count=memo1.Lines.Count then
begin
tstring[notranslate[n]]:='';
for I := 0 to memo2.Lines.Count-1 do
begin
tstring[notranslate[n]]:=tstring[notranslate[n]]+memo2.Lines[i];
if i<memo2.Lines.Count-1 then
tstring[notranslate[n]]:=tstring[notranslate[n]]+'\n';
end;
end;
memo1.Lines.Clear;
////////////////////////////////////////////////////////////
label2.Caption:=inttostr(ListBox1.ItemIndex+1);
n:=strtoint(label2.caption);
memo2.Lines.Clear;
if tstring[notranslate[n]]<>'' then
begin
m:=1;
for i := 1 to length(tstring[notranslate[n]])-1 do
begin
  if copy(tstring[notranslate[n]],i,2)='\n' then
  begin
  memo2.Lines.Add(copy(tstring[notranslate[n]],m,i-m));
  m:=i+2;
  end;
end;
  if m=1
  then memo2.Lines.Add(tstring[notranslate[n]])
  else memo2.Lines.Add(copy(tstring[notranslate[n]],
  m,length(tstring[notranslate[n]])-m+2));
end;
////////////////////////////////////////////////////////////
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
memo2.SetFocus;
end;



procedure TForm1.TClick(Sender: TObject);
begin
if form1.formstyle=fsnormal
then
 begin
t.Hint:='Disable On Top';
form1.formstyle:=fsstayontop;
 end
else
 begin
t.Hint:='Enable On Top';
form1.formstyle:=fsnormal;
 end;
end;

procedure tform1.read;
begin
   listbox1.Items.Clear;
   line:='';
 for i:= 1 to 9999 do
 begin
   full[i]:='';
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

if extractfilename(opendialog.filename)='=CORE=.txt' then
assignfile(translate,ExtractFilePath(Application.ExeName)+
'\..\english\'+extractfilename(opendialog.filename),CP_UTF8)
else
  if fileexists(ExtractFilePath(Application.ExeName)+
'\..\english\plugins\'+extractfilename(opendialog.filename)) then
assignfile(translate,ExtractFilePath(Application.ExeName)+
'\..\english\plugins\'+extractfilename(opendialog.filename),CP_UTF8)
else
  if fileexists(ExtractFilePath(Application.ExeName)+
'\..\english\weather\'+extractfilename(opendialog.filename)) then
assignfile(translate,ExtractFilePath(Application.ExeName)+
'\..\english\weather\'+extractfilename(opendialog.filename),CP_UTF8);
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
listbox1.Items.Add(ustring[i]);
notranslate[listbox1.Items.Count]:=i;
end;
label3.caption:=' of '+inttostr(listbox1.Items.Count)+' untranslate lines.';
refresh;
end;
end.
