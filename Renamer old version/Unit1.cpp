//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
String path_exe=Different::GetPathToExe();
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
srand(time(0));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonRenameClick(TObject *Sender)
{
AnsiString result;
if(!rename(AnsiString(EditOldName->Text).c_str(), AnsiString(EditNewName->Text).c_str()))
	{
	result=result+EditOldName->Text+" переименован в "+EditNewName->Text+"\n";
	}
	else ShowMessage("Не удалось переименовать");
FILE *fp=fopen("result.txt", "a");
fwrite(result.c_str(),result.Length(),1,fp);
fclose(fp);
}
//---------------------------------------------------------------------------
void TForm1::ScanCatsFromMemo()
{
MemoScannedCats->Clear();
TStringList *AddCatInWorkCat=new TStringList;

for(int i=0; i<MemoCats->Lines->Count; i++)
	{
	if(CheckBoxScanCats->Checked)
		{
		CheckBoxPodcat->Checked ?
			Different::ScanDirForDirs_Podcat(MemoCats->Lines->Strings[i],AddCatInWorkCat) :
			Different::ScanDirForDirs(MemoCats->Lines->Strings[i],AddCatInWorkCat);
		MemoScannedCats->Lines->AddStrings(AddCatInWorkCat);
		AddCatInWorkCat->Clear();
		}
	if(CheckBoxScanFiles->Checked)
		{
		CheckBoxPodcat->Checked ?
			Different::ScanDir_Podcat(MemoCats->Lines->Strings[i],AddCatInWorkCat) :
			Different::ScanDir(MemoCats->Lines->Strings[i],AddCatInWorkCat);
		MemoScannedCats->Lines->AddStrings(AddCatInWorkCat);
		AddCatInWorkCat->Clear();
		}
	}
delete AddCatInWorkCat;
if(!MemoScannedCats->Lines->Count) { ShM("Не обнаружено файлов или каталогов!"); return; }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonFindRenameClick(TObject *Sender)
{
//MemoCats->Clear();
//MemoCats->Lines->Add("тест");
//Form1->EditFind->Text="Грозный";
//Form1->EditPaste->Text="93 ЧР";

ScanCatsFromMemo();

TStringList *FindedCatInWorkCat=new TStringList;

for(int i=0; i<MemoScannedCats->Lines->Count; i++)
	{
	if(Different::GetNameWhithExt(MemoScannedCats->Lines->Strings[i]).Pos(EditFind->Text))
		{
		FindedCatInWorkCat->Add(MemoScannedCats->Lines->Strings[i]);
		}
	}

if(!FindedCatInWorkCat->Count) { ShM("Не обнаружено файлов или каталогов для переименования!"); return; }

if(FindedCatInWorkCat->Count && MessageBox(Form1->Handle, String("Обнаружены файлы или каталоги для переименования:\n"+FindedCatInWorkCat->Text+"\nПереименовывать?").c_str(), String("Загрузить").c_str(), MB_YESNO)==IDYES)
	{
	for(int i=FindedCatInWorkCat->Count-1; i>=0; i--)  // начинаем с конца, потому что вложенные каталоги внизу, и если переименовать верхний - к ним уже изменяется путь
		{
		EditOldName->Text=FindedCatInWorkCat->operator [](i);
		String name=Different::GetNameWhithExt(FindedCatInWorkCat->operator [](i));
		String path=Different::GetPath(FindedCatInWorkCat->operator [](i));
		String begin=name.SubString(1,name.Pos(EditFind->Text)-1);
		String end=name.SubString(name.Pos(EditFind->Text)+EditFind->Text.Length(),name.Length()-(begin.Length()+EditFind->Text.Length()));
		EditNewName->Text=path+"\\"+begin+EditPaste->Text+end;
		ButtonRename->Click();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonAddCatClick(TObject *Sender)
{
TStringDynArray dirA;
TSelectDirFileDlgOpts tseldir;
if(!SelectDirectory("",dirA,tseldir,"Выбор папки","","Выбрать папку")) return;
MemoCats->Lines->Add(dirA[0]);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonFindDubSizesClick(TObject *Sender)
{
TButton *btn=(TButton*)Sender;
String btnCaption=btn->Caption;

CheckBoxScanCats->Checked=false;
CheckBoxScanFiles->Checked=true;

//MemoCats->Clear();
//MemoCats->Lines->Add("D:\\Рисунки\\block Videos\\01 Storage");

ScanCatsFromMemo();

StringGrid1->ClearGrid();
StringGrid1->ColCount=2;
StringGrid1->RowCount=MemoScannedCats->Lines->Count;

int *size=new int [MemoScannedCats->Lines->Count];
for(int i=0; i<MemoScannedCats->Lines->Count; i++)
	{
	size[i]=FilesStrings::GetSize(MemoScannedCats->Lines->Strings[i]);
	StringGrid1->Cells[0][i]=MemoScannedCats->Lines->Strings[i];
	StringGrid1->Cells[1][i]=ItS(size[i]);
	//ListBoxView->Items->Add(+":"+);
	if(i%10==0) { btn->Caption="Размер "+ItS(i); btn->Update(); }
	}

for(int i=0; i<MemoScannedCats->Lines->Count; i++)
	{
	bool i_peremeshen=false;
	for(int j=0; j<MemoScannedCats->Lines->Count; j++)
		{
		if(i!=j && size[i]!=-1 && size[i]==size[j])
			{
			if(!i_peremeshen)
				{
				i_peremeshen=true;
				StringGrid1->RowCount++;
				StringGrid1->Rows[StringGrid1->RowCount-1]=StringGrid1->Rows[i];
				}
			StringGrid1->RowCount++;
			StringGrid1->Rows[StringGrid1->RowCount-1]=StringGrid1->Rows[j];
			size[j]=-1;
			}
		}
	if(i%10==0) { btn->Caption="Сравнение "+ItS(i); btn->Update(); }
	size[i]=-1;
	}
delete [] size;
btn->Caption="Удаление лишнего";
btn->Update();
for(int i=0; i<MemoScannedCats->Lines->Count; i++) StringGrid1->DelRow(0);

PanelView->Align=alClient;
PanelView->Visible=!PanelView->Visible;
StringGrid1->ColWidths[0]=StringGrid1->Width*0.85;
StringGrid1->ColWidths[1]=StringGrid1->Width*0.1;

btn->Caption=btnCaption;
ButtonAutoRen->Hint=btn->Caption;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
PanelView->Visible=!PanelView->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::N1Click(TObject *Sender)
{
ShellExecute(Handle,NULL,StringGrid1->Cells[0][StringGrid1->Row].c_str(),NULL,NULL,SW_RESTORE);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::N2Click(TObject *Sender)
{
String path=Different::GetPath(StringGrid1->Cells[0][StringGrid1->Row]);
String file=Different::GetNameWhithExt(StringGrid1->Cells[0][StringGrid1->Row]);
ShellExecute(0, L"open", L"explorer.exe", ("/select, \"" + file + "\"").c_str(), path.c_str(), SW_NORMAL );
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonFindDubNamesClick(TObject *Sender)
{
TButton *btn=(TButton*)Sender;
String btnCaption=btn->Caption;

CheckBoxScanCats->Checked=false;
CheckBoxScanFiles->Checked=true;

//MemoCats->Clear();
//MemoCats->Lines->Add("тест");

ScanCatsFromMemo();

StringGrid1->ClearGrid();
StringGrid1->ColCount=1;
StringGrid1->RowCount=MemoScannedCats->Lines->Count;

for(int i=0; i<MemoScannedCats->Lines->Count; i++)
	{
	StringGrid1->Cells[0][i]=MemoScannedCats->Lines->Strings[i];
	if(i%10==0) { btn->Caption="Файлы "+ItS(i); btn->Update(); }
	}

for(int i=0; i<MemoScannedCats->Lines->Count; i++)
	{
	bool i_peremeshen=false;
	for(int j=0; j<MemoScannedCats->Lines->Count; j++)
		{
		String n_i=Different::GetNameWhithExt(StringGrid1->Cells[0][i]);
		String n_j=Different::GetNameWhithExt(StringGrid1->Cells[0][j]);
		if(i!=j && n_i!="" && n_i==n_j)
			{
			if(!i_peremeshen)
				{
				i_peremeshen=true;
				StringGrid1->RowCount++;
				StringGrid1->Rows[StringGrid1->RowCount-1]=StringGrid1->Rows[i];
				}
			StringGrid1->RowCount++;
			StringGrid1->Rows[StringGrid1->RowCount-1]=StringGrid1->Rows[j];
			StringGrid1->Cells[0][j]="";
			}
		}
	if(i%10==0) { btn->Caption="Сравнение "+ItS(i); btn->Update(); }
	StringGrid1->Cells[0][i]="";
	}
btn->Caption="Удаление лишнего";
btn->Update();
for(int i=0; i<MemoScannedCats->Lines->Count; i++) StringGrid1->DelRow(0);

PanelView->Align=alClient;
PanelView->Visible=!PanelView->Visible;
StringGrid1->ColWidths[0]=StringGrid1->Width*0.95;

btn->Caption=btnCaption;
ButtonAutoRen->Hint=btn->Caption;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonFindSimpleNamesClick(TObject *Sender)
{
TButton *btn=(TButton*)Sender;
String btnCaption=btn->Caption;

CheckBoxScanCats->Checked=false;
CheckBoxScanFiles->Checked=true;

//MemoCats->Clear();
//MemoCats->Lines->Add("тест");

ScanCatsFromMemo();

StringGrid1->ClearGrid();
StringGrid1->ColCount=1;
StringGrid1->RowCount=MemoScannedCats->Lines->Count;

for(int i=0; i<MemoScannedCats->Lines->Count; i++)
	{
	StringGrid1->Cells[0][i]=MemoScannedCats->Lines->Strings[i];
	if(i%10==0) { btn->Caption="Файлы "+ItS(i); btn->Update(); }
	}

for(int i=StringGrid1->RowCount-1; i>=0; i--)
	{
	bool siple_name=false;
	String name=Different::GetNameNoExt(StringGrid1->Cells[0][i]);
	if(name.Length()<5) siple_name=true;
		else
		{
		bool odinak=true;
		char c=name[1];
		for(int j=2; j<=name.Length(); j++)
			if(c!=name[j]) { odinak=false; break; }
		siple_name=odinak;
		}

	if(!siple_name) StringGrid1->DelRow(i);
	}

PanelView->Align=alClient;
PanelView->Visible=!PanelView->Visible;
StringGrid1->ColWidths[0]=StringGrid1->Width*0.95;

btn->Caption=btnCaption;
ButtonAutoRen->Hint=btn->Caption;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonAutoRenClick(TObject *Sender)
{  // автоперименование
if(ButtonAutoRen->Hint=="ButtonFindDubSizes") ShM("Не предусмотрено");
	else if(ButtonAutoRen->Hint=="ButtonFindDubNames")
		for(int i=0; i<StringGrid1->RowCount; i++)
			{
			String path=Different::GetPath(StringGrid1->Cells[0][i]);
			String name=Different::GetNameNoExt(StringGrid1->Cells[0][i]);
			String ext=Different::GetExt(StringGrid1->Cells[0][i]);
			int delta=random(90000)+9999;
			String newName=name+"_"+ItS(delta)+"."+ext;

			EditOldName->Text=StringGrid1->Cells[0][i];
			EditNewName->Text=path+"\\"+newName;
			ButtonRename->Click();

			StringGrid1->Cells[0][i]=path+"\\"+newName;
			}
		else if(ButtonAutoRen->Hint=="ButtonFindSimpleNames")
			for(int i=0; i<StringGrid1->RowCount; i++)
				{
				String path=Different::GetPath(StringGrid1->Cells[0][i]);
				String name=Different::GetNameNoExt(StringGrid1->Cells[0][i]);
				String ext=Different::GetExt(StringGrid1->Cells[0][i]);
				int delta=random(90000)+9999;
				String newName=name+"_"+ItS(delta)+"."+ext;

				EditOldName->Text=StringGrid1->Cells[0][i];
				EditNewName->Text=path+"\\"+newName;
				ButtonRename->Click();

				StringGrid1->Cells[0][i]=path+"\\"+newName;
				}
			else ShM("Ошибка! ButtonAutoRen->Hint="+ButtonAutoRen->Hint);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
SaveDialog1->InitialDir=path_exe;
if(SaveDialog1->Execute())
	{
	if(!SaveDialog1->FileName.Pos(".lcts")) SaveDialog1->FileName=SaveDialog1->FileName+".lcts";
	if(FileExists(SaveDialog1->FileName) && MBox(Handle,"Такое сохранение уже существует! Перезаписать?","Сохранение списка каталогов",MB_YESNO)==IDNO)
		return;
	MemoCats->Lines->SaveToFile(SaveDialog1->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
OpenDialog1->InitialDir=path_exe;
if(OpenDialog1->Execute()) MemoCats->Lines->LoadFromFile(OpenDialog1->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button4Click(TObject *Sender)
{
Different::ScanDir_Podcat(MemoCats->Lines->Strings[0],MemoCats->Lines);
}
//---------------------------------------------------------------------------

