// CC-PC.cpp : Defines the entry point for the application.
//

#include "CC-PC.h"

using namespace std;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


enum class Piece : u32 { S = 0, Z = 1, J = 2, L = 3, T = 4, O = 5, I = 6 };
char PieceToChar(Piece piece) {
    switch (piece)
    {
		case Piece::S: return 'S';
		case Piece::Z: return 'Z';
		case Piece::J: return 'J';
		case Piece::L: return 'L';
		case Piece::T: return 'T';
		case Piece::O: return 'O';
		case Piece::I: return 'I';
	}
    return 'N';
}
struct PieceState {
    Piece piece;
    u32 x;
    u32 y;
    u32 distanceToGround;
    u32 rotation;
    u8 locked;
    u8 isPuyomino;
};

class GameState {
public:
    std::array<std::array<bool, 10>, 40> board{};
    std::vector<Piece> queue;
    PieceState pieceState;
    bool pieceHeld;
    bool pieceInactive;
}game_state;

// Main code
int main(int, char**)
{
    while (true) {
        if (!initlibusb()) {
            
            // didnt init for some reason,
            // could have been that the switch was not setup correctly, or plugged at 
            // wait 5 seconds and try to connect
            std::cout << "trying again in 5 seconds!" << std::endl << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }
        else
        {
			break;
		}
    }
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Our state

    ImVec4 clear_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        {
            ImGui::Begin("Tetris Render WIP");

            ImVec2 winPos = ImGui::GetWindowPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(ImVec2(winPos.x + 50, winPos.y + 50), ImVec2(winPos.x + 100, winPos.y + 100), ImColor(255, 0, 0));

            ImGui::End();
        }


        {

            ImGui::Begin("main thingy");



            if (ImGui::Button("pause")) {
                nlohmann::json payload;
                payload["command"] = "pause";
                sendJson(payload);
            }
            if (ImGui::Button("play")) {
                nlohmann::json payload;
                payload["command"] = "play";
                sendJson(payload);
            }
            if (ImGui::Button("give state")) {
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
                        for (int i = 0; i < game_state.queue.size(); i++) {
				        	ImGui::BulletText("%c", PieceToChar(game_state.queue[i]));
				        }
                    ImGui::Unindent();


                ImGui::Unindent();
            }
            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}