unit Unit2;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.CheckLst, unit1;

type
  TForm2 = class(TForm)
    ComboBox1: TComboBox;
    CheckBox1: TCheckBox;
    Button1: TButton;
    Label1: TLabel;
    Label2: TLabel;
    Button2: TButton;
    ComboBox2: TComboBox;
    procedure Button1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure Button2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form2: TForm2;
  n:integer;
implementation

{$R *.dfm}

procedure TForm2.Button1Click(Sender: TObject);
begin
form1.caption:=ComboBox1.text;
form1.RadioGroup1.Items.Add('english');
if combobox2.text<>'' then
form1.RadioGroup1.Items.Add(combobox2.text);
form1.RadioGroup1.ItemIndex:=0;
form1.showmodal;
form2.Close;
end;

procedure TForm2.Button2Click(Sender: TObject);
begin
form2.Close;
end;

procedure TForm2.ComboBox1Change(Sender: TObject);
var   i:integer;
begin
   combobox2.Items.Clear;
  for i :=0 to combobox1.ItemHeight+5 do    // не понял зачем +5
    if i<>combobox1.ItemIndex then
      combobox2.Items.Add(combobox1.Items[i]);
 button1.Enabled:=true;
end;

procedure TForm2.FormCreate(Sender: TObject);
var
  SR:TSearchRec;
  Res:Integer;
begin

Res:=FindFirst(ExtractFilePath(Application.ExeName)+'*', faAnyFile, SR);
while Res = 0 do
begin
if ((SR.Attr and faDirectory)=faDirectory) and ((SR.Name='.')or(SR.Name='..')) then
begin
Res:=FindNext(SR);
Continue;
end;
if ((SR.Attr and faDirectory)=faDirectory) then
begin
if (fileexists(ExtractFilePath(Application.ExeName)+'/'+SR.Name+'/=HEAD=.txt'))
and (sr.Name<>'english') then
begin
Form2.combobox1.Items.Add(SR.Name);

end;
end;
Res:=FindNext(SR);
end;
FindClose(SR);
end;

end.
