@echo off
for /r %%i in (filters\*.json) do (
	SET _dest=%LOCALAPPDATA%\Tableau\TLV\filters\
	xcopy  /i /e %%i %_dest%
)