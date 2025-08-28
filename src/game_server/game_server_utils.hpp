#pragma once

#include <packet_template/packet_template.hpp>

void apply_player_input(
    PlayerSnapshot& player,
    const InputDirection& input,
    float speed
);

bool outside(int x, int y);
double deg_to_rad(double deg);

bool detect_collision(
    const PlayerSnapshot& player,
    const BulletSnapshot& bullet
);
