REM ============================================================
REM OptiScaler 中文汉化版安装向导（CHS）
REM 基于上游 setup_windows.bat 汉化，逻辑与上游完全一致
REM 注意：本文件必须以 GBK/ANSI 编码保存，否则中文会乱码！
REM ============================================================
REM Setup OptiScaler for your game
@echo off
cls
echo  ::::::::  :::::::::  ::::::::::: :::::::::::  ::::::::   ::::::::      :::     :::        :::::::::: :::::::::  
echo :+:    :+: :+:    :+:     :+:         :+:     :+:    :+: :+:    :+:   :+: :+:   :+:        :+:        :+:    :+: 
echo +:+    +:+ +:+    +:+     +:+         +:+     +:+        +:+         +:+   +:+  +:+        +:+        +:+    +:+ 
echo +#+    +:+ +#++:++#+      +#+         +#+     +#++:++#++ +#+        +#++:++#++: +#+        +#++:++#   +#++:++#:  
echo +#+    +#+ +#+            +#+         +#+            +#+ +#+        +#+     +#+ +#+        +#+        +#+    +#+ 
echo #+#    #+# #+#            #+#         #+#     #+#    #+# #+#    #+# #+#     #+# #+#        #+#        #+#    #+# 
echo  ########  ###            ###     ###########  ########   ########  ###     ### ########## ########## ###    ### 
echo.
echo Coping is strong with this one...
echo v3.0-pre1（汉化版）
echo.

del "!! README_EXTRACT ALL FILES TO GAME FOLDER !!.txt" 2>nul
echo.
echo ============================================
echo   OptiScaler 中文汉化版安装向导
echo   汉化版与上游完全兼容，开箱即用
echo ============================================
echo.
del "!! EXTRACT ALL FILES TO GAME FOLDER !!" 2>nul

setlocal enabledelayedexpansion

if exist OptiScaler.sln (
    echo 检测到 OptiScaler.sln 或 .git 文件！
    echo.
    echo 如果文件夹里有 .sln 或 .git 文件，恭喜你，这是源码目录。
    echo 请从 GitHub Releases 页面下载正式安装包。
    echo.
    echo 提示 - 使用 GitHub Releases 页面，或阅读文档 :^)
    echo.
    echo.
    echo 补充：如果你同时有 OptiScaler.dll 和 .sln 文件，请删除 .sln 文件，重新运行本脚本。
    echo.
    goto end
)

if not exist OptiScaler.dll (
    echo 未找到 OptiScaler "OptiScaler.dll" 文件！
    echo 很可能是文件夹权限问题，尝试以管理员身份运行本脚本。
    echo.
    echo 或者
    echo.
    echo 如果 "OptiScaler.dll" 存在，请手动重命名为支持的文件名（如 dxgi/winmm.dll）即可完成！
    echo 重命名后无需再次运行安装脚本。
    echo.
    echo.
    goto end
)

REM 检查旧版 0.9 之前的残留文件，以及已存在的 Opti 安装
set "OLD_FILES_FOUND=0"
set "OPTI_DLL_LIST="
if exist nvapi64.dll set "OLD_FILES_FOUND=1"
if exist nvngx.dll set "OLD_FILES_FOUND=1"
if exist OptiScaler.asi set "OLD_FILES_FOUND=1"
if exist "Remove OptiScaler.bat" set "OLD_FILES_FOUND=1"
if exist "Remove_OptiScaler.bat" set "OLD_FILES_FOUND=1"

for %%F in (dxgi.dll winmm.dll d3d12.dll dbghelp.dll version.dll wininet.dll winhttp.dll) do (
    if exist "%%F" (
        set "origname="
        for /f "tokens=*" %%P in ('powershell -NoProfile -Command "(Get-Item '%%F').VersionInfo.OriginalFilename"') do (
            set "origname=%%P"
        )
        if /i "!origname!"=="OptiScaler.dll" (
            set "OLD_FILES_FOUND=1"
            set "OPTI_DLL_LIST=!OPTI_DLL_LIST! %%F"
        )
    )
)

