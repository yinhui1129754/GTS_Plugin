
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    void PlayCrushSound(Actor* giant, NiAVObject* node, bool only_once, bool StrongSound);
    void PlayMatchingSound(Actor* giant, NiAVObject* node, bool strong, int crushed, float size) ;
}