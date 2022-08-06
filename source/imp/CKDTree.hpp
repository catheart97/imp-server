#pragma once

#include <cmath>
#include <concepts>
#include <memory>
#include <vector>

#include "imp/Configuration.hpp"
#include "imp/json/JSON.hpp"

namespace imp
{

/**
 * @brief 
 * 
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct CKDTreeBox : public imp::json::JSONable
{
    /////////
    // json
    /////////
public:
    JSON_IMPL(JSOND(Min) JSON(Max))

    /////////
    // data
    /////////
public:
    Configuration Min;
    Configuration Max;

    /////////
    // constructors
    /////////
public:
    CKDTreeBox(const fcl::Vector3f & min_pos, const fcl::Quaternionf & min_rot,
               const fcl::Vector3f & max_pos, const fcl::Quaternionf & max_rot)
        : Min{min_pos, min_rot}, Max{max_pos, max_rot}
    {}

    CKDTreeBox(const Configuration & min, const Configuration & max) : Min{min}, Max(max) {}

    CKDTreeBox()
        : Min{fcl::Vector3f{std::numeric_limits<float>::max(), //
                            std::numeric_limits<float>::max(), //
                            std::numeric_limits<float>::max()}},
          Max{fcl::Vector3f{std::numeric_limits<float>::lowest(),
                            std::numeric_limits<float>::lowest(),
                            std::numeric_limits<float>::lowest()}}
    {}
};

/**
 * @brief Data structure to use with CKDTree.
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct CKDData : public imp::json::JSONable
{
public:
    JSON_IMPL(JSOND(Config) JSON(Rating))

public:
    Configuration Config;
    size_t Rating{0};

public:
    CKDData() = default;
    CKDData(const Configuration & c) : Config{c}, Rating{0} {}
};

using CKDDistancePair = std::pair<float, float>;

/**
 * @brief
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct CKDTreeSplit : public imp::json::JSONable
{
    /////////
    // json
    /////////
public:
    JSON_IMPL(JSOND(SplitValue) JSON(Direction))

    /////////
    // data
    /////////
public:
    float SplitValue;
    size_t Direction;

    /////////
    // constructors
    /////////
public:
    CKDTreeSplit(float split, size_t dir) : SplitValue{split}, Direction{dir} {}
};

/**
 * @brief
 *
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
struct CKDTreeNode : public imp::json::JSONable
{
    /////////
    // json
    /////////
public:
    JSON_IMPL(       //
        JSOND(Split) //
        JSOND(Box)   //
        JSOND(Left)  //
        JSOND(Right) //
        JSON(Points) //
    )

    /////////
    // data / properties
    /////////
public:
    std::optional<CKDTreeSplit> Split;

    CKDTreeBox Box;

    std::shared_ptr<CKDTreeNode> Left{nullptr};
    std::shared_ptr<CKDTreeNode> Right{nullptr};

    std::vector<size_t> Points;

    /////////
    // constructors
    /////////
public:
    CKDTreeNode(const CKDTreeBox box) : Box{box} {}
};

/**
 * @brief A KD Tree implementation on 7 Dimensions for the configuration space.
 *
 * @tparam storage_t             The type the data is stored in
 * @tparam access_f(storage_t &) The function the access the configuration from the storage_t
 * @author Ronja Schnur (rschnur@students.uni-mainz.de)
 */
template <class storage_t>
requires std::derived_from<storage_t, CKDData>
class CKDTree : public imp::json::JSONable
{
    /////////
    // json
    /////////
public:
    JSON_IMPL(                       //
        auto & Root{_root};          //
        auto & Size{_size};          //
        auto & LeafSize{_LEAF_SIZE}; //
        auto & Data{_data};          //
        JSOND(Root)                  //
        JSOND(Data)                  //
        JSOND(Size)                  //
        JSON(LeafSize)               //
    )

    /////////
    // data
    /////////
private:
    const DistancePair _DISTANCES;

    const CKDTreeBox _BOX;   // domain
    const size_t _LEAF_SIZE; // maximum nodes per leaf

    // the root node
    std::shared_ptr<CKDTreeNode> _root{nullptr};

    // marks until which value configurations used in the tree
    size_t _explored_size{0};
    // The vector in which the configurations are stored
    std::vector<storage_t> & _data;

    // number of elements in the tree
    size_t _size{0};

    /////////
    // constructors
    /////////
public:
    CKDTree(std::vector<storage_t> & data,       //
            const CKDTreeBox & BOX,              //
            const imp::DistancePair & DISTANCES, //
            const size_t LEAF_SIZE = 1024)
        : _DISTANCES{DISTANCES}, _BOX{BOX}, _LEAF_SIZE{LEAF_SIZE}, _data{data}
    {
        _root = std::make_shared<CKDTreeNode>(BOX);
        revalidate();
    }

    /////////
    // methods
    /////////
protected:
    std::shared_ptr<CKDTreeNode> findNode(const Configuration & c)
    {
        std::shared_ptr<CKDTreeNode> result = _root;

        while (result->Split.has_value())
        {
            if (c[result->Split.value().Direction] < result->Split.value().SplitValue)
                result = result->Left;
            else
                result = result->Right;
        }

        return result;
    }

