#include "game_server_utils.hpp"

#include <cmath>        // std::sqrt
#include <algorithm>    // std::clamp
#include "game_server_constants.hpp"

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

double deg_to_rad(double deg) {
    return deg * 3.141592653589793 / 180.0;
}

bool detect_collision(
    const PlayerSnapshot& player,
    const BulletSnapshot& bullet
) {
    float dx = bullet.pos.x - player.pos.x;
    float dy = bullet.pos.y - player.pos.y;

    float dist_squared = dx * dx + dy * dy;
    float radius_sum = player.radius + bullet.radius;

    return dist_squared <= radius_sum * radius_sum;
}
