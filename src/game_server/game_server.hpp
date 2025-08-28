#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <atomic>
#include <socket/socket.hpp>

class GameServerMaster {
public:
    GameServerMaster(uint16_t server_port, size_t max_instances);
    ~GameServerMaster();

    bool initialize();
    void run();
    void run_async();   // It's useful if you want to run the server in the same process as the client
    void stop();
    bool wait_for_accept_ready(size_t timeout_msec, size_t max_attempts);

private:
    void accept_loop();
    void handle_client(std::shared_ptr<ClientConnection> client_conn);

    std::shared_ptr<ServerSocket>   m_server_socket;
    std::atomic<bool>               m_running;
    std::atomic<bool>               m_ready_to_accept;
    std::thread                     m_accept_thread;
    size_t                          m_max_instances;
    std::atomic<size_t>             m_active_instances;
};
