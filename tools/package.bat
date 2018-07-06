@ECHO OFF
CALL environment.bat

CD /D "%ROOT%"
mkdir build
CD /D "%ROOT%\build"

SETLOCAL EnableDelayedExpansion
FOR /L %%i IN (1,1,%COMPILER#%) DO (
	ECHO -- BUILD FOR "!COMPILER[%%i]!" --
	mkdir "!PATH[%%i]!"
	pushd "!PATH[%%i]!"
	start "obs-enc-aomedia-av1 !COMPILER[%%i]!" /D "!PATH[%%i]!" /LOW cmake --build . --target PACKAGE_ZIP --config RelWithDebInfo
	start "obs-enc-aomedia-av1 !COMPILER[%%i]!" /D "!PATH[%%i]!" /LOW cmake --build . --target PACKAGE_7ZIP --config RelWithDebInfo
	popd
)
ENDLOCAL
