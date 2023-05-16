#include <switch.h>
#include "usb_cc/usb_cc.hpp"
#include "mem/mem.hpp"
#include "input/input.hpp"
#include "Shak/commands.hpp"

class CcSwitchSysmod {
    
public:
    static Result run();
};

//one frame of spawn delay after the piece became visible
const int PIECE_SPAWN_DELAY = 2;
