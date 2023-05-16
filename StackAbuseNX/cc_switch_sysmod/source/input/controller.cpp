#include "input.hpp"
namespace PptInput {
    HiddbgHdlsSessionId sessionID={};
}
Result PptInput::Controller::tryInit(Controller& controller) {
    Result rc;
    rc = hiddbgAttachHdlsWorkBuffer(&sessionID, NULL, 0);
    if (R_FAILED(rc)) {
        return rc;
    }

    controller = {};
    controller.valid = true;

    u32 singleColorBody = RGBA8_MAXALPHA(255, 255, 255);
    u32 colorLeftGrip = RGBA8_MAXALPHA(230, 255, 0);
    u32 colorRightGrip = RGBA8_MAXALPHA(0, 40, 20);
    controller.deviceInfo = {
        .deviceType = HidDeviceType_FullKey3,
        .npadInterfaceType = HidNpadInterfaceType_Bluetooth,
        .singleColorBody = singleColorBody,
        .colorLeftGrip = colorLeftGrip,
        .colorRightGrip = colorRightGrip
    };
    controller.state = {
        .battery_level = 4,
        .flags = 0,
        .buttons = 0,
        .analog_stick_l = {
            .x = 0,
            .y = 0
        },
        .analog_stick_r = {
            .x = 0,
            .y = 0
        }
    };
    return hiddbgAttachHdlsVirtualDevice(&controller.handle, &controller.deviceInfo);
}

Result PptInput::Controller::update() {
    return hiddbgSetHdlsState(handle, &state);
}

// never called because sysmods are cringe
PptInput::Controller::~Controller() {
    if (false && valid) {
        Result rc = hiddbgDetachHdlsVirtualDevice(handle);
        if (R_FAILED(rc)) {
            //uh oh, we just failed to destroy something
            diagAbortWithResult(rc);
        }
        
    }
}
