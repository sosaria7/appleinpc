# Microsoft Developer Studio Project File - Name="aipc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=aipc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aipc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aipc.mak" CFG="aipc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aipc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "aipc - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "aipc - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "dx80\include" /I "include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"arch/frame/stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib dx80\lib\dsound.lib dx80\lib\dxerr8.lib dx80\lib\ddraw.lib dx80\lib\dxguid.lib dx80\lib\dinput8.lib /nologo /subsystem:windows /pdb:none /machine:I386
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "aipc - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "dx80\include" /I "include" /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"arch\frame\stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /i "dx80\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib dx80\lib\dsound.lib dx80\lib\dxerr8.lib dx80\lib\ddraw.lib dx80\lib\dxguid.lib dx80\lib\dinput8.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"dx80\lib"
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "aipc - Win32 Release"
# Name "aipc - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\aipc.ico
# End Source File
# Begin Source File

SOURCE=.\aipc.rc
# End Source File
# Begin Source File

SOURCE=.\res\aipc.rc2
# End Source File
# Begin Source File

SOURCE=.\aipc.rgs
# End Source File
# Begin Source File

SOURCE=.\RES\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\RES\cursor2.cur
# End Source File
# Begin Source File

SOURCE=.\res\disk_off.bmp
# End Source File
# Begin Source File

SOURCE=.\res\disk_rea.bmp
# End Source File
# Begin Source File

SOURCE=.\res\disk_wri.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\diskOff.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\diskRead.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\diskWrite.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\toolbar1.bmp
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter "*.h"
# Begin Group "arch.h"

# PROP Default_Filter "*.h"
# Begin Group "directx.h"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\include\arch\frame\colorbutton.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\ddutil.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\dibase.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\dijoystick.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\dikeyboard.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\dimouse.h
# End Source File
# Begin Source File

SOURCE=.\include\diskimagepo.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\dlgcolor.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\dxsound.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\directx\dxutil.h
# End Source File
# End Group
# Begin Group "frame.h"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\include\arch\frame\aipc.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\applestatusbar.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\dlgconfigdisk.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\dlgconfigmockingboard.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\dlgconfigsddisk.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\dlgconfigure.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\dlgdebug.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\mainfrm.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\screen.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\stdafx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\arch\CustomThread.h
# End Source File
# Begin Source File

SOURCE=.\include\arch\resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\6522.h
# End Source File
# Begin Source File

SOURCE=.\include\65c02.h
# End Source File
# Begin Source File

SOURCE=.\include\6821.h
# End Source File
# Begin Source File

SOURCE=.\include\8913.h
# End Source File
# Begin Source File

SOURCE=.\include\aipcdefs.h
# End Source File
# Begin Source File

SOURCE=.\include\appleclock.h
# End Source File
# Begin Source File

SOURCE=.\include\applekeymap.h
# End Source File
# Begin Source File

SOURCE=.\include\card.h
# End Source File
# Begin Source File

SOURCE=.\include\cpu.h
# End Source File
# Begin Source File

SOURCE=.\include\debug.h
# End Source File
# Begin Source File

SOURCE=.\include\diskdrive.h
# End Source File
# Begin Source File

SOURCE=.\include\diskimage.h
# End Source File
# Begin Source File

SOURCE=.\include\diskimagedos.h
# End Source File
# Begin Source File

SOURCE=.\include\diskimagenib.h
# End Source File
# Begin Source File

SOURCE=.\include\diskinterface.h
# End Source File
# Begin Source File

SOURCE=.\include\fontdata.h
# End Source File
# Begin Source File

SOURCE=.\include\gcr_table.h
# End Source File
# Begin Source File

SOURCE=.\include\iou.h
# End Source File
# Begin Source File

SOURCE=.\include\joystick.h
# End Source File
# Begin Source File

SOURCE=.\include\keyboard.h
# End Source File
# Begin Source File

SOURCE=.\include\localclock.h
# End Source File
# Begin Source File

SOURCE=.\include\lockmgr.h
# End Source File
# Begin Source File

SOURCE=.\include\memory.h
# End Source File
# Begin Source File

SOURCE=.\include\messageque.h
# End Source File
# Begin Source File

SOURCE=.\include\mousecard.h
# End Source File
# Begin Source File

SOURCE=.\include\optable.h
# End Source File
# Begin Source File

SOURCE=.\include\phasor.h
# End Source File
# Begin Source File

SOURCE=.\include\psg.h
# End Source File
# Begin Source File

SOURCE=.\include\scanline.h
# End Source File
# Begin Source File

SOURCE=.\include\sddiskii.h
# End Source File
# Begin Source File

SOURCE=.\include\slots.h
# End Source File
# Begin Source File

SOURCE=.\include\speaker.h
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter "*.cpp"
# Begin Group "arch"

# PROP Default_Filter "*.cpp"
# Begin Group "directx"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\src\arch\directx\ddutil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\directx\dibase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\directx\dijoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\directx\dikeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\directx\dimouse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\directx\dxsound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\directx\dxutil.cpp
# End Source File
# End Group
# Begin Group "frame"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\src\arch\frame\apple2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\applestatusbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\colorbutton.cpp
# ADD CPP /I "..\..\.."
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\dlgcolor.cpp
# ADD CPP /I "..\..\.."
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\dlgconfigdisk.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\dlgConfigmockingboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\dlgconfigsddisk.cpp
# ADD CPP /I "..\..\.."
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\dlgconfigure.cpp
# ADD CPP /I "..\..\.."
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\dlgdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\mainfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\screen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\wcSliderButton.cpp
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\wcSliderButton.h
# End Source File
# Begin Source File

SOURCE=.\src\arch\frame\wcSliderPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\include\arch\frame\wcSliderPopup.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\arch\CustomThread.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\6522.cpp
# End Source File
# Begin Source File

SOURCE=.\src\65c02.cpp
# End Source File
# Begin Source File

SOURCE=.\src\6821.cpp
# End Source File
# Begin Source File

SOURCE=.\src\8913.cpp
# End Source File
# Begin Source File

SOURCE=.\src\appleclock.cpp
# End Source File
# Begin Source File

SOURCE=.\src\card.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cpu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\diskdrive.cpp
# End Source File
# Begin Source File

SOURCE=.\src\diskimage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\diskimagedos.cpp
# End Source File
# Begin Source File

SOURCE=.\src\diskimagenib.cpp
# End Source File
# Begin Source File

SOURCE=.\src\diskimagepo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\diskinterface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\iou.cpp
# End Source File
# Begin Source File

SOURCE=.\src\joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lockmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\messageque.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mousecard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\phasor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\psg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sddiskii.cpp
# End Source File
# Begin Source File

SOURCE=.\src\slots.cpp
# End Source File
# Begin Source File

SOURCE=.\src\speaker.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section aipc : {000013A5-13BA-0000-D213-000006130000}
# 	1:10:IDR_APPLE2:104
# End Section