    void growSubtree(std::shared_ptr<imp::CKDTreeNode> & node, //
                     std::vector<size_t> & points,             //
                     const size_t BEGIN,                       //
                     const size_t END,                         //
                     imp::CKDTreeBox box)
    {
        // compute split direction
        size_t direction{0};
        float l_max{-1.0f};
        for (size_t i = 0; i < 7; ++i)
        {
            if (float l{(box.Max[i] - box.Min[i]) / (_BOX.Max[i] - _BOX.Min[i])}; l > l_max)
            {
                l_max = l;
                direction = i;
            }
        }

        // compute split value and generate child domains
        float split{(box.Min[direction] + box.Max[direction]) / 2.0f};
        CKDTreeBox left_box{box}, right_box{box};
        left_box.Max[direction] = split;
        right_box.Min[direction] = split;

        // partition points
        size_t MIDDLE =
            std::partition(points.begin() + BEGIN, points.begin() + END,
                           [&](size_t & k) { return _data[k].Config[direction] <= split; }) -
            points.begin();

        // create children
        node->Split = CKDTreeSplit{split, direction};
        node->Left = std::make_shared<CKDTreeNode>(left_box);
        node->Right = std::make_shared<CKDTreeNode>(right_box);

        // left child
        if ((MIDDLE - BEGIN) < _LEAF_SIZE / 2)
        {
            node->Left->Points.clear();
            node->Left->Points.resize(MIDDLE - BEGIN);
            std::copy(points.begin() + BEGIN, points.begin() + MIDDLE, node->Left->Points.begin());
        }
        else
        {
            growSubtree(node->Left, points, BEGIN, MIDDLE, node->Left->Box);
        }

        // right child
        if ((END - MIDDLE) < _LEAF_SIZE / 2)
        {
            node->Right->Points.clear();
            node->Right->Points.resize(END - MIDDLE);
            std::copy(points.begin() + MIDDLE, points.begin() + END, node->Right->Points.begin());
        }
        else
        {
            growSubtree(node->Right, points, MIDDLE, END, node->Right->Box);
        }
    }

    inline void growSubtree(std::shared_ptr<CKDTreeNode> & node)
    {
        growSubtree(node, node->Points, 0, node->Points.size(), node->Box);
        if (node->Left) node->Points.clear();
    }

    template <bool RATING>
    size_t search(std::shared_ptr<CKDTreeNode> node, //
                  Configuration & c_near,            //
                  Configuration & c,                 //
                  const DistancePair & distances)
    {
        if (!node) return 0;

        size_t result{0};
        if (!node->Split.has_value())
        {
            for (size_t i : node->Points)
            {
                auto pair = PairDistance(_data[i].Config, c);
                if (pair.first < distances.first && pair.second < distances.second)
                {
                    result++;
                    if (RATING) _data[i].Rating += 1;
                }
            }
        }
        else
        {
            const size_t DIRECTION{node->Split.value().Direction};
            const float SPLIT{node->Split.value().SplitValue};
            const float BACKUP{c_near[DIRECTION]};
            if (c[DIRECTION] <= SPLIT)
            {
                result += search<RATING>(node->Left, c_near, c, distances);
                c_near[DIRECTION] = SPLIT;
                auto pair = PairDistance(c_near, c);
                if (pair.first < distances.first && pair.second < distances.second)
                    result += search<RATING>(node->Right, c_near, c, distances);
                c_near[DIRECTION] = BACKUP;
            }
            else
            {
                result += search<RATING>(node->Right, c_near, c, distances);
                c_near[DIRECTION] = SPLIT;
                auto pair = PairDistance(c_near, c);
                if (pair.first < distances.first && pair.second < distances.second)
                    result += search<RATING>(node->Left, c_near, c, distances);
                c_near[DIRECTION] = BACKUP;
            }
        }

        return result;
    }

    template <bool RATING> size_t search(imp::Configuration c, const DistancePair & distances)
    {
        imp::Configuration c_near{c};
        size_t result{search<RATING>(_root, c_near, c, distances)};
        c.Rotation.x() *= -1.0f;
        c.Rotation.y() *= -1.0f;
        c.Rotation.z() *= -1.0f;
        c.Rotation.w() *= -1.0f;
        c_near.Rotation.x() *= -1.0f;
        c_near.Rotation.y() *= -1.0f;
        c_near.Rotation.z() *= -1.0f;
        c_near.Rotation.w() *= -1.0f;
        return result + search<RATING>(_root, c_near, c, distances);
    }

    bool expand()
    {
        if (_data.size() == _size) return false;

        auto & c{_data[_size]};
        _data[_size].Rating = search<true>(c.Config, _DISTANCES);
        auto node = findNode(c.Config);

        node->Points.emplace_back(_size++);
        if (node->Points.size() > _LEAF_SIZE) growSubtree(node);

        return true;
    }

public:
    void revalidate()
    {
        while (expand())
        {}
    }
};

} // namespace imp