#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
    void AI_TryAction(Actor* actor);
    void AI_DoStompAndButtCrush(Actor* pred);
    void AI_DoSandwich(Actor* pred);
    void AI_DoHugs(Actor* pred);
    void AI_StartHugs(Actor* pred, Actor* prey);
    void AI_StartHugsTask(Actor* giant, Actor* tiny);
}