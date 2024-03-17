#pragma once

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	std::string GetRawName(const void* obj);

	std::string Vector2Str(const hkVector4& vector);

	std::string Vector2Str(const hkVector4* vector);

	std::string Vector2Str(const NiPoint3& vector);

	std::string Vector2Str(const NiPoint3* vector);

	std::string Vector2Str(const NiQuaternion& vector);

	std::string Vector2Str(const NiQuaternion* vector);
}
