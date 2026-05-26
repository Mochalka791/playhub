$msbuild = Get-ChildItem 'C:\Program Files\Microsoft Visual Studio' -Recurse -Filter 'MSBuild.exe' -ErrorAction SilentlyContinue | Where-Object { $_.FullName -match 'amd64' } | Select-Object -First 1 -ExpandProperty FullName
Write-Host $msbuild
