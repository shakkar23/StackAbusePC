#include "libusb-1.0/libusb.h"

#include <hex/plugin.hpp>

#include <hex/api/content_registry.hpp>
#include <hex/ui/view.hpp>

#include "usb_comm.hpp"

class ViewExample : public hex::View
{
public:
    ViewExample() : hex::View("Example") {}
    ~ViewExample() override = default;

    void drawContent() override
    {
        if (ImGui::Begin("Example"))

        {
            {

                if (ImGui::Button("pause"))
                {
                    nlohmann::json payload;
                    payload["command"] = "pause";
                    sendJson(payload);
                }
                if (ImGui::Button("play"))
                {
                    nlohmann::json payload;
                    payload["command"] = "play";
                    sendJson(payload);
                }
                if (ImGui::Button("give state"))
                {
                    nlohmann::json payload;
                    payload["command"] = "give state";
                    sendJson(payload);
                    payload = receiveJson();

                    game_state.board = payload["Board"].get<decltype(game_state.board)>();
                    game_state.queue = payload["Queue"].get<decltype(game_state.queue)>();

                    auto &pieceState = game_state.pieceState;
                    pieceState.x = payload["Piece"]["x"].get<decltype(pieceState.x)>();
                    pieceState.y = payload["Piece"]["y"].get<decltype(pieceState.y)>();
                    pieceState.rotation = payload["Piece"]["rotation"].get<decltype(pieceState.rotation)>();
                    pieceState.distanceToGround = payload["Piece"]["distanceToGround"].get<decltype(pieceState.distanceToGround)>();
                    pieceState.locked = payload["Piece"]["locked"].get<decltype(pieceState.locked)>();
                    pieceState.isPuyomino = payload["Piece"]["isPuyomino"].get<decltype(pieceState.isPuyomino)>();
                    pieceState.piece = payload["Piece"]["piece"].get<decltype(pieceState.piece)>();
                }

                ImGui::NewLine();
                ImGui::BulletText("Pointers:\n");
                {
                    ImGui::Indent();

                    ImGui::BulletText("PieceInactive: %d", game_state.pieceInactive);
                    ImGui::BulletText("PieceState: ");

                    ImGui::Indent();

                    ImGui::BulletText("type: %c", PieceToChar(game_state.pieceState.piece));
                    ImGui::BulletText("x: %d", game_state.pieceState.x);
                    ImGui::BulletText("y: %d", game_state.pieceState.y);
                    ImGui::BulletText("distanceToGround: %d", game_state.pieceState.distanceToGround);
                    ImGui::BulletText("locked: %d", game_state.pieceState.locked);

                    ImGui::Unindent();

                    ImGui::BulletText("PieceHeld: %d", (game_state.pieceHeld));

                    ImGui::Indent();
                    ImGui::BulletText("Queue: ");
                    for (size_t i = 0; i < game_state.queue.size(); i++)
                    {
                        ImGui::BulletText("%c", PieceToChar(game_state.queue[i]));
                    }
                    ImGui::Unindent();

                    ImGui::Unindent();
                }
            }
        }
        ImGui::End();
    }
};

IMHEX_PLUGIN_SETUP("StackAbusePC", "Shakkar23", "Talks to StackAbuseNX")
{

    hex::ContentRegistry::Views::add<ViewExample>();
}
