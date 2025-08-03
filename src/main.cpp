#define SDL_MAIN_HANDLED

#include <iostream>
#include "game_server/game_server.hpp"
#include "config_constants.hpp"

int main(int argc, char* args[]) {
    // Unused
    static_cast<void>(argc);
    static_cast<void>(args);

    std::cout << "[main] Hello" << "\n";

    auto game_server_master = std::make_shared<GameServerMaster>(
        socket_constants::SERVER_PORT,
        socket_constants::SERVER_MAX_INSTANCES
    );

    if (!game_server_master->initialize())
    {
        std::cerr << "[main] Failed to initialize game server master" << "\n";

        return EXIT_FAILURE;
    }

    game_server_master->run();

    std::cout << "[main] Goodbye" << "\n";

    return 0;
}