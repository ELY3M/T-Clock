# Microsoft Developer Studio Project File - Name="langen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=langen - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "langen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "langen.mak" CFG="langen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "langen - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "langen - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "langen - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f langja.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "langja.exe"
# PROP BASE Bsc_Name "langja.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f langja.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "langen.exe"
# PROP Bsc_Name "langen.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "langen - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f langja.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "langja.exe"
# PROP BASE Bsc_Name "langja.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "NMAKE /f langja.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "langen.exe"
# PROP Bsc_Name "langen.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "langen - Win32 Release"
# Name "langen - Win32 Debug"

!IF  "$(CFG)" == "langen - Win32 Release"

!ELSEIF  "$(CFG)" == "langen - Win32 Debug"

!ENDIF 

# Begin Source File

SOURCE=.\langja.mak
# End Source File
# Begin Source File

SOURCE=.\langja.rc
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Target
# End Project
