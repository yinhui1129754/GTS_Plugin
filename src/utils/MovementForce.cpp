#include "utils/MovementForce.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace Gts {
	float Get_Bone_Movement_Speed(Actor* giant, NodeMovementType Type) {
		float scale = get_visual_scale(giant);
		auto Data = Transient::GetSingleton().GetData(giant);
		NiPoint3 InputCoordinates = NiPoint3();
		NiPoint3 DataCoordinates = NiPoint3();
		NiAVObject* Node = nullptr;

		float NodeMovementForce = 0.0;
		
		if (Data) {
			switch (Type) {
				case NodeMovementType::Movement_LeftLeg: 
					Node = find_node(giant, "NPC L Foot [Lft ]");
					DataCoordinates = Data->POS_Last_Leg_L;
				break;
				case NodeMovementType::Movement_RightLeg: 
					Node = find_node(giant, "NPC R Foot [Rft ]");
					DataCoordinates = Data->POS_Last_Leg_R;
				break;
				case NodeMovementType::Movement_LeftHand: 
					Node = find_node(giant, "NPC L Hand [LHnd]");
					DataCoordinates = Data->POS_Last_Hand_L;
				break;
				case NodeMovementType::Movement_RightHand: 
					Node = find_node(giant, "NPC R Hand [RHnd]");
					DataCoordinates = Data->POS_Last_Hand_R;
				break;
			}

			if (Node) {
				InputCoordinates = Node->world.translate; // Record input node coordinates
				log::info("Input coords: {}", InputCoordinates);
				if (InputCoordinates.Length() > 0 && DataCoordinates.Length() > 0) {
					NodeMovementForce = (InputCoordinates - DataCoordinates).Length();

					// ^ Compare values, get movement force of Node X over 1 frame
					// ^ And also compensate speed with scale, since nodes travel further distance at large distances
				}

				DataCoordinates = Node->world.translate; // Record new pos of bone
				log::info("Data coords: {}", DataCoordinates);
			}
		}
		
		log::info("Movement Force: {}", NodeMovementForce);
		// Compensate speed with scale
		if (NodeMovementForce > 0) {
			return NodeMovementForce;
		}
		return 0.0;
	}
}