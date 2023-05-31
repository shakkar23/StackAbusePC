#ifndef MEM_HPP
#define MEM_HPP

#include "../Shak/PcTalk.hpp"
#include <array>
#include <stdint.h>
#include <switch.h>
#include <vector>

namespace PptMemory {
	enum class Player { One = 0, Two = 1, Three = 2, Four = 3 };

	enum class Piece : u32 { S = 0, Z = 1, J = 2, L = 3, T = 4, O = 5, I = 6 };

	struct PieceState {
		Piece piece;
		u32 x;
		u32 y;
		u32 distanceToGround;
		u32 rotation;
		u8 locked;
		u8 isPuyomino;
	};

	namespace PptMemoryLock {

		Result readPointerChain(Handle handle, const std::vector<u64>& chain, void* buffer, u64 size);
		Result readBoard(Handle handle, u64 mainAddr, Player player, std::array<std::array<bool, 10>, 40>& board);
		Result readQueue(Handle handle, u64 mainAddr, Player player, std::vector<Piece>& queue);
		Result readPieceState(Handle handle, u64 mainAddr, Player player, PieceState& pieceState);
		Result readPieceInactive(Handle handle, u64 mainAddr, Player player, bool& pieceInactive);
		Result readPieceHeld(Handle handle, u64 mainAddr, Player player, bool& pieceHeld);
	};

	class PptOffsets {
	public:
		static u64 DISTANCE_TO_NEXT_PLAYER;
		static u64 BASEPOINTER;
		static u64 PLAYER_1;
		static u64 TETRIS_PLAYER_MAIN_STRUCT;
		//static u64 PIECEINACTIVE;
		static u64 BOARDPTR;
		static u64 PIECEPTR;
		static u64 PIECEHELD;
		static u64 QUEUE_STRUCT_TETRIS;
		static u64 QUEUE;
		static u64 PIECESTATE;

		// version 1-3-1
		static void ppt2v131() {
			DISTANCE_TO_NEXT_PLAYER = 24;

			BASEPOINTER               = 0x1626F60; // Puyo::ManagerBase::instance_
			PLAYER_1                  = 0x208;
			TETRIS_PLAYER_MAIN_STRUCT = 0x88;
			//PIECEINACTIVE             = 0x12C;
			BOARDPTR                  = 0x1CB8;
			PIECEPTR                  = 0x1CC0;
			PIECEHELD                 = 0x1AF8;
			QUEUE_STRUCT_TETRIS       = 0x98;
			QUEUE                     = 0x158;
		}

		// version 1-3-2
		// not looked into completely
		static void ppt2() {
			DISTANCE_TO_NEXT_PLAYER = 24;

			BASEPOINTER               = 0x1629318; // Puyo::ManagerBase::instance_
			PLAYER_1                  = 0x208;
			TETRIS_PLAYER_MAIN_STRUCT = 0x88;
			//PIECEINACTIVE             = 0x12C;
			BOARDPTR                  = 0x1CB8;
			PIECEPTR                  = 0x1CC0;
			PIECEHELD                 = 0x1AF8;
			QUEUE_STRUCT_TETRIS       = 0x98;
			QUEUE                     = 0x158;
		}

		static void ppt1() {
			DISTANCE_TO_NEXT_PLAYER   = 8;
				BASEPOINTER               = 0x6E9900; // Puyo::ManagerBase::instance_
					PLAYER_1                  = 0x350;
						TETRIS_PLAYER_MAIN_STRUCT = 0x80;
						//PIECEINACTIVE             = 0xDD; // less sus than before
						//[[[main+6E9900]+350]+80]+CC is piece state (2 is movable, 4 is just harddropped, 6 is not, everything else is in the middle of code)
						// decimal version: 
						//[[[main+7248128]+848]+128]+204
						// variable version:
						//[[[main+BASEPOINTER]+PLAYER_1]+TETRIS_PLAYER_MAIN_STRUCT]+PIECESTATE
						PIECESTATE          = 0xCC;
						BOARDPTR            = 0x398;
						PIECEPTR            = 0x3A0;
						// piece held:
						//[[[main+6E9900]+350]+80]+2F4
						//decimal version:
						//[[[main+7248128]+848]+128]+756
						PIECEHELD           = 0x2F4;
					QUEUE_STRUCT_TETRIS = 0x90;
						QUEUE               = 0x154;
						// queue
						//[[[main+6E9900]+350]+90]+154 / [[[main+6E98F8]+350]+90]+154
						//decimal version:
						//[[[main+7248128]+848]+144]+340
						//variable version
						//[[[main+BASEPOINTER]+PLAYER_1]+QUEUE_STRUCT_TETRIS]+QUEUE
						
	}
	};

	namespace PptContext {
		static u64 pid;
		static LoaderModuleInfo process_info;

		static inline Result hookppt1(u64 pid) {
			const std::array<u8, 32> TARGET_PROCESS_BUILD_ID { 0x71, 0x44, 0x66, 0x6e, 0x1c, 0x99, 0x43, 0x9d, 0xd1, 0xed, 0x0f, 0x60, 0x59, 0xda, 0xf8, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

			Result rc;
			LoaderModuleInfo process_modules[2];
			s32 module_count;

			rc = ldrDmntGetProcessModuleInfo(pid, process_modules, std::size(process_modules), &module_count);

			if(R_FAILED(rc)) {
				return rc;
			}

			PptContext::pid          = pid;
			PptContext::process_info = process_modules[module_count - 1];
			auto is_ppt              = std::equal(std::begin(PptContext::process_info.build_id), std::end(PptContext::process_info.build_id), TARGET_PROCESS_BUILD_ID.begin(), TARGET_PROCESS_BUILD_ID.end());
			if(!is_ppt) {
				return 0x1;
			}

			PptOffsets::ppt1();
			return rc;
		}

		static inline Result hookppt2(u64 pid) {

			const std::array<u8, 32> TARGET_PROCESS_BUILD_ID { 0xf8, 0x3f, 0xbd, 0x3e, 0x1e, 0x60, 0x7d, 0x15, 0x3e, 0x89, 0x13, 0x68, 0x92, 0x7c, 0x6c, 0xde, 0x42, 0x15, 0x4a, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

			Result rc;
			LoaderModuleInfo process_modules[2];
			s32 module_count;

			rc = ldrDmntGetProcessModuleInfo(pid, process_modules, std::size(process_modules), &module_count);

			if(R_FAILED(rc)) {
				return rc;
			}
			PptContext::pid          = pid;
			PptContext::process_info = process_modules[module_count - 1];
			auto is_ppt2             = std::equal(std::begin(PptContext::process_info.build_id), std::end(PptContext::process_info.build_id), TARGET_PROCESS_BUILD_ID.begin(), TARGET_PROCESS_BUILD_ID.end());
			if(!is_ppt2) {
				return 0x1;
			}
			PptOffsets::ppt2();
			return rc;
		}
		Result debug();
	}
};

#endif