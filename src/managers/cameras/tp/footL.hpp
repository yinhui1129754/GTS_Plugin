#pragma once
#include "managers/cameras/tp/foot.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class FootL : public Foot {
		protected:
			virtual NiPoint3 GetFootPos() override;
	};
}
