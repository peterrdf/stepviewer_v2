
echo Setup BCF engine

set ProjectDir=%~1
set Platform=%2
set OutputPath=%~3

if .%RDF_BCF_ENGINE%.==.. goto Updated

echo echo Update BCF engine 

copy %RDF_BCF_ENGINE%bcfEngine\bcfTypes.h %ProjectDir%..\bcfEngine /Y
copy %RDF_BCF_ENGINE%bcfEngine\bcfAPI.h %ProjectDir%..\bcfEngine /Y

copy %RDF_BCF_ENGINE%output\bcfEngine.dll %ProjectDir%..\bcfEngine\%Platform%
copy %RDF_BCF_ENGINE%output\bcfEngine.lib %ProjectDir%..\bcfEngine\%Platform%
copy %RDF_BCF_ENGINE%output\zip.dll %ProjectDir%..\bcfEngine\%Platform%
copy %RDF_BCF_ENGINE%output\zlib.dll %ProjectDir%..\bcfEngine\%Platform%

:Updated

echo Copy dll to output
xcopy %ProjectDir%..\bcfEngine\%Platform%\*.dll %OutputPath% /F /Y