if "!OLD_FILES_FOUND!"=="1" (
    echo 警告：检测到可能的旧版 OptiScaler 文件！
    if exist nvapi64.dll echo   - nvapi64.dll
    if exist nvngx.dll echo   - nvngx.dll
    if exist OptiScaler.asi echo   - OptiScaler.asi
	if exist "Remove OptiScaler.bat" echo   - Remove OptiScaler.bat
    if exist "Remove_OptiScaler.bat" echo   - Remove_OptiScaler.bat
    for %%F in (!OPTI_DLL_LIST!) do echo   - %%F （原始文件名：OptiScaler.dll）
    echo.
    echo 这些文件可能与当前版本的 OptiScaler 冲突，建议删除。
    echo 建议删除。
    echo.
    echo 是否删除这些文件？
    echo.
	echo [1] 是
    echo [2] 否
    echo.
	set /p "USER_CHOICE=请选择 - "
    echo.
    if /i "!USER_CHOICE!"=="1" (
        if exist nvapi64.dll (
            del nvapi64.dll
            echo 已删除 nvapi64.dll
        )
        if exist nvngx.dll (
            del nvngx.dll
            echo 已删除 nvngx.dll
        )
        if exist OptiScaler.asi (
            del OptiScaler.asi
            echo 已删除 OptiScaler.asi
        )
		if exist "Remove OptiScaler.bat" (
            del "Remove OptiScaler.bat"
            echo 已删除 Remove OptiScaler.bat
        )
        if exist "Remove_OptiScaler.bat" (
            del "Remove_OptiScaler.bat"
            echo 已删除 Remove_OptiScaler.bat
        )
        for %%F in (!OPTI_DLL_LIST!) do (
            del "%%F"
            echo 已删除 %%F
        )
        echo 完成！
    ) else (
        echo 已跳过删除。注意这些文件可能引起问题。
    )
    echo.
)

REM 根据当前目录设置路径

set "optiScalerFile=.\OptiScaler.dll"
set setupSuccess=false

REM 检查是否存在 Engine 文件夹
if exist ".\Engine" (
    echo 检测到 Engine 文件夹。如果是虚幻引擎游戏，请将 OptiScaler 解压到 #CODENAME#\Binaries\Win64
	echo 不要解压到 Engine 文件夹！
    echo.
	echo 示例 - \Jedi Survivor\SwGame\Binaries\Win64, \Witchfire\Witchfire\Binaries\Win64
    echo.
    echo 是否继续安装到当前文件夹？
	echo. 
    echo [1] 是
    echo [2] 否
    echo.
	set /p continueChoice="请选择 - "
    set continueChoice=!continueChoice: =!

    if "!continueChoice!"=="1" (
        goto selectFilename
    )

    goto end
)

REM 提示用户选择 OptiScaler 文件名
:selectFilename
echo.
echo 请为 OptiScaler 选择文件名（默认 dxgi.dll，兼容性最好）：
echo （Vulkan 游戏请用 winmm.dll；XGP/微软商店 winmm/version.dll 可能更好）
echo.
echo  [1] dxgi.dll
echo  [2] winmm.dll
echo  [3] version.dll
echo  [4] dbghelp.dll
echo  [5] d3d12.dll
echo  [6] wininet.dll
echo  [7] winhttp.dll
echo  [8] OptiScaler.asi
echo.
set /p filenameChoice="请输入 1-8（回车使用默认值）: "

if "%filenameChoice%"=="" (
    set selectedFilename="dxgi.dll"
) else if "%filenameChoice%"=="1" (
    set selectedFilename="dxgi.dll"
) else if "%filenameChoice%"=="2" (
    set selectedFilename="winmm.dll"
) else if "%filenameChoice%"=="3" (
    set selectedFilename="version.dll"
) else if "%filenameChoice%"=="4" (
    set selectedFilename="dbghelp.dll"
) else if "%filenameChoice%"=="5" (
    set selectedFilename="d3d12.dll"
) else if "%filenameChoice%"=="6" (
    set selectedFilename="wininet.dll"
) else if "%filenameChoice%"=="7" (
    set selectedFilename="winhttp.dll"
) else if "%filenameChoice%"=="8" (
    set selectedFilename="OptiScaler.asi"
) else (
    echo 无效选项，请重新选择。
    echo.
    goto selectFilename
)

if exist %selectedFilename% (
    echo.
    echo 警告：%selectedFilename% 已存在于当前文件夹。
    echo.
	echo 是否覆盖 %selectedFilename%？
    echo.
    echo [1] 是
    echo [2] 否
    echo.
	set /p overwriteChoice="请选择 - "
    set overwriteChoice=!overwriteChoice: =!
    
    echo.
    if "!overwriteChoice!"=="1" (
        goto checkWine
    )

    goto selectFilename
)

