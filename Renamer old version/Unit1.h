//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H

//---------------------------------------------------------------------------
#include <Shortings.hpp>
#include <Different.hpp>
#include <FilesStrings.hpp>
#include <MyGrid.hpp>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
#include <Vcl.FileCtrl.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TEdit *EditOldName;
	TEdit *EditNewName;
	TButton *ButtonRename;
	TButton *ButtonFindRename;
	TEdit *EditFind;
	TEdit *EditPaste;
	TLabel *Label1;
	TMemo *MemoCats;
	TCheckBox *CheckBoxPodcat;
	TButton *ButtonAddCat;
	TCheckBox *CheckBoxScanFiles;
	TCheckBox *CheckBoxScanCats;
	TMemo *MemoScannedCats;
	TButton *ButtonFindDubSizes;
	TLabel *Label3;
	TPanel *PanelView;
	TPanel *Panel2;
	TButton *Button1;
	TStringGrid *StringGrid1;
	TStaticText *StaticText1;
	TPopupMenu *PopupMenu1;
	TMenuItem *N1;
	TMenuItem *N2;
	TButton *ButtonFindDubNames;
	TLabel *Label2;
	TButton *ButtonFindSimpleNames;
	TButton *ButtonAutoRen;
	TLabel *Label4;
	TButton *Button2;
	TButton *Button3;
	TOpenDialog *OpenDialog1;
	TSaveDialog *SaveDialog1;
	TButton *Button4;
	void __fastcall ButtonRenameClick(TObject *Sender);
	void __fastcall ButtonFindRenameClick(TObject *Sender);
	void __fastcall ButtonAddCatClick(TObject *Sender);
	void __fastcall ButtonFindDubSizesClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall N1Click(TObject *Sender);
	void __fastcall N2Click(TObject *Sender);
	void __fastcall ButtonFindDubNamesClick(TObject *Sender);
	void __fastcall ButtonFindSimpleNamesClick(TObject *Sender);
	void __fastcall ButtonAutoRenClick(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);

	void ScanCatsFromMemo();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
