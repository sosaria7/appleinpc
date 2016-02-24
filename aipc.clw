; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "aipc.h"
LastPage=0

ClassCount=14
Class1=CApple2App
Class2=CAppleStatusBar
Class3=CColorButton
Class4=CDlgColor
Class5=CDlgConfigDisk
Class6=CDlgConfigMockingBoard
Class7=CDlgConfigure
Class8=CDlgDebug
Class9=CMainFrame
Class10=CScreen
Class11=wcSliderButton
Class12=wcSliderPopup
Class13=CAboutDlg

ResourceCount=11
Resource1=IDR_ACCELERATOR
Resource2=IDD_DEBUG (English (U.S.))
Resource3=IDR_CONFIG_SLOT
Resource4=IDD_CONFIGURE_MOCKINGBOARD (English (U.S.))
Resource5=IDD_CONFIGURE_DISK (English (U.S.))
Resource6=IDD_CONFIGURE_SD_DISK (English (U.S.))
Resource7=IDR_MAINFRAME
Resource8=IDD_CONFIGURE (English (U.S.))
Resource9=IDD_ABOUTBOX (English (U.S.))
Resource10=IDD_CONFIGURE_COLOR (English (U.S.))
Class14=CDlgConfigSDDisk
Resource11=IDR_TOOLBAR (English (U.S.))

[CLS:CApple2App]
Type=0
BaseClass=CWinApp
HeaderFile=include\arch\frame\aipc.h
ImplementationFile=src\arch\frame\apple2.cpp

[CLS:CAppleStatusBar]
Type=0
BaseClass=CStatusBar
HeaderFile=include\arch\frame\applestatusbar.h
ImplementationFile=src\arch\frame\applestatusbar.cpp

[CLS:CColorButton]
Type=0
BaseClass=CButton
HeaderFile=include\arch\frame\colorbutton.h
ImplementationFile=src\arch\frame\colorbutton.cpp

[CLS:CDlgColor]
Type=0
BaseClass=CDialog
HeaderFile=include\arch\frame\dlgcolor.h
ImplementationFile=src\arch\frame\dlgcolor.cpp
Filter=D
VirtualFilter=dWC
LastObject=ID_APP_EXIT

[CLS:CDlgConfigDisk]
Type=0
BaseClass=CDialog
HeaderFile=include\arch\frame\dlgconfigdisk.h
ImplementationFile=src\arch\frame\dlgconfigdisk.cpp

[CLS:CDlgConfigMockingBoard]
Type=0
BaseClass=CDialog
HeaderFile=include\arch\frame\dlgconfigmockingboard.h
ImplementationFile=src\arch\frame\dlgconfigmockingboard.cpp

[CLS:CDlgConfigure]
Type=0
BaseClass=CDialog
HeaderFile=include\arch\frame\dlgconfigure.h
ImplementationFile=src\arch\frame\dlgconfigure.cpp

[CLS:CDlgDebug]
Type=0
BaseClass=CDialog
HeaderFile=include\arch\frame\dlgdebug.h
ImplementationFile=src\arch\frame\dlgdebug.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=include\arch\frame\mainfrm.h
ImplementationFile=src\arch\frame\mainfrm.cpp
LastObject=ID_SCANLINE
Filter=T
VirtualFilter=fWC

[CLS:CScreen]
Type=0
BaseClass=CWnd
HeaderFile=include\arch\frame\screen.h
ImplementationFile=src\arch\frame\screen.cpp
Filter=W
LastObject=CScreen

[CLS:wcSliderButton]
Type=0
BaseClass=CEdit
HeaderFile=include\arch\frame\wcSliderButton.h
ImplementationFile=src\arch\frame\wcSliderButton.cpp

[CLS:wcSliderPopup]
Type=0
BaseClass=CWnd
HeaderFile=include\arch\frame\wcSliderPopup.h
ImplementationFile=src\arch\frame\wcSliderPopup.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=src\arch\frame\apple2.cpp
ImplementationFile=src\arch\frame\apple2.cpp
LastObject=CAboutDlg

[DLG:IDD_CONFIGURE_COLOR]
Type=1
Class=CDlgColor

[DLG:IDD_CONFIGURE_DISK]
Type=1
Class=CDlgConfigDisk

[DLG:IDD_CONFIGURE_MOCKINGBOARD]
Type=1
Class=CDlgConfigMockingBoard

[DLG:IDD_CONFIGURE]
Type=1
Class=CDlgConfigure

[DLG:IDD_DEBUG]
Type=1
Class=CDlgDebug

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_POWER
Command2=ID_REBOOT
Command3=ID_RESET
Command4=ID_APP_EXIT
Command5=ID_DEBUG
Command6=ID_CONFIGURE_SLOTS
Command7=ID_DISKETTE
Command8=ID_HARDDISK
Command9=ID_MONITOR
Command10=ID_2XSCREEN
Command11=ID_SCANLINE
Command12=ID_FULL_SCREEN
Command13=ID_APP_ABOUT
CommandCount=13

