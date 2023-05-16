#ifndef MEM_HPP
#define MEM_HPP


#include <switch.h>
#include <vector>
#include <array>
#include <stdint.h>
#include "../Shak/PcTalk.hpp"

namespace PptMemory {
    enum class Player {
        One = 0,
        Two = 1,
        Three = 2,
        Four = 3
    };

    enum class Piece : u32 {
        S = 0,
        Z = 1,
        J = 2,
        L = 3,
        T = 4,
        O = 5,
        I = 6
    };

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
        static Handle handle{};
        static LoaderModuleInfo process_info{};

        Result readPointerChain(const std::vector<u64>& chain, void* buffer, u64 size);
        Result readBoard(Player player, std::array<std::array<bool, 10>, 40>& board);
        Result readQueue(Player player, std::vector<Piece>& queue);
        Result readPieceState(Player player, PieceState& queue);
        Result readPieceInactive(Player player, bool& pieceInactive);
        Result readPieceHeld(Player player, bool& pieceHeld);
    };

    class PptOffsets {
        public:
        static u64 DISTANCE_TO_NEXT_PLAYER;
        static u64 BASEPOINTER;
        static u64 PLAYER_1;
        static u64 TETRIS_PLAYER_MAIN_STRUCT;
        static u64 PIECEINACTIVE;
        static u64 BOARDPTR;
        static u64 PIECEPTR;
		static u64 PIECEHELD;
        static u64 QUEUE_STRUCT_TETRIS;
		static u64 QUEUE;
		static u64 PIECESTATE;

		//version 1-3-1
		static void ppt2v131() {
            DISTANCE_TO_NEXT_PLAYER = 24;
            
            BASEPOINTER = 0x1626F60;//Puyo::ManagerBase::instance_
                PLAYER_1 = 0x208;
                    TETRIS_PLAYER_MAIN_STRUCT = 0x88;
                        PIECEINACTIVE = 0x12C;
                        BOARDPTR = 0x1CB8;
                        PIECEPTR = 0x1CC0;
					    PIECEHELD = 0x1AF8;
                    QUEUE_STRUCT_TETRIS = 0x98;
                        QUEUE = 0x158;
		}

        // version 1-3-2
        // not looked into completely
        static void ppt2() {
            DISTANCE_TO_NEXT_PLAYER = 24; 
            
            BASEPOINTER = 0x1629318; //Puyo::ManagerBase::instance_
                PLAYER_1 = 0x208;
                    TETRIS_PLAYER_MAIN_STRUCT = 0x88;
                        PIECEINACTIVE = 0x12C;
                        BOARDPTR = 0x1CB8;
                        PIECEPTR = 0x1CC0;
					    PIECEHELD = 0x1AF8;
                    QUEUE_STRUCT_TETRIS = 0x98;
                        QUEUE = 0x158;
		}

        static void ppt1(){
            DISTANCE_TO_NEXT_PLAYER = 8;
            BASEPOINTER = 0x6E9900; //Puyo::ManagerBase::instance_
                PLAYER_1 = 0x350;
                    TETRIS_PLAYER_MAIN_STRUCT = 0x80;
                        PIECEINACTIVE = 0xDD; // less sus than before
                        //[[[main+6E9900]+350]+80]+CC is piece state (2 is movable, 6 is not, everything else is in the middle of code)
                        PIECESTATE = 0xCC;
                        BOARDPTR = 0x398;
                        PIECEPTR = 0x3A0;
                        PIECEHELD = 0x2F4;
                    QUEUE_STRUCT_TETRIS = 0x90;
                        QUEUE = 0x150;
        }

    };


	namespace PptContext {
		static u64 pid;
        static LoaderModuleInfo process_info;

        static Result hookppt2(u64 pid);
        static Result hookppt1(u64 pid);
		Result debug();
	};
}

#endif