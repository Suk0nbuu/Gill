#pragma once
#include "mathpp.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <optional>


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
        size_t UpperBound(float frame)  const {
            size_t low = 0;
            size_t high = keyframes.size();
            size_t mid;
            while (low < high) {
                  mid =  low + (high - low) / 2;
                if (keyframes[mid].frame <= frame) {
                    low = mid + 1;
                }
                else {
                    high = mid;
                }
            }
            return low;
        }
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
    template <typename T>
    std::optional<T> SampleChannel(const Channel<T>& channel, float frame) {
        if (channel.keyframes.empty()) {return std::nullopt;}
        size_t index = channel.UpperBound(frame);
        if (index == 0) {return channel.keyframes[index].value;}
        if ( index == channel.keyframes.size()) {return channel.keyframes[index-1].value;}
        const Keyframe<T>& a = channel.keyframes[index-1];
        const Keyframe<T>& b = channel.keyframes[index];
        if (a.mode == InterpolationMode::Constant) {return a.value;}
        else {
            if constexpr (std::is_same_v<T, mathpp::vec3f>) {
                float divisor = b.frame - a.frame;
                if (divisor < mathpp::epsilon<float>) { return a.value;}
                float t = (frame-a.frame) / divisor;
                return mathpp::Lerp(t, a.value, b.value);
            }
            else if constexpr (std::is_same_v<T, mathpp::quatf>) {
                float divisor = b.frame - a.frame;
                float t = (frame-a.frame) / divisor;
                return mathpp::Slerp(t,a.value, b.value);
            }
        }
    }
}

class AnimationSystem {
public:

    void SampleClip(animation::AnimationClip& clip, float frame);
private:


};