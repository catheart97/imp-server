#pragma once

#include <chrono>

namespace imp::time
{

using namespace std::literals::chrono_literals;

using duration_t =
    std::chrono::duration<std::chrono::system_clock::rep, std::chrono::system_clock::period>;

/**
 * @brief Simple timer class that starts automatically on construction with an elapsed method.
 * 
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class Timer
{
    /////////
    // data
    /////////
private:
    std::chrono::system_clock::time_point _startup;

    /////////
    // constructors
    /////////
public:
    Timer() : _startup{std::chrono::system_clock::now()} {}

public:
    /**
     * @brief Get the elapsed duration since construction.
     */
    duration_t elapsed() { return std::chrono::system_clock::now() - _startup; }
};

} // namespace imp::time