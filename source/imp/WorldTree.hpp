#pragma once

#include <vector>

#include <fcl/fcl.h>

#include "CKDTree.hpp"
#include "ObjectManager.hpp"
#include "ESTNode.hpp"
#include "json/JSON.hpp"

namespace imp
{

using WorldNode = ESTNode;

class EST;

class WorldTree : public json::JSONable
{
    // data
private:
    std::vector<WorldNode> _nodes;
    std::shared_ptr<CKDTree<WorldNode>> _kdtree{nullptr};

    const size_t _MOVABLE_ID{0};

    std::mutex _edit_mtx;

    size_t _position{0};                    // "current" position of the object
    std::vector<size_t> _position_children; // todo: validation function

    // properties
public:
    size_t size() const noexcept { return _nodes.size(); }

    bool initialize(Configuration root)
    {
        if (size())
            return false;
        else
        {
            makeNode(root);
            _kdtree->revalidate();
            return true;
        }
    }

    bool moveFromToInsert(const Configuration & start, const Configuration & end)
    {
        std::lock_guard<std::mutex> guard(_edit_mtx);
        
        if (!size())
            initialize(start);

        if (Distance(_nodes[_position].Config, start) < 1e-10f)
        {
            moveToInsert(end);
            return true;
        }
        return false;
    }

    void moveToInsert(const Configuration & end)
    {
        _position = makeNode(end, _position);
    }

    bool join(imp::EST & est);

    // constructors etc.
public:
    WorldTree(const size_t MOVABLE_ID)
        : _MOVABLE_ID{MOVABLE_ID}
    {
        CKDTreeBox world_domain(fcl::Vector3f{std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest()},
                                fcl::Quaternionf{-1.0f, -1.0f, -1.0f, -1.0f},
                                fcl::Vector3f{std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max()},
                                fcl::Quaternionf{1.0f, 1.0f, 1.0f, 1.0f});
        _kdtree = // ! irrelevant rating
            std::make_shared<CKDTree<WorldNode>>(_nodes, world_domain, std::make_pair(1.0f, 1.0f));
    }

    // methods
private:
    size_t makeNode(const Configuration & c, size_t parent = 0)
    {
        WorldNode node;
        node.Config = c;
        node.Rating = 0;
        node.IsRoot = !size();
        node.Parent = parent;
        _nodes.emplace_back(node);
        return size() - 1;
    }

public:
    JSON_IMPL(
        auto & Nodes{_nodes}; 
        auto & KDTree{_kdtree}; 
        auto & Position{_position}; 
        JSOND(Nodes) JSOND(KDTree) JSON(Position)
    )
};

} // namespace imp