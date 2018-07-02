@echo off
CLS
:MENU
ECHO.
ECHO PRESS 1 or 2 to select your task, or 3 to EXIT.
ECHO ############################################################################
ECHO ##                              Git Menu                                  ##
ECHO ############################################################################
ECHO.
ECHO 1 - Git status
ECHO 2 - Initialise submodules.
ECHO 3 - Update submodules to latest.
ECHO 4 - Push all submodules.
ECHO 9 - EXIT
ECHO.
SET /P M=Type 1 - 9, then press ENTER:
IF %M%==1 GOTO GIT_STATUS
IF %M%==2 GOTO INITIALISE_GIT_SUBMODULES
IF %M%==3 GOTO GET_LATEST_SUBMODULES
IF %M%==4 GOTO PUSH_ALL_SUBMODULES
IF %M%==9 GOTO EOF


:GIT_STATUS
git status
goto MENU


:INITIALISE_GIT_SUBMODULES
git submodule update --init
goto MENU


:GET_LATEST_SUBMODULES
::
:: http://stackoverflow.com/questions/1030169/easy-way-pull-latest-of-all-submodules
::
git submodule foreach "(git checkout master; git pull)"
::git submodule foreach git checkout master
::git submodule foreach git pull origin master
goto MENU


:PUSH_ALL_SUBMODULES
::
:: http://stackoverflow.com/questions/1030169/easy-way-pull-latest-of-all-submodules
::
git submodule foreach "(git push)"
goto MENU