REM Wine 不支持 PowerShell
:checkWine
reg query HKEY_CURRENT_USER\Software\Wine\DllOverrides >nul 2>&1
if %errorlevel%==0 (
    echo.
    echo 检测到 Wine，跳过伪装检查。
    echo 如需关闭伪装，可在配置中设置 Dxgi=false
    echo.
    pause
    goto completeSetup
) 

if exist %windir%\system32\nvapi64.dll (
    echo.
    echo 检测到 N 卡驱动文件。
    set isNvidia=true
) else (
    set isNvidia=false
)

REM 询问用户 GPU 类型
echo.
echo 你使用的是 N 卡还是 A 卡/Intel 核显？
echo.
echo [1] A 卡/Intel
echo [2] N 卡
echo.

:gpuPrompt
if "%isNvidia%"=="true" (
    set /p gpuChoice="请输入 1 或 2（检测到 N 卡）: "
) else (
    set /p gpuChoice="请输入 1 或 2（检测到 A 卡/Intel）: "
)

if "%gpuChoice%"=="1" goto gpuValid
if "%gpuChoice%"=="2" goto gpuValid
echo 无效输入，请输入 1 或 2。
echo.
goto gpuPrompt

:gpuValid

REM N 卡跳过伪装
if "%gpuChoice%"=="2" (
    goto completeSetup
)

REM 询问 DLSS 输入
echo.
echo 是否使用 DLSS 输入替换为 FSR/XeSS？（启用 N 卡伪装，DLSS-FG、Reflex-^>AL2 必需）
echo 如需更改，可编辑 OptiScaler.ini 设置 Dxgi=false 关闭伪装。
echo.
echo [1] 是
echo [2] 否
echo.
set /p enablingSpoofing="请输入 1 或 2（回车默认是）: "

set configFile=OptiScaler.ini
if "%enablingSpoofing%"=="2" (
    if not exist "%configFile%" (
        echo 未找到配置文件：%configFile%
        pause
    )

    powershell -Command "(Get-Content '%configFile%') -replace 'Dxgi=auto', 'Dxgi=false' | Set-Content '%configFile%'"
)

REM 决定是否运行 OptiPatcher
echo.
if "%gpuChoice%"=="1" (
    echo 检测到 A 卡/Intel，正在检查 OptiPatcher 兼容性。
    goto checkExistingOptiPatcher
)

:checkExistingOptiPatcher
set "foundOptiPatcher="
for %%F in (OptiScaler\plugins\*OptiPatcher*.asi) do (
    set "foundOptiPatcher=%%F"
)

if defined foundOptiPatcher (
    echo.
    echo 发现 OptiPatcher：!foundOptiPatcher!
    echo 如果现有版本工作正常，建议保留。
	echo 是否重新下载可能更新的版本？
    echo.
    echo [1] 是
    echo [2] 否
    echo.
	set /p optiRedownload="请选择 - "
        
    if /i "!optiRedownload!"=="1" (
        echo.
        echo 正在删除 !foundOptiPatcher!...
        del "!foundOptiPatcher!"
        goto checkOptiPatcher
    ) else (
        echo.
        echo 保留现有 OptiPatcher，跳过下载。
        goto completeSetup
    )
)

REM 未安装，继续下载
goto checkOptiPatcher

:checkOptiPatcher
REM 检查网络连接
echo.
echo 正在检查 OptiPatcher 兼容性...
echo 网络不通时最多等待 15 秒后自动跳过。

ping -n 1 -w 3000 github.com >nul 2>&1 || ping -n 1 -w 3000 ghfast.top >nul 2>&1
if %errorlevel% neq 0 (
    echo 无法连接 GitHub 与 CDN 加速通道，跳过 OptiPatcher 检查。
    goto completeSetup
)

