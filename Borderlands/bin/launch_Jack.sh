#!/bin/sh

# You need to have screen installed

#screen -S jackd -d -m env DISPLAY=:0 jackd -P70 -p16 -t2000 -d alsa -dhw:1 -p 128 -n 3 -r 44100 -s
sudo killall -9 pulseaudio
sudo killall -9 pulseaudio
sudo killall -9 pulseaudio

sleep 3
screen -S jackd -d -m env DISPLAY=:0 jackd -R -P79 -dalsa -dhw:1 -r44100 -p2560 -n3

