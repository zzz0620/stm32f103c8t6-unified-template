param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'
$firmware = Join-Path $ProjectRoot 'legacy\nun_dx_original'
$mdk = Join-Path $firmware 'MDK-ARM'
$targetProject = Join-Path $mdk 'stm32f103_template.uvprojx'
if (-not (Test-Path -LiteralPath $targetProject)) {
    throw "Keil project seed not found: $targetProject"
}

[xml]$xml = Get-Content -LiteralPath $targetProject -Raw
$target = $xml.Project.Targets.Target
$target.TargetName = 'stm32f103_template'
$target.TargetOption.TargetCommonOption.OutputName = 'stm32f103_template'
$target.TargetOption.TargetCommonOption.OutputDirectory = '.\stm32f103_template\'
$target.TargetOption.TargetCommonOption.ListingPath = '.\stm32f103_template\'

$includeDirs = @(
    '../Core/Inc',
    '../app/dart_board',
    '../components/template',
    '../components/loadcell',
    '../components/nun_dx/common',
    '../components/nun_dx/control',
    '../components/nun_dx/debug',
    '../components/nun_dx/device',
    '../components/nun_dx/driver',
    '../components/nun_dx/state_machine',
    '../Drivers/STM32F1xx_HAL_Driver/Inc',
    '../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy',
    '../Drivers/CMSIS/Device/ST/STM32F1xx/Include',
    '../Drivers/CMSIS/Include'
) -join ';'

$target.TargetOption.TargetArmAds.Cads.VariousControls.IncludePath = $includeDirs
$target.TargetOption.TargetArmAds.Aads.VariousControls.IncludePath = $includeDirs

function Add-KeilGroup {
    param(
        [string]$Name,
        [string[]]$Paths
    )

    $groupNode = $xml.CreateElement('Group')
    $nameNode = $xml.CreateElement('GroupName')
    $nameNode.InnerText = $Name
    [void]$groupNode.AppendChild($nameNode)
    $filesNode = $xml.CreateElement('Files')
    [void]$groupNode.AppendChild($filesNode)

    foreach ($path in $Paths) {
        $fileNode = $xml.CreateElement('File')
        $fileNameNode = $xml.CreateElement('FileName')
        $fileNameNode.InnerText = Split-Path $path -Leaf
        [void]$fileNode.AppendChild($fileNameNode)
        $fileTypeNode = $xml.CreateElement('FileType')
        $fileTypeNode.InnerText = if ([IO.Path]::GetExtension($path) -in @('.s', '.S')) { '2' } else { '1' }
        [void]$fileNode.AppendChild($fileTypeNode)
        $filePathNode = $xml.CreateElement('FilePath')
        $filePathNode.InnerText = $path
        [void]$fileNode.AppendChild($filePathNode)
        [void]$filesNode.AppendChild($fileNode)
    }

    [void]$target.Groups.AppendChild($groupNode)
}

# Rebuild the group list so no stale RTOS or old source-tree entry survives.
# The empty ::CMSIS group is supplied by the Keil device pack and is retained.
foreach ($group in @($target.Groups.Group)) {
    if ($group.GroupName -ne '::CMSIS') {
        [void]$target.Groups.RemoveChild($group)
    }
}

Add-KeilGroup 'Application/Startup' @('startup_stm32f103xb.s')

Add-KeilGroup 'Application/Core' @(
    '..\Core\Src\main.c',
    '..\Core\Src\gpio.c',
    '..\Core\Src\dma.c',
    '..\Core\Src\i2c.c',
    '..\Core\Src\usart.c',
    '..\Core\Src\tim.c',
    '..\Core\Src\can.c',
    '..\Core\Src\stm32_template_port.c',
    '..\Core\Src\stm32f1xx_it.c',
    '..\Core\Src\stm32f1xx_hal_msp.c',
    '..\Core\Src\stm32f1xx_hal_timebase_tim.c',
    '..\Core\Src\system_stm32f1xx.c'
)

Add-KeilGroup 'Components/Template' @('..\components\template\stm32_template.c')

Add-KeilGroup 'Components/LoadCell' @(
    '..\components\loadcell\loadcell.c',
    '..\components\loadcell\loadcell_service.c'
)

Add-KeilGroup 'Application/DartBoard' @(
    '..\app\dart_board\font.c',
    '..\app\dart_board\oled.c',
    '..\app\dart_board\oled_ui.c'
)

Add-KeilGroup 'Components/NUN_DX/Common' @(
    '..\components\nun_dx\common\DX_common_fifo.c',
    '..\components\nun_dx\common\DX_common_font.c',
    '..\components\nun_dx\common\DX_common_interrupt.c',
    '..\components\nun_dx\control\DX_control_pid.c',
    '..\components\nun_dx\debug\DX_debug.c',
    '..\components\nun_dx\state_machine\state_machine.c'
)

$driverPaths = Get-ChildItem -LiteralPath (Join-Path $firmware 'components\nun_dx\driver') -Filter '*.c' |
    Sort-Object Name |
    ForEach-Object { '..\components\nun_dx\driver\' + $_.Name }
Add-KeilGroup 'Components/NUN_DX/Drivers' $driverPaths

$devicePaths = Get-ChildItem -LiteralPath (Join-Path $firmware 'components\nun_dx\device') -Filter '*.c' |
    Sort-Object Name |
    ForEach-Object { '..\components\nun_dx\device\' + $_.Name }
Add-KeilGroup 'Components/NUN_DX/Devices' $devicePaths

Add-KeilGroup 'Drivers/STM32F1xx_HAL' @(
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_adc.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_adc_ex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_can.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_cortex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_dma.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_exti.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_flash.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_flash_ex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_gpio.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_gpio_ex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_i2c.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_pwr.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_rcc.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_rcc_ex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_spi.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_tim.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_tim_ex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_uart.c'
)

# The CMSIS pack metadata is part of the project XML. Keep its target name in
# sync with the single project name so Keil does not recreate an old RTE target.
foreach ($targetInfo in $xml.SelectNodes('//targetInfo')) {
    $targetInfo.SetAttribute('name', 'stm32f103_template')
}

$settings = [System.Xml.XmlWriterSettings]::new()
$settings.Indent = $true
$settings.IndentChars = '  '
$settings.Encoding = [System.Text.UTF8Encoding]::new($false)
$settings.NewLineChars = "`r`n"
$settings.NewLineHandling = 'Replace'
$writer = [System.Xml.XmlWriter]::Create($targetProject, $settings)
try {
    $xml.Save($writer)
}
finally {
    $writer.Dispose()
}

Write-Output $targetProject
