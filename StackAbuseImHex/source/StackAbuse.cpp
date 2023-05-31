#include "libusb-1.0/libusb.h"

#include <hex/plugin.hpp>

#include <hex/api/content_registry.hpp>
#include <hex/ui/view.hpp>
#include <hex/api/imhex_api.hpp>

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
                    // std::cout << payload;
                    game_state.board = payload["Board"].get<decltype(game_state.board)>();
                    game_state.queue = payload["Queue"].get<decltype(game_state.queue)>();
                    game_state.pieceHeld = payload["PieceHeld"].get<decltype(game_state.pieceHeld)>();

                    auto &pieceState = game_state.pieceState;
                    pieceState.x = payload["Piece"]["x"].get<decltype(pieceState.x)>();
                    pieceState.y = payload["Piece"]["y"].get<decltype(pieceState.y)>();
                    pieceState.rotation = payload["Piece"]["rotation"].get<decltype(pieceState.rotation)>();
                    pieceState.distanceToGround = payload["Piece"]["distanceToGround"].get<decltype(pieceState.distanceToGround)>();
                    pieceState.locked = payload["Piece"]["locked"].get<decltype(pieceState.locked)>();
                    pieceState.isPuyomino = payload["Piece"]["isPuyomino"].get<decltype(pieceState.isPuyomino)>();
                    pieceState.piece = payload["Piece"]["piece"].get<decltype(pieceState.piece)>();
                }
                if (ImGui::Button("frame advance"))
                {

                    nlohmann::json payload;
                    payload["command"] = "frame advance";
                    sendJson(payload);
                }
                ImGui::Text("chain size: %d", (int)chain.size());
                ImGui::Checkbox("dereference last offset", &dereference_last_offset);

                if (ImGui::Button("get memory"))
                {

                    nlohmann::json payload;
                    payload["command"] = "get memory";
                    payload["size"] = data_size;

                    std::vector<s32> new_chain;

                    for (auto obj : chain)
                    {
                        new_chain.push_back(obj);

                        if (obj == 0)
                            break;
                    }
                    if (new_chain.size() > 1)
                    {
                        std::cout << "chain size: " << new_chain.size() << std::endl;
                        // if we dereference the last offset, we need to keep the 0 at the end of the chain
                        if (new_chain[new_chain.size() - 1] == 0)
                            if (dereference_last_offset)
                                new_chain.push_back(0);
                    }
                    std::cout << "chain size real: " << new_chain.size() << std::endl;
                    payload["chain"] = new_chain;

                    sendJson(payload);
                    payload.clear();
                    payload = receiveJson();

                    hex::prv::Provider *provider = hex::ImHexApi::Provider::get();

                    if (payload.contains("error"))
                    {

                        provider->resize(sizeof(int));
                        int error = payload["error"].get<int>();
                        provider->write(0, &error, sizeof(int));
                    }
                    else
                    {
                        auto data = payload["data"].get<std::vector<u8>>();

                        provider->resize(data.size());
                        provider->write(0, data.data(), data.size());
                    }
                }

                ImGui::InputInt("data size", &data_size);
                ImGui::Text("chain: ");
                // go through the chain and delete all the 0s at the end except the last one

                // this should transform a vector of [1,0] into [1,0]
                // but should transform a vector of [1,0,0] into [1,0]

                // display every element of the chain and allow modification of the chain
                for (size_t i = 0; i < chain.size() - 1; i++)
                {
                    std::string label = "offset " + std::to_string(i);
                    ImGui::InputInt(label.c_str(), &chain[i]);
                }
                {
                    std::string label = "offset " + std::to_string(chain.size() - 1);
                    if (ImGui::InputInt(label.c_str(), &chain[chain.size() - 1]))
                        chain.push_back(0);
                }
                while (chain.size() > 1 && chain[chain.size() - 1] == 0 && chain[chain.size() - 2] == 0)
                    chain.pop_back();

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

                    // game_state.pieceHeld is boolean
                    ImGui::BulletText("PieceHeld: %d", game_state.pieceHeld);

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

private:
    std::vector<s32> chain = {0};
    s32 data_size = 0x200;
    bool dereference_last_offset = false;
};

IMHEX_PLUGIN_SETUP("StackAbusePC", "Shakkar23", "Talks to StackAbuseNX")
{

    hex::ContentRegistry::Views::add<ViewExample>();
}
