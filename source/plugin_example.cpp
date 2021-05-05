#include <hex/plugin.hpp> // ImHex files
#include <hex/views/view.hpp>

#ifdef _WIN32
extern "C"
{
#include "libusb.h"
}
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <libusb-1.0/libusb.h>
#endif

#include <chrono>
#include <mutex>
#include <thread> //std files

extern "C"
{
#include "coldclear.h"
}
#include "shak.cpp"
//#include"doesntexist.h"
#define PAGESIZE 0x800

class StackAbuseProvider : public hex::prv::Provider {
private:
    hex::prv::Overlay overlay;
    std::mutex mtx;

public:
    StackAbuseProvider() : hex::prv::Provider() {
        overlay = *newOverlay();
        overlay.getData().resize((PAGESIZE * 3));
    }
    ~StackAbuseProvider() override {}

    bool isAvailable() override {
        return true;
    }
    bool isReadable() override {
        return true;
    }
    bool isWritable() override {
        return true;
    }

    void readRaw(u64 offset, void* buffer, size_t size) override {
        if ((offset + size) > this->getActualSize() || buffer == nullptr || size == 0)
            return;
        mtx.lock();
        std::memcpy(buffer, (overlay.getData().data() + offset), size);
        mtx.unlock();
    }

    void writeRaw(u64 offset, const void* buffer, size_t size) override {
        if ((offset + size) > this->getActualSize() || buffer == nullptr || size == 0)
            return;

        mtx.lock();
        std::memcpy((overlay.getData().data() + offset), buffer, size);
        mtx.unlock();
    }
    size_t getActualSize() override {
        return overlay.getData().size();
    }
    std::vector<std::pair<std::string, std::string>> getDataInformation() {
        std::vector<std::pair<std::string, std::string>> result;

        result.emplace_back("hex.builtin.provider.file.size"_lang, hex::toByteString(this->getActualSize()));

        return result;
    }
};

const int ccpiece[7] = { CC_S, CC_Z, CC_J, CC_L, CC_T, CC_O, CC_I };
constexpr u128 BIT(u64 n) { return (1U << (n)); }

class StackAbuseManager : public hex::View {
public:
    std::thread worker;
    manybools bools{ (u32)0 };
    std::atomic_bool shouldContinue = true;
    std::atomic_bool isUsbConnected = false;
    u32 pointer_chain[8]{};
    bool pointerMode = false;
    bool addLastOffset = true;
    //hex::prv::Provider *&currentProvider;
    u8* NXData;

    StackAbuseManager() : hex::View("StackAbusePC") {

        startthread();
        NXData = new u8[PAGESIZE];
    }
    ~StackAbuseManager() override {
        shouldContinue = false;
        delete[] NXData;
        worker.join();
        uninitlibusb();
    }

