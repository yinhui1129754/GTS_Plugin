#pragma once
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
// Module for accurate size-related damage

namespace Gts {
        float GetLaunchPower_Object(float sizeRatio, bool Launch);
        
        void PushObjectsUpwards(Actor* giant, std::vector<NiPoint3> footPoints, float maxFootDistance, float power, bool IsFoot);
        void PushObjectsTowards(Actor* giant, TESObjectREFR* object, NiAVObject* Bone, float power, float radius, bool Kick);
        void PushObjects(std::vector<ObjectRefHandle> refs, Actor* giant, NiAVObject* bone, float power, float radius, bool Kick);
        std::vector<ObjectRefHandle> GetNearbyObjects(Actor* giant);
}