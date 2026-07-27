param(
    [ValidateSet("arm64-v8a", "armeabi-v7a", "all")]
    [string]$Abi = "arm64-v8a",

    [string]$NdkVersion = "28.2.13676358",

    [string]$AndroidHome = $env:ANDROID_HOME
)

$ErrorActionPreference = "Stop"

if (-not $AndroidHome) {
    throw "ANDROID_HOME is not set. Pass -AndroidHome or set the environment variable."
}

$ndkRoot = Join-Path $AndroidHome "ndk/$NdkVersion"
$toolchain = Join-Path $ndkRoot "build/cmake/android.toolchain.cmake"

if (-not (Test-Path $toolchain)) {
    throw "NDK toolchain not found: $toolchain"
}

$projectRoot = Split-Path -Parent $PSScriptRoot

function Build-Abi([string]$TargetAbi) {
    $buildDir = Join-Path $projectRoot "build-$TargetAbi"

    Write-Host "==> Configuring $TargetAbi"
    & cmake -S $projectRoot -B $buildDir `
        -G Ninja `
        -DCMAKE_TOOLCHAIN_FILE="$toolchain" `
        -DANDROID_ABI="$TargetAbi" `
        -DANDROID_PLATFORM=android-28 `
        -DANDROID_STL=c++_shared `
        -DMOD_ID=levivision `
        -DMOD_NAME="LeviVision" `
        -DMOD_AUTHOR="Say" `
        -DMOD_VERSION=1.0.0 `
        -DMOD_LIBRARY_NAME=levivision `
        -DMOD_MINECRAFT_VERSIONS='["1.21.100*"]' `
        -DMOD_ICON="assets/icon.png"

    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed for $TargetAbi" }

    Write-Host "==> Building + packaging $TargetAbi"
    & cmake --build $buildDir --target levi_package
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed for $TargetAbi" }

    Get-ChildItem -Path $buildDir -Filter "*.levipack" | ForEach-Object {
        Write-Host "Packaged: $($_.FullName)"
    }
}

if ($Abi -eq "all") {
    Build-Abi "arm64-v8a"
    Build-Abi "armeabi-v7a"
} else {
    Build-Abi $Abi
}