set "OPTI_MATCH=NO"
for /f "usebackq tokens=*" %%A in (`powershell -Command "& { $rawUrl = 'https://raw.githubusercontent.com/optiscaler/OptiPatcher/main/OptiPatcher/dllmain.cpp'; try { $code = (Invoke-WebRequest -Uri $rawUrl -UseBasicParsing -TimeoutSec 15).Content } catch { try { $code = (Invoke-WebRequest -Uri 'https://cdn.jsdelivr.net/gh/optiscaler/OptiPatcher@main/OptiPatcher/dllmain.cpp' -UseBasicParsing -TimeoutSec 15).Content } catch { return 'ERR' } }; $supported = @(); $ueMatches = [Regex]::Matches($code, 'CHECK_UE\s*\(\s*([a-zA-Z0-9_]+)\s*\)'); foreach ($m in $ueMatches) { $base = $m.Groups[1].Value; $supported += ($base + '-win64-shipping.exe').ToLower(); $supported += ($base + '-wingdk-shipping.exe').ToLower(); }; $directMatches = [Regex]::Matches($code, 'exeName\s*==\s*[\x22\x27]([^\x22\x27]+)[\x22\x27]'); foreach ($m in $directMatches) { $supported += $m.Groups[1].Value.ToLower(); }; $localFiles = Get-ChildItem *.exe | Select-Object -ExpandProperty Name; foreach ($file in $localFiles) { if ($supported -contains $file.ToLower()) { Write-Output 'YES'; exit; } }; Write-Output 'NO'; }"`) do (
    set "OPTI_MATCH=%%A"
)

if "!OPTI_MATCH!"=="YES" (
    echo.
    echo 检测到 OptiPatcher 支持！
    echo 一个用于解锁 DLSS/DLSS-FG 输入的 Opti 插件，可避免伪装和性能开销。
    echo 更多信息见 OptiPatcher GitHub
    echo.
	echo 下载 OptiPatcher.asi？
    echo.
	echo [1] 是
    echo [2] 否
    echo.
	set /p downloadOptiPatcher="请选择 - "
    set downloadOptiPatcher=!downloadOptiPatcher: =!
    
    if "!downloadOptiPatcher!"=="1" (
        echo.
        echo 正在准备 plugins 文件夹...
        if not exist "OptiScaler\plugins" mkdir "OptiScaler\plugins"
        
        echo 正在下载 OptiPatcher...
        echo 下载超时限制 60 秒，失败自动跳过。
        echo.
        powershell -Command "$url='https://github.com/optiscaler/OptiPatcher/releases/download/rolling/OptiPatcher.asi'; try { Invoke-WebRequest -Uri $url -OutFile 'OptiScaler\plugins\OptiPatcher.asi' -TimeoutSec 60 } catch { try { Invoke-WebRequest -Uri ('https://ghfast.top/'+$url) -OutFile 'OptiScaler\plugins\OptiPatcher.asi' -TimeoutSec 60 } catch { try { Invoke-WebRequest -Uri ('https://gh-proxy.com/'+$url) -OutFile 'OptiScaler\plugins\OptiPatcher.asi' -TimeoutSec 60 } catch { try { Invoke-WebRequest -Uri ('https://ghproxy.net/'+$url) -OutFile 'OptiScaler\plugins\OptiPatcher.asi' -TimeoutSec 60 } catch {} } } }
        if errorlevel 1 goto completeSetup
        
        if exist "OptiScaler\plugins\OptiPatcher.asi" (
            echo OptiPatcher.asi 下载成功。
            echo 正在启用 OptiScaler.ini 中的 ASI 加载...
            if exist "%configFile%" (
                powershell -Command "(Get-Content '%configFile%') -replace 'LoadAsiPlugins=auto', 'LoadAsiPlugins=true' | Set-Content '%configFile%'"
                echo 已在 OptiScaler.ini 中启用 ASI 加载！
            ) else (
                echo 警告：未找到 OptiScaler.ini，无法启用 LoadAsiPlugins。
            )
        ) else (
            echo OptiPatcher.asi 下载失败。
        )
     timeout /t 3
    )
)
echo.

goto completeSetup

:completeSetup
REM 重命名 OptiScaler 文件
echo.
if "!overwriteChoice!"=="1" (
    echo 正在删除旧 %selectedFilename%...
    del /F %selectedFilename% 
)

echo 正在将 OptiScaler 文件重命名为 %selectedFilename%...
rename "%optiScalerFile%" %selectedFilename%
if errorlevel 1 (
    echo.
    echo 错误：重命名失败，很可能是文件夹权限问题。
    echo 请手动将 OptiScaler.dll 重命名为 %selectedFilename%！之后无需再次运行脚本。
    echo.
    goto end
)

goto create_uninstaller

:create_uninstaller_return

