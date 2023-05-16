#pragma once
#ifndef COMMANDS_HPP
#define COMMANDS_HPP
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

// Result rc;

// void poke(u64 offset, u64 size, u8* val);
// u8* peek(u64 offset, u64 size);


u64 getMainNsoBase();
void BuildID(u8 joe[0x20]);
bool issame(u8 one[0x20], u8 two[0x20]);


bool sysmodMainLoop();
void DestroySysmod();


u64 getPID();
#endif