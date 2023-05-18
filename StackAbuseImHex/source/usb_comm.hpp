#include <chrono>
#include <iostream>
#ifdef _WIN32
extern "C"
{
#include "libusb-1.0/libusb.h"
}
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <libusb-1.0/libusb.h>
#endif
#include <string>
#include <thread>
#include <stdint.h>
#include <nlohmann/json.hpp>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

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

void resetNX();
int libusbCmdLog();

void uninitlibusb();
bool initlibusb();

void sendJson(const nlohmann::json &json);
const nlohmann::json receiveJson();

enum class Piece : u32
{
    S = 0,
    Z = 1,
    J = 2,
    L = 3,
    T = 4,
    O = 5,
    I = 6
};
char PieceToChar(Piece piece)
{
    switch (piece)
    {
    case Piece::S:
        return 'S';
    case Piece::Z:
        return 'Z';
    case Piece::J:
        return 'J';
    case Piece::L:
        return 'L';
    case Piece::T:
        return 'T';
    case Piece::O:
        return 'O';
    case Piece::I:
        return 'I';
    }
    return 'N';
}
struct PieceState
{
    Piece piece;
    u32 x;
    u32 y;
    u32 distanceToGround;
    u32 rotation;
    u8 locked;
    u8 isPuyomino;
};

class GameState
{
public:
    std::array<std::array<bool, 10>, 40> board{};
    std::vector<Piece> queue;
    PieceState pieceState;
    bool pieceHeld;
    bool pieceInactive;
} game_state;