@echo off

cd /d "%~dp0"

rmdir /s /q dist

mkdir dist
mkdir dist\x86
mkdir dist\x64

xcopy x86\Release\optusbx.exe dist\x86\
xcopy x64\Release\optusbx.exe dist\x64\

xcopy libusb\Windows\x86\dll\*.dll dist\x86\
xcopy libusb\Windows\x64\dll\*.dll dist\x64\

copy libusb\COPYING dist\libusb_copying

del optusbx.zip
"%ProgramFiles%\7-Zip\7z.exe" a -y -mx=9 -mpass=16 -mfb=65536 optusbx.zip .\dist\*
