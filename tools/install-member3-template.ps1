param(
    [Parameter(Mandatory = $true)]
    [string]$TemplateRoot
)

$ErrorActionPreference = 'Stop'
$member3RepoRoot = Split-Path -Parent $PSScriptRoot
$member3TemplateRoot = (Resolve-Path -LiteralPath $TemplateRoot).Path
$member3Destination = Join-Path $member3TemplateRoot 'app_program/subsystem3'
$member3Source = Join-Path $member3RepoRoot 'app_program/subsystem3'

if (-not (Test-Path -LiteralPath (Join-Path $member3TemplateRoot 'app_program/app.h'))) {
    throw 'Target must be an existing INF2004 project template checkout.'
}
if (-not (Test-Path -LiteralPath $member3Destination -PathType Container)) {
    throw 'Target subsystem3 folder is missing; no files were changed.'
}
# No delete/move. Installation overwrites Member 3 files only. Back up or
# commit local subsystem3 work before installing; other owners are untouched.
if ($member3Destination -eq $member3Source) {
    throw 'Source and destination are the same folder.'
}
$member3Before = @{}
foreach ($member3Owner in @('subsystem1', 'subsystem2', 'subsystem4', 'subsystem5')) {
    foreach ($member3File in Get-ChildItem -LiteralPath (Join-Path $member3TemplateRoot "app_program/$member3Owner") -File -Recurse) {
        $member3Before[$member3File.FullName] = (Get-FileHash -LiteralPath $member3File.FullName -Algorithm SHA256).Hash
    }
}
foreach ($member3File in Get-ChildItem -LiteralPath $member3Source -File -Recurse) {
    $member3Relative = $member3File.FullName.Substring($member3Source.Length + 1)
    $member3Target = Join-Path $member3Destination $member3Relative
    New-Item -ItemType Directory -Path (Split-Path -Parent $member3Target) -Force | Out-Null
    Copy-Item -LiteralPath $member3File.FullName -Destination $member3Target -Force
}
foreach ($member3Path in $member3Before.Keys) {
    if ((Get-FileHash -LiteralPath $member3Path -Algorithm SHA256).Hash -ne $member3Before[$member3Path]) {
        throw "Other owner's file changed unexpectedly: $member3Path"
    }
}
Write-Output 'Member 3 installed. SHA256 checks confirm Subsystems 1, 2, 4 and 5 are unchanged.'
