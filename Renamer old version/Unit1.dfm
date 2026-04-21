object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Form1'
  ClientHeight = 573
  ClientWidth = 553
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 294
    Top = 346
    Width = 251
    Height = 56
    AutoSize = False
    Caption = 
      #1042' '#1082#1072#1090#1072#1083#1086#1075#1072#1093' MemoCats '#1080#1097#1077#1090' '#1087#1072#1087#1082#1080' '#1080#1083#1080' '#1092#1072#1081#1083#1099' '#1089#1086#1076#1077#1088#1078#1072#1097#1080#1077' '#1090#1077#1082#1089#1090' '#1080#1079' Ed' +
      'itFind '#1080' '#1087#1077#1088#1077#1080#1084#1077#1085#1086#1074#1099#1074#1072#1077#1090' '#1080#1093', '#1079#1072#1084#1077#1085#1103#1103' '#1080#1089#1082#1086#1084#1099#1081' '#1092#1088#1072#1075#1084#1077#1085#1090' '#1085#1072' '#1090#1077#1082#1089#1090' '#1080 +
      #1079' EditPaste'
    WordWrap = True
  end
  object Label3: TLabel
    Left = 8
    Top = 448
    Width = 134
    Height = 26
    Caption = #1048#1097#1077#1090' '#1086#1076#1080#1085#1072#1082#1086#1074#1099#1077' '#1092#1072#1081#1083#1099' '#1087#1086' '#1088#1072#1079#1084#1077#1088#1072#1084
    WordWrap = True
  end
  object Label2: TLabel
    Left = 151
    Top = 448
    Width = 134
    Height = 26
    Caption = #1048#1097#1077#1090' '#1086#1076#1080#1085#1072#1082#1086#1074#1099#1077' '#1092#1072#1081#1083#1099' '#1087#1086' '#1080#1084#1077#1085#1080
    WordWrap = True
  end
  object Label4: TLabel
    Left = 294
    Top = 448
    Width = 119
    Height = 26
    Caption = #1048#1097#1077#1090' '#1082#1086#1088#1086#1090#1082#1080#1077' '#1080' '#1086#1076#1085#1086#1089#1080#1084#1074#1086#1083#1100#1085#1099#1077' '#1080#1084#1077#1085#1072
    WordWrap = True
  end
  object ButtonAddCat: TButton
    Left = 8
    Top = 8
    Width = 161
    Height = 25
    Caption = #1042#1099#1073#1088#1072#1090#1100' '#1082#1072#1090#1072#1083#1086#1075
    TabOrder = 8
    OnClick = ButtonAddCatClick
  end
  object Button2: TButton
    Left = 175
    Top = 8
    Width = 185
    Height = 25
    Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1089#1087#1080#1089#1086#1082' '#1082#1072#1090#1072#1083#1086#1075#1086#1074
    TabOrder = 17
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 366
    Top = 8
    Width = 179
    Height = 25
    Caption = #1054#1090#1082#1088#1099#1090#1100' '#1089#1087#1080#1089#1086#1082' '#1082#1072#1090#1072#1083#1086#1075#1086#1074
    TabOrder = 18
    OnClick = Button3Click
  end
  object ButtonFindSimpleNames: TButton
    Left = 294
    Top = 417
    Width = 137
    Height = 25
    Caption = 'ButtonFindSimpleNames'
    TabOrder = 16
    OnClick = ButtonFindSimpleNamesClick
  end
  object StaticText1: TStaticText
    Left = 85
    Top = 542
    Width = 333
    Height = 17
    Caption = #1055#1077#1088#1077#1080#1084#1077#1085#1086#1074#1099#1074#1072#1077#1090' '#1092#1072#1081#1083' '#1080#1083#1080' '#1082#1072#1090#1072#1083#1086#1075' EditOldName '#1074' EditNewName'
    TabOrder = 14
  end
  object EditOldName: TEdit
    Left = 8
    Top = 488
    Width = 537
    Height = 21
    TabOrder = 0
    Text = 'EditOldName'
  end
  object EditNewName: TEdit
    Left = 8
    Top = 515
    Width = 537
    Height = 21
    TabOrder = 1
    Text = 'EditNewName'
  end
  object ButtonRename: TButton
    Left = 424
    Top = 542
    Width = 121
    Height = 25
    Caption = 'ButtonRename'
    TabOrder = 2
    OnClick = ButtonRenameClick
  end
  object ButtonFindRename: TButton
    Left = 151
    Top = 346
    Width = 137
    Height = 56
    Caption = 'ButtonFindRename'
    TabOrder = 3
    OnClick = ButtonFindRenameClick
  end
  object EditFind: TEdit
    Left = 8
    Top = 292
    Width = 537
    Height = 21
    TabOrder = 4
    Text = 'EditFind'
  end
  object EditPaste: TEdit
    Left = 8
    Top = 319
    Width = 537
    Height = 21
    TabOrder = 5
    Text = 'EditPaste'
  end
  object MemoCats: TMemo
    Left = 8
    Top = 39
    Width = 537
    Height = 247
    ScrollBars = ssBoth
    TabOrder = 6
  end
  object CheckBoxPodcat: TCheckBox
    Left = 8
    Top = 394
    Width = 137
    Height = 17
    Caption = #1055#1086#1080#1089#1082' '#1074' '#1087#1086#1076#1082#1072#1090#1072#1083#1086#1075#1072#1093
    TabOrder = 7
  end
  object CheckBoxScanFiles: TCheckBox
    Left = 8
    Top = 346
    Width = 137
    Height = 17
    Caption = #1055#1086#1080#1089#1082' '#1092#1072#1081#1083#1086#1074
    TabOrder = 9
  end
  object CheckBoxScanCats: TCheckBox
    Left = 8
    Top = 369
    Width = 137
    Height = 17
    Caption = #1055#1086#1080#1089#1082' '#1082#1072#1090#1072#1083#1086#1075#1086#1074
    TabOrder = 10
  end
  object MemoScannedCats: TMemo
    Left = 80
    Top = 39
    Width = 121
    Height = 247
    Lines.Strings = (
      'MemoScannedCats')
    ScrollBars = ssBoth
    TabOrder = 11
    Visible = False
  end
  object ButtonFindDubSizes: TButton
    Left = 8
    Top = 417
    Width = 137
    Height = 25
    Caption = 'ButtonFindDubSizes'
    TabOrder = 12
    OnClick = ButtonFindDubSizesClick
  end
  object ButtonFindDubNames: TButton
    Left = 151
    Top = 417
    Width = 137
    Height = 25
    Caption = 'ButtonFindDubNames'
    TabOrder = 15
    OnClick = ButtonFindDubNamesClick
  end
  object PanelView: TPanel
    Left = 217
    Top = 39
    Width = 168
    Height = 247
    BevelOuter = bvNone
    BorderWidth = 5
    TabOrder = 13
    Visible = False
    object Panel2: TPanel
      Left = 5
      Top = 212
      Width = 158
      Height = 30
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 0
      DesignSize = (
        158
        30)
      object Button1: TButton
        Left = 80
        Top = 5
        Width = 75
        Height = 25
        Anchors = [akRight, akBottom]
        Caption = #1057#1082#1088#1099#1090#1100
        TabOrder = 0
        OnClick = Button1Click
      end
      object ButtonAutoRen: TButton
        Left = -56
        Top = 5
        Width = 130
        Height = 25
        Anchors = [akRight, akBottom]
        Caption = #1040#1074#1090#1086#1087#1077#1088#1077#1080#1084#1077#1085#1086#1074#1072#1085#1080#1077
        TabOrder = 1
        OnClick = ButtonAutoRenClick
      end
    end
    object StringGrid1: TStringGrid
      Left = 5
      Top = 5
      Width = 158
      Height = 207
      Align = alClient
      FixedCols = 0
      FixedRows = 0
      PopupMenu = PopupMenu1
      TabOrder = 1
      ColWidths = (
        64
        64
        64
        64
        64)
      RowHeights = (
        24
        24
        24
        24
        24)
    end
  end
  object Button4: TButton
    Left = 456
    Top = 417
    Width = 75
    Height = 25
    Caption = 'Button4'
    TabOrder = 19
    OnClick = Button4Click
  end
  object PopupMenu1: TPopupMenu
    Left = 297
    Top = 127
    object N1: TMenuItem
      Caption = #1054#1090#1082#1088#1099#1090#1100
      OnClick = N1Click
    end
    object N2: TMenuItem
      Caption = #1054#1090#1082#1088#1099#1090#1100' '#1088#1072#1089#1087#1086#1083#1086#1078#1077#1085#1080#1077
      OnClick = N2Click
    end
  end
  object OpenDialog1: TOpenDialog
    Filter = #1057#1087#1080#1089#1086#1082' '#1082#1072#1090#1072#1083#1086#1075#1086#1074'|*.lcts'
    Left = 432
    Top = 40
  end
  object SaveDialog1: TSaveDialog
    Filter = #1057#1087#1080#1089#1086#1082' '#1082#1072#1090#1072#1083#1086#1075#1086#1074'|*.lcts'
    Left = 208
    Top = 40
  end
end
