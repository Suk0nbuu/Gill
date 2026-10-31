#include "core/system/animation/animation.hpp"
#include <unordered_map>
#include <cassert>


void AnimationSystem::SampleClip(const animation::AnimationClip& clip, float frame,const Armature& armature, const std::vector<int>& bindList, std::vector<Transform>& outTransform) {
outTransform = armature.restTransforms;
    assert(clip.tracks.size() == bindList.size());

    for (size_t i=0;i<clip.tracks.size();i++) {
        int bone = bindList.at(i);
        if (bone == -1) {continue;}
         auto& track = clip.tracks.at(i);
         auto& out = outTransform.at(bone);
        auto t = animation::SampleChannel(track.position,frame);
        auto r = animation::SampleChannel(track.rotation,frame);
        auto s = animation::SampleChannel(track.scale,frame);
        if (t.has_value()) {
            out.position = t.value();
        }
        if (r.has_value()) {
            out.rotation = r.value();
        }
        if (s.has_value()) {
            out.scale = s.value();
        }
    }

}
