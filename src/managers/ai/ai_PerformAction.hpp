#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
    void AI_StrongStomp(Actor* pred, int rng);
    void AI_LightStomp(Actor* pred, int rng);
    void AI_Tramples(Actor* pred, int rng);
    void AI_Kicks(Actor* pred, int rng);
    void AI_FastButtCrush(Actor* pred);
}