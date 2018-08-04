# MontySim

This is a small project for simulating Monty Hall style games and measuring the probability of various outcomes.
Normally I wouldn't bother putting something like this on something like github, but I'm currently having computer storage problems, so this will have to do. Also I've never used github before so this is a good chance to learn more about it.


###### Todo:
Add argument for total number of games allowed to play.
Solve the bias in +1billion runs of the simulation. If it's too ineffecient to do so, perhaps provide an option of PRNG algorithms to use
Currently, if we ctrl+c while a game delay is set, we don't actually stop the program until the delay is up. If the delay is long, this might be frustrating. The problem is that only one thread (main thread probably) gets SIGINT, the other threads ignore it so nanosleep() isn't interrupted. Should try and fix this at some stage. 
