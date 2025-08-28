#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace fs = std::filesystem;

class GameLogger {
public:
    GameLogger(
        const std::string& base_cache_dir = "/mnt/cache",
        const std::string& base_data_dir  = "/mnt/data"
    );

    ~GameLogger() noexcept;

    void async_log(const std::string& log_message);

private:
    // File / Directory
    std::string m_base_cache_dir;
    std::string m_base_data_dir;
    std::string m_hostname;
    std::string m_thread_id;
    std::string m_timestamp;
    std::string m_filename;

    fs::path      m_cache_dir;
    fs::path      m_data_dir;
    std::ofstream m_cache_file;
    std::ofstream m_data_file;

    // Variables for Async Logging
    std::atomic<bool>       m_running{false};
    std::thread             m_worker;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    std::queue<std::string> m_log_queue;

    // Worker thread
    void writing_worker();

    // Helpers
    std::string get_hostname();
    std::string get_thread_id(std::thread::id id);
    std::string get_timestamp();
};
