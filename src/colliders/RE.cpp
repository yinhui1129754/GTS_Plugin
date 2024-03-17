// Holds RE and dummy classes
#include "colliders/RE.hpp"

namespace RE {

	// DRAGON SLAYING
	//
	// HERE LIES SUPER CTD INDUCING CODE
	//
	// Touch only if you want to wake the dragon that lies
	// dorment in your machine
	//

	// Hack in the vtables in a super hacky way
	//
	// Seriously don't use this, just replace the vtable
	// with the real one via some other hacky code
	hkpShape::~hkpShape()  // 00
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpShape*)> func(result);
		func(this);
	}

	// add
	float hkpShape::GetMaximumProjection(const hkVector4& a_direction) const // 03
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x03;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::GetMaximumProjection)> func(result);
		return func(this, a_direction);
	}
	const hkpShapeContainer* hkpShape::GetContainer() const // 04 - { return 0; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x04;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::GetContainer)> func(result);
		return func(this);
	}
	bool hkpShape::IsConvex() const // 05 - { return false; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x05;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::IsConvex)> func(result);
		return func(this);
	}
	std::int32_t hkpShape::CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const// 06 - { return -1; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x06;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::CalcSizeForSpu)> func(result);
		return func(this, a_input, a_spuBufferSizeLeft);
	}
	hkVector4Comparison hkpShape::CastRayBundleImpl(const hkpShapeRayBundleCastInput& a_input, hkpShapeRayBundleCastOutput& a_output, const hkVector4Comparison& a_mask) const// 0A
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x0A;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::CastRayBundleImpl)> func(result);
		return func(this, a_input, a_output, a_mask);
	}

	hkpSphereRepShape::~hkpSphereRepShape()  // 00
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpSphereRepShape*)> func(result);
		func(this);
	}

	hkpConvexShape::~hkpConvexShape() {
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpConvexShape*)> func(result);
		func(this);
	}

	float hkpConvexShape::GetMaximumProjection(const hkVector4& a_direction) const { // 03
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x03;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpConvexShape::GetMaximumProjection)> func(result);
		return func(this, a_direction);
	}
	bool hkpConvexShape::IsConvex() const { // 05 - { return true; }
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x05;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpConvexShape::IsConvex)> func(result);
		return func(this);
	}
	void hkpConvexShape::CastRayWithCollectorImpl(const hkpShapeRayCastInput& a_input, const hkpCdBody& a_cdBody, hkpRayHitCollector& a_collector) const // 09
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x09;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpConvexShape::CastRayWithCollectorImpl)> func(result);
		return func(this, a_input, a_cdBody, a_collector);
	}
	std::uint32_t hkpConvexShape::Unk_10(void) // 10 - { return 2; }
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		// const auto a_idx = 0x10;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpConvexShape::Unk_10)> func(result);
		// return func(this);
		return 0;
	}
	void hkpConvexShape::Unk_11(void) // 11
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		// const auto a_idx = 0x11;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpConvexShape::Unk_11)> func(result);
		// func(this);
	}

	// VTABLE_hkpCapsuleShape
	hkpCapsuleShape::~hkpCapsuleShape()  // 00
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpCapsuleShape*)> func(result);
		func(this);
	}

	// override (hkpConvexShape)
	void hkpCapsuleShape::CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const // 02
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x02;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::CalcContentStatistics)> func(result);
		return func(this, a_collector, a_class);
	}
	std::int32_t hkpCapsuleShape::CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const // 06 - { return 56; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x06;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::CalcSizeForSpu)> func(result);
		return func(this, a_input, a_spuBufferSizeLeft);
	}
	void hkpCapsuleShape::GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const // 07
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x07;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetAabbImpl)> func(result);
		func(this, a_localToWorld, a_tolerance, a_out);
	}
	bool hkpCapsuleShape::CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const // 08
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x08;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::CastRayImpl)> func(result);
		return func(this, a_input, a_output);
	}
	std::int32_t hkpCapsuleShape::GetNumCollisionSpheresImpl() // 0B - { return 8; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x0B;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetNumCollisionSpheresImpl)> func(result);
		return func(this);
	}
	const hkpSphere* hkpCapsuleShape::GetCollisionSpheresImpl(hkSphere* a_sphereBuffer)// 0C
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x0C;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetCollisionSpheresImpl)> func(result);
		return func(this, a_sphereBuffer);
	}
	void hkpCapsuleShape::GetCentreImpl(hkVector4& a_centreOut) // 0D
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x0D;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetCentreImpl)> func(result);
		func(this, a_centreOut);
	}
	void hkpCapsuleShape::Unk_0E(void) // 0E
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		// const auto a_idx = 0x0E;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpCapsuleShape::Unk_0E)> func(result);
		// func(this);
	}
	void hkpCapsuleShape::Unk_0F(void) // 0F
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		// const auto a_idx = 0x0F;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpCapsuleShape::Unk_0F)> func(result);
		// func(this);
	}
	void hkpCapsuleShape::Unk_11(void) // 11
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		// const auto a_idx = 0x11;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpCapsuleShape::Unk_11)> func(result);
		// func(this);
	}

	void SetMotionType(hkpRigidBody* a_this, hkpMotion::MotionType a_newState, hkpEntityActivation a_preferredActivationState, hkpUpdateCollisionFilterOnEntityMode a_collisionFilterUpdateMode) {
		typedef void (*DefRealSetMotionType)(hkpRigidBody* a_this, hkpMotion::MotionType a_newState, hkpEntityActivation a_preferredActivationState, hkpUpdateCollisionFilterOnEntityMode a_collisionFilterUpdateMode);
		REL::Relocation<DefRealSetMotionType> RealSetMotionType{ RELOCATION_ID(60153, 60908) };
		RealSetMotionType(a_this, a_newState, a_preferredActivationState, a_collisionFilterUpdateMode);
	}

	void UpdateCollisionFilterOnEntity(hkpWorld *world, hkpEntity* entity, hkpUpdateCollisionFilterOnEntityMode updateMode, hkpUpdateCollectionFilterMode updateShapeCollectionFilter) {
		typedef void (*DefUpdateCollisionFilterOnEntity)(hkpWorld *world, hkpEntity* entity, hkpUpdateCollisionFilterOnEntityMode updateMode, hkpUpdateCollectionFilterMode updateShapeCollectionFilter);
		REL::Relocation<DefUpdateCollisionFilterOnEntity> RealUpdateCollisionFilterOnEntity{ RELOCATION_ID(60509, 61321) };
		RealUpdateCollisionFilterOnEntity(world, entity, updateMode, updateShapeCollectionFilter);
	}

	void UpdateCollisionFilterOnPhantom(hkpWorld *world, hkpPhantom* phantom, hkpUpdateCollectionFilterMode updateShapeCollectionFilter ) {
		typedef void (*DefUpdateCollisionFilterOnPhantom)(hkpWorld *world, hkpPhantom* entity, hkpUpdateCollectionFilterMode updateShapeCollectionFilter);
		REL::Relocation<DefUpdateCollisionFilterOnPhantom> RealUpdateCollisionFilterOnPhantom{ RELOCATION_ID(60509, 61321) };
		RealUpdateCollisionFilterOnPhantom(world, phantom, updateShapeCollectionFilter);
	}
}
