#pragma once
#include "mathpp.hpp"
#include <string>
#include <vector>



namespace animation {
    enum class InterpolationMode {
        Linear,
        Constant,
        /*Bezier next*/
    };
    template <typename T>
    struct Keyframe{
        float frame = -1.0; // -1.0 = Not initialized
        T value;
        InterpolationMode mode = InterpolationMode::Linear; // Earlier key decides the interpolation
    };
    template <typename T>
    struct Channel {
        std::vector<Keyframe<T>> keyframes;
    };


    struct Track {
        std::string name; // Bone name so it can be reused
        Channel<mathpp::vec3f> position;
        Channel<mathpp::quatf> rotation;
        Channel<mathpp::vec3f> scale;
    };

    struct AnimationClip {
        std::string name;
        float fps = 30.0f; // the rate of animation
        float start;
        float end ; // in frames;
        std::vector<Track> tracks;
    };
}

class AnimationSystem {
public:




private:

};