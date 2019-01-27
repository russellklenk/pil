@ECHO OFF

:: The build.cmd script implements the build process for Microsoft Visual C++
:: on Microsoft Windows platforms.

:: Ensure that setenv.cmd has been called.
IF [%ROOTDIR%] EQU [] (
    CALL setenv.cmd
)
IF [%ROOTDIR%] EQU [] (
    ECHO ERROR: No ROOTDIR defined. Did setenv.cmd fail? Skipping build.
    EXIT /B 1
)

:: Everything past this point is done in a local environment to allow switching
:: between debug and release build configurations.
SETLOCAL

:: Process command line arguments passed to the script
:Process_Argument
IF [%1] EQU [] GOTO Default_Arguments
IF /I "%1" == "debug" SET BUILD_CONFIGURATION=debug
IF /I "%1" == "release" SET BUILD_CONFIGURATION=release
SHIFT
GOTO Process_Argument

:Default_Arguments
IF [%BUILD_CONFIGURATION%] EQU [] SET BUILD_CONFIGURATION=release

:: Ensure that a Vulkan SDK is installed.
IF [%VULKAN_SDK%] EQU [] (
    ECHO ERROR: No VULKAN_SDK environment variable defined.
    EXIT /b 1
)

:: Specify the location of the Vulkan SDK headers.
SET VULKAN_INCLUDES=%VULKAN_SDK%\Include

:: Specify the source files for the PIL library.
SET COMMON_SOURCES="%SOURCESDIR%\*.cc"
SET PLATFORM_SOURCES="%SOURCESDIR%\win32\*.cc"

:: Specify the libraries the test drivers should link with.
SET LIBRARIES=User32.lib Gdi32.lib Shell32.lib Advapi32.lib winmm.lib

:: Specify cl.exe and link.exe settings.
SET DEFINES_COMMON=/D WINVER=%WINVER% /D _WIN32_WINNT=%WINVER% /D UNICODE /D _UNICODE /D _STDC_FORMAT_MACROS /D _CRT_SECURE_NO_WARNINGS
SET DEFINES_COMMON_DEBUG=%DEFINES_COMMON% /D DEBUG /D _DEBUG
SET DEFINES_COMMON_RELEASE=%DEFINES_COMMON% /D NDEBUG /D _NDEBUG
SET INCLUDES_COMMON=-I"%VULKAN_INCLUDES%" -I"%INCLUDESDIR%" -I"%RESOURCEDIR%" -I"%SOURCESDIR%"
SET CPPFLAGS_COMMON=%INCLUDES_COMMON% /FC /FAs /nologo /W4 /WX /wd4505 /wd4205 /wd4204 /Zi /EHsc
SET CPPFLAGS_DEBUG=%CPPFLAGS_COMMON% /Od
SET CPPFLAGS_RELEASE=%CPPFLAGS_COMMON% /Ob2it

:: Specify build-configuration settings.
IF /I "%BUILD_CONFIGURATION%" == "release" (
    SET DEFINES=%DEFINES_COMMON_RELEASE%
    SET CPPFLAGS=%CPPFLAGS_RELEASE%
    SET LNKFLAGS=%LIBRARIES% /MT
) ELSE (
    SET DEFINES=%DEFINES_COMMON_DEBUG%
    SET CPPFLAGS=%CPPFLAGS_DEBUG%
    SET LNKFLAGS=%LIBRARIES% /MTd
)

ECHO Build output will be placed in "%OUTPUTDIR%".
ECHO Using VULKAN_SDK found at "%VULKAN_SDK%".
ECHO.

:: Ensure that the output directory exists.
IF NOT EXIST "%OUTPUTDIR%" MKDIR "%OUTPUTDIR%"
IF NOT EXIST "%LIBOUTPUTDIR%" MKDIR "%LIBOUTPUTDIR%"
IF NOT EXIST "%EXEOUTPUTDIR%" MKDIR "%EXEOUTPUTDIR%"
IF NOT EXIST "%EXEOUTPUTDIR%\vulkan-1.dll" XCOPY "%VULKAN_SDK%\Source\lib\vulkan-1.dll" "%EXEOUTPUTDIR%" /Y /Q > nul 2>&1
IF NOT EXIST "%EXEOUTPUTDIR%\vulkan-1.pdb" XCOPY "%VULKAN_SDK%\Source\lib\vulkan-1.pdb" "%EXEOUTPUTDIR%" /Y /Q > nul 2>&1

:: Initialize the build result state.
SET BUILD_FAILED=

:: Build the primary artifact, a static library named libpil.lib.
:: cl.exe compiles the .cc files to .obj files.
:: The c switch means compile without linking.
:: lib.exe packages the .obj files into a static library
PUSHD "%LIBOUTPUTDIR%"
ECHO Building "%LIBOUTPUTDIR%\libpil.lib"...
cl.exe /c %CPPFLAGS% %COMMON_SOURCES% %PLATFORM_SOURCES% %DEFINES%
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: Build failed for libpil.lib.
    SET BUILD_FAILED=1
    GOTO Check_Build
)
lib.exe /NOLOGO /WX /OUT:"libpil.lib" *.obj
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: Build failed for libpil.lib.
    SET BUILD_FAILED=1
    GOTO Check_Build
)
ECHO.
POPD

:: Build all of the test module entry points.
PUSHD "%EXEOUTPUTDIR%"
FOR %%x IN ("%MAINDIR%"\*.c*) DO (
    IF NOT EXIST "%EXEOUTPUTDIR%\%%~nx" MKDIR "%EXEOUTPUTDIR%\%%~nx"
    PUSHD "%EXEOUTPUTDIR%\%%~nx"
    ECHO Building "%EXEOUTPUTDIR%\%%~nx\%%~nx.exe"...
    cl.exe %CPPFLAGS% "%%x" %DEFINES% libpil.lib %LNKFLAGS% /Fe%%~nx.exe /link /LIBPATH:"%LIBSDIR%" /LIBPATH:"%LIBOUTPUTDIR%" /OPT:NOREF
    IF %ERRORLEVEL% NEQ 0 (
        ECHO ERROR: Build failed for %%~nx.exe.
        SET BUILD_FAILED=1
    )
    XCOPY "%EXEOUTPUTDIR%\%%~nx\%%~nx.exe" "%EXEOUTPUTDIR%" /Y /Q > nul 2>&1
    ECHO.
    POPD
)
POPD

:Check_Build
IF [%BUILD_FAILED%] NEQ [] (
    GOTO Build_Failed
) ELSE (
    GOTO Build_Succeeded
)

:Build_Failed
ECHO BUILD FAILED.
ENDLOCAL
EXIT /B 1

:Build_Succeeded
ECHO BUILD SUCCEEDED.
ENDLOCAL
EXIT /B 0

:SetEnv_Failed
EXIT /b 1

