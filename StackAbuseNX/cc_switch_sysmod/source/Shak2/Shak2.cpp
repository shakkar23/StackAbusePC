#include "Shak2.hpp"
#include "../Shak/commands.hpp"
#include "../mem/mem.hpp"

#include <nlohmann_json.hpp>
#include <optional>
#include <string>

// debug libraries for outputting to a file
#include <filesystem>
#include <fstream>

// non memory related variables
ViDisplay disp {};
Event vsync_event {};

static u64 mainAddr {};
static std::optional<Handle> handle;
static u64 PPTPID {};

void sendJson(const nlohmann::json& json_payload) {

	USBResponse payload;
	std::string temp = json_payload.dump();
	payload.size     = temp.size();
	payload.data     = temp.data();

	usbCommsWrite(&payload.size, sizeof(decltype(payload.size)));
	usbCommsWrite(payload.data, payload.size);
}

std::string receiveJson() {
	int len {};
	usbCommsRead(&len, 4);

	std::string linebuf(len, '\0');

	usbCommsRead((void*)linebuf.data(), len);

	return linebuf;
}

bool parseJson(nlohmann::json& command) {
	std::ofstream file((std::filesystem::current_path() / "logFile2.txt").string());
	file << "got to line " << __LINE__ << std::endl;
	std::string str = command["command"].get<std::string>();
	file << "got to line " << __LINE__ << std::endl;

	if(str == "exit")
		diagAbortWithResult(0);
	else if(str == "give state") {
		file << "got to line " << __LINE__ << std::endl;
		Result r;
		// pause ppt before and after this scope
		if(!handle.has_value()) {
			handle = std::make_optional<Handle>();
			r      = svcDebugActiveProcess(&handle.value(), PPTPID);
			if(R_FAILED(r)) {
				fatalThrow(r);
			}
		}

		nlohmann::json jsonPayload;

		PptMemory::PieceState PieceState;
		r = PptMemory::PptMemoryLock::readPieceState(handle.value(), mainAddr, PptMemory::Player::One, PieceState);
		if(R_SUCCEEDED(r)) {
			jsonPayload["Piece"]["x"]                = PieceState.x;
			jsonPayload["Piece"]["y"]                = PieceState.y;
			jsonPayload["Piece"]["rotation"]         = PieceState.rotation;
			jsonPayload["Piece"]["distanceToGround"] = PieceState.distanceToGround;
			jsonPayload["Piece"]["locked"]           = PieceState.locked;
			jsonPayload["Piece"]["isPuyomino"]       = PieceState.isPuyomino;
			jsonPayload["Piece"]["piece"]            = (int)PieceState.piece;
		}

		bool pieceInactive;
		r = PptMemory::PptMemoryLock::readPieceInactive(handle.value(), mainAddr, PptMemory::Player::One, pieceInactive);
		if(R_SUCCEEDED(r)) {
			jsonPayload["PieceInactive"] = pieceInactive;
		}

		bool pieceHeld;
		r = PptMemory::PptMemoryLock::readPieceHeld(handle.value(), mainAddr, PptMemory::Player::One, pieceHeld);
		if(R_SUCCEEDED(r)) {
			jsonPayload["PieceHeld"] = pieceHeld;
		}

		std::array<std::array<bool, 10>, 40> board;
		r = PptMemory::PptMemoryLock::readBoard(handle.value(), mainAddr, PptMemory::Player::One, board);
		if(R_SUCCEEDED(r)) {
			nlohmann::json jsonBoard(board);

			jsonPayload["Board"] = jsonBoard;
		}

		std::vector<PptMemory::Piece> queue;
		r = PptMemory::PptMemoryLock::readQueue(handle.value(), mainAddr, PptMemory::Player::One, queue);
		if(R_SUCCEEDED(r)) {
			nlohmann::json jsonQueue(queue);
			jsonPayload["Queue"] = jsonQueue;
		}

		USBResponse payload;
		std::string temp = jsonPayload.dump();
		payload.size     = temp.size();
		payload.data     = temp.data();
		file << "sending payload of size " << payload.size << std::endl;
		file << "sending payload of data " << payload.data << std::endl;

		usbCommsWrite(&payload.size, sizeof(decltype(payload.size)));
		usbCommsWrite(payload.data, payload.size);

		// unpause ppt
		if(handle.has_value()) {
			r = svcCloseHandle(*handle);
			if(R_FAILED(r)) {
				fatalThrow(r);
			}
			handle.reset();
		}

	} else if(str == "frame advance") {
		if(handle.has_value()) {
			svcCloseHandle(*handle);
			handle.reset();
		}
		handle = std::make_optional<Handle>();
		svcBreakDebugProcess(handle.value());
		eventWait(&vsync_event, UINT64_MAX);
		svcDebugActiveProcess(&handle.value(), PPTPID);

	} else if(str == "play") {
		if(handle.has_value()) {
			svcCloseHandle(*handle);
			handle.reset();
		}
	} else if(str == "pause") {
		if(!handle.has_value()) {
			handle = std::make_optional<Handle>();

			Result rc = svcDebugActiveProcess(&handle.value(), PPTPID);
			if(R_FAILED(rc)) {
				fatalThrow(rc);
			}
		}
	}
	return false;
}

// this part of the sysmod is to frame advance and peek at memory at the same time
// nothing else
int PcTalk2() {
	std::ofstream file((std::filesystem::current_path() / "logFile.txt").string());
	{
		std::ofstream file2((std::filesystem::current_path() / "logFile2.txt").string());
		file2.clear();
	}
	file.clear();
	// Result rc;
	USBResponse x;
	auto rc = viOpenDefaultDisplay(&disp);
	if(R_FAILED(rc)) {
		return 0x1;
	}
	rc = viGetDisplayVsyncEvent(&disp, &vsync_event);
	if(R_FAILED(rc)) {
		return 0x2;
	}

	while(true) {
		PPTPID = getPID();
		rc     = PptMemory::PptContext::hookppt2(PPTPID);
		if(R_FAILED(rc)) {
			rc = PptMemory::PptContext::hookppt1(PPTPID);
			if(R_FAILED(rc)) {
				continue;
			} else
				break;
		} else
			break;

		// sleep 10 seconds
		svcSleepThread(1'000'000'000);
	}
	// hooked into process!

	mainAddr = getMainNsoBase();
	while(true) {
		file << "got to line " << __LINE__ << std::endl;

		int len {};
		usbCommsRead(&len, 4);

		std::string linebuf(len, '\0');

		usbCommsRead((void*)linebuf.data(), len);

		file << linebuf << std::endl;
		nlohmann::json object;
		try {
			object = nlohmann::json::parse(linebuf);
		} catch(const std::exception& e) {
			file << "errored: " << e.what() << std::endl;
		}

		file << "got to line " << __LINE__ << std::endl;

		try {
			if(parseJson(object))
				break;
		} catch(const std::exception& e) {
			file << "errored: " << e.what() << std::endl;
		}
		file << "got to line " << __LINE__ << std::endl;
	}
	return 0;
}
