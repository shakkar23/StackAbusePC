/*
#include "app.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <array>
#include <vector>


struct GameContext {
	UsbCc::Handle bot;
	bool prevPieceInactive;
	std::optional<CcTypes::Move> move;
	PptMemory::PieceState prevPieceState;
};

Result CcSwitchSysmod::run() {
    std::ofstream log_file("sdmc:/cc_switch_sysmod_log.txt");
	Result rc;
	const std::array<CcTypes::Piece, 7> pptToCCPieces {
		CcTypes::Piece::S,
		CcTypes::Piece::Z,
		CcTypes::Piece::J,
		CcTypes::Piece::L,
		CcTypes::Piece::T,
		CcTypes::Piece::O,
		CcTypes::Piece::I
	};
	
	ViDisplay disp;
	rc = viOpenDefaultDisplay(&disp);
	if (R_FAILED(rc)) {
		log_file << "display died" << std::endl;
		return 0x1;
	}
	Event vsync_event;
	rc = viGetDisplayVsyncEvent(&disp, &vsync_event);
	if (R_FAILED(rc)) {
		log_file << "vsync died" << std::endl;
		return 0x2;
	}
	
	log_file << "Waiting for target process..." << std::endl;
	while (true) {
		if (!sysmodMainLoop()) {
			log_file << "exited sysmod" << std::endl;
			return 0x3;
		}
		svcSleepThread(1'000'000'000);
		u64 pid;
		rc = pmdmntGetApplicationProcessId(&pid);
		if (R_FAILED(rc)) {
			continue;
		}
		rc = PptMemory::PptContext::hookppt2(pid);
		if (R_FAILED(rc)) {
		    rc = PptMemory::PptContext::hookppt1(pid);
		    if (R_FAILED(rc)) {
		    	continue;
		    }
			break;
		}
		break;
	}
	log_file << "Hooked into process." << std::endl;
	PptInput::Controller controller;
	rc = PptInput::Controller::tryInit(controller);
	if (R_FAILED(rc)) {
		log_file << "couldnt init controller" << std::endl;
		return 0x4;
	}
	
	std::optional<GameContext> game {};
	while (true) {
		rc = eventWait(&vsync_event, UINT64_MAX);
		if (R_FAILED(rc)) {
			log_file << "eventWait crashed" << std::endl;
			return 0x5;
		}
		rc = debug();
		std::array<std::array<bool, 10>, 40> board;
		if (R_SUCCEEDED(rc)) {
			rc = readBoard(PptMemory::Player::One, board);
		}
		std::vector<PptMemory::Piece> queue;
		if (R_SUCCEEDED(rc)) {
			rc = readQueue(PptMemory::Player::One, queue);
		}
		bool pieceInactive = true;
		if (R_SUCCEEDED(rc)) {
			rc = readPieceInactive(PptMemory::Player::One, pieceInactive);
		}
		bool pieceHeld = false;
		if (R_SUCCEEDED(rc)) {
			rc = readPieceHeld(PptMemory::Player::One, pieceHeld);
		}
		PptMemory::PieceState piece{};
		Result rs = 0;
		if (R_SUCCEEDED(rc)) {
			rs = readPieceState(PptMemory::Player::One, piece);
			if(R_FAILED(rs))
			{
				piece = { queue.at(0), 3, 20, 18, 0, false, false};
				queue.erase(queue.begin());
			}
			pieceInactive |= piece.locked;
		} else 
			pieceInactive = true;
		
		if (R_SUCCEEDED(rc)) {
			bool initGame = !game.has_value();
			if (initGame) {
				auto options = UsbCc::defaultOptions();
				// set custom options here
				options.max_nodes = 1000;
				options.mode = CcTypes::MovementMode::HardDropOnly;
				options.use_hold = false;
				
				GameContext gameCtx{
					.bot = UsbCc::Handle(options, UsbCc::defaultEvaluator()),
					.prevPieceInactive = true,
					.move = {},
					.prevPieceState = piece
				};
				for (auto piece: queue) {
					gameCtx.bot.addNextPiece(pptToCCPieces.at((size_t)piece));
				}
				game = std::move(gameCtx);
			}
			auto& gameCtx = *game;
			if (!pieceInactive && gameCtx.prevPieceInactive) { 
				// new piece spawned, lets just wait a frame for gay frame lol
				for(int i = 0; i < PIECE_SPAWN_DELAY; i++)	
					eventWait(&vsync_event, UINT64_MAX);

				if (!initGame) {
					gameCtx.bot.addNextPiece(pptToCCPieces.at((size_t)queue.back()));
				}
				if (!gameCtx.move.has_value()) {
					gameCtx.bot.requestNextMove(0);
					auto maybeMove = gameCtx.bot.blockNextMove();
					if (!maybeMove.has_value()) {
						log_file << "maybemove died" << std::endl;
						return 0x6;
					}
					auto [move, info] = *maybeMove;
					gameCtx.move.emplace(move);
				}
			}
			if (gameCtx.move.has_value()) {
				auto& move = *gameCtx.move;
				bool finished;
				if (move.hold) {
					finished = pieceHeld;
				} else if (move.inputs.size() > 0) {
					auto movement = move.inputs.front();
					switch (movement) {
						case CcTypes::PieceMovement::Left:
							finished = piece.x < gameCtx.prevPieceState.x;
							break;
						case CcTypes::PieceMovement::Right:
							finished = piece.x > gameCtx.prevPieceState.x;
							break;
						case CcTypes::PieceMovement::Cw:
						case CcTypes::PieceMovement::Ccw:
							finished = piece.rotation != gameCtx.prevPieceState.rotation;
							break;
						default:
						log_file << "stupid movement given by the PC" << std::endl;
							return 0xBEEF1;
					}
				} else {
					finished = true;
				}
				if (finished) {
					if (move.hold) {
						move.hold = false;
						controller.state.buttons = 0;
					} else if (move.inputs.size() > 0) {
						move.inputs.erase(move.inputs.begin());
						controller.state.buttons = 0;
					} else if (!pieceInactive) {
						controller.state.buttons ^= HidNpadButton_Up;
					} else {
						gameCtx.move.reset();
						controller.state.buttons = 0;
					}
				} else {
					if (move.hold) {
						controller.state.buttons ^= HidNpadButton_R;
					} else {
						switch (move.inputs.front()) {
							case CcTypes::PieceMovement::Left:
								controller.state.buttons ^= HidNpadButton_Left;
								break;
							case CcTypes::PieceMovement::Right:
								controller.state.buttons ^= HidNpadButton_Right;
								break;
							case CcTypes::PieceMovement::Cw:
								controller.state.buttons ^= HidNpadButton_A;
								break;
							case CcTypes::PieceMovement::Ccw:
								controller.state.buttons ^= HidNpadButton_B;
								break;
							default:
							log_file << "inputs from PC died" << std::endl;
								return 0xBEEF2;
						}
					}
				}
			} else {
				controller.state.buttons = 0;
			}
			gameCtx.prevPieceState = piece;
			gameCtx.prevPieceInactive = pieceInactive;
			
			// log_file << std::bitset<64>(controller.state.buttons) << "\n";
			rc = controller.update();
			if (R_FAILED(rc)) {
				log_file << "controller update died" << std::endl;
				return 0x7;
			}
		} else {
			game.reset();
		}
	}
	log_file << "while loop died" << std::endl;
	return 0x8;
}
*/