#include <gtest/gtest.h>
#include "game_logger/game_logger.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

TEST(GameLoggerTest, WritesLogToCacheAndData) {
    // Create temp dir
    fs::path tmp_cache = fs::temp_directory_path() / "log_cache";
    fs::path tmp_data  = fs::temp_directory_path() / "log_data";

    fs::create_directories(tmp_cache);
    fs::create_directories(tmp_data);

    {
        GameLogger logger(tmp_cache.string(), tmp_data.string());

        // Write pseudo logs
        logger.async_log("test message 1");
        logger.async_log("test message 2");

        // Wait until the logfiles to be present
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Check if the file is created at data directory
    auto files = fs::directory_iterator(tmp_data);
    bool found = false;
    fs::path logfile;

    for (auto& entry : files)
    {
        if (entry.is_regular_file())
        {
            logfile = entry.path();
            found = true;
            break;
        }
    }
    
    EXPECT_TRUE(found);

    // Validate file contents
    std::ifstream ifs(logfile);
    std::string line;

    std::getline(ifs, line);
    EXPECT_EQ(line, "test message 1");

    std::getline(ifs, line);
    EXPECT_EQ(line, "test message 2");

    // Finalization
    fs::remove_all(tmp_cache);
    fs::remove_all(tmp_data);
}
