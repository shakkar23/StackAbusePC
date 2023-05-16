#include "Shak2.hpp"
#include "../Shak/commands.hpp"
#include "../mem/mem.hpp"

#include <optional>
#include <string>
#include <jsoncons/json.hpp>

// non memory related variables
ViDisplay disp{};
Event vsync_event{};

u64 mainAddr{};
std::optional<Handle> handle;
u64 pid{};
bool parseJson(const jsoncons::json& command){
    std::string str = command["command"].as_string();
    if(str == "exit")
        return true;
    else if(str == "give state"){

        jsoncons::json jsonPayload;

        PptMemory::PieceState PieceState;
        auto r = PptMemory::PptMemoryLock::readPieceState(PptMemory::Player::One, PieceState);
        if(R_SUCCEEDED(r)){
            jsonPayload["Piece"]["x"] = PieceState.x;
            jsonPayload["Piece"]["y"] = PieceState.y;
            jsonPayload["Piece"]["rotation"] = PieceState.rotation;
            jsonPayload["Piece"]["distanceToGround"] = PieceState.distanceToGround;
            jsonPayload["Piece"]["locked"] = PieceState.locked;
            jsonPayload["Piece"]["isPuyomino"] = PieceState.isPuyomino;
            jsonPayload["Piece"]["piece"] = (int)PieceState.piece;
        }

        bool pieceInactive;
        r = PptMemory::PptMemoryLock::readPieceInactive(PptMemory::Player::One, pieceInactive);
        if(R_SUCCEEDED(r)){
            jsonPayload["PieceInactive"] = pieceInactive;
        }

        bool pieceHeld;
        r = PptMemory::PptMemoryLock::readPieceHeld(PptMemory::Player::One, pieceHeld);
        if(R_SUCCEEDED(r)){
            jsonPayload["PieceHeld"] = pieceHeld;
        }

        std::array<std::array<bool, 10>, 40> board;
        r = PptMemory::PptMemoryLock::readBoard(PptMemory::Player::One, board);
        if(R_SUCCEEDED(r)){
            jsoncons::json jsonBoard;
            for(int i = 0; i < 40; i++){
                for(int j = 0; j < 10; j++){
                    jsonBoard[i][j] = board[i][j];
                }
            }
            jsonPayload["Board"] = jsonBoard;
        }

        std::vector<PptMemory::Piece> queue;
        r = PptMemory::PptMemoryLock::readQueue(PptMemory::Player::One, queue);
        if(R_SUCCEEDED(r)){
            jsoncons::json jsonQueue;
            for(int i = 0; i < queue.size(); i++){
                jsonQueue[i] = (int)queue[i];
            }
            jsonPayload["Queue"] = jsonQueue;
        }

        USBResponse payload;
        std::string temp = jsonPayload.as_string();
        payload.size = temp.size();
        payload.data = temp.data();

        usbCommsWrite(&payload, sizeof(payload));
        usbCommsWrite(payload.data, payload.size);
    } else if( str == "frame advance"){
        if(handle.has_value()){
            svcCloseHandle(*handle);
            handle.reset();
        }
        handle = std::make_optional<Handle>();
        svcBreakDebugProcess(handle.value());
        eventWait(&vsync_event, UINT64_MAX);
        svcDebugActiveProcess(&handle.value(), pid);
        

    } else if( str == "play"){
        if(handle.has_value()){
            svcCloseHandle(*handle);
            handle.reset();
        }
    } else if( str == "pause"){
        if(!handle.has_value()){
            handle = std::make_optional<Handle>();
            svcDebugActiveProcess(&handle.value(), pid);
        }

    }
        

    return false;
}



// this part of the sysmod is to frame advance and peek at memory at the same time
// nothing else
int PcTalk2()
{
	// Result rc;
	USBResponse x;
	auto rc = viOpenDefaultDisplay(&disp);
	if (R_FAILED(rc)) {
		return 0x1;
	}
	rc = viGetDisplayVsyncEvent(&disp, &vsync_event);
	if (R_FAILED(rc)) {
		return 0x2;
	}



    while(true){
        pid = getPID();
        rc = PptMemory::PptContext::hookppt2(pid);
		if (R_FAILED(rc)) {
		    rc = PptMemory::PptContext::hookppt1(pid);
		    if (R_FAILED(rc)) {
		    	continue;
		    } else
			    break;
		} else
            break;
		svcSleepThread(10'000'000'000);
    }
    // hooked into process!


	mainAddr = getMainNsoBase();
	while (true) 
	{
		int len;
		usbCommsRead(&len, sizeof(len));

		char linebuf[len + 1];

		for (int i = 0; i < len + 1; i++)
		{
			linebuf[i] = 0;
		}

		usbCommsRead(&linebuf, len);

		// Adds necessary escape characters for pasrser
		linebuf[len - 1] = '\0';

        jsoncons::json object = jsoncons::json::parse(linebuf);


		if(parseJson(object))
            break;
	}
	return 0;
}
