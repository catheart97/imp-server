#include "imp/EST.hpp"

std::vector<size_t> imp::EST::kSmallest(const size_t K)
{
    std::partial_sort(                                  //
        _sorted_indices.begin(),                        //
        _sorted_indices.begin() + K,                    //
        _sorted_indices.end(),                          //
        [this](const size_t a, const size_t b) {        //
            return _nodes[a].Rating < _nodes[b].Rating; //
        });

    std::vector<size_t> result(K);
    std::copy(_sorted_indices.begin(), _sorted_indices.begin() + K, result.begin());
    return result;
}

void imp::EST::emplaceBack(const ESTNode & config)
{
    _nodes.emplace_back(config);
    _sorted_indices.emplace_back(_nodes.size() - 1);
}

std::vector<imp::Configuration> imp::EST::construct(size_t node_index)
{
    std::vector<Configuration> result;

    auto node = _nodes[node_index];
    while (!node.IsRoot)
    {
        result.emplace_back(node.Config);
        node = _nodes[node.Parent];
    }

    std::reverse(result.begin(), result.end());
    return result;
}

std::pair<int64_t, std::vector<imp::Configuration>> imp::EST::explore( //
    const Configuration & ROOT,                                        //
    std::pair<size_t, Configuration> MATCHEE,                          //
    bool collision_free_matchee)
{
    // clean all nodes in tree
    clear();
    std::lock_guard<std::mutex> guard(_explore_mutex);

    const Configuration CENTER{
        imp::math::lerp(ROOT.Position, MATCHEE.second.Position, 0.5f),
        imp::math::lerp(ROOT.Rotation, MATCHEE.second.Rotation, 0.5f),
    };

    // compute sampling area
    float max_rot_distance{EST_DOMAIN_INITIAL_ROTATION_LIMIT};
    float max_pos_distance{PDistance(CENTER, ROOT) * EST_DOMAIN_GROW_FACTOR};

    const float TOTAL_MAX_ROT_DISTANCE{std::numbers::pi_v<float>};
    const float TOTAL_MAX_POS_DISTANCE{(CENTER.Position - ROOT.Position).norm() +
                                       _manager.bounding(_MOVABLE_ID) / 2.0f};

    CKDTreeBox domain;
    for (size_t i = 0; i < 3; ++i)
    {
        domain.Min[i] = CENTER[i] - TOTAL_MAX_POS_DISTANCE;
        domain.Max[i] = CENTER[i] + TOTAL_MAX_POS_DISTANCE;
    }
    for (size_t i = 4; i < 7; ++i)
    {
        domain.Min[i] = -1.0f;
        domain.Max[i] = 1.0f;
    }

    // add root node to tree
    ESTNode root_node;
    root_node.Config = ROOT;
    root_node.Rating = 0;
    root_node.IsRoot = true;
    emplaceBack(root_node);

    // setup kd-tree
    CKDTree<ESTNode> kdtree(
        _nodes, domain,
        std::make_pair(EST_POSITIONAL_CLUSTER_DISTANCE, EST_ROTATIONAL_CLUSTER_DISTANCE));

    std::optional<size_t> solution;
    std::mutex solution_lock;

#define __IMP_EST_EXECUTION_FAIL                                                                   \
    if (!_execution_allowed) return std::make_pair(-1, std::vector<Configuration>());

    time::Timer timer;
    size_t steps{0};
    while (_execution_allowed && timer.elapsed() < EST_MAX_EXPLORATION_RUNTIME &&
           _nodes.size() < EST_MAX_SIZE)
    {
        std::vector<ESTNodeCandidate> candidates(std::min(_nodes.size(), EST_MAX_NEW_SAMPLES));

        if (steps % EST_DOMAIN_ROTATION_INCREASE_STEP == 0)
        {
            max_rot_distance *= EST_DOMAIN_GROW_FACTOR;
            if (max_rot_distance > TOTAL_MAX_ROT_DISTANCE)
                max_rot_distance = TOTAL_MAX_ROT_DISTANCE;
        }

        if (steps % EST_DOMAIN_POSITIONAL_INCREASE_STEP == 0)
        {
            max_pos_distance *= EST_DOMAIN_GROW_FACTOR;
            if (max_pos_distance > TOTAL_MAX_POS_DISTANCE)
                max_rot_distance = TOTAL_MAX_POS_DISTANCE;
        }

        steps++;

        // get the first k nodes with the smallest rating
        auto k_smallest = kSmallest(candidates.size());

        // sample new local configurations
#pragma omp parallel for
        for (int64_t i = 0; i < candidates.size(); ++i)
        {
            auto & candidate = candidates[i];
            candidate.Parent = k_smallest[i];
            candidate.Start = _nodes[k_smallest[i]].Config;

            if (random::Sampler().rand() < EST_BIASED_SAMPLE_PROPABILITY)
            {
                candidate.End = random::Sampler().randConfigurationArroundMinimumDistance(
                    _nodes[k_smallest[i]].Config,        // config to sample arroung
                    EST_SAMPLE_MAX_POSITIONAL_DISTANCE,  // maximum positional distance
                    EST_SAMPLE_MAX_ROTATIONAL_DISTANCE,  // maximum rotational distance
                    EST_SAMPLE_MIN_POSITIONAL_DISTANCE,  // minimum positional distance
                    EST_SAMPLE_MIN_ROTATIONAL_DISTANCE); // minimum rotational distance
            }
            else
            {
                Configuration change{random::Sampler().randTargetConfigurationChange(
                    candidate.Start, MATCHEE.second)};

                candidate.End = {candidate.Start.Position + change.Position,
                                 candidate.Start.Rotation * change.Rotation};
            }
            candidate.Rating = 0;
        }

        __IMP_EST_EXECUTION_FAIL

// check which are collision free
#pragma omp parallel for
        for (int64_t i = 0; i < candidates.size(); ++i)
        {
            if (PDistance(candidates[i].End, CENTER) > max_pos_distance ||
                RDistance(candidates[i].End, CENTER) > max_rot_distance)
            {
                candidates[i].Valid = false;
            }
            else
            {
                candidates[i].Valid = _manager.isCollisionFreePath(_MOVABLE_ID,         //
                                                                   candidates[i].Start, //
                                                                   candidates[i].End);
            }
        }

        __IMP_EST_EXECUTION_FAIL

        // remove invalid candidates
        std::vector<ESTNodeCandidate> candidates_buffer;
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            if (candidates[i].Valid) candidates_buffer.emplace_back(candidates[i]);
        }
        std::swap(candidates, candidates_buffer);

        __IMP_EST_EXECUTION_FAIL

        // keep the previous size
        const size_t PREVIOUS_SIZE{_nodes.size()};

        // insert into tree
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            auto & candidate = candidates[i];

            ESTNode node;
            node.Config = candidate.End;
            node.Parent = candidate.Parent;
            node.Rating = candidate.Rating;

            emplaceBack(node);
        }
        kdtree.revalidate(); // ranking is updated here !

        __IMP_EST_EXECUTION_FAIL

        // check if we match any target

        if (collision_free_matchee)
        {
#pragma omp parallel for
            for (int64_t i = 0; i < candidates.size(); ++i)
            {
                auto & candidate = candidates[i];
                if (Distance(MATCHEE.second, candidate.End, _manager.bounding(_MOVABLE_ID)) <
                    EST_MIN_MATCHEE_DISTANCE)
                {
                    if (_manager.isCollisionFreePath(_MOVABLE_ID, MATCHEE.second, candidate.End))
                    {
                        std::lock_guard<std::mutex> solution_guard(solution_lock);
                        solution = i + PREVIOUS_SIZE;
                    }
                }
            }
        }
        else
        {
            for (int64_t i = 0; i < candidates.size(); ++i)
            {
                auto & candidate = candidates[i];
                if (Distance(MATCHEE.second, candidate.End) < EST_MIN_MATCHEE_DISTANCE / 2.0f)
                {
                    break;
                }
            }
        }

        if (solution.has_value()) break;
    }

    __IMP_EST_EXECUTION_FAIL

    // prepare data for client
    size_t matchee_index = MATCHEE.first;
    bool complete_solution = solution.has_value();
    if (!solution.has_value())
    {
        float best_distance = std::numeric_limits<float>::max();
        for (size_t i = 0; i < _nodes.size(); ++i)
        {
            if (float dst =
                    Distance(MATCHEE.second, _nodes[i].Config, _manager.bounding(_MOVABLE_ID));
                dst < best_distance)
            {
                best_distance = dst;
                solution = i;
            }
        }
    }

    __IMP_EST_EXECUTION_FAIL

    _last_solution = solution.value();

// #ifdef DUMP_REQUESTS
    {
        auto & Matchee{MATCHEE.second};
        auto & CompleteSolution{complete_solution};
        auto & SolutionIndex{solution.value()};
        auto & Objects{_manager};
        auto & KDTree{kdtree};

        // construct filename
        std::stringstream fn;
        fn << "est-" << _session_id << "-" << _exploration_counter << ".json";

        std::ofstream ss;
        ss.open(fn.str(), std::ios::out);

        ss << "{";
        JSONND(EST, *this)
        JSOND(Matchee)
        JSOND(KDTree)
        JSOND(SolutionIndex)
        JSOND(CompleteSolution)
        JSON(Objects)
        ss << "}";

        ss.close();
    }
// #endif

#undef __IMP_EST_EXECUTION_FAIL

    // if (complete_solution)
    // {
    //     auto path = construct(solution.value());
    //     path.emplace_back(MATCHEE.second);
    //     return std::make_pair(matchee_index, path);
    // }
    // else
    // {
    return std::make_pair(-1, construct(solution.value()));
    // }
}
