# Microsoft Developer Studio Project File - Name="tcdll1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=tcdll1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tcdll1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tcdll1.mak" CFG="tcdll1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tcdll1 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "tcdll1 - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "tcdll1 - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f tcdll.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "tcdll.exe"
# PROP BASE Bsc_Name "tcdll.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f tcdll.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "tcdll1.exe"
# PROP Bsc_Name "tcdll1.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "tcdll1 - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f tcdll.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "tcdll.exe"
# PROP BASE Bsc_Name "tcdll.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "NMAKE /f tcdll.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "tcdll1.exe"
# PROP Bsc_Name "tcdll1.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "tcdll1 - Win32 Release"
# Name "tcdll1 - Win32 Debug"

!IF  "$(CFG)" == "tcdll1 - Win32 Release"

!ELSEIF  "$(CFG)" == "tcdll1 - Win32 Debug"

!ENDIF 

# Begin Source File

SOURCE=.\bmp.c
# End Source File
# Begin Source File

SOURCE=.\cpu.c
# End Source File
# Begin Source File

SOURCE=.\desktop.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\format.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\mbm.c
# End Source File
# Begin Source File

SOURCE=.\net.c
# End Source File
# Begin Source File

SOURCE=.\newapi.c
# End Source File
# Begin Source File

SOURCE=.\permon.c
# End Source File
# Begin Source File

SOURCE=.\startbtn.c
# End Source File
# Begin Source File

SOURCE=.\startmenu.c
# End Source File
# Begin Source File

SOURCE=.\sysres.c
# End Source File
# Begin Source File

SOURCE=.\taskswitch.c
# End Source File
# Begin Source File

SOURCE=.\tcdll.h
# End Source File
# Begin Source File

SOURCE=.\tcdll.mak
# End Source File
# Begin Source File

SOURCE=.\tcdll.rc
# End Source File
# Begin Source File

SOURCE=.\tclock.c
# End Source File
# Begin Source File

SOURCE=.\traynotify.c
# End Source File
# Begin Source File

SOURCE=.\utl.c
# End Source File
# End Target
# End Project