[MNU:IDR_CONFIG_SLOT]
Type=1
Class=?
CommandCount=0

[ACL:IDR_ACCELERATOR]
Type=1
Class=?
Command1=ID_RESET
Command2=ID_CONFIGURE_SLOTS
Command3=ID_POWER
Command4=ID_REBOOT
Command5=ID_MONITOR
Command6=ID_FULL_SCREEN
Command7=ID_DISKETTE
Command8=ID_HARDDISK
CommandCount=8

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=7
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[DLG:IDD_DEBUG (English (U.S.))]
Type=1
Class=?
ControlCount=17
Control1=IDOK,button,1342242817
Control2=IDC_LIST_PROCESS,SysListView32,1350658061
Control3=IDC_EIDT_DATA,edit,1350631552
Control4=IDC_LIST_REGISTERS,SysListView32,1350656517
Control5=IDC_FLAGBOX,button,1342177287
Control6=IDC_FLAG_N,static,1342308353
Control7=IDC_FLAG_V,static,1342308353
Control8=IDC_FLAG_EXTEND,static,1342308353
Control9=IDC_FLAG_B,static,1342308353
Control10=IDC_FLAG_D,static,1342308353
Control11=IDC_FLAG_I,static,1342308353
Control12=IDC_STATIC_PROCESS,static,1342308352
Control13=IDC_STATIC_DATA,static,1342308352
Control14=IDC_BUTTON_TRACE,button,1342242816
Control15=IDC_FLAG_Z,static,1342308353
Control16=IDC_FLAG_C,static,1342308353
Control17=IDC_STATIC,static,1342308352

[DLG:IDD_CONFIGURE_DISK (English (U.S.))]
Type=1
Class=?
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT_FILENAME1,edit,1350631552
Control4=IDC_BTN_BROWSE1,button,1342242816
Control5=IDC_STATIC1,static,1342308352
Control6=IDC_EDIT_FILENAME2,edit,1350631552
Control7=IDC_BTN_BROWSE2,button,1342242816
Control8=IDC_CHK_DISK1,button,1342242819
Control9=IDC_CHK_DISK2,button,1342242819
Control10=IDC_ENHANCED_DISK1,button,1342242819
Control11=IDC_ENHANCED_DISK2,button,1342242819

[DLG:IDD_CONFIGURE_MOCKINGBOARD (English (U.S.))]
Type=1
Class=?
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_PHASOR_BALANCE,msctls_trackbar32,1342242853
Control5=IDC_PHASOR_VOLUME,msctls_trackbar32,1342242851
Control6=IDC_PHASOR_MUTE,button,1342242819
Control7=IDC_PHASOR_MOCKING,button,1342308361
Control8=IDC_PHASOR_NATIVE,button,1342177289
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,button,1342177287

[DLG:IDD_CONFIGURE (English (U.S.))]
Type=1
Class=?
ControlCount=55
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_SLOT1,combobox,1344471043
Control4=IDC_SLOT1_SETUP,button,1342242816
Control5=IDC_SLOT2,combobox,1344471043
Control6=IDC_SLOT2_SETUP,button,1342242816
Control7=IDC_SLOT3,combobox,1344471043
Control8=IDC_SLOT3_SETUP,button,1342242816
Control9=IDC_SLOT4,combobox,1344471043
Control10=IDC_SLOT4_SETUP,button,1342242816
Control11=IDC_SLOT5,combobox,1344471043
Control12=IDC_SLOT5_SETUP,button,1342242816
Control13=IDC_SLOT6,combobox,1344471043
Control14=IDC_SLOT6_SETUP,button,1342242816
Control15=IDC_SLOT7,combobox,1344471043
Control16=IDC_SLOT7_SETUP,button,1342242816
Control17=IDC_MACHINE_2PLUS,button,1476591625
Control18=IDC_MACHINE_2E,button,1342242825
Control19=IDC_MACHINE_2C,button,1476460553
Control20=IDC_JOYSTICK_NONE,button,1342373897
Control21=IDC_JOYSTICK_NUMPAD,button,1342242825
Control22=IDC_JOYSTICK_PCJOYSTICK,button,1342242825
Control23=IDC_COLOR_SETTING,button,1342242816
Control24=IDC_MASTER_BALANCE,msctls_trackbar32,1342242853
Control25=IDC_MASTER_VOLUME,msctls_trackbar32,1342242851
Control26=IDC_MASTER_MUTE,button,1342242819
Control27=IDC_SPEAKER_VOLUME,msctls_trackbar32,1342242851
Control28=IDC_SPEAKER_MUTE,button,1342242819
Control29=IDC_KEYBOARD_DELAY,edit,1350631552
Control30=IDC_KEYBOARD_REPEAT,edit,1350631552
Control31=IDC_DEFAULT,button,1476460544
Control32=IDC_STATIC,button,1342177287
Control33=IDC_STATIC,button,1342177287
Control34=IDC_STATIC,button,1342177287
Control35=IDC_STATIC,button,1342177287
Control36=IDC_STATIC,static,1342308352
Control37=IDC_STATIC,static,1342308352
Control38=IDC_STATIC,static,1342308352
Control39=IDC_STATIC,static,1342308352
Control40=IDC_STATIC,static,1342308352
Control41=IDC_STATIC,static,1342308352
Control42=IDC_STATIC,static,1342308352
Control43=IDC_STATIC,static,1342308352
Control44=IDC_STATIC,static,1342308352
Control45=IDC_STATIC,static,1342308352
Control46=IDC_STATIC,static,1342308352
Control47=IDC_STATIC,static,1342308352
Control48=IDC_STATIC,button,1342177287
Control49=IDC_STATIC,button,1342177287
Control50=IDC_STATIC,static,1342308352
Control51=IDC_STATIC,static,1342308352
Control52=IDC_JOYDEAD,combobox,1344339971
Control53=IDC_JOYSAT,combobox,1344339971
Control54=IDC_STATIC,static,1342308352
Control55=IDC_STATIC,static,1342308352

