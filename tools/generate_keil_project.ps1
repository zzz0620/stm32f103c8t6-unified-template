param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

$firmware = Join-Path $ProjectRoot 'firmware'
$mdk = Join-Path $firmware 'MDK-ARM'
$sourceProject = Join-Path $mdk 'dart_loadcell.uvprojx'
$targetProject = Join-Path $mdk 'stm32f103_template.uvprojx'

if (-not (Test-Path -LiteralPath $sourceProject)) {
    throw "Base Keil project not found: $sourceProject"
}

[xml]$xml = Get-Content -LiteralPath $sourceProject -Raw
$target = $xml.Project.Targets.Target
$target.TargetName = 'stm32f103_template'
$target.TargetOption.TargetCommonOption.OutputName = 'stm32f103_template'
$target.TargetOption.TargetCommonOption.OutputDirectory = '.\stm32f103_template\'
$target.TargetOption.TargetCommonOption.ListingPath = '.\stm32f103_template\'

$includeDirs = @(
    '../Core/Inc',
    '../Object',
    '../Components/Template',
    '../Components/NUN_DX/common',
    '../Components/NUN_DX/control',
    '../Components/NUN_DX/debug',
    '../Components/NUN_DX/device',
    '../Components/NUN_DX/driver',
    '../Components/NUN_DX/state_machine',
    '../Components/NUN_DX/zdt',
    '../Drivers/STM32F1xx_HAL_Driver/Inc',
    '../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy',
    '../Middlewares/Third_Party/FreeRTOS/Source/include',
    '../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS',
    '../Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3',
    '../Drivers/CMSIS/Device/ST/STM32F1xx/Include',
    '../Drivers/CMSIS/Include'
) -join ';'

$target.TargetOption.TargetArmAds.Cads.VariousControls.IncludePath = $includeDirs

function Add-KeilGroup {
    param(
        [string]$Name,
        [string[]]$Paths
    )

    $old = @($target.Groups.Group | Where-Object { $_.GroupName -eq $Name })
    foreach ($group in $old) {
        [void]$target.Groups.RemoveChild($group)
    }

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
        $fileTypeNode.InnerText = '1'
        [void]$fileNode.AppendChild($fileTypeNode)
        $filePathNode = $xml.CreateElement('FilePath')
        $filePathNode.InnerText = $path
        [void]$fileNode.AppendChild($filePathNode)
        [void]$filesNode.AppendChild($fileNode)
    }

    [void]$target.Groups.AppendChild($groupNode)
}

Add-KeilGroup 'Application/User/Template' @(
    '..\Core\Src\stm32_template_port.c',
    '..\Components\Template\stm32_template.c'
)

Add-KeilGroup 'Components/NUN_DX/Common' @(
    '..\Components\NUN_DX\common\DX_common_fifo.c',
    '..\Components\NUN_DX\common\DX_common_font.c',
    '..\Components\NUN_DX\common\DX_common_interrupt.c',
    '..\Components\NUN_DX\control\DX_control_pid.c',
    '..\Components\NUN_DX\debug\DX_debug.c',
    '..\Components\NUN_DX\state_machine\State_Machine.c'
)

$driverPaths = Get-ChildItem -LiteralPath (Join-Path $firmware 'Components\NUN_DX\driver') -Filter '*.c' |
    Sort-Object Name |
    ForEach-Object { '..\Components\NUN_DX\driver\' + $_.Name }
Add-KeilGroup 'Components/NUN_DX/Drivers' $driverPaths

$devicePaths = Get-ChildItem -LiteralPath (Join-Path $firmware 'Components\NUN_DX\device') -Filter '*.c' |
    Sort-Object Name |
    ForEach-Object { '..\Components\NUN_DX\device\' + $_.Name }
Add-KeilGroup 'Components/NUN_DX/Devices' $devicePaths

Add-KeilGroup 'Drivers/HAL/Optional' @(
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_adc.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_adc_ex.c',
    '..\Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_spi.c'
)

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

