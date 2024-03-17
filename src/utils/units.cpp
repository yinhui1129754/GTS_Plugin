#include "utils/units.hpp"

using namespace RE;
using namespace SKSE;

namespace {
	const float CONVERSION_FACTOR = 70.0;
}

namespace Gts {
	float unit_to_meter(const float& unit) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return unit / CONVERSION_FACTOR;
	}

	float meter_to_unit(const float& meter) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return meter * CONVERSION_FACTOR;
	}

	NiPoint3 unit_to_meter(const NiPoint3& unit) {
		return unit / CONVERSION_FACTOR;
	}
	NiPoint3 meter_to_unit(const NiPoint3& meter) {
		return meter * CONVERSION_FACTOR;
	}
}
