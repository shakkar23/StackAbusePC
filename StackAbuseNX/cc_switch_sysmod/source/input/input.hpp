#include <switch.h>

namespace PptInput {
   extern HiddbgHdlsSessionId sessionID;
    class Controller {
        bool valid;
        HiddbgHdlsHandle handle;
        HiddbgHdlsDeviceInfo deviceInfo;
    public:
        HiddbgHdlsState state;

    public:
        Controller() {}
        Controller(Controller&& other)
            : valid(true)
            , handle(other.handle)
            , deviceInfo(other.deviceInfo){
            other.valid = false;
        }
        
        Controller& operator= (Controller&& other) {
            handle = other.handle;
            deviceInfo = other.deviceInfo;
            state = other.state;
            valid = true;
            other.valid = false;
            return *this;
        }

        Controller(const Controller&) = delete;
        Controller& operator= (const Controller&) = delete;

        static Result tryInit(Controller& controller);
        ~Controller();
        Result update();
    };
}
