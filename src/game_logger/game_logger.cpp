#include "game_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <fstream>

namespace {
    std::string default_hostname() {
        if (const char* env = std::getenv("HOSTNAME"))
        {
            return env;
        }
        
        return "unknown-host";
    }

    std::string thread_id_str(std::thread::id id) {
        std::ostringstream oss;
        oss << id;

        std::string s = oss.str();

        // replace any characters unsafe for filenames (just in case)
        for (auto& c : s)
        {
            if (!(std::isalnum((unsigned char)c) || c=='-' || c=='_'))
            {
                c = '_';
            }
        }

        return s;
    }

    std::string current_timestamp() {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto t = system_clock::to_time_t(now);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm buff;
        localtime_r(&t, &buff);

        std::ostringstream oss;

        oss << std::put_time(&buff, "%Y%m%d_%H%M%S");
        oss << '_' << std::setw(3) << std::setfill('0') << ms.count();

        return oss.str();
    }

}

GameLogger::GameLogger(const std::string& base_cache_dir, const std::string& base_data_dir)
    : m_base_cache_dir(base_cache_dir)
    , m_base_data_dir(base_data_dir)
{
    m_hostname  = default_hostname();
    m_thread_id = thread_id_str(std::this_thread::get_id());
    m_timestamp = current_timestamp();

    m_cache_dir = fs::path(m_base_cache_dir);
    m_data_dir  = fs::path(m_base_data_dir);

    // Create directories
    std::error_code ec;
    fs::create_directories(m_cache_dir, ec);
    if (ec)
    {
        std::cerr << "[GameLogger] ERROR: cache_dir creation failed: " << ec.message() << "\n";
    }

    fs::create_directories(m_data_dir, ec);
    if (ec)
    {
        std::cerr << "[GameLogger] ERROR: data_dir creation failed: " << ec.message() << "\n";
    }

    // File name
    m_filename = m_timestamp + "_" + m_hostname + "_" + m_thread_id + "_playlog.json";

    m_cache_file.open(m_cache_dir / m_filename, std::ios::app);
    m_data_file.open(m_data_dir / m_filename, std::ios::app);

    m_running.store(true);
    m_worker = std::thread(&GameLogger::writing_worker, this);
}

GameLogger::~GameLogger() noexcept {
    m_running.store(false);
    m_cv.notify_one();

    if (m_worker.joinable())
    {
        m_worker.join();
    }

    // Close ofstream
    m_cache_file.flush();
    m_cache_file.close();
    m_data_file.flush();
    m_data_file.close();

    // Copy logfile from cache to data directory
    std::ifstream src(m_cache_dir / m_filename, std::ios::binary);
    std::ofstream dst(m_data_dir / m_filename, std::ios::binary);

    if (src && dst)
    {
        dst << src.rdbuf();
    }

    // Remove cache
    auto cache_file = fs::path(m_cache_dir / m_filename);

    try
    {
        fs::remove(cache_file);
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "[GameLogger] ERROR: Failed to remove cache file" << "\n";
    }
}

void GameLogger::async_log(const std::string& log_message) {
    if (!m_running.load())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_log_queue.push(log_message);
    }

    m_cv.notify_one();
}

void GameLogger::writing_worker() {
    while (m_running.load() || !m_log_queue.empty())
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_cv.wait(
            lock,
            [&]{
                return !m_log_queue.empty() || !m_running.load();
            }
        );

        while (!m_log_queue.empty())
        {
            auto log = std::move(m_log_queue.front());
            m_log_queue.pop();

            lock.unlock();

            if (m_cache_file.is_open())
            {
                m_cache_file << log << "\n";
                m_cache_file.flush();
            }

            lock.lock();
        }
    }
}
