param(
    [string]$CodexMainJob = "/home/sjw/harbor-codex-jobs/uoj-73-gpt61sol-codex-20261114-003733",
    [string]$CodexSmokeJob = "/home/sjw/harbor-codex-jobs/uoj-1-gpt61sol-codex-20261114-000653"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$workspaceRoot = Split-Path $repoRoot -Parent
$siteRoot = Join-Path $repoRoot "site"
$generatedRoot = Join-Path $siteRoot "generated"
$leaderboardPath = Join-Path $siteRoot "data\leaderboard.json"
$problemsPath = Join-Path $siteRoot "data\problems.json"
$manifestPath = Join-Path $siteRoot "data\accepted-solutions.json"

$leaderboard = Get-Content -Raw -LiteralPath $leaderboardPath | ConvertFrom-Json
$problems = Get-Content -Raw -LiteralPath $problemsPath | ConvertFrom-Json
$problemById = @{}
foreach ($problem in $problems) {
    $problemById[[int]$problem.id] = $problem
}

function Get-LeaderboardEntry([string]$track, [string]$modelKey) {
    $entry = $leaderboard.tracks.$track | Where-Object { $_.modelKey -eq $modelKey } | Select-Object -First 1
    if ($null -eq $entry) {
        throw "Missing leaderboard entry: $track/$modelKey"
    }
    if (-not ($entry.PSObject.Properties.Name -contains "solvedProblemIds")) {
        throw "Leaderboard entry has no solvedProblemIds: $track/$modelKey"
    }
    if (@($entry.solvedProblemIds).Count -ne [int]$entry.solved) {
        throw "Solved count mismatch: $track/$modelKey"
    }
    return $entry
}

function Convert-ToWslPath([string]$path) {
    $resolved = [IO.Path]::GetFullPath($path)
    if ($resolved -notmatch '^([A-Za-z]):\\(.*)$') {
        throw "Cannot convert path to WSL: $resolved"
    }
    $drive = $Matches[1].ToLowerInvariant()
    $tail = $Matches[2] -replace '\\', '/'
    return "/mnt/$drive/$tail"
}

$expectedFiles = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$manifestEntries = [System.Collections.Generic.List[object]]::new()

function Add-ManifestEntry($entry, [string]$track, [int]$problemId, [string]$target) {
    $resolved = (Resolve-Path -LiteralPath $target).Path
    [void]$expectedFiles.Add($resolved)
    $digest = (Get-FileHash -Algorithm SHA256 -LiteralPath $resolved).Hash.ToLowerInvariant()
    $info = Get-Item -LiteralPath $resolved
    $manifestEntries.Add([ordered]@{
        problemId = $problemId
        title = [string]$problemById[$problemId].title
        track = $track
        model = [string]$entry.model
        modelKey = [string]$entry.modelKey
        harness = if ($track -eq "agent") { [string]$entry.framework } else { $null }
        path = "./generated/$track/$($entry.modelKey)/$($problemId.ToString('000')).cpp"
        bytes = [int64]$info.Length
        sha256 = $digest
    })
}

function Copy-PackageModel([string]$track, [string]$modelKey, [string]$sourceRoot) {
    $entry = Get-LeaderboardEntry $track $modelKey
    $targetRoot = Join-Path $generatedRoot "$track\$modelKey"
    New-Item -ItemType Directory -Path $targetRoot -Force | Out-Null
    foreach ($rawId in $entry.solvedProblemIds) {
        $problemId = [int]$rawId
        $prefix = $problemId.ToString("000") + "_*.cpp"
        $matches = @(Get-ChildItem -LiteralPath $sourceRoot -Filter $prefix -File)
        if ($matches.Count -ne 1) {
            throw "Expected one source for $track/$modelKey/#$problemId, found $($matches.Count)"
        }
        $target = Join-Path $targetRoot ($problemId.ToString("000") + ".cpp")
        Copy-Item -LiteralPath $matches[0].FullName -Destination $target -Force
        Add-ManifestEntry $entry $track $problemId $target
    }
}

$multiPackage = Join-Path $workspaceRoot "deliverables\AlgoScale_leaderboard_passed_cpp_Opus_Kimi_Terra_20261114"
$deepSeekPackage = Join-Path $workspaceRoot "deliverables\AlgoScale_leaderboard_passed_cpp_DeepSeek_20261114"

Copy-PackageModel "direct" "kimi-k3-1-pro" (Join-Path $multiPackage "direct\kimi-k3-1-pro")
Copy-PackageModel "direct" "deepseek-flash" (Join-Path $deepSeekPackage "direct\deepseek-flash")
Copy-PackageModel "agent" "claude-opus-5-5" (Join-Path $multiPackage "agent\claude-opus-5-5")
Copy-PackageModel "agent" "kimi-k3-1-pro" (Join-Path $multiPackage "agent\kimi-k3-1-pro")
Copy-PackageModel "agent" "gpt-6-1-terra" (Join-Path $multiPackage "agent\gpt-6-1-terra")
Copy-PackageModel "agent" "deepseek-flash" (Join-Path $deepSeekPackage "agent\deepseek-flash")

$glmEntry = Get-LeaderboardEntry "direct" "glm-5-3-flash"
$glmTargetRoot = Join-Path $generatedRoot "direct\glm-5-3-flash"
New-Item -ItemType Directory -Path $glmTargetRoot -Force | Out-Null
foreach ($rawId in $glmEntry.solvedProblemIds) {
    $problemId = [int]$rawId
    $source = Join-Path $workspaceRoot ("artifacts\local_uoj_glm53_flash_direct_20261113_aa2\problem_{0:D4}\openrouter__z-ai_glm-5.3-flash\answer.cpp" -f $problemId)
    if (-not (Test-Path -LiteralPath $source)) {
        throw "Missing GLM source for problem #$problemId"
    }
    $target = Join-Path $glmTargetRoot ($problemId.ToString("000") + ".cpp")
    Copy-Item -LiteralPath $source -Destination $target -Force
    Add-ManifestEntry $glmEntry "direct" $problemId $target
}

$solEntry = Get-LeaderboardEntry "agent" "gpt-6-1-sol"
$solTargetRoot = Join-Path $generatedRoot "agent\gpt-6-1-sol"
New-Item -ItemType Directory -Path $solTargetRoot -Force | Out-Null
foreach ($rawId in $solEntry.solvedProblemIds) {
    $problemId = [int]$rawId
    $jobRoot = if ($problemId -eq 1) { $CodexSmokeJob } else { $CodexMainJob }
    $pattern = "$jobRoot/uoj-local-${problemId}__*/artifacts/app/solution.cpp"
    $linuxSource = (& wsl.exe -d Ubuntu -- find $jobRoot -path $pattern -type f -print -quit).Trim()
    if (-not $linuxSource) {
        throw "Missing GPT-6.1 Sol source for problem #$problemId"
    }
    $target = Join-Path $solTargetRoot ($problemId.ToString("000") + ".cpp")
    $wslTarget = Convert-ToWslPath $target
    & wsl.exe -d Ubuntu -- cp -- $linuxSource $wslTarget
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $target)) {
        throw "Failed to copy GPT-6.1 Sol source for problem #$problemId"
    }
    Add-ManifestEntry $solEntry "agent" $problemId $target
}

$generatedRootResolved = [IO.Path]::GetFullPath($generatedRoot).TrimEnd('\') + '\'
foreach ($file in Get-ChildItem -LiteralPath $generatedRoot -Recurse -Filter "*.cpp" -File) {
    $resolved = [IO.Path]::GetFullPath($file.FullName)
    if (-not $resolved.StartsWith($generatedRootResolved, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove file outside generated root: $resolved"
    }
    if (-not $expectedFiles.Contains($resolved)) {
        Remove-Item -LiteralPath $resolved -Force
    }
}

$orderedEntries = @($manifestEntries | Sort-Object problemId, track, model)
$manifest = [ordered]@{
    updated = [string]$leaderboard.updated
    total = $orderedEntries.Count
    entries = $orderedEntries
}
$json = $manifest | ConvertTo-Json -Depth 8
[IO.File]::WriteAllText($manifestPath, $json + "`n", [Text.UTF8Encoding]::new($false))

Write-Output "Synced $($orderedEntries.Count) accepted C++ files."
Write-Output "Manifest: $manifestPath"
