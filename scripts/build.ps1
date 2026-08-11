# OptiScaler CHS 构建脚本（GitHub Actions windows-latest）
param(
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutDir,
    [string]$MsbuildPath = ""
)

$ErrorActionPreference = 'Stop'

# 1. 找 MSBuild（可选参数优先，用于本地验证；CI 走 vswhere）
if ($MsbuildPath -and (Test-Path $MsbuildPath)) {
    $msbuild = $MsbuildPath
    Write-Host "MSBuild(指定): $msbuild"
} else {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { throw "vswhere not found" }
    $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
    if (-not $msbuild) { throw "MSBuild not found" }
    Write-Host "MSBuild: $msbuild"
}

# 2. 编译
Push-Location $SourceDir
try {
    & $msbuild OptiScaler.sln -p:Configuration=Release -p:Platform=x64 -m -v:m
    if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE" }
} finally {
    Pop-Location
}

# 3. 收集产物（完全对齐上游：a 目录全部内容直接打包）
$prod = Join-Path $SourceDir "x64\Release\a"
if (-not (Test-Path $prod)) { throw "Build output not found: $prod" }

New-Item $OutDir -ItemType Directory -Force | Out-Null

# a 目录 = 编译产物 + post-build 自动拷贝（setup 脚本、运行库、Licenses、提示文件），全量打包
Copy-Item (Join-Path $prod "*") $OutDir -Recurse -Force

# 中文字体（汉化版特色）
$fontSrc = Join-Path $SourceDir "font"
if (Test-Path $fontSrc) {
    Copy-Item $fontSrc $OutDir -Recurse -Force
}

# 第三方随包组件（与上游 release 包一致：dlssg-to-fsr3 / fakenvapi）
$binSrc = Join-Path $SourceDir "binaries"
if (Test-Path $binSrc) {
    Copy-Item (Join-Path $binSrc "*") $OutDir -Force
    Write-Host "已包含: dlssg_to_fsr3 / fakenvapi"
} else {
    Write-Host "警告: binaries/ 不存在，跳过 dlssg/fakenvapi"
}

Write-Host "产物目录: $OutDir"
Get-ChildItem $OutDir -Recurse -File | Select-Object FullName, Length | Format-Table -AutoSize
