#pragma once

#include "imp/CKDTree.hpp"
#include "json/JSON.hpp"

namespace imp
{

struct ESTNode : public CKDData
{
    JSON_IMPL(        //
        JSOND(Config) //
        JSOND(Rating) //
        JSOND(Parent) //
        JSON(IsRoot)  //
    )

    size_t Parent{0};
    bool IsRoot{false};
};

}
