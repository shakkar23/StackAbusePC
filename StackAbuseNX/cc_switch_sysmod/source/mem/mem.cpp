#include "mem.hpp"


	u64 PptMemory::PptOffsets::DISTANCE_TO_NEXT_PLAYER = 0;
	u64 PptMemory::PptOffsets::BASEPOINTER = 0;
	u64 PptMemory::PptOffsets::PLAYER_1 = 0;
	u64 PptMemory::PptOffsets::TETRIS_PLAYER_MAIN_STRUCT = 0;
	u64 PptMemory::PptOffsets::PIECEINACTIVE = 0;
	u64 PptMemory::PptOffsets::PIECESTATE = 0; // 1, before piece spawns after garbage added, 2 movable, anything else is no
    u64 PptMemory::PptOffsets::BOARDPTR = 0;
	u64 PptMemory::PptOffsets::PIECEPTR = 0;
	u64 PptMemory::PptOffsets::PIECEHELD = 0;
	u64 PptMemory::PptOffsets::QUEUE_STRUCT_TETRIS = 0;
	u64 PptMemory::PptOffsets::QUEUE = 0;



Result PptMemory::PptContext::hookppt2(u64 pid) {
    const std::array<u8, 32> TARGET_PROCESS_BUILD_ID {
		0xf8, 0x3f, 0xbd, 0x3e, 0x1e, 0x60, 0x7d, 0x15, 
        0x3e, 0x89, 0x13, 0x68, 0x92, 0x7c, 0x6c, 0xde, 
        0x42, 0x15, 0x4a, 0xd2, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
    
    Result rc;
    LoaderModuleInfo process_modules[2];
    s32 module_count;

    rc = ldrDmntGetProcessModuleInfo(
        pid,
        process_modules,
        std::size(process_modules),
        &module_count
    );
    
    if (R_FAILED(rc)) {
        return rc;
    }
    PptContext::pid = pid;
    PptContext::process_info = process_modules[module_count - 1];
    auto is_ppt2 = std::equal(std::begin(PptContext::process_info.build_id),std::end(PptContext::process_info.build_id),TARGET_PROCESS_BUILD_ID.begin(),TARGET_PROCESS_BUILD_ID.end());
    if (!is_ppt2) {

        return 1;
    }
	PptOffsets::ppt2();
	return rc;
}

Result PptMemory::PptContext::hookppt1(u64 pid) {
	const std::array<u8, 32> TARGET_PROCESS_BUILD_ID {
        0x71, 0x44, 0x66, 0x6e, 0x1c, 0x99, 0x43, 0x9d,
        0xd1, 0xed, 0x0f, 0x60, 0x59, 0xda, 0xf8, 0xd5,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

	Result rc;
	LoaderModuleInfo process_modules[2];
	s32 module_count;

	rc = ldrDmntGetProcessModuleInfo(pid, process_modules, std::size(process_modules), &module_count);

	if(R_FAILED(rc)) {
		return rc;
	}
	
	PptContext::pid                = pid;
	PptContext::process_info       = process_modules[module_count - 1];
	auto is_ppt = std::equal(std::begin(PptContext::process_info.build_id), std::end(PptContext::process_info.build_id), TARGET_PROCESS_BUILD_ID.begin(), TARGET_PROCESS_BUILD_ID.end());
	if(!is_ppt) {
		return 1;
	}

	PptOffsets::ppt1();
	return rc;
}

Result PptMemory::PptContext::debug() {
    PptMemoryLock::process_info = process_info;
    return svcDebugActiveProcess(&PptMemoryLock::handle, PptContext::pid);
}


Result PptMemory::PptMemoryLock::readPointerChain(const std::vector<u64>& chain, void* buffer, u64 size) {
	if (chain.size() == 0) {
		return 1;
	}
	u64 ptr = chain.at(0);
	for (size_t i = 1; i < chain.size(); i++) {
		Result rc = svcReadDebugProcessMemory(&ptr, handle, ptr, sizeof(decltype(ptr)));
		if (R_FAILED(rc)) {
			return rc;
		}
		ptr += chain.at(i);
	}
	return svcReadDebugProcessMemory(buffer, handle, ptr, size);
}

Result PptMemory::PptMemoryLock::readBoard(Player player, std::array<std::array<bool, 10>, 40>& board) {
    const std::vector<u64> chain {
        process_info.base_address + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::BOARDPTR,
        0x18,
        0x0
    };
    
    Result rc;

    std::array<u64, 10> columns;
    rc = readPointerChain(chain, columns.data(), sizeof(columns));
    if (R_FAILED(rc)) {
        return rc;
    }

    for (size_t x = 0; x < columns.size(); x++) {
        std::array<int32_t, 40> column;
        rc = svcReadDebugProcessMemory(column.data(), handle, columns.at(x), sizeof(column));
        if (R_FAILED(rc)) {
            break;
        }
        for (size_t y = 0; y < column.size(); y++) {
            board.at(y).at(x) = column.at(y) != -1;
        }
    }
    return rc;
}

Result PptMemory::PptMemoryLock::readQueue(Player player, std::vector<Piece>& queue) {
    const std::vector<u64> chain {
        process_info.base_address + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::QUEUE_STRUCT_TETRIS,
        PptOffsets::QUEUE
    };

    Result rc;

    std::array<std::array<u8, 8>, 5> rawQueue;
    rc = readPointerChain(chain, rawQueue.data(), sizeof(rawQueue));
    if (R_FAILED(rc)) {
        return rc;
    }
    queue.clear();
    for (auto element: rawQueue) {
        auto piece = element.at(0);
        if (piece >= 7) {
            return 1;
        }
        queue.push_back((Piece)piece);
    }
    return rc;
}

Result PptMemory::PptMemoryLock::readPieceState(Player player, PieceState& pieceState) {
    const std::vector<u64> chain {
        process_info.base_address + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::PIECEPTR,
        0x8
    };

    struct RawPieceState {
        Piece piece;
        u32 x;
        u32 y;
        u32 distanceToGround;
        u32 rotation;
        bool locked;
    } __attribute__((packed));
    RawPieceState rawPieceState;

    Result rc = readPointerChain(chain, &rawPieceState, sizeof(decltype(rawPieceState)));
    if (R_SUCCEEDED(rc)) {
        pieceState = {
            .piece = rawPieceState.piece,
            .x = rawPieceState.x,
            .y = rawPieceState.y,
            .distanceToGround = rawPieceState.distanceToGround,
            .rotation = rawPieceState.rotation,
            .locked = rawPieceState.locked,
            .isPuyomino = false
        };
    }
    return rc;
}

Result PptMemory::PptMemoryLock::readPieceInactive(Player player, bool& pieceInactive) {
    const std::vector<u64> chain {
        process_info.base_address + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::PIECEINACTIVE
    };

    return readPointerChain(chain, &pieceInactive, sizeof(decltype(pieceInactive)));
}

Result PptMemory::PptMemoryLock::readPieceHeld(Player player, bool& pieceHeld) {
    const std::vector<u64> chain {
        process_info.base_address + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::PIECEHELD
    };

    return readPointerChain(chain, &pieceHeld, sizeof(decltype(pieceHeld)));
}
