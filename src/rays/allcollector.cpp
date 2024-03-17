#include "rays/allcollector.hpp"

using namespace Gts;
using namespace RE;

namespace Gts {

	void AllRayCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
		AllRayCollectorOutput output;

		const hkpCdBody* body = &a_body;
		while( body->parent )
		{
			body = body->parent;
		}
		auto root = reinterpret_cast<const hkpCollidable*>(body);

		output.rootCollidable = root;
		output.hitFraction = a_hitInfo.hitFraction;

		this->hits.push_back(output);

		this->earlyOutHitFraction = 1.0;
	}

	unique_ptr<AllRayCollector> AllRayCollector::Create() {
		return make_unique<AllRayCollector>();
	}

	std::vector<AllRayCollectorOutput>& AllRayCollector::GetHits() {
		return this->hits;
	}

	bool AllRayCollector::HasHit() {
		return !this->hits.empty();
	}
}


void hkpClosestRayHitCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
	// Dummy
}

hkpClosestRayHitCollector::~hkpClosestRayHitCollector() {
	// Dummy
}

hkpRayHitCollector::~hkpRayHitCollector() {
	// Dummy
}


ExtraDataList::ExtraDataList() {
	// Dummy
}
