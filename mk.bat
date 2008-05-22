move object >nul
wcl386 gui.c pe.obj export.obj util.obj rep.obj /zw /l=win95 /fe=fr.exe
wrc rsrc.res fr.exe
move *.obj object >nul
