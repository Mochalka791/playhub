$msbuild = 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
$sln     = 'C:\Users\adm.vr\source\repos\Casino\Casino.sln'
& $msbuild $sln /p:Configuration=Debug /p:Platform=x64 /m /nologo
