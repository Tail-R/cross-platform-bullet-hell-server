#include <iostream>

#include "game_server.hpp"

GameServerMaster::GameServerMaster(uint16_t server_port, size_t max_instances)
    : m_ready_to_accept(false)
    , m_running(false)
    , m_max_instances(max_instances)
    , m_active_instances(0)
{
    m_server_socket = std::make_shared<ServerSocket>(
        server_port
    );
}

GameServerMaster::~GameServerMaster() {
    stop();
}

bool GameServerMaster::initialize() {
    return m_server_socket->initialize();
}

void GameServerMaster::run() {
    if (!m_running)
    {
        std::cout << "[GameServerMaster] DEBUG: Game server has been started" << "\n";

        m_running = true;
        accept_loop();
    }
}

void GameServerMaster::run_async() {
    if (!m_running)
    {
        m_running = true;
        m_accept_thread = std::thread(&GameServerMaster::accept_loop, this);

        std::cout << "[GameServerMaster] DEBUG: Accept thread has been created" << "\n";
    }
}

void GameServerMaster::stop() {
    if (m_running)
    {
        m_running = false;

        m_server_socket->disconnect();

        // Accept thread
        if (m_accept_thread.joinable())
        { 
            m_accept_thread.join();

            std::cout << "[GameServerMaster] DEBUG: Accept thread has been joined" << "\n";
        }
    }
}

bool GameServerMaster::wait_for_accept_ready(size_t timeout_msec, size_t max_attempts) {
    size_t attempt = 0;

    while (true)
    {
        attempt++;

        if (m_ready_to_accept)
        {
            return true;
        }

        if (attempt > max_attempts)
        {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_msec));
    }
}

void GameServerMaster::accept_loop() {
    m_ready_to_accept = true;

    while (m_running)
    {
        std::cerr << "[GameServerMaster] DEBUG: Now accept will block this thread" << "\n";

        // Block until the client to connect
        auto client_opt = m_server_socket->accept_client();

        std::cerr << "[GameServerMaster] DEBUG: Thread is back from accept" << "\n";

        if (!client_opt.has_value())
        {
            std::cerr << "[GameServerMaster] ERROR: Invalid connection attempt from the client" << "\n";

            continue;
        }

        auto client_conn = std::make_shared<ClientConnection>(
            std::move(client_opt.value())
        );

        std::cout << "[GameServerMaster] DEBUG: client_conn accepted" << "\n";
        
        auto current = m_active_instances.load();

        // CAS (Compare-And-Swap)
        if (m_active_instances >= m_max_instances || !m_active_instances.compare_exchange_strong(current, current + 1))
        {
            std::cerr << "[GameServerMaster] DEBUG: The maximum number of instances has been reached"
                      << " and the client connection has been refused." << "\n";

            client_conn->disconnect();

            continue;
        }

        // Create thread
        auto worker_thread = std::thread([this, client_conn]() {
            handle_client(client_conn);
            m_active_instances.fetch_sub(1);
        });
            
        worker_thread.detach();

        std::cout << "[GameServerMaster] DEBUG: Game Instance has been created" << "\n"
                  << "[GameServerMaster] DEBUG: " << m_active_instances << " instances are active" << "\n";
    }

    m_ready_to_accept = false;
}