[DLG:IDD_CONFIGURE_COLOR (English (U.S.))]
Type=1
Class=CDlgColor
ControlCount=49
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_COLOR_MODE0,button,1342308361
Control4=IDC_COLOR_MODE1,button,1342177289
Control5=IDC_COLOR_MODE2,button,1342177289
Control6=IDC_COLOR_MODE3,button,1342177289
Control7=IDC_PREVIEW,button,1342373891
Control8=IDC_COLOR0,button,1342373899
Control9=IDC_COLOR1,button,1342242827
Control10=IDC_COLOR2,button,1342242827
Control11=IDC_COLOR3,button,1342242827
Control12=IDC_COLOR4,button,1342242827
Control13=IDC_COLOR5,button,1342242827
Control14=IDC_COLOR6,button,1342242827
Control15=IDC_COLOR7,button,1342242827
Control16=IDC_COLOR8,button,1342242827
Control17=IDC_COLOR9,button,1342242827
Control18=IDC_COLOR10,button,1342242827
Control19=IDC_COLOR11,button,1342242827
Control20=IDC_COLOR12,button,1342242827
Control21=IDC_COLOR13,button,1342242827
Control22=IDC_COLOR14,button,1342242827
Control23=IDC_COLOR15,button,1342242827
Control24=IDC_VIDEO_H,edit,1350762624
Control25=IDC_VIDEO_S,edit,1350631552
Control26=IDC_VIDEO_B,edit,1350631552
Control27=IDC_DEFAULT,button,1342373888
Control28=IDC_STATIC,static,1342308352
Control29=IDC_STATIC,static,1342308352
Control30=IDC_STATIC,static,1342308352
Control31=IDC_COLOR_NAME0,static,1342308352
Control32=IDC_STATIC,static,1342308352
Control33=IDC_STATIC,static,1342308352
Control34=IDC_COLOR_NAME3,static,1342308352
Control35=IDC_STATIC,static,1342308352
Control36=IDC_STATIC,static,1342308352
Control37=IDC_COLOR_NAME6,static,1342308352
Control38=IDC_STATIC,static,1342308352
Control39=IDC_STATIC,static,1342308352
Control40=IDC_COLOR_NAME9,static,1342308352
Control41=IDC_STATIC,static,1342308352
Control42=IDC_STATIC,static,1342308352
Control43=IDC_COLOR_NAME12,static,1342308352
Control44=IDC_STATIC,static,1342308352
Control45=IDC_STATIC,static,1342308352
Control46=IDC_COLOR_NAME15,static,1342308352
Control47=IDC_STATIC,button,1342177287
Control48=IDC_STATIC,button,1342177287
Control49=IDC_SCANLINE,button,1342242819

[TB:IDR_TOOLBAR (English (U.S.))]
Type=1
Class=CMainFrame
Command1=ID_POWERON
Command2=ID_POWEROFF
Command3=ID_RESET
Command4=ID_DISKETTE
Command5=ID_HARDDISK
Command6=ID_FULL_SCREEN
CommandCount=6

[DLG:IDD_CONFIGURE_SD_DISK (English (U.S.))]
Type=1
Class=CDlgConfigSDDisk
ControlCount=9
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT_FILENAME1,edit,1350631552
Control4=IDC_BTN_BROWSE1,button,1342242816
Control5=IDC_STATIC1,static,1342308352
Control6=IDC_EDIT_FILENAME2,edit,1350631552
Control7=IDC_BTN_BROWSE2,button,1342242816
Control8=IDC_CHK_DISK1,button,1342242819
Control9=IDC_CHK_DISK2,button,1342242819

[CLS:CDlgConfigSDDisk]
Type=0
HeaderFile=include\arch\frame\dlgconfigsddisk.h
ImplementationFile=src\arch\frame\dlgconfigsddisk.cpp
BaseClass=CDialog
Filter=D
LastObject=ID_HARDDISK

