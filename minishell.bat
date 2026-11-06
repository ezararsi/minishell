@echo off
rem Startet die minishell aus PowerShell/CMD ueber WSL (Ubuntu)
wsl --cd "%~dp0." bash -c "make -s > /dev/null && ./minishell"
