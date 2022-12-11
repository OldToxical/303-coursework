cd /d %~dp0
powershell -Command "Invoke-WebRequest https://github.com/Abertay-University-SDI/CMP301_Libraries/archive/refs/heads/main.zip -OutFile GenLibs.zip"
powershell Expand-Archive GenLibs.zip 
move /Y GenLibs\CMP301_Libraries-main\lib %~dp0
move /Y GenLibs\CMP301_Libraries-main\x64 %~dp0
pause
del GenLibs.zip
rmdir GenLibs /S /Q
