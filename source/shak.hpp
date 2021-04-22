#include <chrono>
#include <hex.hpp>
#include <iostream>
#ifdef _WIN32
extern "C"
{
#include "libusb.h"
}
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <libusb-1.0/libusb.h>
#endif
#include <string>
#include <thread>

// enum buttons {
// 	BUTTONA,
// 	BUTTONB,
// 	BUTTONX,
// 	BUTTONY,
// 	BUTTONR,
// 	BUTTONL,
// 	BUTTONZR,
// 	BUTTONZL,
// 	PLUS,
// 	MINUS,
// 	DLEFT,
// 	DRIGHT,
// 	DUP,
// 	DDOWN,
// 	HOME,   // please dont use this
// 	CAPTURE // or this
// };

typedef struct
{
    int size;
    uint8_t *data;
} USBResponse;

enum Switch
{
    switchsend0 = 1,
    switchread0 = 129,
    switchsend1 = 2,
    switchread1 = 130,
    switchread2 = 9,
    switchsend2 = 9
};

enum PcToSwitchCntrlr
{
    switch_DLEFT = 0,
    switch_DRIGHT = 1,
    switch_DUP = 2,
    switch_A = 3,
    switch_B = 4,
    switch_L = 5,
    switch_DDOWN = 6
};

struct manybools
{
    bool boolean0  : 1;
    bool boolean1  : 1;
    bool boolean2  : 1;
    bool boolean3  : 1;
    bool boolean4  : 1;
    bool boolean5  : 1;
    bool boolean6  : 1;
    bool boolean7  : 1;
    bool boolean8  : 1;
    bool boolean9  : 1;
    bool boolean10 : 1;
    bool boolean11 : 1;
    bool boolean12 : 1;
    bool boolean13 : 1;
    bool boolean14 : 1;
    bool boolean15 : 1;
    bool boolean16 : 1;
    bool boolean17 : 1;
    bool boolean18 : 1;
    bool boolean19 : 1;
    bool boolean20 : 1;
    bool boolean21 : 1;
    bool boolean22 : 1;
    bool boolean23 : 1;
    bool boolean24 : 1;
    bool boolean25 : 1;
    bool boolean26 : 1;
    bool boolean27 : 1;
    bool boolean28 : 1;
    bool boolean29 : 1;
    bool boolean30 : 1;
    bool boolean31 : 1;

    void Reset()
    {
        *this = {(u32) false};
    }
};