    void drawContent() override {
        if (ImGui::Begin("StackAbusePC")) {
            bools.boolean6 = ImGui::Button("print all usb devices:");

            if (pointerMode) {
                bool pass = true;
                ImGui::Text("Command");
                bools.boolean0 = ImGui::Button("detatch from switch");
                ImGui::Text("dereference the last offset?");
                ImGui::Checkbox("dereference the last offset? (0 for first offset does nothing as of now)", &addLastOffset);
                ImGui::Text("Pointer chain:");
                ImGui::InputInt("first", (int*)&pointer_chain[0], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[0] != 0)
                    ImGui::InputInt("second", (int*)&pointer_chain[1], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[1] != 0)
                    ImGui::InputInt("thrid", (int*)&pointer_chain[2], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[2] != 0)
                    ImGui::InputInt("fourth", (int*)&pointer_chain[3], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[3] != 0)
                    ImGui::InputInt("fifth", (int*)&pointer_chain[4], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[4] != 0)
                    ImGui::InputInt("sixth", (int*)&pointer_chain[5], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[5] != 0)
                    ImGui::InputInt("seventh", (int*)&pointer_chain[6], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (pointer_chain[6] != 0)
                    ImGui::InputInt("eighth", (int*)&pointer_chain[7], sizeof(int*), 8, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                bools.boolean10 = true;
            } else if (isUsbConnected) {
                ImGui::Text("Commands");
                bools.boolean0 = ImGui::Button("detatch from switch");
                //bools.boolean1 = ImGui::Button("retatch to switch");
                //bools.boolean2 = ImGui::Button("sports");
                //bools.boolean3 = ImGui::Button("its");
                //bools.boolean4 = ImGui::Button("in");
                bools.boolean5 = ImGui::Button("turn off sysmod");
                bools.boolean7 = ImGui::Button("getMainNsoBase");
                bools.boolean8 = ImGui::Button("BuildID");
                bools.boolean9 = ImGui::Button("Pointer Watch Mode");
            } else {
                ImGui::Text("You Are Not Connected To the Switch");
                if (ImGui::Button("Try To Connect To Switch")) {
                    isUsbConnected = tryOpenNX();
                    resetNX();
                }
            }
        }
        ImGui::End();
    }

private:
    void initlibusbloop() {
        while (!isUsbConnected) {
            isUsbConnected = tryOpenNX();
            //libusbCmdLog();
            // 	std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // a second
        }
    }

    void startthread() {

        worker = std::thread([&] {
            auto& currentProvider = hex::SharedData::currentProvider;

            // if (currentProvider != nullptr)
            //     delete currentProvider;

            // currentProvider = new StackAbuseProvider();
            std::stringstream stream;

            while (shouldContinue) {

                /*code___________________________________________________________________*/

                u32 buffer;
                memcpy(&buffer, &bools, sizeof(u32));
                switch (buffer) {
                case BIT(0):
                    uninitlibusb();
                    isUsbConnected = false;
                    bools.Reset();
                    break;
                case BIT(1):
                    uninitlibusb();
                    initlibusb();
                    bools.Reset();
                    break;
                case BIT(2):
                    std::cout << "sports" << std::endl;
                    bools.Reset();
                    break;
                case BIT(3):
                    std::cout << "its" << std::endl;
                    bools.Reset();
                    break;
                case BIT(4):
                    std::cout << "in" << std::endl;
                    bools.Reset();
                    break;
                case BIT(5):
                    stopsysmod();
                    uninitlibusb();
                    isUsbConnected = false;
                    bools.Reset();
                    break;
                case BIT(6):
                    std::cout << "print all usb devices:" << std::endl;
                    libusbCmdLog();
                    bools.Reset();
                    break;
                case BIT(7):
                {
                    std::cout << "getMainNsoBase" << std::endl;
                    u64 data = 0;
                    data = getMain();
                    std::cout << data << std::endl;
                }
                bools.Reset();
                break;
                case BIT(8): //BuildID
                {
                    u8 iter = 0;
                    char GameID[32] = { 0 };
                    BuildID(GameID);
                    std::cout << "0x" << std::hex << ((uint32_t)GameID[0] & 0xff);
                    for (iter = 1; iter < sizeof(GameID); iter++) {
                        std::cout << ", 0x" << std::hex << ((uint32_t)GameID[iter] & 0xff); // change this
                    }
                    std::cout << std::endl;
                }
                bools.Reset();
                break;
                case BIT(9): //turning on pointer mode (should happen once before infinitely calling BIT(10))
                {
                    enablePointerMode();
                    pointerMode = true;
                }
                bools.Reset();
                break;
                case BIT(10): //pointermode
                {

                    u64 baseaddress = currentProvider->getBaseAddress();
                    u32 numberOfOffsets = 0;
                    bool result = true; // worked by default
                    for (int i = 0; i < (sizeof(pointer_chain) / sizeof(*pointer_chain)); i++) {
                        if (pointer_chain[i] != (u32)0)
                            numberOfOffsets++;
                        else
                            break;
                    }
                    if (addLastOffset || numberOfOffsets == 0)
                        numberOfOffsets++;
                    populate_pointers(pointer_chain, numberOfOffsets);
                    if (numberOfOffsets == 0)
                        break;

                    for (int i = 0; i < 3; i++) {
                        recievePointerData(NXData);
                        if (NXDataIsValid(NXData))
                            currentProvider->writeRaw((baseaddress + (i * PAGESIZE)), (void*)NXData, PAGESIZE);
                        else {
                            currentProvider->writeRaw((baseaddress), (void*)NXData, ((PAGESIZE * 3) - (PAGESIZE * i)));
                            result = false;
                            break;
                        }
                    }
                    if (result)
                        currentProvider->setBaseAddress(getBaseAddrNX());
                }

                bools.Reset();
                break;
                default:
                    break;
                }
                /*more code______________________________________________________________*/
            }
            delete currentProvider;
                             });
    }
};

IMHEX_PLUGIN_SETUP("StackAbusePC", "Shakkar23", "this is a pc plugin to talk to StackAbuseNX on the Nintendo Switch") {

    ContentRegistry::Views::add<StackAbuseManager>();
}
