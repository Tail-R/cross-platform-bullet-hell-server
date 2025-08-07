#include <iostream>
#include <cmath>        // std::sqrt
#include <algorithm>    // std::clamp
#include <random>
#include "game_server.hpp"
#include "game_server_constants.hpp"
#include <packet_stream/packet_stream.hpp>
#include <packet_template/packet_template.hpp>

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

void apply_player_input(
    PlayerSnapshot& player,
    const InputDirection& input,
    float speed
) {
    constexpr float inv_sqrt2 = 0.70710678f;

    float dx = 0.0f;
    float dy = 0.0f;

    switch (input)
    {
        case InputDirection::Up:        { dy = +1.0f;                       break; }
        case InputDirection::Down:      { dy = -1.0f;                       break; }
        case InputDirection::Right:     { dx = +1.0f;                       break; }
        case InputDirection::Left:      { dx = -1.0f;                       break; }
        case InputDirection::UpRight:   { dx = +inv_sqrt2; dy = +inv_sqrt2; break; }
        case InputDirection::DownRight: { dx = +inv_sqrt2; dy = -inv_sqrt2; break; }
        case InputDirection::UpLeft:    { dx = -inv_sqrt2; dy = +inv_sqrt2; break; }
        case InputDirection::DownLeft:  { dx = -inv_sqrt2; dy = -inv_sqrt2; break; }

        case InputDirection::Stop:
        default: return;
    }

    player.pos.x = std::clamp(player.pos.x + dx * speed, -192.0f, 192.f);
    player.pos.y = std::clamp(player.pos.y + dy * speed, -224.0f, 224.0f);
}

bool outside(int x, int y) {
    const auto expr_1 = x >  game_constants::GAME_WIDTH_HALF;
    const auto expr_2 = x < -game_constants::GAME_WIDTH_HALF;
    const auto expr_3 = y >  game_constants::GAME_HEIGHT_HALF;
    const auto expr_4 = y < -game_constants::GAME_HEIGHT_HALF;

    return expr_1 || expr_2 || expr_3 || expr_4;
}

constexpr double deg_to_rad(double deg) {
    return deg * 3.141592653589793 / 180.0;
}

constexpr bool detect_collision(
    const PlayerSnapshot& player,
    const BulletSnapshot& bullet
) {
    float dx = bullet.pos.x - player.pos.x;
    float dy = bullet.pos.y - player.pos.y;

    float dist_squared = dx * dx + dy * dy;
    float radius_sum = player.radius + bullet.radius;

    return dist_squared <= radius_sum * radius_sum;
}

