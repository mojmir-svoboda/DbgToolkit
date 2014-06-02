# This is install script for NSIS installer for BlackBox 4 Windows

# where to create install.exe
OutFile c:\_builds\install.exe

# where to install program
InstallDir c:\bbzero_beta

# good for debugging
ShowInstDetails Show

!include LogicLib.nsh
!include WinVer.nsh
!include nsDialogs.nsh
!include x64.nsh
!include Sections.nsh
!addplugindir .

# Set the text to prompt user to enter a directory
DirText "This will install trace server program on your computer. Choose a directory"

Name "BlackBox 4 Windows"
RequestExecutionLevel admin
AddBrandingImage left 256

Page Custom brandimage "" ": Brand Image"
Page License
Page Directory
Page Components
Page Custom windetectionPageEnter windetectionPageLeave
Page InstFiles
Page Custom shellPageEnter shellPageLeave
#UninstPage uninstConfirm
#UninstPage instfiles

LicenseData "GPL.txt" 

Function brandimage
  SetOutPath "$TEMP"
  SetFileAttributes installer.bmp temporary
  File installer.bmp
  SetBrandingImage "$TEMP\installer.bmp" /resizetofit
FunctionEnd


!define PRODUCT "SIG Beta Ver. 1.0"
!define FILE "savefile"
!define VERSION ""
!define BRANDINGTEXT "SIG Beta Ver. 1.0"
CRCCheck On

; Bij deze moeten we waarschijnlijk een absoluut pad gaan gebruiken
; dit moet effe uitgetest worden.
!include "${NSISDIR}\Contrib\Modern UI\System.nsh"


;---------------------------------
;General

OutFile "installsig.exe"
ShowInstDetails "nevershow"
ShowUninstDetails "nevershow"
;SetCompressor "bzip2"

!define ICON "icon.ico"
!define UNICON "icon.ico"
!define SPECIALBITMAP "Bitmap.bmp"


;--------------------------------
;Folder selection page

InstallDir "$PROGRAMFILES\${PRODUCT}"


;--------------------------------
;Modern UI Configuration

!define WELCOMEPAGE  
!define LICENSEPAGE
!define DIRECTORYPAGE
!define ABORTWARNING
!define UNINSTALLER
!define UNCONFIRMPAGE
!define FINISHPAGE  


;--------------------------------
;Language

!insertmacro LANGUAGE "English"


;-------------------------------- 
;Modern UI System

!insertmacro SYSTEM 


;--------------------------------
;Data

LicenseData "Lees_mij.txt"


;-------------------------------- 
;Installer Sections     
Section "install" Installation info

;Add files
SetOutPath "$INSTDIR"

File "${FILE}.exe"
File "${FILE}.ini"
File "Lees_mij.txt"
SetOutPath "$INSTDIR\playlists"
file "playlists\${FILE}.epp"
SetOutPath "$INSTDIR\data"
file "data\*.cst"
file "data\errorlog.txt"
; hier komen dan nog de bestanden die in de playlist staan
SetOutPath "$INSTDIR"  
file /r mpg
SetOutPath "$INSTDIR"  
file /r xtras  

;create desktop shortcut
CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\${FILE}.exe" ""

;create start-menu items
CreateDirectory "$SMPROGRAMS\${PRODUCT}"
CreateShortCut "$SMPROGRAMS\${PRODUCT}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
CreateShortCut "$SMPROGRAMS\${PRODUCT}\${PRODUCT}.lnk" "$INSTDIR\${FILE}.exe" "" "$INSTDIR\${FILE}.exe" 0

;write uninstall information to the registry
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT} (remove only)"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"

WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd


;--------------------------------    
;Uninstaller Section  
Section "Uninstall"

;Delete Files 
RMDir /r "$INSTDIR\*.*"    

;Remove the installation directory
RMDir "$INSTDIR"

;Delete Start Menu Shortcuts
Delete "$DESKTOP\${PRODUCT}.lnk"
Delete "$SMPROGRAMS\${PRODUCT}\*.*"
RmDir  "$SMPROGRAMS\${PRODUCT}"

;Delete Uninstaller And Unistall Registry Entries
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT}"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"  

SectionEnd


;--------------------------------    
;MessageBox Section


;Function that calls a messagebox when installation finished correctly
Function .onInstSuccess
MessageBox MB_OK "You have successfully installed ${PRODUCT}. Use the desktop icon to start the program."
FunctionEnd


Function un.onUninstSuccess
MessageBox MB_OK "You have successfully uninstalled ${PRODUCT}."
FunctionEnd

