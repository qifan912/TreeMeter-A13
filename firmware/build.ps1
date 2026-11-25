param([string]$KeilRoot = 'D:\KeilSTM32', [switch]$CompileOnly)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $compilerDir = Join-Path $KeilRoot 'ARM\ARMCLANG\bin'
    $deviceInclude = Join-Path $KeilRoot 'Keil\STM32F1xx_DFP\2.4.1\Device\Include'
    $cmsisInclude = Join-Path $KeilRoot 'ARM\CMSIS\6.3.0\CMSIS\Core\Include'
    New-Item -ItemType Directory -Force Objects,Listings | Out-Null
    if (!$CompileOnly) {
        Remove-Item -LiteralPath Objects\GTI-test.hex,Objects\GTI-test.axf -Force -ErrorAction SilentlyContinue
    }
    $sources = @('mainD.c','display_text.c','oled.c','oled_port.c','mpu_bus.c','mpu6050.c','laser_protocol.c','laser_uart.c','laser_control.c','RTE\Device\STM32F103C8\system_stm32f10x.c')
    $objectFiles = @()
    foreach ($source in $sources) {
        $objectFile = 'Objects\' + [IO.Path]::GetFileNameWithoutExtension($source) + '.o'
        & (Join-Path $compilerDir 'armclang.exe') --target=arm-arm-none-eabi -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c99 -O0 -g -ffunction-sections -fshort-enums -fshort-wchar -DSTM32F10X_MD -D_RTE_ -I. -IRTE\_Target_1 -IRTE\Device\STM32F103C8 "-I$deviceInclude" "-I$cmsisInclude" -c $source -o $objectFile
        if ($LASTEXITCODE -ne 0) { throw "Compile failed: $source" }
        $objectFiles += $objectFile
    }
    if ($CompileOnly) { Write-Output 'C COMPILE PASSED - linking not performed'; return }
    & (Join-Path $compilerDir 'armasm.exe') --cpu Cortex-M3 --debug RTE\Device\STM32F103C8\startup_stm32f10x_md.s -o Objects\startup_stm32f10x_md.o
    if ($LASTEXITCODE -ne 0) { throw 'Assembly failed' }
    $objectFiles += 'Objects\startup_stm32f10x_md.o'
    & (Join-Path $compilerDir 'armlink.exe') --cpu Cortex-M3 --strict --scatter Objects\GTI-test.sct --summary_stderr --info summarysizes --map --list Listings\GTI-test.map @objectFiles -o Objects\GTI-test.axf
    if ($LASTEXITCODE -ne 0) { throw 'Link failed' }
    & (Join-Path $compilerDir 'fromelf.exe') --i32combined --output Objects\GTI-test.hex Objects\GTI-test.axf
    if ($LASTEXITCODE -ne 0) { throw 'HEX generation failed' }
    Write-Output 'BUILD PASSED - Objects\GTI-test.hex'
} finally { Pop-Location }
