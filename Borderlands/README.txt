# Augmenta Borderlands

This is a fork of Christopher Carlson's Borderlands interactive granular sampler

Original code is hosted here : https://github.com/Lyptik/Borderlands

Project website:
	http::/ccrma.stanford.edu/~carlsonc/256a/Borderlands/index.html

## Features

This fork adds this following features :

	- Augmenta integration for real-time tracking adaptation (see : https://github.com/Theoriz/Augmenta)
	- Openframeworks integration
	- Syphon integration
	- Linux and OSX multiplatform code (potentially windows)
	- MultiTouch interface with Linux and ginn
	- Scaling algorithm
	- XML settings
	- Install scripts
	- Documentation
	- Many more...

## Install

Use the instal script in the scripts folder

## Use

In linux you need to start the jack server.

This command works on Ubuntu 14.04.2 in a gnome3 environment (with jackd2)

	jackd -P70 -p16 -t2000 -d alsa -dhw:1 -p 128 -n 3 -r 44100 -s

You might need to change it depending on your hardware sound card configuration

## Credits

Copyright (C) 2015 Théoriz Studio
contact@theoriz.com
www.theoriz.com

Copyright (C) 2011  Christopher Carlson
carlsonc@ccrma.stanford.edu
www.modulationindex.com

Libraries included in Borderlands:
Synthesis Toolkit
https://ccrma.stanford.edu/software/stk/

RtAudio is used for real-time audio I/O
http://www.music.mcgill.ca/~gary/rtaudio/

Openframeworks
http://openframeworks.cc/