cls
echo  OptiScaler 安装成功！
echo.
echo   ___                 
echo  (_         '        
echo  /__  /)   /  () (/  
echo          _/      /    
echo.
echo 按 Insert 可在游戏内打开 OptiScaler 菜单
echo.
echo.

set setupSuccess=true

:end
pause

if "%setupSuccess%"=="true" (
    del "setup_linux.sh"
    del "%~nx0"
)

exit /b

:create_uninstaller
setlocal DisableDelayedExpansion

(
echo @echo off
echo setlocal EnableDelayedExpansion
echo cls
echo echo  ::::::::  :::::::::  ::::::::::: :::::::::::  ::::::::   ::::::::      :::     :::        :::::::::: :::::::::  
echo echo :+:    :+: :+:    :+:     :+:         :+:     :+:    :+: :+:    :+:   :+: :+:   :+:        :+:        :+:    :+: 
echo echo +:+    +:+ +:+    +:+     +:+         +:+     +:+        +:+         +:+   +:+  +:+        +:+        +:+    +:+ 
echo echo +#+    +:+ +#++:++#+      +#+         +#+     +#++:++#++ +#+        +#++:++#++: +#+        +#++:++#   +#++:++#:  
echo echo +#+    +#+ +#+            +#+         +#+            +#+ +#+        +#+     +#+ +#+        +#+        +#+    +#+ 
echo echo #+#    #+# #+#            #+#         #+#     #+#    #+# #+#    #+# #+#     #+# #+#        #+#        #+#    #+# 
echo echo  ########  ###            ###     ###########  ########   ########  ###     ### ########## ########## ###    ### 
echo echo.
echo echo Coping is strong with this one...
echo echo v3.0-pre1（汉化版）
echo echo.
echo echo REM 检查 OptiScaler 是否已安装
echo set "OLD_FILES_FOUND=0"
echo set "OPTI_DLL_LIST="
echo if exist OptiScaler.asi set "OLD_FILES_FOUND=1"

echo for %%%%F in ^(dxgi.dll winmm.dll d3d12.dll dbghelp.dll version.dll wininet.dll winhttp.dll^) do ^(
echo     if exist "%%%%F" ^(
echo         set "origname="
echo         for /f "tokens=*" %%%%P in ^('powershell -NoProfile -Command "(Get-Item '%%%%F').VersionInfo.OriginalFilename"'^) do ^(
echo             set "origname=%%%%P"
echo         ^)
echo         if /i "!origname!"=="OptiScaler.dll" ^(
echo             set "OLD_FILES_FOUND=1"
echo             set "OPTI_DLL_LIST=!OPTI_DLL_LIST! %%%%F"
echo         ^)
echo     ^)
echo ^)

echo if "!OLD_FILES_FOUND!"=="1" ^(
echo     echo 检测到已安装的 OptiScaler！
echo     if exist OptiScaler.asi echo   - OptiScaler.asi
echo     for %%%%F in ^(!OPTI_DLL_LIST!^) do echo   - %%%%F - 原始文件名：OptiScaler.dll
echo     echo.
echo ^)

echo echo 是否卸载 OptiScaler？
echo echo.
echo echo [1] 是
echo echo [2] 否
echo echo.
echo set /p removeChoice="请选择 - "
echo echo.

echo if "%%removeChoice%%"=="1" ^(
echo     del OptiScaler.log
echo     del OptiScaler.ini
echo     del OptiScaler.asi
echo     for %%%%F in ^(!OPTI_DLL_LIST!^) do ^(del "%%%%F"^)
echo     del /Q Licenses\*
echo     rd Licenses
echo     del /Q OptiScaler\D3D12_Optiscaler\*
echo     rd OptiScaler\D3D12_Optiscaler
echo     del /Q OptiScaler\Streamline\*
echo     rd OptiScaler\Streamline
echo     del /Q OptiScaler\streamline\*
echo     rd OptiScaler\streamline
echo     echo.
echo     echo 删除 OptiPatcher（如果存在）
echo     del /Q OptiScaler\plugins\*
echo     rd OptiScaler\plugins
echo     echo.
echo     del /Q OptiScaler\*
echo     rd OptiScaler
echo     echo.
echo     echo OptiScaler 已卸载！忽略关于缺失文件的警告。
echo     echo.
echo ^) else ^(
echo     echo.
echo     echo 操作已取消。
echo     echo.
echo ^)

echo.
echo pause
echo if "%%removeChoice%%"=="1" ^(
echo     del "%%~nx0"
echo ^)
) > "Remove_OptiScaler.bat"

endlocal
echo.
echo 卸载脚本已生成。
echo.

goto create_uninstaller_return
