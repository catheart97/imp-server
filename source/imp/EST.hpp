#pragma once

#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "imp/CKDTree.hpp"
#include "imp/Configuration.hpp"
#include "imp/ObjectManager.hpp"
#include "imp/Settings.hpp"
#include "imp/json/JSON.hpp"
#include "imp/time/Timer.hpp"
#include "imp/ESTNode.hpp"

namespace imp
{

using namespace std::chrono_literals;

class WorldTree;

/**
 * @brief Representing an exploring space tree.
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
class EST : public json::JSONable
{
    friend WorldTree;

    /////////
    // json
    /////////
public:
    JSON_IMPL(auto & Nodes{_nodes}; JSON(Nodes))

    /////////
    // nested
    /////////
protected:
    struct ESTNodeCandidate
    {
        Configuration Start, End;
        size_t Parent{0};
        bool Valid{false};
        float Rating{0};
    };

    /////////
    // data
    /////////
private:
    std::atomic<size_t> _session_id{0};
    std::atomic<size_t> _exploration_counter{0};

    std::mutex _explore_mutex;
    std::vector<ESTNode> _nodes;
    std::vector<size_t> _sorted_indices;
    ObjectManager & _manager;
    const size_t _MOVABLE_ID;
    bool _execution_allowed = true;
    size_t _last_solution = -1;

    /////////
    // constructors
    /////////
public:
    EST(imp::ObjectManager & manager, const size_t MOVABLE_ID)
        : _session_id{static_cast<size_t>(
              std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now())
                  .time_since_epoch()
                  .count())},
          _manager{manager}, _MOVABLE_ID{MOVABLE_ID} {};

    /////////
    // methods
    /////////
public:
    inline void stop() { _execution_allowed = false; }

    /////////
    // methods
    /////////
private:
    std::vector<size_t> kSmallest(const size_t K);

    void emplaceBack(const ESTNode & config);

public:
    /**
     * @brief Clear this tree.
     */
    inline void clear()
    {
        std::lock_guard<std::mutex> guard(_explore_mutex);
        _nodes.clear();
        _sorted_indices.clear();
        _execution_allowed = true;
        _exploration_counter++;
    }

    /**
     * @brief Constructs path from root to given node (node_index).
     */
    std::vector<Configuration> construct(size_t node_index);

    /**
     * @brief Explore arround the given ROOT configuration and try to match any
     * of the given matchees.
     */
    std::pair<int64_t, std::vector<Configuration>> explore( //
        const Configuration & ROOT,                         //
        std::pair<size_t, Configuration> MATCHEE, bool collision_free_matchee = true);
};

} // namespace imp
