#!/bin/bash

source .env

gcc -DPORT="$PORT" -DSERVER_ADDRESS="\"$SERVER_IP\"" -o main.out main.c ui.c client.c game.c -lncurses -lmenu -lpanel -lform 

if [ $? -eq 0 ]; then
	echo "[COMPILE AND RUN]: Compiled succesfully. running program...";
	./main.out;
else 
	echo "[COMPILE AND RUN]: Failed to compile. See error log above";
	exit 1
fi 
