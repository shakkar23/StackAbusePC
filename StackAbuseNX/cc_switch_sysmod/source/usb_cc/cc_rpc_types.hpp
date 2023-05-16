#ifndef CC_RPC_TYPES_HPP
#define CC_RPC_TYPES_HPP

#include <stdint.h>
#include <stdbool.h>
#include <tuple>
#include <string_view>
#include <array>
#include <jsoncons/json.hpp>

namespace CcRpcTypes {
    template <typename T>
    struct Command {
        std::string_view command;
        T args;
        
        Command(T arguments) {
            command = T::command;
            args = arguments;
        }
    };

    struct Launch {
        static constexpr auto command = std::string_view("Launch");
        CcTypes::Options options;
        CcTypes::Evaluator evaluator;
    };

    struct Drop {
        static constexpr auto command = std::string_view("Drop");
        uint32_t handle;
    };

    struct RequestNextMove {
        static constexpr auto command = std::string_view("RequestNextMove");
        uint32_t handle;
        uint32_t incoming;
    };

    struct PollNextMove {
        static constexpr auto command = std::string_view("PollNextMove");
        uint32_t handle;
    };

    struct BlockNextMove {
        static constexpr auto command = std::string_view("BlockNextMove");
        uint32_t handle;
    };

    struct AddNextPiece {
        static constexpr auto command = std::string_view("AddNextPiece");
        uint32_t handle;
        CcTypes::Piece piece;
    };

    struct Reset {
        static constexpr auto command = std::string_view("Reset");
        uint32_t handle;
        std::array<std::array<bool, 10>, 40> field;
        bool b2b_active;
        uint32_t combo;
    };

    struct DefaultOptionsCommand {
        std::string_view command;
        DefaultOptionsCommand() : command(std::string_view("DefaultOptions")) {}
    };

    struct DefaultEvaluatorCommand {
        std::string_view command;
        DefaultEvaluatorCommand() : command(std::string_view("DefaultEvaluator")) {}
    };

    namespace PollNextMoveResultVariant {
        struct Ok {
            std::tuple<CcTypes::Move, CcTypes::MoveInfo> Ok;
        };

        struct Err {
            CcTypes::BotPollState Err;
        };
    }

    using PollNextMoveResult = std::variant<
        PollNextMoveResultVariant::Ok, 
        PollNextMoveResultVariant::Err
    >;
}

JSONCONS_TPL_ALL_MEMBER_TRAITS(
    1,
    CcRpcTypes::Command,
    command,
    args
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::Launch,
    options,
    evaluator
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::Drop,
    handle
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::RequestNextMove,
    handle,
    incoming
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::PollNextMove,
    handle
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::BlockNextMove,
    handle
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::AddNextPiece,
    handle,
    piece
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::Reset,
    handle,
    field,
    b2b_active,
    combo
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::DefaultOptionsCommand,
    command
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::DefaultEvaluatorCommand,
    command
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::PollNextMoveResultVariant::Ok,
    Ok
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcRpcTypes::PollNextMoveResultVariant::Err,
    Err
)

#endif
