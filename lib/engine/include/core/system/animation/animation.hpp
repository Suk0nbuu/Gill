#pragma once
#include "mathpp.hpp"


struct Content {
    mathpp::vec3f position;
    mathpp::quatf rotation;
    mathpp::vec3f scale;

};


struct Channel {
    std::vector<std::pair<float,Content>> v_contents; //indexed by bones
};

