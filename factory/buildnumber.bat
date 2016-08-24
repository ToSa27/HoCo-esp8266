@set /p Build=<buildnumber.txt
@echo Current BuildNumber: %Build%
@set /a Build+=1
@echo %Build% > buildnumber.txt
@echo Next BuildNumber: %Build%
