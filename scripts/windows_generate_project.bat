\@echo off
REM ---------------------------------------------------------------------
REM Moves to the parent directory of this script, invokes Premake to 
REM generate a Visual Studio 2022 solution, and then returns. 
REM ---------------------------------------------------------------------

REM Move to the script’s parent folder
pushd "%~dp0\..\" || (
  echo Failed to navigate to parent directory. Aborting.
  pause
  exit /b 1
)

REM Check for Premake existence
if not exist "vendor\premake\bin\premake5.exe" (
  echo Premake executable not found at vendor\premake\bin\premake5.exe
  echo Please ensure Premake is installed in the expected location.
  popd
  pause
  exit /b 1
)

REM Run Premake
echo Running Premake to generate VS2022 solution...
call vendor\premake\bin\premake5.exe vs2022
if errorlevel 1 (
  echo Premake encountered an error. 
  popd
  pause
  exit /b 1
)

REM Return to original directory
popd

echo Premake generation completed successfully.
pause
exit /b 0
