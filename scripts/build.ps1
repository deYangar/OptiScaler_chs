# OptiScaler CHS 构建脚本（GitHub Actions windows-latest）
param(
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutDir
)

$ErrorActionPreference = 'Stop'

# 1. 找 MSBuild
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { throw "vswhere not found" }
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
if (-not $msbuild) { throw "MSBuild not found" }
Write-Host "MSBuild: $msbuild"

# 2. 编译
Push-Location $SourceDir
try {
    & $msbuild OptiScaler.sln -p:Configuration=Release -p:Platform=x64 -m -v:m
    if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE" }
} finally {
    Pop-Location
}

# 3. 收集产物
$prod = Join-Path $SourceDir "x64\Release\a"
if (-not (Test-Path $prod)) { throw "Build output not found: $prod" }

New-Item $OutDir -ItemType Directory -Force | Out-Null

Copy-Item (Join-Path $prod "OptiScaler.dll") (Join-Path $OutDir "dxgi.dll") -Force
Copy-Item (Join-Path $prod "OptiScaler.dll") (Join-Path $OutDir "nvngx.dll") -Force
Copy-Item (Join-Path $prod "OptiScaler.ini") $OutDir -Force -ErrorAction SilentlyContinue

foreach ($sub in @("OptiScaler", "Licenses")) {
    $src = Join-Path $prod $sub
    if (Test-Path $src) {
        Copy-Item $src $OutDir -Recurse -Force
    }
}

# 字体
$fontSrc = Join-Path $SourceDir "font"
if (Test-Path $fontSrc) {
    Copy-Item $fontSrc $OutDir -Recurse -Force
}

Write-Host "产物目录: $OutDir"
Get-ChildItem $OutDir -Recurse -File | Select-Object FullName, Length | Format-Table -AutoSize
