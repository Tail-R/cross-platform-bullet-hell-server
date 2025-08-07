#pragma once

#include <cstdint>

namespace math_constants {
    constexpr double PI         = 3.14159265358979323846;
    constexpr double TWO_PI     = PI * 2;
    constexpr double HALF_PI    = PI / 2;
}

namespace game_logic_constants {
    /*
        General configuration
    */
    constexpr float GAME_WIDTH              = 384.0f;
    constexpr float GAME_HEIGHT             = 448.0f;
    constexpr float GAME_WIDTH_HALF         = GAME_WIDTH / 2.0f;
    constexpr float GAME_HEIGHT_HALF        = GAME_HEIGHT / 2.0f;

    /*
        Player configuration
    */
    constexpr float PLAYER_RADIUS           = 5.0f;
    constexpr float PLAYER_SPEED            = 4.0f;
    constexpr float PLAYER_BULLET_RADIUS    = 5.0f;
    constexpr float PLAYER_BULLET_SPEED     = 10.0f;

    /*
        Enemy
    */
    constexpr float ENEMY_RADIUS = 10.0f;

    /*
        Enemy Bullet configurations
    */
    // Normal bullet
    constexpr float ENEMY_NORMAL_BULLET_RADIUS = 10.0f;

    // Big bullet
    constexpr float ENEMY_BIG_BULLET_RADIUS = 20.0f;

    // Rice bullet
    constexpr float ENEMY_RICE_BULLET_RADIUS = 5.0f;

    // Wedge bullet
    constexpr float ENEMY_WEDGE_BULLET_RADIUS = 10.0f;
}
