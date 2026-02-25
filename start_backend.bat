@echo off
setlocal

echo ========================================
echo Starting IoT Load Meter Backend
echo ========================================
echo.

REM ---- Ensure Mosquitto is running (service-aware) ----
echo Checking Mosquitto service...
sc query mosquitto >nul 2>&1
if %errorlevel%==0 (
    echo Mosquitto service exists.
    sc query mosquitto | find "RUNNING" >nul
    if %errorlevel%==0 (
        echo Mosquitto service already running.
    ) else (
        echo Starting Mosquitto service...
        net start mosquitto
        echo net start exit code: %errorlevel%
    )
) else (
    echo Mosquitto service NOT found. Checking port 1883...
    netstat -aon | findstr ":1883 .*LISTENING"
    if %errorlevel%==0 (
        echo Port 1883 already in use. Not starting another broker.
    ) else (
        echo Starting Mosquitto broker manually...
        start "Mosquitto Broker" cmd /k ^
        cd /d "C:\Program Files\Mosquitto" ^&^& ^
        mosquitto -v -c mosquitto.conf
        timeout /t 2 >nul
    )
)

echo.
echo Starting Python subscriber...
start "Python CSV Subscriber" cmd /k ^
cd /d "C:\A Data Folder\Projects\IoT load_meter" ^&^& ^
python -u -m pi.src.main csv

echo.
echo Done launching windows.
pause