/*
This file is meant to abstract everything to do with time.
It just wraps the systems specific high resolution timers for now
*/

#ifndef CLOCK_H
#define CLOCK_H

#include <SDL/SDL.h>

Uint64 GetNanoSeconds();

#endif