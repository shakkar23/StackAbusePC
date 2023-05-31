#include "mem.hpp"
#include <filesystem>
#include <fstream>

	u64 PptMemory::PptOffsets::DISTANCE_TO_NEXT_PLAYER = 0;
	u64 PptMemory::PptOffsets::BASEPOINTER = 0;
	u64 PptMemory::PptOffsets::PLAYER_1 = 0;
	u64 PptMemory::PptOffsets::TETRIS_PLAYER_MAIN_STRUCT = 0;
	//u64 PptMemory::PptOffsets::PIECEINACTIVE = 0;
	u64 PptMemory::PptOffsets::PIECESTATE = 0; // 1, before piece spawns after garbage added, 2 movable, anything else is no
    u64 PptMemory::PptOffsets::BOARDPTR = 0;
	u64 PptMemory::PptOffsets::PIECEPTR = 0;
	u64 PptMemory::PptOffsets::PIECEHELD = 0;
	u64 PptMemory::PptOffsets::QUEUE_STRUCT_TETRIS = 0;
	u64 PptMemory::PptOffsets::QUEUE = 0;





Result PptMemory::PptMemoryLock::readPointerChain(Handle handle,const std::vector<u64>& chain, void* buffer, u64 size) {
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

Result PptMemory::PptMemoryLock::readBoard(Handle handle, u64 mainAddr, Player player, std::array<std::array<bool, 10>, 40>& board) {
    const std::vector<u64> chain {
        mainAddr + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::BOARDPTR,
        0x18,
        0x0
    };
    
    Result rc;

    std::array<u64, 10> columns;
    rc = readPointerChain(handle, chain, columns.data(), sizeof(columns));
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

Result PptMemory::PptMemoryLock::readQueue(Handle handle, u64 mainAddr,Player player, std::vector<Piece>& queue) {
    const std::vector<u64> chain {
        mainAddr + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::QUEUE_STRUCT_TETRIS,
        PptOffsets::QUEUE
        
			// [[[main+BASEPOINTER]+PLAYER_1]+QUEUE_STRUCT_TETRIS]+QUEUE
    };

    Result rc;

    std::array<u32, 5> rawQueue;
    rc = readPointerChain(handle, chain, rawQueue.data(), sizeof(rawQueue));
    if (R_FAILED(rc)) {
        return rc;
    }
    queue.clear();
    for (auto element: rawQueue) {
        auto piece = element;
        if (piece >= 7) {
            return 1;
        }
        queue.push_back((Piece)piece);
    }
    return rc;
}

Result PptMemory::PptMemoryLock::readPieceState(Handle handle, u64 mainAddr, Player player, PieceState& pieceState) {
    const std::vector<u64> chain {
        mainAddr + PptOffsets::BASEPOINTER,
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

    Result rc = readPointerChain(handle,chain, &rawPieceState, sizeof(decltype(rawPieceState)));
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

Result PptMemory::PptMemoryLock::readPieceInactive(Handle handle, u64 mainAddr,Player player, bool& pieceInactive) {
    const std::vector<u64> chain {
        mainAddr + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::PIECESTATE
    };
	//[[[main+BASEPOINTER]+PLAYER_1]+TETRIS_PLAYER_MAIN_STRUCT]+PIECESTATE
    u8 temp = 0;
    auto r = readPointerChain(handle, chain, (void *)&temp, sizeof(decltype(pieceInactive)));
    pieceInactive = !!temp;
    return r;
}

Result PptMemory::PptMemoryLock::readPieceHeld(Handle handle, u64 mainAddr,Player player, bool& pieceHeld) {
        const std::vector<u64> chain {
        mainAddr + PptOffsets::BASEPOINTER,
        PptOffsets::PLAYER_1 + (u64)player * PptOffsets::DISTANCE_TO_NEXT_PLAYER,
        PptOffsets::TETRIS_PLAYER_MAIN_STRUCT,
        PptOffsets::PIECEHELD
    };
    //[[[main+BASEPOINTER]+PLAYER_1]+TETRIS_PLAYER_MAIN_STRUCT]+PIECEHELD
    u8 temp = 0;
    auto r= readPointerChain(handle,chain, &pieceHeld, sizeof(decltype(pieceHeld)));

    pieceHeld = !!temp;
    return r;
}