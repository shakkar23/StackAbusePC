#ifndef USB_CC_HPP
#define USB_CC_HPP

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <optional>
#include <tuple>
#include "cc_types.hpp"
#include "cc_rpc_types.hpp"
#include <fstream>
#include <iostream>

//#define ONEINTERFACE

namespace UsbCc {
    class Handle {
        bool valid;
        uint32_t handle;
        
    public:
        Handle(Handle&& other) : handle(other.handle) {
            other.valid = false;
        }
        
        Handle& operator= (Handle&& other) {

            this->~Handle();

            handle = other.handle;
            valid = other.valid;
            other.valid = false;
            return *this;
        }

        Handle(const Handle&) = delete;
        Handle& operator= (const Handle&) = delete;

        Handle(CcTypes::Options options, CcTypes::Evaluator evaluator);
        ~Handle();

        void requestNextMove(uint32_t incoming);
        CcRpcTypes::PollNextMoveResult pollNextMove();
        std::optional<std::tuple<CcTypes::Move, CcTypes::MoveInfo>> blockNextMove();
        void addNextPiece(CcTypes::Piece piece);
        void reset(std::array<std::array<bool, 10>, 40> field, bool b2b_active, uint32_t combo);
    };

    CcTypes::Options defaultOptions();
    CcTypes::Evaluator defaultEvaluator();
}

#endif
