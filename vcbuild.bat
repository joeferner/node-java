@echo OFF

echo Check for nodejs build location variable: %NODE_ROOT%
if not defined NODE_ROOT goto nodebuild-not-found
if not exist "%NODE_ROOT%\src\node.h" goto nodebuild-not-found
if not exist "%NODE_ROOT%\deps\v8\include\v8.h" goto nodebuild-not-found
if not exist "%NODE_ROOT%\deps\uv\include\uv.h" goto nodebuild-not-found

echo detect the location of the node.lib file
set nodelibpath=
if exist "%NODE_ROOT%\Release\node.lib" set nodelibpath=%NODE_ROOT%\Release
if not defined nodelibpath if exist "%NODE_ROOT%\Debug\node.lib" set nodelibpath=%NODE_ROOT%\Debug
if not defined nodelibpath goto nodebuild-not-found

echo detect java
if not defined NODE_ROOT goto java-not-found
if not exist "%JAVA_HOME%\include\jni.h" goto java-not-found
if not exist "%JAVA_HOME%\lib\jvm.lib" goto java-not-found

echo Check for visual studio tools if not already loaded
if defined VCINSTALLDIR goto start-compilation
if not defined VS100COMNTOOLS goto msbuild-not-found
if not exist "%VS100COMNTOOLS%..\..\vc\vcvarsall.bat" goto msbuild-not-found
call "%VS100COMNTOOLS%..\..\vc\vcvarsall.bat"
if not defined VCINSTALLDIR goto msbuild-not-found

:start-compilation
echo Compiling...
@rem "support throws" "don't strip comments" "no banner" "disable intrinsic functions" "no optimization" "calling conversion __cdecl" "no analysis" "the /I adds some folders in the include path"
rem /ZI /nologo /W3 /WX- /Od /Oy- /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "NODEJAVA_EXPORTS" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /Gm /EHsc /RTC1 /GS /fp:precise /Zc:wchar_t /Zc:forScope /Yu"StdAfx.h" /Fp"Debug\node-java.pch" /Fa"Debug\" /Fo"Debug\" /Fd"Debug\vc100.pdb" /Gd /analyze- /errorReport:queue 
cl.exe src\*.cpp /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /EHsc /c /nologo /Oi- /Od /Gd /analyze- /I "%NODE_ROOT%\deps\v8\include" /I "%NODE_ROOT%\src" /I "%NODE_ROOT%\deps\uv\include" /I "%JAVA_HOME%\include" /I "%JAVA_HOME%\include\win32"
if errorlevel 1 goto exit-error
echo Done compiling. Linking...
echo Using %nodelibpath%\node.lib file to link to
link *.obj node.lib jvm.lib uv.lib /OUT:"nodejavabridge_bindings.dll" /NOLOGO /DLL /MANIFEST:NO /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X86 /LIBPATH:%nodelibpath% /LIBPATH:%nodelibpath%/lib /LIBPATH:"%JAVA_HOME%\lib"
if errorlevel 1 goto exit-error
echo Done linking
echo Cleaning up
if not exist build mkdir build
if not exist build\Release mkdir build\Release
move nodejavabridge_bindings.dll build\Release\nodejavabridge_bindings.node
echo Finished
goto exit

:msbuild-not-found
echo Visual studio tools were not found! Please check the VS100COMNTOOLS path variable
goto exit

:nodebuild-not-found
echo Node build path not found! Please check the NODE_ROOT path variable exists and that it points to the root of the git repo where you have build 
goto exit

:java-not-found
echo Java not found! Please check JAVA_HOME variable.
goto exit

:exit-error
echo An error occured. Please check the above output
:exit
