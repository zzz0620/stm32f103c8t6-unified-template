param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'
$buildDirectory = Join-Path $ProjectRoot 'build'
$toolchain = Join-Path $ProjectRoot 'cmake\arm-none-eabi-gcc.cmake'

foreach ($command in @('cmake', 'arm-none-eabi-gcc')) {
    if (-not (Get-Command $command -ErrorAction SilentlyContinue)) {
        throw "Required command not found in PATH: $command"
    }
}

$configure = @(
    '-S', $ProjectRoot,
    '-B', $buildDirectory,
    "-DCMAKE_BUILD_TYPE=$Configuration"
)

if (-not (Test-Path -LiteralPath (Join-Path $buildDirectory 'CMakeCache.txt'))) {
    $configure += "-DCMAKE_TOOLCHAIN_FILE=$toolchain"
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        $configure += @('-G', 'Ninja')
    }
    elseif ($make = Get-Command mingw32-make -ErrorAction SilentlyContinue) {
        $configure += @(
            '-G', 'MinGW Makefiles',
            "-DCMAKE_MAKE_PROGRAM=$($make.Source -replace '\\', '/')"
        )
    }
}

& cmake @configure
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE"
}

& cmake --build $buildDirectory --clean-first --parallel
if ($LASTEXITCODE -ne 0) {
    throw "GCC build failed with exit code $LASTEXITCODE"
}

Write-Output "GCC build passed: $buildDirectory"