void GameServerMaster::handle_client(std::shared_ptr<ClientConnection> client_conn) {
    PacketStreamServer packet_stream(client_conn);
    packet_stream.start();

    // A closure that waits for a specific packet to arrive.
    auto wait_packet = [&](PayloadType payload_type, size_t timeout_msec, size_t max_attempts) -> bool {
        for (size_t attempt = 0; attempt < max_attempts; attempt++)
        {
            std::optional<Packet> packet_opt = packet_stream.poll_packet();

            if (packet_opt.has_value())
            {
                if (packet_opt.value().header.payload_type == payload_type)
                {
                    return true;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_msec));
        }

        return false;
    };

    FrameSnapshot frame = {};

    // Stage
    frame.stage.id = 0;
    frame.stage.name = StageName::Default;

    // Player
    PlayerSnapshot player = {};
    player.id = 0;
    player.name = PlayerName::Default;
    player.pos = {
        0,
        -120
    };
    player.vel = {
        game_constants::PLAYER_SPEED,
        game_constants::PLAYER_SPEED
    };
    player.radius = game_constants::PLAYER_RADIUS;
    player.lives = 1;
    frame.player_vector.push_back(player);
    frame.player_count = 1; 

    // Enemy
    EnemySnapshot enemy = {};
    enemy.id = 0;
    enemy.name = EnemyName::Default;
    enemy.pos = {
        0,
        120
    };
    enemy.vel = {
        2,
        2
    };
    enemy.radius = game_constants::ENEMY_RADIUS;
    frame.enemy_vector.push_back(enemy);
    frame.enemy_count = 1;

    ArrowState arrow_state = {};

    // 1sec / Target FPS
    constexpr auto target_frame_duration = std::chrono::duration<double>(1.0 / 60);

    // Wait for client hello
    if (!wait_packet(PayloadType::ClientHello, 1000, 10))
    {
        std::cout << "[GameServerMaster] DEBUG: Client hello timeout" << "\n";

        return;
    }

    // Send server accept
    packet_stream.send_packet(make_packet<ServerAccept>({}));
    std::cout << "[GameServerMaster] DEBUG: Server accept has been sent" << "\n";

    // Wait for client game request
    if (!wait_packet(PayloadType::ClientGameRequest, 1000, 1000))
    {
        std::cout << "[GameServerMaster] DEBUG: Client game request timeout" << "\n";

        return;
    }

    // Send server game response
    packet_stream.send_packet(make_packet<ServerGameResponse>({}));
    std::cout << "[GameServerMaster] DEBUG: Server game response has been sent" << "\n";

    auto quit = false;
    auto frame_count = 0;
    auto bullet_id = 0;

    std::random_device rd;
    std::mt19937 gen(rd());

    // Create a distribution in the range [0, 359]
    std::uniform_int_distribution<> dist(0, 359);

    // Game logic loop
    while (m_running && !quit)
    {
        frame_count++;

        auto frame_start = std::chrono::steady_clock::now();

        // Check if the recv thread is alive
        const auto expr_1 = packet_stream.get_recv_exception() == nullptr;
        const auto expr_2 = packet_stream.is_running();

        if (!expr_1 || !expr_2)
        {
            break;
        }

        // Process the packet queue
        while (true && !quit)
        {
            std::optional<Packet> packet_opt = packet_stream.poll_packet();

            if (!packet_opt.has_value())
            {
                break;
            }

            Packet packet = std::move(*packet_opt);

            switch (packet.header.payload_type)
            {
                case PayloadType::ClientInput:
                {
                    const auto input_snapshot = std::get<ClientInput>(packet.payload);
                    arrow_state.held |= input_snapshot.game_input.arrows.pressed;
                    arrow_state.held &= ~input_snapshot.game_input.arrows.released;

                    break;
                }

                case PayloadType::ClientGoodbye:
                {
                    std::cout << "[GameServerMaster] DEBUG: Received client goodbye" << "\n";

                    const auto packet = make_packet<ServerGoodbye>({});
                    packet_stream.send_packet(packet);

                    quit = true;

                    continue;
                }

                default:
                {
                    std::cerr << "[GameServerMaster] DEBUG: Unexpected message type: "
                            << static_cast<uint32_t>(packet.header.payload_type) << "\n";
                    break;
                }
            }
        }

        /*
            Logic update
        */
        // Update player
        if (frame.player_vector[0].lives > 0)
        {
            auto direction = get_direction_from_arrows(arrow_state);
            apply_player_input(frame.player_vector[0], direction, frame.player_vector[0].vel.x);
        }
        
        // Update enemy direction
        auto& enemy = frame.enemy_vector[0];
        if (enemy.pos.x > game_constants::GAME_WIDTH_HALF)
        {
            enemy.vel.x = -2;
        }
        else if (enemy.pos.x < -game_constants::GAME_WIDTH_HALF)
        {
            enemy.vel.x = 2;
        }
        
        if (enemy.pos.y > game_constants::GAME_HEIGHT_HALF)
        {
            enemy.vel.y = -2;
        }
        else if (enemy.pos.y < 60)
        {
            enemy.vel.y = 2;
        }

        // Move enemy
        if ((frame_count % 360) < 120)
        {
            frame.enemy_vector[0].pos.x += frame.enemy_vector[0].vel.x;
            frame.enemy_vector[0].pos.y += frame.enemy_vector[0].vel.y;
        }

        // Circle shot
        if ((frame_count % 360) > 120 && frame_count % 60 == 0)
        {
            constexpr double two_pi = 2*math_constants::PI;
            constexpr double step = two_pi / 8;

            const float rad_offset = static_cast<float>(deg_to_rad(frame_count % 360));

            for (double r = 0; r < two_pi; r += step)
            {
                auto bullet = BulletSnapshot{};

                bullet.id = bullet_id++;
                bullet.name = BulletName::BigRed;
                bullet.pos = frame.enemy_vector[0].pos;
                bullet.vel = {
                    2 * std::cos(rad_offset + static_cast<float>(r)),
                    2 * std::sin(rad_offset + static_cast<float>(r))
                };
                bullet.radius = game_constants::ENEMY_BIG_BULLET_RADIUS;
                bullet.angle = std::atan2(bullet.vel.y, bullet.vel.x) - math_constants::HALF_PI;
                frame.bullet_vector.push_back(bullet);
                frame.bullet_count++;
            }
        }

        // Homing shot
        if ((frame_count % 360) > 240 && frame_count % 8 == 0)
        {
            float vx = frame.player_vector[0].pos.x - frame.enemy_vector[0].pos.x;
            float vy = frame.player_vector[0].pos.y - frame.enemy_vector[0].pos.y;

            float length = std::sqrt(vx * vx + vy * vy);

            float dx = 1.0f;
            float dy = 1.0f;

            if (length != 0.0f)
            {
                dx = vx / length * 4.0f;
                dy = vy / length * 4.0f;
            }

            auto bullet = BulletSnapshot{};

            bullet.id = bullet_id++;
            bullet.name = BulletName::WedgeRed;
            bullet.pos = frame.enemy_vector[0].pos;
            bullet.vel = { dx, dy };
            bullet.radius = game_constants::ENEMY_WEDGE_BULLET_RADIUS;
            bullet.angle = std::atan2(dy, dx) - math_constants::HALF_PI;
            frame.bullet_vector.push_back(bullet);
            frame.bullet_count++;
        }

        // Spiral shot
        if ((frame_count % 360) > 120 && frame_count % 6 == 0)
        {
            constexpr double two_pi = 2*math_constants::PI;
            constexpr double step = two_pi / 7;

            const float rad_offset = static_cast<float>(deg_to_rad(frame_count % 360));
            size_t sprite_index = 0;

            for (double r = 0; r < two_pi; r += step)
            {
                sprite_index++;

                const float dx = cos(rad_offset + r) * 1.5;
                const float dy = sin(rad_offset + r) * 1.5;

                auto bullet = BulletSnapshot{};

                bullet.id = bullet_id++;
                bullet.name = static_cast<BulletName>(
                    static_cast<size_t>(BulletName::RiceRed) + (sprite_index % 8)
                );
                bullet.pos = frame.enemy_vector[0].pos;
                bullet.vel = { dx, dy };
                bullet.radius = game_constants::ENEMY_RICE_BULLET_RADIUS;
                bullet.angle = std::atan2(dy, dx) - math_constants::HALF_PI;
                frame.bullet_vector.push_back(bullet);
                frame.bullet_count++;
            }
        }

        // Random shot
        if (frame_count % 60 == 0 && frame_count > 120)
        {
            size_t number_of_rand_shot = 7;

            for (size_t i = 0; i < number_of_rand_shot; i++)
            {
                const double rand_1 = dist(gen);
                const double rand_2 = dist(gen);
                const float rad_1 = static_cast<float>(deg_to_rad(rand_1));
                const float rad_2 = static_cast<float>(deg_to_rad(rand_2));
                const float dx = cos(rad_1) * 2;
                const float dy = sin(rad_2) * 2;

                auto bullet = BulletSnapshot{};

                bullet.id = bullet_id++;
                bullet.name = static_cast<BulletName>(i % 8 + 1);
                bullet.pos = frame.enemy_vector[0].pos;
                bullet.vel = { dx, dy };
                bullet.radius = game_constants::ENEMY_NORMAL_BULLET_RADIUS;
                bullet.angle = std::atan2(dy, dx) - math_constants::HALF_PI;
                frame.bullet_vector.push_back(bullet);
                frame.bullet_count++;
            }
        }

        // Update and detect collision of bullets
        for (auto& bullet : frame.bullet_vector)
        {
            bullet.pos.x += bullet.vel.x;
            bullet.pos.y += bullet.vel.y;

            const auto collided = detect_collision(
                frame.player_vector[0],
                bullet
            );
            
            if (collided)
            {
                frame.player_vector[0].lives = 0;
                frame.state = frame.state | GameState::GameOver;
            }
        }

        // Remove the dead bullets
        auto& vec = frame.bullet_vector;
        for (size_t i = 0; i < vec.size(); )
        {
            if (outside(vec[i].pos.x, vec[i].pos.y))
            {
                std::swap(vec[i], vec.back());
                vec.pop_back();
                frame.bullet_count--;
            }
            else
            {
                i++;
            }
        }

        // Send frame
        const auto packet = make_packet<FrameSnapshot>(frame);
        packet_stream.send_packet(packet);

        // Adjust the frame rate
        auto frame_end = std::chrono::steady_clock::now();
        auto frame_duration = frame_end - frame_start;

        if (frame_duration < target_frame_duration)
        {
            std::this_thread::sleep_for(target_frame_duration - frame_duration);
        }
        else
        {
            std::cerr << "[GameServerMaster] ERROR: The game logic update could not be completed within the specified FPS" << "\n";
        }
    }

    packet_stream.stop();
    client_conn->disconnect();

    std::cout << "[GameServerMaster] DEBUG: Game Instance has been terminated successfully" << "\n";
}