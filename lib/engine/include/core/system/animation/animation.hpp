#pragma once
#include "mathpp.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <iostream>

#include "component/armature.hpp"


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
        else if (a.mode == InterpolationMode::Linear) {
            float divisor = b.frame - a.frame;
            if (divisor < mathpp::epsilon<float>) { return a.value;}
            float t = (frame-a.frame) / divisor;
          return mathpp::Blend(t,a.value,b.value);
        }
    }
   inline std::vector<int> MakeList(const AnimationClip& clip,const Armature& armature) {
        std::vector<int> bindList(clip.tracks.size(),-1);
        int missCount = 0;
        std::unordered_map<std::string,int> nameToBone;
        nameToBone.reserve(armature.boneNames.size());
        for (size_t i = 0; i < armature.boneNames.size(); ++i) {
            nameToBone.try_emplace(armature.boneNames[i],i);
        }
        for (size_t i = 0; i < clip.tracks.size(); ++i) {
            const animation::Track& track = clip.tracks[i];
            auto it = nameToBone.find(track.name);
            if (it != nameToBone.end()) {
                bindList[i] = it->second;
            }
            else {
                bindList[i] = -1;
                missCount++;
            }
        }
        if (missCount == bindList.size()) {
            std::cerr<<"all bone not compatible or Clip is Empty "<<std::endl;
        }
        else if (missCount >= 1) {
            std::cerr<<"not all bone is being animated"<<std::endl;
        }
        return bindList;
    }
}

class AnimationSystem {
public:

    void SampleClip(const animation::AnimationClip& clip, float frame,const Armature& armature, const std::vector<int>& bindList,std::vector<Transform>& outTransform);
private:


};