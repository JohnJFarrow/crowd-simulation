rem d:\EpicSource\581\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -project=%cd%\MZ.uproject -server ^
rem  -target=MZServer -cook -map=L_Demo -skipstage
"C:/EpicGames/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" -ScriptsForProject="D:/work/ZombieShooter/MZ.uproject" Turnkey ^
     -command=VerifySdk -platform=Win64 -UpdateIfNeeded -project="D:/work/ZombieShooter/MZ.uproject" BuildCookRun -nop4 ^
     -nocompileeditor -skipbuildeditor -cook  -project="D:/work/ZombieShooter/MZ.uproject" -target=MZClient ^
     -unrealexe="C:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" -platform=Win64 -installed ^
     -zenstore -skipstage -nocompile -nocompileuat 
     