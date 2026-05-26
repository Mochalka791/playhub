$releases = (Invoke-WebRequest 'https://api.github.com/repos/libsdl-org/SDL/releases' -UseBasicParsing).Content | ConvertFrom-Json
$sdl2Release = ($releases | Where-Object { $_.tag_name -like 'release-2.*' })[0]
$tag = $sdl2Release.tag_name
Write-Host "Latest SDL2: $tag"

# Build download URL:  release-2.30.9 -> SDL2-devel-2.30.9-VC.zip
$version = $tag -replace 'release-', ''
$zipName = "SDL2-devel-$version-VC.zip"
$url = "https://github.com/libsdl-org/SDL/releases/download/$tag/$zipName"
Write-Host "Downloading $url ..."

$dest   = "C:\Users\adm.vr\source\repos\Casino\$zipName"
$tmpDir = "C:\Users\adm.vr\source\repos\Casino\SDL2_tmp"
$vendorDir = "C:\Users\adm.vr\source\repos\Casino\Casino\vendor\SDL2"

Invoke-WebRequest -Uri $url -OutFile $dest -UseBasicParsing
Write-Host "Extracting..."

Expand-Archive -Path $dest -DestinationPath $tmpDir -Force

# Find the extracted folder (e.g. SDL2-2.30.9)
$sdlDir = (Get-ChildItem $tmpDir -Directory)[0].FullName
Write-Host "Extracted to $sdlDir"

# Create vendor structure with SDL2/ subdirectory for headers
$includeSDL2 = "$vendorDir\include\SDL2"
New-Item -ItemType Directory -Force -Path $includeSDL2 | Out-Null
New-Item -ItemType Directory -Force -Path "$vendorDir\lib\x64" | Out-Null

# Copy headers into include/SDL2/ so #include <SDL2/SDL.h> works
Copy-Item "$sdlDir\include\*" -Destination $includeSDL2 -Force

# Copy libs
Copy-Item "$sdlDir\lib\x64\SDL2.lib"     -Destination "$vendorDir\lib\x64\" -Force
Copy-Item "$sdlDir\lib\x64\SDL2main.lib" -Destination "$vendorDir\lib\x64\" -Force
Copy-Item "$sdlDir\lib\x64\SDL2.dll"     -Destination "$vendorDir\lib\x64\" -Force

# Also copy SDL2.dll next to where the exe will be
$binDebug = "C:\Users\adm.vr\source\repos\Casino\bin\Debug"
$binRelease = "C:\Users\adm.vr\source\repos\Casino\bin\Release"
New-Item -ItemType Directory -Force -Path $binDebug   | Out-Null
New-Item -ItemType Directory -Force -Path $binRelease | Out-Null
Copy-Item "$sdlDir\lib\x64\SDL2.dll" -Destination $binDebug   -Force
Copy-Item "$sdlDir\lib\x64\SDL2.dll" -Destination $binRelease -Force

# Cleanup
Remove-Item $dest -Force
Remove-Item $tmpDir -Recurse -Force

Write-Host "SDL2 setup complete."
