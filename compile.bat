@echo off
echo ============================================
echo   SPCMS - Smart Public Complaint System
echo   Compiling...
echo ============================================

g++ -std=c++17 -o spcms.exe ^
    src\User.cpp ^
    src\Officer.cpp ^
    src\SeniorOfficer.cpp ^
    src\Citizen.cpp ^
    src\Admin.cpp ^
    src\Trackable.cpp ^
    src\Complaint.cpp ^
    src\RoadComplaint.cpp ^
    src\WaterComplaint.cpp ^
    src\ElectricityComplaint.cpp ^
    src\ConsoleUI.cpp ^
    src\ComplaintSystem.cpp ^
    src\EscalationManager.cpp ^
    main.cpp ^
    -I include

if %ERRORLEVEL% == 0 (
    echo.
    echo [SUCCESS] Build complete: spcms.exe
    echo.
) else (
    echo.
    echo [FAILED] Build errors detected. Check above.
    echo.
)
pause
