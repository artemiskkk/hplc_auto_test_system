param(
    [Parameter(Mandatory = $true)]
    [string]$BuildExeDir,

    [Parameter(Mandatory = $true)]
    [string]$InputExe,

    [Parameter(Mandatory = $true)]
    [string]$OutputExe,

    [Parameter(Mandatory = $true)]
    [string]$EvbFile
)

$ErrorActionPreference = "Stop"

function Escape-XmlText {
    param(
        [AllowNull()]
        [string]$Text
    )

    if ($null -eq $Text) {
        return ""
    }

    return [System.Security.SecurityElement]::Escape($Text)
}

function Append-Line {
    param(
        [Parameter(Mandatory = $true)]
        [System.Text.StringBuilder]$Builder,

        [Parameter(Mandatory = $true)]
        [int]$Indent,

        [Parameter(Mandatory = $true)]
        [string]$Text
    )

    [void]$Builder.Append((" " * $Indent))
    [void]$Builder.AppendLine($Text)
}

function Get-SortedChildren {
    param(
        [Parameter(Mandatory = $true)]
        [string]$DirPath
    )

    return Get-ChildItem -LiteralPath $DirPath -Force |
        Sort-Object @{ Expression = { if ($_.PSIsContainer) { 0 } else { 1 } } }, Name
}

function Write-FileNode {
    param(
        [Parameter(Mandatory = $true)]
        [System.Text.StringBuilder]$Builder,

        [Parameter(Mandatory = $true)]
        [System.IO.FileInfo]$Item,

        [Parameter(Mandatory = $true)]
        [int]$Indent
    )

    $fullPath = [System.IO.Path]::GetFullPath($Item.FullName)

    Append-Line $Builder $Indent "<File>"
    Append-Line $Builder ($Indent + 2) "<Type>2</Type>"
    Append-Line $Builder ($Indent + 2) "<Name>$(Escape-XmlText $Item.Name)</Name>"
    Append-Line $Builder ($Indent + 2) "<File>$(Escape-XmlText $fullPath)</File>"
    Append-Line $Builder ($Indent + 2) "<ActiveX>False</ActiveX>"
    Append-Line $Builder ($Indent + 2) "<ActiveXInstall>False</ActiveXInstall>"
    Append-Line $Builder ($Indent + 2) "<Action>0</Action>"
    Append-Line $Builder ($Indent + 2) "<OverwriteDateTime>False</OverwriteDateTime>"
    Append-Line $Builder ($Indent + 2) "<OverwriteAttributes>False</OverwriteAttributes>"
    Append-Line $Builder ($Indent + 2) "<PassCommandLine>False</PassCommandLine>"
    Append-Line $Builder ($Indent + 2) "<HideFromDialogs>0</HideFromDialogs>"
    Append-Line $Builder $Indent "</File>"
}

function Write-DirectoryNode {
    param(
        [Parameter(Mandatory = $true)]
        [System.Text.StringBuilder]$Builder,

        [Parameter(Mandatory = $true)]
        [System.IO.DirectoryInfo]$Item,

        [Parameter(Mandatory = $true)]
        [int]$Indent
    )

    Append-Line $Builder $Indent "<File>"
    Append-Line $Builder ($Indent + 2) "<Type>3</Type>"
    Append-Line $Builder ($Indent + 2) "<Name>$(Escape-XmlText $Item.Name)</Name>"
    Append-Line $Builder ($Indent + 2) "<Action>0</Action>"
    Append-Line $Builder ($Indent + 2) "<OverwriteDateTime>False</OverwriteDateTime>"
    Append-Line $Builder ($Indent + 2) "<OverwriteAttributes>False</OverwriteAttributes>"
    Append-Line $Builder ($Indent + 2) "<HideFromDialogs>0</HideFromDialogs>"
    Append-Line $Builder ($Indent + 2) "<Files>"

    foreach ($child in Get-SortedChildren -DirPath $Item.FullName) {
        if ($child.PSIsContainer) {
            Write-DirectoryNode -Builder $Builder -Item $child -Indent ($Indent + 4)
        } else {
            Write-FileNode -Builder $Builder -Item $child -Indent ($Indent + 4)
        }
    }

    Append-Line $Builder ($Indent + 2) "</Files>"
    Append-Line $Builder $Indent "</File>"
}

$BuildExeDir = [System.IO.Path]::GetFullPath($BuildExeDir)
$InputExe = [System.IO.Path]::GetFullPath($InputExe)
$OutputExe = [System.IO.Path]::GetFullPath($OutputExe)
$EvbFile = [System.IO.Path]::GetFullPath($EvbFile)

if (-not (Test-Path -LiteralPath $BuildExeDir -PathType Container)) {
    throw "Build EXE directory not found: $BuildExeDir"
}

if (-not (Test-Path -LiteralPath $InputExe -PathType Leaf)) {
    throw "Input EXE not found: $InputExe"
}

$parentDir = Split-Path -Parent $EvbFile
if ($parentDir -and -not (Test-Path -LiteralPath $parentDir -PathType Container)) {
    New-Item -ItemType Directory -Path $parentDir -Force | Out-Null
}

$builder = New-Object System.Text.StringBuilder

