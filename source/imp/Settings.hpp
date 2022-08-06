#pragma once

#include <chrono>
#include <numbers>
#include <string>

#include "imp/time/Timer.hpp"

using namespace std::chrono_literals;

/**************************************************************************************************/
/* COMPILE TIME CONFIGURATION *********************************************************************/
/**************************************************************************************************/

// #define DUMP_REQUESTS

////////////////////////////////////////////////////////////////////////////////////////////////////
// server settings
inline const char * HOST_IP = "192.168.188.99";
constexpr int HOST_PORT = 8000;
constexpr int MAX_OMP_THREADS = 6;

////////////////////////////////////////////////////////////////////////////////////////////////////
// path verification settings
constexpr float PATH_VERIFICATION_POSITIONAL_STEP = 0.005f;
constexpr float PATH_VERIFICATION_ROTATIONAL_STEP = .05f;

////////////////////////////////////////////////////////////////////////////////////////////////////
// configuration sampling settings
constexpr float REPAIR_MAX_POSITIONAL_DISTANCE = 0.075f;
constexpr float REPAIR_MAX_ROTATIONAL_DISTANCE{std::numbers::pi_v<float> * 0.3f};
constexpr size_t REPAIR_NUM_SAMPLES = 32;
constexpr size_t REPAIR_MAX_SAMPLE_TRIES = 16;

////////////////////////////////////////////////////////////////////////////////////////////////////
// est settings
constexpr size_t EST_MAX_NEW_SAMPLES = 6 * 6;

constexpr float EST_SAMPLE_MIN_POSITIONAL_DISTANCE{0.005f};
constexpr float EST_SAMPLE_MAX_POSITIONAL_DISTANCE{0.015f};
constexpr float EST_SAMPLE_MIN_ROTATIONAL_DISTANCE{0.05f * std::numbers::pi_v<float>};
constexpr float EST_SAMPLE_MAX_ROTATIONAL_DISTANCE{0.3f * std::numbers::pi_v<float>};

constexpr float EST_POSITIONAL_CLUSTER_DISTANCE{0.05f};
constexpr float EST_ROTATIONAL_CLUSTER_DISTANCE{0.4f * std::numbers::pi_v<float>};
constexpr float EST_SEARCH_DISTANCE_OFFSET{.05f};
constexpr float EST_MIN_MATCHEE_DISTANCE{0.5f};
constexpr size_t EST_MIN_MATCHEE_SECTION{1};
constexpr imp::time::duration_t EST_MAX_EXPLORATION_RUNTIME{3s};
constexpr size_t EST_MAX_SIZE{1 << 15};

constexpr float EST_DOMAIN_GROW_FACTOR{1.1f};
constexpr size_t EST_DOMAIN_POSITIONAL_INCREASE_STEP{4};
constexpr size_t EST_DOMAIN_ROTATION_INCREASE_STEP{EST_DOMAIN_POSITIONAL_INCREASE_STEP};
constexpr float EST_DOMAIN_INITIAL_ROTATION_LIMIT{std::numbers::pi_v<float> * 0.1};
constexpr float EST_BIASED_SAMPLE_PROPABILITY{.4f};

/**************************************************************************************************/
/* RUNTIME CONFIGURATION **************************************************************************/
/**************************************************************************************************/

inline size_t __db_helper;
#define DB() std::cout << "DB : " << __db_helper++ << std::endl;

#define DBAREA() __db_helper = 0;