#include "WorldTree.hpp"
#include "EST.hpp"

bool imp::WorldTree::join(imp::EST & est)
{
    std::lock_guard<std::mutex> guard_est(est._explore_mutex);
    std::lock_guard<std::mutex> guard_wt(_edit_mtx);
    
    if (Distance(_nodes[_position].Config, est._nodes[0].Config) < 1e-10)
    {
        const size_t NODE_OFFSET{_nodes.size()};

        for (size_t i = 0; i < est._nodes.size(); ++i)
        {
            WorldNode node = est._nodes[i];
            node.IsRoot = false;
            node.Parent = i ? NODE_OFFSET + node.Parent : _position;
            _nodes.emplace_back(node);
        }
        _position = NODE_OFFSET + est._last_solution;
        return true;
    }
    else
    {
        std::stringstream ss;
        JSON(_nodes[_position].Config)
        ss << "\n";
        JSON(est._nodes[0].Config)
        std::cout << ss.str() << std::endl;
        return false;
    }
}