Append-Line $builder 0 '<?xml version="1.0" encoding="windows-1252"?>'
Append-Line $builder 0 '<>'
Append-Line $builder 2 "<InputFile>$(Escape-XmlText $InputExe)</InputFile>"
Append-Line $builder 2 "<OutputFile>$(Escape-XmlText $OutputExe)</OutputFile>"
Append-Line $builder 2 "<Files>"
Append-Line $builder 4 "<Enabled>True</Enabled>"
Append-Line $builder 4 "<DeleteExtractedOnExit>False</DeleteExtractedOnExit>"
Append-Line $builder 4 "<CompressFiles>True</CompressFiles>"
Append-Line $builder 4 "<Files>"
Append-Line $builder 6 "<File>"
Append-Line $builder 8 "<Type>3</Type>"
Append-Line $builder 8 "<Name>%DEFAULT FOLDER%</Name>"
Append-Line $builder 8 "<Action>0</Action>"
Append-Line $builder 8 "<OverwriteDateTime>False</OverwriteDateTime>"
Append-Line $builder 8 "<OverwriteAttributes>False</OverwriteAttributes>"
Append-Line $builder 8 "<HideFromDialogs>0</HideFromDialogs>"
Append-Line $builder 8 "<Files>"

foreach ($child in Get-SortedChildren -DirPath $BuildExeDir) {
    if ($child.PSIsContainer) {
        Write-DirectoryNode -Builder $builder -Item $child -Indent 10
    } else {
        Write-FileNode -Builder $builder -Item $child -Indent 10
    }
}

Append-Line $builder 8 "</Files>"
Append-Line $builder 6 "</File>"
Append-Line $builder 4 "</Files>"
Append-Line $builder 2 "</Files>"
Append-Line $builder 2 "<Registries>"
Append-Line $builder 4 "<Enabled>False</Enabled>"
Append-Line $builder 4 "<Registries>"
Append-Line $builder 6 "<Registry>"
Append-Line $builder 8 "<Type>1</Type>"
Append-Line $builder 8 "<Virtual>True</Virtual>"
Append-Line $builder 8 "<Name>Classes</Name>"
Append-Line $builder 8 "<ValueType>0</ValueType>"
Append-Line $builder 8 "<Value/>"
Append-Line $builder 8 "<Registries/>"
Append-Line $builder 6 "</Registry>"
Append-Line $builder 6 "<Registry>"
Append-Line $builder 8 "<Type>1</Type>"
Append-Line $builder 8 "<Virtual>True</Virtual>"
Append-Line $builder 8 "<Name>User</Name>"
Append-Line $builder 8 "<ValueType>0</ValueType>"
Append-Line $builder 8 "<Value/>"
Append-Line $builder 8 "<Registries/>"
Append-Line $builder 6 "</Registry>"
Append-Line $builder 6 "<Registry>"
Append-Line $builder 8 "<Type>1</Type>"
Append-Line $builder 8 "<Virtual>True</Virtual>"
Append-Line $builder 8 "<Name>Machine</Name>"
Append-Line $builder 8 "<ValueType>0</ValueType>"
Append-Line $builder 8 "<Value/>"
Append-Line $builder 8 "<Registries/>"
Append-Line $builder 6 "</Registry>"
Append-Line $builder 6 "<Registry>"
Append-Line $builder 8 "<Type>1</Type>"
Append-Line $builder 8 "<Virtual>True</Virtual>"
Append-Line $builder 8 "<Name>Users</Name>"
Append-Line $builder 8 "<ValueType>0</ValueType>"
Append-Line $builder 8 "<Value/>"
Append-Line $builder 8 "<Registries/>"
Append-Line $builder 6 "</Registry>"
Append-Line $builder 6 "<Registry>"
Append-Line $builder 8 "<Type>1</Type>"
Append-Line $builder 8 "<Virtual>True</Virtual>"
Append-Line $builder 8 "<Name>Config</Name>"
Append-Line $builder 8 "<ValueType>0</ValueType>"
Append-Line $builder 8 "<Value/>"
Append-Line $builder 8 "<Registries/>"
Append-Line $builder 6 "</Registry>"
Append-Line $builder 4 "</Registries>"
Append-Line $builder 2 "</Registries>"
Append-Line $builder 2 "<Packaging>"
Append-Line $builder 4 "<Enabled>False</Enabled>"
Append-Line $builder 2 "</Packaging>"
Append-Line $builder 2 "<Options>"
Append-Line $builder 4 "<ShareVirtualSystem>False</ShareVirtualSystem>"
Append-Line $builder 4 "<MapExecutableWithTemporaryFile>True</MapExecutableWithTemporaryFile>"
Append-Line $builder 4 "<TemporaryFileMask/>"
Append-Line $builder 4 "<AllowRunningOfVirtualExeFiles>True</AllowRunningOfVirtualExeFiles>"
Append-Line $builder 4 "<ProcessesOfAnyPlatforms>False</ProcessesOfAnyPlatforms>"
Append-Line $builder 2 "</Options>"
Append-Line $builder 2 "<Storage>"
Append-Line $builder 4 "<Files>"
Append-Line $builder 6 "<Enabled>False</Enabled>"
Append-Line $builder 6 "<Folder>%DEFAULT FOLDER%\</Folder>"
Append-Line $builder 6 "<RandomFileNames>False</RandomFileNames>"
Append-Line $builder 6 "<EncryptContent>False</EncryptContent>"
Append-Line $builder 4 "</Files>"
Append-Line $builder 2 "</Storage>"
Append-Line $builder 0 "</>"

Set-Content -LiteralPath $EvbFile -Value $builder.ToString() -Encoding ASCII
Write-Host "Generated Enigma config: $EvbFile"
