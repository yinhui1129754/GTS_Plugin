#pragma once
// This adds the missing shapes and other data
// that is not yet REd by common lib
//
// TODO: Add these to common lib some day
//



#include "RE/H/hkVector4.h"
#include "RE/H/hkpConvexShape.h"
namespace RE
{
	class hkpTriangleShape : public hkpConvexShape
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpTriangleShape;

			~hkpTriangleShape() override;  // 00

			// override (hkpConvexShape)
			void                     CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;     // 02
			std::int32_t             CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const override;  // 06
			void                     GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const override;      // 07
			bool                     CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const override;     // 08
			hkVector4Comparison      CastRayBundleImpl(const hkpShapeRayBundleCastInput& a_input, hkpShapeRayBundleCastOutput& a_output, const hkVector4Comparison& a_mask) const override;  // 0A
			std::int32_t             GetNumCollisionSpheresImpl() override;                                                                // 0B
			const hkpSphere*         GetCollisionSpheresImpl(hkSphere* a_sphereBuffer) override;                                           // 0C

			// members
			std::uint16_t weldingInfo;
			stl::enumeration<hkpWeldingUtility::WeldingType, std::uint8_t>       weldingType;
			std::uint8_t isExtruded;
			hkVector4 vertexA;
			hkVector4 vertexB;
			hkVector4 vertexC;
			hkVector4 extrusion;
	};

	class hkpConvexTransformShapeBase : public hkpConvexShape {
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexTransformShapeBase;

			~hkpConvexTransformShapeBase() override;  // 00

			hkpSingleShapeContainer childShape;
			mutable std::int32_t childSize;
	};

	class hkpConvexTransformShape : public hkpConvexTransformShapeBase
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexTransformShape;

			~hkpConvexTransformShape() override;  // 00

			// override (hkpConvexShape)
			void                     CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;     // 02
			const hkpShapeContainer* GetContainer() const override;                                                                        // 04
			std::int32_t             CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const override;  // 06
			void                     GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const override;      // 07
			bool                     CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const override;     // 08
			void                     CastRayWithCollectorImpl(const hkpShapeRayCastInput& a_input, const hkpCdBody& a_cdBody, hkpRayHitCollector& a_collector) const override;               // 09
			std::int32_t             GetNumCollisionSpheresImpl() override;                                                                // 0B
			const hkpSphere*         GetCollisionSpheresImpl(hkSphere* a_sphereBuffer) override;                                           // 0C

			// members
			hkQsTransform transform;
			hkVector4 extraScale;
	};

	class hkFourTransposedPoints
	{
		public:
			hkVector4 vertices[3];
	};

	class hkpConvexVerticesConnectivity : public hkReferencedObject {
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexVerticesConnectivity;

			~hkpConvexVerticesConnectivity() override;  // 00

			virtual void clear();

			hkArray<std::uint16_t> vertexIndices;

			hkArray<std::uint8_t> numVerticesPerFace;
	};

	class hkpConvexVerticesShape : public hkpConvexShape
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexVerticesShape;

			~hkpConvexVerticesShape() override;  // 00

			// override (hkpConvexShape)
			void                     CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;     // 02
			std::int32_t             CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const override;  // 06
			void                     GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const override;      // 07
			bool                     CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const override;     // 08
			std::int32_t             GetNumCollisionSpheresImpl() override;                                                                // 0B
			const hkpSphere*         GetCollisionSpheresImpl(hkSphere* a_sphereBuffer) override;                                           // 0C

			// members
			// Cached AABB
			hkVector4 aabbHalfExtents;
			hkVector4 aabbCenter;

			hkArray<hkFourTransposedPoints> rotatedVertices;
			std::int32_t numVertices;

			bool useSpuBuffer;

			mutable hkArray<hkVector4> planeEquations;

			// FYI: Cannot get connectivity to work
			mutable hkpConvexVerticesConnectivity* connectivity;
	};

	class hkpSimpleConstraintContactMgr;
	class hkpCharacterRigidBodyListener;
	class hkpCharacterRigidBody;

	class hkpCharacterRigidBodyListener : public hkReferencedObject
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpCharacterRigidBodyListener;

			virtual ~hkpCharacterRigidBodyListener() override;

			virtual void characterCallback( hkpWorld* world, hkpCharacterRigidBody* characterRB );

			virtual void processActualPoints( const hkpWorld* world, hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, hkArray<std::uint16_t>& contactPointIds );

			virtual void unweldContactPoints( hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, const hkArray<std::uint16_t>& contactPointIds );

			virtual void considerCollisionEntryForSlope( const hkpWorld* world, hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, hkArray<std::uint16_t>& contactPointIds );

			virtual void considerCollisionEntryForMassModification( const hkpWorld* world, hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, const hkArray<std::uint16_t>& contactPointIds );
	};

	class hkpCharacterRigidBody : public hkReferencedObject, public hkpEntityListener, public hkpWorldPostSimulationListener
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpCharacterRigidBody;

			~hkpCharacterRigidBody() override;

			virtual void checkSupport( const hkStepInfo& stepInfo, hkpSurfaceInfo& ground ) const;

			struct SupportInfo
			{
				hkContactPoint m_point;
				hkpRigidBody* m_rigidBody;
				hkVector4 m_surfaceVelocity;
			};

			virtual hkpSurfaceInfo::SupportedState getSupportInfo( const hkStepInfo& stepInfo, hkArray<SupportInfo>& supportInfo ) const;

			virtual void getGround( const hkArray<SupportInfo>& supportInfo, bool useDynamicBodyVelocities, hkpSurfaceInfo& ground ) const;

			// Listener methods.
			virtual void entityAddedCallback( hkpEntity* entity );

			virtual void entityRemovedCallback( hkpEntity* entity );

			virtual void postSimulationCallback( hkpWorld* world );

			hkpRigidBody* m_character;

			hkpCharacterRigidBodyListener* m_listener;

			hkVector4 m_up;

			float m_unweldingHeightOffsetFactor;

			float m_maxSlopeCosine;

			float m_maxSpeedForSimplexSolver;

			float m_supportDistance;

			float m_hardSupportDistance;

			hkVector4 m_acceleration;
			float m_maxForce;

			struct CollectorPair;

			struct VertPointInfo
			{
				hkContactPoint m_vertPoint;
				hkpSimpleConstraintContactMgr* m_mgr;
			};

			hkArray<VertPointInfo> m_verticalContactPoints;
	};

	struct bhkCharacterRigidBody : bhkSerializable
	{
		public:
			inline static constexpr auto RTTI = RTTI_bhkCharacterRigidBody;

			~bhkCharacterRigidBody() override;  // 00
			void          Unk_2E(void) override;                              // 2E
			void          Unk_2F(void) override;                              // 2F

			bhkRigidBody *rigidBody;
			NiAVObject *unk28;
			bhkCharacterPointCollector ignoredCollisionStartCollector;
	};
	static_assert(offsetof(bhkCharacterRigidBody, ignoredCollisionStartCollector) == 0x30);

	struct bhkCharRigidBodyController :
		public bhkCharacterController, // 00
		public hkpCharacterRigidBodyListener // 330
	{
		public:
			inline static constexpr auto RTTI = RTTI_bhkCharRigidBodyController;

			~bhkCharRigidBodyController() override;  // 00

			// override (hkpCharacterRigidBodyListener)
			void characterCallback( hkpWorld* world, hkpCharacterRigidBody* characterRB ) override;
			void processActualPoints( const hkpWorld* world, hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, hkArray<std::uint16_t>& contactPointIds ) override;
			void unweldContactPoints( hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, const hkArray<std::uint16_t>& contactPointIds ) override;
			void considerCollisionEntryForSlope( const hkpWorld* world, hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, hkArray<std::uint16_t>& contactPointIds ) override;
			void considerCollisionEntryForMassModification( const hkpWorld* world, hkpCharacterRigidBody* characterRB, const hkpLinkedCollidable::CollisionEntry& entry, hkpSimpleConstraintContactMgr* mgr, const hkArray<std::uint16_t>& contactPointIds ) override;

			// override (bhkCharacterController)
			void GetLinearVelocityImpl(hkVector4& a_velocity) const override;  // 06
			void SetLinearVelocityImpl(const hkVector4& a_velocity) override;  // 07

			bhkCharacterRigidBody characterRigidBody; // 340
	};
	static_assert(offsetof(bhkCharRigidBodyController, characterRigidBody) == 0x340);

	class hkaFootPlacementIkSolver : public hkReferencedObject
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkaFootPlacementIkSolver;

			struct Setup
			{
				const hkaSkeleton* m_skeleton;

				std::uint16_t m_hipIndex;

				std::uint16_t m_kneeIndex;

				std::uint16_t m_ankleIndex;

				hkVector4 m_kneeAxisLS;

				hkVector4 m_footEndLS;


				hkVector4 m_worldUpDirectionWS;

				hkVector4 m_modelUpDirectionMS;

				float m_originalGroundHeightMS;

				float m_footPlantedAnkleHeightMS;

				float m_footRaisedAnkleHeightMS;


				float m_maxAnkleHeightMS;

				float m_minAnkleHeightMS;

				float m_cosineMaxKneeAngle;

				float m_cosineMinKneeAngle;


				float m_raycastDistanceUp;

				float m_raycastDistanceDown;

				bool m_useFootLocking;
			};

			Setup m_setup;

			float m_currentWeight;

			float m_previousGroundHeightWS;

			hkVector4 m_previousGroundNormalWS;

			float m_previousVerticalDisplacement;

			bool m_isFootLocked;

			hkVector4 m_lockedFootPositionInWS;

			hkVector4 m_lockedFootEndPositionInWS;

			hkVector4 m_footUnlockingOffset;
	};

	class hkbFootIkDriver : public hkReferencedObject
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkbFootIkDriver;

			struct InternalLegData
			{
				hkVector4 m_groundPosition;

				hkVector4 m_groundNormal;

				hkaFootPlacementIkSolver* m_footIkSolver;

				float m_verticalError;

				bool m_hitSomething;

				bool m_isPlantedMS;
			};

			hkArray< struct InternalLegData > m_internalLegData;

			float m_worldFromModelFeedback;

			float m_prevIsFootIkEnabled;

			bool m_isSetUp;

			bool m_isGroundPositionValid;

	};

	enum class hkpEntityActivation
	{
		kDoNotActivate,
		kDoActivate
	};

	enum class hkpUpdateCollisionFilterOnEntityMode
	{
		kFullCheck,
		kDisableEntityEntityCollisionsOnly
	};

	enum class hkpUpdateCollectionFilterMode {
		kIgnoreCollections,
		kIncludeCollections,
	};

	void SetMotionType(hkpRigidBody* a_this, hkpMotion::MotionType a_newState, hkpEntityActivation a_preferredActivationState, hkpUpdateCollisionFilterOnEntityMode a_collisionFilterUpdateMode);

	// SSE: 60509, AE: 60908
	// 55 53 41 54 41 55 41 56 48 8d ac 24 90 f0 ff ff
	void UpdateCollisionFilterOnEntity(hkpWorld *world, hkpEntity* entity, hkpUpdateCollisionFilterOnEntityMode updateMode, hkpUpdateCollectionFilterMode updateShapeCollectionFilter);

	// SSE: 60511, AE: 61323
	// 40 55 53 41 54 41 56 48 8b ec 48 83 ec 68 83 b9 f8 00 00 00 00
	void UpdateCollisionFilterOnPhantom(hkpWorld *world, hkpPhantom* phantom, hkpUpdateCollectionFilterMode updateShapeCollectionFilter );
}
