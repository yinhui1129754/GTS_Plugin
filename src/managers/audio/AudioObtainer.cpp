#include "managers/audio/AudioObtainer.hpp"
#include "managers/highheel.hpp"
#include "managers/modevent.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {


    float volume_function(float scale, const VolumeParams& params) {
        float k = params.k;
        float a = params.a;
        float n = params.n;
        float s = params.s;
        // https://www.desmos.com/calculator/ygoxbe7hjg
        return k*pow(s*(scale-a), n);
    }

    float frequency_function(float scale, const VolumeParams& params) {
        float a = params.a;
        return soft_core(scale, 0.01, 1.0, 1.0, a, 0.0)*0.5+0.5;
    }

    BSISoundDescriptor* get_lFootstep_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("lFootstep");
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("lFootstep");
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_lJumpLand_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::JumpLand:
                return Runtime::GetSound("lJumpLand");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xlFootstep_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("xlFootstep"); 
            break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("xlFootstep");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xlRumble_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("xlRumble");
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("xlRumble");
                break;
            case FootEvent::JumpLand:
                return Runtime::GetSound("xlRumble");
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xlSprint_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("xlSprintL");
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("xlSprintR");
                break;
            case FootEvent::JumpLand:
                return Runtime::GetSound("xlSprintR");
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xxlFootstep_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("xxlFootstep");
            break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("xxlFootstep");
            break;
            case FootEvent::JumpLand:
                return Runtime::GetSound("xxlFootstep");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* GetNormalSound(float scale) {
        if (scale == 2.0) {
            return Runtime::GetSound("Normal_x2");
        } else if (scale == 4.0) {
            return Runtime::GetSound("Normal_x4");
        } else if (scale == 8.0) {
            return Runtime::GetSound("Normal_x8");
        } else if (scale == 12.0) {
            return Runtime::GetSound("Normal_x12");
        } else if (scale == 24.0) {
            return Runtime::GetSound("Normal_x24");
        } else if (scale == 48.0) {
            return Runtime::GetSound("Normal_x48");
        } else if (scale == 96.0) {
            return Runtime::GetSound("Normal_x96");
        } else if (scale > 96.0) {
            return Runtime::GetSound("Normal_Mega");
        }
        return nullptr;
    }
    BSISoundDescriptor* GetNormalSound_Jump(float scale) {
        if (scale == 2.0) {
            return Runtime::GetSound("NormalLand_x2");
        } else if (scale == 4.0) {
            return Runtime::GetSound("NormalLand_x4");
        } else if (scale == 8.0) {
            return Runtime::GetSound("NormalLand_x8");
        } else if (scale == 12.0) {
            return Runtime::GetSound("NormalLand_x12");
        } else if (scale == 24.0) {
            return Runtime::GetSound("NormalLand_x24");
        } else if (scale == 48.0) {
            return Runtime::GetSound("NormalLand_x48");
        } else if (scale == 96.0) {
            return Runtime::GetSound("NormalLand_x96");
        } else if (scale > 96.0) {
            return Runtime::GetSound("NormalLand_Mega");
        }
        return nullptr;
    }

    BSISoundDescriptor* GetHHSound_Normal(float scale) {
        if (scale == 2.0) {
            return Runtime::GetSound("HighHeel_x2");
        } else if (scale == 4.0) {
            return Runtime::GetSound("HighHeel_x4");
        } else if (scale == 8.0) {
            return Runtime::GetSound("HighHeel_x8");
        } else if (scale == 12.0) {
            return Runtime::GetSound("HighHeel_x12");
        } else if (scale == 24.0) {
            return Runtime::GetSound("HighHeel_x24");
        } else if (scale == 48.0) {
            return Runtime::GetSound("HighHeel_x48");
        } else if (scale == 96.0) {
            return Runtime::GetSound("HighHeel_x96");
        } else if (scale > 96.0) {
            return Runtime::GetSound("HighHeel_Mega");
        }
        return nullptr;
    }
    BSISoundDescriptor* GetHHSound_Jump(float scale) {
        if (scale == 2.0) {
            return Runtime::GetSound("HighHeelLand_x2");
        } else if (scale == 4.0) {
            return Runtime::GetSound("HighHeelLand_x4");
        } else if (scale == 8.0) {
            return Runtime::GetSound("HighHeelLand_x8");
        } else if (scale == 12.0) {
            return Runtime::GetSound("HighHeelLand_x12");
        } else if (scale == 24.0) {
            return Runtime::GetSound("HighHeelLand_x24");
        } else if (scale == 48.0) {
            return Runtime::GetSound("HighHeelLand_x48");
        } else if (scale == 96.0) {
            return Runtime::GetSound("HighHeelLand_x96");
        } else if (scale > 96.0) {
            return Runtime::GetSound("HighHeelLand_Mega");
        }
        return nullptr;
    }
    BSISoundDescriptor* get_footstep_highheel(const FootEvent& foot_kind, float scale) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return GetHHSound_Normal(scale);
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return GetHHSound_Normal(scale);
                break;
            case FootEvent::JumpLand:
                return GetHHSound_Jump(scale);
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_footstep_normal(const FootEvent& foot_kind, float scale) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return GetNormalSound(scale);
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return GetNormalSound(scale);
                break;
            case FootEvent::JumpLand:
                return GetNormalSound_Jump(scale);
                break;
        }
        return nullptr;
    }
}