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

# ⚠️ 对齐官方包结构：运行库必须位于根目录（OptiScaler 在 dll 同目录查找 libxess/ffx 等）
# post-build 把它们拷进 a\OptiScaler\ 子目录，官方发布时整理到根目录，我们同样处理
$libSub = Join-Path $OutDir "OptiScaler"
if (Test-Path $libSub) {
    # 1. 子目录里的运行库全部移到根目录
    Get-ChildItem $libSub -File | Move-Item -Destination $OutDir -Force
    # 2. D3D12 子目录移到根目录（官方名 D3D12_Optiscaler）
    foreach ($d in @("D3D12_OptiScaler", "D3D12_Optiscaler")) {
        $srcD = Join-Path $libSub $d
        if (Test-Path $srcD) {
            $dstD = Join-Path $OutDir "D3D12_Optiscaler"
            if (-not (Test-Path $dstD)) { New-Item $dstD -ItemType Directory -Force | Out-Null }
            Get-ChildItem $srcD -File | Move-Item -Destination $dstD -Force
            Remove-Item $srcD -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
    # 3. 清理空的 OptiScaler/ 目录
    if (-not (Get-ChildItem $libSub -Force -ErrorAction SilentlyContinue)) {
        Remove-Item $libSub -Force
    }
    Write-Host "已对齐官方结构：运行库移至根目录"
}

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

# OptiPatcher 插件（从官方最新版本化 release 下载，放入 plugins/，供 AMD/Intel 解锁 DLSS 输入）
Write-Host "正在获取 OptiPatcher 最新 release..."
$pluginsDir = Join-Path $OutDir "plugins"
New-Item $pluginsDir -ItemType Directory -Force | Out-Null
$opDownloaded = $false
$opName = "OptiPatcher.asi"
$opUrl = "https://github.com/optiscaler/OptiPatcher/releases/download/rolling/OptiPatcher.asi"
# 1. 查 GitHub API，找最新版本化 release（跳过 rolling）
try {
    $rels = Invoke-RestMethod -Uri "https://api.github.com/repos/optiscaler/OptiPatcher/releases?per_page=5" -Headers @{ 'User-Agent' = 'OptiScaler-CHS-Build' } -TimeoutSec 30
    $rel = $rels | Where-Object { $_.tag_name -ne 'rolling' -and -not $_.prerelease } | Select-Object -First 1
    if ($rel) {
        $asset = $rel.assets | Where-Object { $_.name -like '*.asi' } | Select-Object -First 1
        if ($asset) {
            $opName = $asset.name
            $opUrl = $asset.browser_download_url
            Write-Host "  最新版本: $($rel.tag_name) → $opName"
        }
    }
} catch {
    Write-Host "  API 查询失败，回退 rolling 源: $_"
}
# 2. 下载（官方 + CDN 回退）
$opDst = Join-Path $pluginsDir $opName
foreach ($u in @($opUrl, "https://ghfast.top/$opUrl", "https://gh-proxy.com/$opUrl")) {
    try {
        Invoke-WebRequest -Uri $u -OutFile $opDst -TimeoutSec 60 -UseBasicParsing
        if ((Get-Item $opDst).Length -gt 1000) {
            $opDownloaded = $true
            Write-Host "✅ 已下载 $opName ($((Get-Item $opDst).Length) 字节)"
            break
        }
        Remove-Item $opDst -Force -ErrorAction SilentlyContinue
    } catch {
        Write-Host "  下载失败（${u}），尝试下一源"
        Remove-Item $opDst -Force -ErrorAction SilentlyContinue
    }
}
if (-not $opDownloaded) {
    Write-Host "警告: OptiPatcher 下载失败（网络受限？），产物将不含 OptiPatcher，不影响核心功能"
}

# 汉化版 OptiScaler.ini 兜底强制覆盖（最后一步，防 checkout 竞态/上游覆盖，确保产物一定是中文注释版）
$iniSrc = Join-Path $SourceDir "OptiScaler.ini"
if (Test-Path $iniSrc) {
    Copy-Item $iniSrc (Join-Path $OutDir "OptiScaler.ini") -Force
    Write-Host "已强制覆盖汉化版 OptiScaler.ini"
} else {
    Write-Host "警告: 仓库根目录无 OptiScaler.ini，产物将保留 MSBuild 拷贝的版本"
}

Write-Host "产物目录: $OutDir"
Get-ChildItem $OutDir -Recurse -File | Select-Object FullName, Length | Format-Table -AutoSize
