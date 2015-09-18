﻿# Augmenta Borderlands

This is a fork of Christopher Carlson's Borderlands interactive granular sampler

Original code is hosted here : https://github.com/Lyptik/Borderlands

Project website:
	http://ccrma.stanford.edu/~carlsonc/256a/Borderlands/index.html

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

This code was developped and tested on OSX 10.10 with Of 0.8.4 and on Linux Ubuntu 14.04.2 with Of 0.8.4

You can use these free sounds to test the application (provided by Chris Carlson) : http://www.borderlands-granular.com/app/borderlands_defaults.zip

## Notes

Performance obtained with Jackd2 on Linux are far more efficient than OSX and iPad version

## Notice

TOFIX :

While lastminute debuggin we might have done wrong modification to track down remaining segfault (which were coming from discussion between jack and borderlands in linux through the jack driver, changing jack parameter fixed that (buffer was eaten in a too fast way by the api in borderlands))

-> API is still not protected, should be, the application shoudl not crash (segfault) when jack has wrong parameter

-> We might have removed some delete or other part of code which were suspicious, but there were not responsible finally so they should be reintroducesd and tested

-> We were working without tracking with Augmenta at Gaite so we might have as well modified the way Borderlands use the point, this should also be checked
(This could be broken with a normal use of Augmenta)

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

