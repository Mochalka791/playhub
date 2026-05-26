$releases = (Invoke-WebRequest 'https://api.github.com/repos/libsdl-org/SDL_mixer/releases' -UseBasicParsing).Content | ConvertFrom-Json
$rel = ($releases | Where-Object { $_.tag_name -like 'release-2.*' })[0]
$tag = $rel.tag_name
Write-Host "Latest SDL2_mixer: $tag"

$version = $tag -replace 'release-', ''
$zipName = "SDL2_mixer-devel-$version-VC.zip"
$url     = "https://github.com/libsdl-org/SDL_mixer/releases/download/$tag/$zipName"
Write-Host "Downloading $url ..."

$dest   = "C:\Users\adm.vr\source\repos\Casino\$zipName"
$tmpDir = "C:\Users\adm.vr\source\repos\Casino\SDL2_mixer_tmp"
$vendor = "C:\Users\adm.vr\source\repos\Casino\Casino\vendor\SDL2_mixer"

Invoke-WebRequest -Uri $url -OutFile $dest -UseBasicParsing
Write-Host "Extracting..."
Expand-Archive -Path $dest -DestinationPath $tmpDir -Force

$sdlDir = (Get-ChildItem $tmpDir -Directory)[0].FullName
Write-Host "Extracted: $sdlDir"

New-Item -ItemType Directory -Force -Path "$vendor\include\SDL2" | Out-Null
New-Item -ItemType Directory -Force -Path "$vendor\lib\x64" | Out-Null

Copy-Item "$sdlDir\include\*" -Destination "$vendor\include\SDL2\" -Force
Copy-Item "$sdlDir\lib\x64\SDL2_mixer.lib" -Destination "$vendor\lib\x64\" -Force
Copy-Item "$sdlDir\lib\x64\SDL2_mixer.dll" -Destination "$vendor\lib\x64\" -Force

# Copy DLL to bin folders
foreach ($d in @('C:\Users\adm.vr\source\repos\Casino\bin\Debug',
                 'C:\Users\adm.vr\source\repos\Casino\bin\Release')) {
    New-Item -ItemType Directory -Force -Path $d | Out-Null
    Copy-Item "$sdlDir\lib\x64\SDL2_mixer.dll" -Destination $d -Force
}

Remove-Item $dest   -Force
Remove-Item $tmpDir -Recurse -Force
Write-Host "SDL2_mixer setup done."
