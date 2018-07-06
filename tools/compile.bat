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
	cmake --build . --target lib-datalane --config RelWithDebInfo
	popd
)
ENDLOCAL
PAUSE