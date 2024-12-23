#!/bin/bash

gcc -o main.out main.c high_scores.c game.c server.c movements.c pieces.c queue.c -lpthread

if [ $? -eq 0 ]; then
	echo "[COMPILE AND RUN]: Compiled succesfully. running program...";
	./main.out;
else 
	echo "[COMPILE AND RUN]: Failed to compile. See error log above";
	exit 1
fi 
