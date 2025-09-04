#include <gtest/gtest.h>
#include <game_server/game_server_constants.hpp>
#include <game_server/game_server_utils.hpp>

/***** apply_player_input *******************************************/
TEST(ApplyPlayerInputTest, MovesCorrectlyCardinal) {
    PlayerSnapshot player{
        0, {}, {}, 0,        // id, name, state, attack_pattern
        { 0.0f, 0.0f },      // pos
        { 0.0f, 0.0f },      // vel
        0.0f,                // radius
        0.0f,                // angle
        0, 0, 0, 0           // current_spell, lives, bombs, power
    };

    float speed = 10.0f;

    apply_player_input(player, InputDirection::Up, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 0.0f);
    EXPECT_FLOAT_EQ(player.pos.y, 10.0f);

    apply_player_input(player, InputDirection::Down, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 0.0f);
    EXPECT_FLOAT_EQ(player.pos.y, 0.0f);

    apply_player_input(player, InputDirection::Right, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 10.0f);
    EXPECT_FLOAT_EQ(player.pos.y, 0.0f);

    apply_player_input(player, InputDirection::Left, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 0.0f);
    EXPECT_FLOAT_EQ(player.pos.y, 0.0f);
}

TEST(ApplyPlayerInputTest, MovesCorrectlyDiagonal) {
    PlayerSnapshot player{
        0, {}, {}, 0,
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        0.0f,
        0.0f,
        0, 0, 0, 0
    };

    float speed = 10.0f;
    constexpr float inv_sqrt2 = 0.70710678f;

    apply_player_input(player, InputDirection::UpRight, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 10.0f * inv_sqrt2);
    EXPECT_FLOAT_EQ(player.pos.y, 10.0f * inv_sqrt2);

    apply_player_input(player, InputDirection::DownLeft, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 0.0f);
    EXPECT_FLOAT_EQ(player.pos.y, 0.0f);

    apply_player_input(player, InputDirection::UpLeft, speed);
    EXPECT_FLOAT_EQ(player.pos.x, -10.0f * inv_sqrt2);
    EXPECT_FLOAT_EQ(player.pos.y,  10.0f * inv_sqrt2);

    apply_player_input(player, InputDirection::DownRight, speed);
    EXPECT_FLOAT_EQ(player.pos.x, 0.0f);
    EXPECT_FLOAT_EQ(player.pos.y, 0.0f);
}

TEST(ApplyPlayerInputTest, StopsAtBounds) {
    PlayerSnapshot player{
        0, {}, {}, 0,
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        0.0f,
        0.0f,
        0, 0, 0, 0
    };

    float speed = 10.0f;

    player.pos = { 192.0f, 224.0f };
    apply_player_input(player, InputDirection::UpRight, speed);
    EXPECT_TRUE(player.pos.x <= 192.0f && player.pos.y <= 224.0f);

    player.pos = { -192.0f, 224.0f };
    apply_player_input(player, InputDirection::UpLeft, speed);
    EXPECT_TRUE(player.pos.x >= -192.0f && player.pos.y <= 224.0f);

    player.pos = { 192.0f, -224.0f };
    apply_player_input(player, InputDirection::DownRight, speed);
    EXPECT_TRUE(player.pos.x <= 192.0f && player.pos.y >= -224.0f);

    player.pos = { -192.0f, -224.0f };
    apply_player_input(player, InputDirection::DownLeft, speed);
    EXPECT_TRUE(player.pos.x >= -192.0f && player.pos.y >= -224.0f);
}

/***** outside ******************************************************/
TEST(OutsideTest, DetectsOutside) {
    EXPECT_TRUE(outside(200, 0));
    EXPECT_TRUE(outside(-200, 0));
    EXPECT_TRUE(outside(0, 230));
    EXPECT_TRUE(outside(0, -230));

    EXPECT_FALSE(outside(0, 0));
    EXPECT_FALSE(outside(game_constants::GAME_WIDTH_HALF, 0));
    EXPECT_FALSE(outside(-game_constants::GAME_WIDTH_HALF, 0));
    EXPECT_FALSE(outside(0, game_constants::GAME_HEIGHT_HALF));
    EXPECT_FALSE(outside(0, -game_constants::GAME_HEIGHT_HALF));
}

/***** deg_to_rad ***************************************************/
TEST(DegToRadTest, ConvertsCorrectly) {
    EXPECT_DOUBLE_EQ(deg_to_rad(0.0), 0.0);
    EXPECT_DOUBLE_EQ(deg_to_rad(180.0), 3.141592653589793);
    EXPECT_DOUBLE_EQ(deg_to_rad(360.0), 2 * 3.141592653589793);
}

/***** detect_collision *********************************************/
TEST(DetectCollisionTest, CollidesCorrectly) {
    PlayerSnapshot player{
        0, {}, {}, 0,
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        5.0f,
        0.0f,
        0, 0, 0, 0
    };

    BulletSnapshot bullet{
        0,                 // id
        { 3.0f, 4.0f },    // pos
        { 0.0f, 0.0f },    // vel
        2.0f,              // radius
        0.0f,              // angle
        0,                 // damage
        {}, {}, 0, 0       // name, state, flight_pattern, owner
    };

    EXPECT_TRUE(detect_collision(player, bullet));

    BulletSnapshot bullet2{
        0,
        { 10.0f, 10.0f },
        { 0.0f, 0.0f },
        2.0f,
        0.0f,
        0,
        {}, {}, 0, 0
    };

    EXPECT_FALSE(detect_collision(player, bullet2));
}
