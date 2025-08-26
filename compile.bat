@echo off
REM compile.bat

REM Compile all cpp files in cpp/src with headers in cpp/include
g++ cpp\src\*.cpp -Icpp\include -std=c++17 -Wall -o program.exe

if %errorlevel% neq 0 (
    echo.
    echo Build failed.
    exit /b %errorlevel%
)

echo.
echo Build complete. Run program.exe
