# Microsoft Developer Studio Project File - Name="emailBackEnd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=emailBackEnd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "emailBackEnd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "emailBackEnd.mak" CFG="emailBackEnd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "emailBackEnd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "emailBackEnd - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "emailBackEnd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EMAILBACKEND_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EMAILBACKEND_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\emailBackEnd.ocx"

!ELSEIF  "$(CFG)" == "emailBackEnd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EMAILBACKEND_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EMAILBACKEND_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /win32 /char ascii7
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\emailBackEnd.ocx" /pdbtype:sept
# Begin Custom Build
OutDir=.\Debug
TargetPath=\CursiVision\emailBackEnd.ocx
InputPath=\CursiVision\emailBackEnd.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "emailBackEnd - Win32 Release"
# Name "emailBackEnd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "COM Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dllMain.cpp
# End Source File
# Begin Source File

SOURCE=.\emailBackEnd.odl

!IF  "$(CFG)" == "emailBackEnd - Win32 Release"

!ELSEIF  "$(CFG)" == "emailBackEnd - Win32 Debug"

# ADD MTL /tlb "..\emailBackEnd.tlb" /h "..\emailBackEnd_i.h" /iid "..\emailBackEnd_i.c"
# SUBTRACT MTL /mktyplib203

!ENDIF 

# End Source File
# End Group
# Begin Group "COM Interfaces"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ICursiVisionBackEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\IPropertiesClient.cpp
# End Source File
# Begin Source File

SOURCE=.\IPropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=.\IUnknown.cpp
# End Source File
# End Group
# Begin Group "emails"

# PROP Default_Filter ""
# Begin Group "munpack"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\emails\munpack\codes.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\decode.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\encode.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\magic.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\md5c.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\mpack.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\part.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\string.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\unixos.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\uudecode.c
# End Source File
# Begin Source File

SOURCE=.\emails\munpack\xmalloc.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\emails\emailUtilities.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\emailBackEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\emailBackEnd.def
# End Source File
# Begin Source File

SOURCE=.\propertiesHandler.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\emailBackEnd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\emailBackEnd.rc
# End Source File
# End Group
# End Target
# End Project
