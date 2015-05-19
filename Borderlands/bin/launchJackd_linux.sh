#!/bin/sh

# Launch jack server in linux

# You need to have creen installed

screen -S jackd -d -m env DISPLAY=:0 jackd -P70 -p16 -t2000 -d alsa -dhw:1 -p 128 -n 3 -r 44100 -s
