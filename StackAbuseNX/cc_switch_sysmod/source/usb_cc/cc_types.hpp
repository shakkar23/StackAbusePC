#ifndef CC_TYPES_HPP
#define CC_TYPES_HPP

#include <stdbool.h>
#include <stdint.h>
#include <tuple>
#include <optional>
#include <string>
#include <variant>
#include <array>
#include <vector>
#include <jsoncons/json.hpp>

namespace CcTypes {
    enum class Piece {
        I, O, T, L, J, S, Z
    };
    
    enum class RotationState {
        North, South, East, West
    };

    enum class TspinStatus {
        None,
        Mini,
        Full,
    };

    struct FallingPiece {
        std::tuple<Piece, RotationState> kind;
        int32_t x;
        int32_t y;
        TspinStatus tspin;
    };

    enum class PlacementKind {
        None,
        Clear1,
        Clear2,
        Clear3,
        Clear4,
        MiniTspin,
        MiniTspin1,
        MiniTspin2,
        Tspin,
        Tspin1,
        Tspin2,
        Tspin3
    };

    struct LockResult {
        PlacementKind placement_kind;
        bool locked_out;
        bool b2b;
        bool perfect_clear;
        std::optional<uint32_t> combo;
        uint32_t garbage_sent;
        std::vector<int32_t> cleared_lines;
    };

    enum class PieceMovement {
        Left,
        Right,
        Cw,
        Ccw,
        SonicDrop
    };

    enum class MovementMode {
        ZeroG,
        TwentyG,
        HardDropOnly
    };

    enum class SpawnRule {
        Row19Or20,
        Row21AndFall
    };
    
    enum class BotPollState {
        Waiting,
        Dead
    };

    struct Move {
        std::vector<PieceMovement> inputs;
        FallingPiece expected_location;
        bool hold;
    };

    struct NormalMoveInfo {
        uint32_t nodes;
        uint32_t depth;
        uint32_t original_rank;
        std::vector<std::tuple<FallingPiece, LockResult>> plan;
    };

    struct PcLoopInfo {
        uint32_t depth;
        std::vector<std::tuple<FallingPiece, LockResult>> plan;
    };

    namespace MoveInfoVariant {
        struct Normal {
            NormalMoveInfo Normal;
        };

        enum class Book {
            Book
        };

        struct PcLoop {
            PcLoopInfo PcLoop;
        };
    }

    using MoveInfo = std::variant<
        MoveInfoVariant::Normal,
        MoveInfoVariant::Book,
        MoveInfoVariant::PcLoop
    >;
    
    struct Evaluator {
        int32_t back_to_back;
        int32_t bumpiness;
        int32_t bumpiness_sq;
        int32_t row_transitions;
        int32_t height;
        int32_t top_half;
        int32_t top_quarter;
        int32_t jeopardy;
        int32_t cavity_cells;
        int32_t cavity_cells_sq;
        int32_t overhang_cells;
        int32_t overhang_cells_sq;
        int32_t covered_cells;
        int32_t covered_cells_sq;
        std::array<int32_t, 4> tslot;
        int32_t well_depth;
        int32_t max_well_depth;
        std::array<int32_t, 10> well_column;

        int32_t b2b_clear;
        int32_t clear1;
        int32_t clear2;
        int32_t clear3;
        int32_t clear4;
        int32_t tspin1;
        int32_t tspin2;
        int32_t tspin3;
        int32_t mini_tspin1;
        int32_t mini_tspin2;
        int32_t perfect_clear;
        int32_t combo_garbage;
        int32_t move_time;
        int32_t wasted_t;

        bool use_bag;
        bool timed_jeopardy;
        bool stack_pc_damage;
        std::optional<std::string> sub_name;
    };

    enum class PcPriority {
        Fastest,
        HighestAttack
    };

    struct Options {
        MovementMode mode;
        SpawnRule spawn_rule;
        bool use_hold;
        bool speculate;
        std::optional<PcPriority> pcloop;
        uint32_t min_nodes;
        uint32_t max_nodes;
        uint32_t threads;
    };
}

JSONCONS_ENUM_TRAITS(
    CcTypes::Piece,
    I,
    O,
    T,
    L,
    J,
    S,
    Z
)

JSONCONS_ENUM_TRAITS(
    CcTypes::RotationState,
    North,
    South,
    East,
    West
)

JSONCONS_ENUM_TRAITS(
    CcTypes::TspinStatus,
    None,
    Mini,
    Full
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::FallingPiece,
    kind,
    x,
    y,
    tspin
)

JSONCONS_ENUM_TRAITS(
    CcTypes::PlacementKind,
    None,
    Clear1,
    Clear2,
    Clear3,
    Clear4,
    MiniTspin,
    MiniTspin1,
    MiniTspin2,
    Tspin,
    Tspin1,
    Tspin2,
    Tspin3
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::LockResult,
    placement_kind,
    locked_out,
    b2b,
    perfect_clear,
    combo,
    garbage_sent,
    cleared_lines
)

JSONCONS_ENUM_TRAITS(
    CcTypes::PieceMovement,
    Left,
    Right,
    Cw,
    Ccw,
    SonicDrop
)

JSONCONS_ENUM_TRAITS(
    CcTypes::MovementMode,
    ZeroG,
    TwentyG,
    HardDropOnly
)

JSONCONS_ENUM_TRAITS(
    CcTypes::SpawnRule,
    Row19Or20,
    Row21AndFall
)

JSONCONS_ENUM_TRAITS(
    CcTypes::BotPollState,
    Waiting,
    Dead
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::Move,
    inputs,
    expected_location,
    hold
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::NormalMoveInfo,
    nodes,
    depth,
    original_rank,
    plan
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::PcLoopInfo,
    depth,
    plan
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::MoveInfoVariant::Normal,
    Normal
)

JSONCONS_ENUM_TRAITS(
    CcTypes::MoveInfoVariant::Book,
    Book
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::MoveInfoVariant::PcLoop,
    PcLoop
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::Evaluator,
    back_to_back,
    bumpiness,
    bumpiness_sq,
    row_transitions,
    height,
    top_half,
    top_quarter,
    jeopardy,
    cavity_cells,
    cavity_cells_sq,
    overhang_cells,
    overhang_cells_sq,
    covered_cells,
    covered_cells_sq,
    tslot,
    well_depth,
    max_well_depth,
    well_column,

    b2b_clear,
    clear1,
    clear2,
    clear3,
    clear4,
    tspin1,
    tspin2,
    tspin3,
    mini_tspin1,
    mini_tspin2,
    perfect_clear,
    combo_garbage,
    move_time,
    wasted_t,

    use_bag,
    timed_jeopardy,
    stack_pc_damage,
    sub_name
)

JSONCONS_ENUM_TRAITS(
    CcTypes::PcPriority,
    Fastest,
    HighestAttack
)

JSONCONS_ALL_MEMBER_TRAITS(
    CcTypes::Options,
    mode,
    spawn_rule,
    use_hold,
    speculate,
    pcloop,
    min_nodes,
    max_nodes,
    threads
)

#endif
