#pragma once
// Module that handles footsteps

#include "managers/impact.hpp"
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
    struct VolumeParams {
        float a;
        float k;
        float n;
        float s;
    };
    
    float volume_function(float scale, const VolumeParams& params);
    float frequency_function(float scale, const VolumeParams& params);

    BSISoundDescriptor* get_lFootstep_sounddesc(const FootEvent& foot_kind);
    BSISoundDescriptor* get_lJumpLand_sounddesc(const FootEvent& foot_kind);

    BSISoundDescriptor* get_xlFootstep_sounddesc(const FootEvent& foot_kind);
    BSISoundDescriptor* get_xlRumble_sounddesc(const FootEvent& foot_kind);

    BSISoundDescriptor* get_xlSprint_sounddesc(const FootEvent& foot_kind);
    BSISoundDescriptor* get_xxlFootstep_sounddesc(const FootEvent& foot_kind);

    BSISoundDescriptor* GetNormalSound(float scale);
    BSISoundDescriptor* GetNormalSound_Jump(float scale);

    BSISoundDescriptor* GetHHSound_Normal(float scale);
    BSISoundDescriptor* GetHHSound_Jump(float scale);

    BSISoundDescriptor* get_footstep_highheel(const FootEvent& foot_kind, float scale);
    BSISoundDescriptor* get_footstep_normal(const FootEvent& foot_kind, float scale);
}