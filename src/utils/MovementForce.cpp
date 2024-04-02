#include "utils/MovementForce.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	float Record_Node_Coordinates(NiAVObject* Node, NiPoint3& coords_out, TempActorData* Data) {
		float NodeMovementForce = 0.0;
		if (Node) {
			NiPoint3 coords_in = Node->world.translate;

			//log::info("Input coords: {}", Vector2Str(coords_in));
			if (coords_in.Length() > 0 && coords_out.Length() > 0) {
				NodeMovementForce = (coords_in - coords_out).Length();
				// ^ Compare values, get movement force of Node X over 1 frame
			}

			if (coords_in != coords_out) { // We don't want to apply it on the same frame, will result in 0
				coords_out = coords_in; // Record new pos of bone
			}
		}
		
		return NodeMovementForce;
	}
}

namespace Gts {
	float Get_Bone_Movement_Speed(Actor* giant, NodeMovementType Type) {
		auto profiler = Profilers::Profile("NodeMovement");
		NiAVObject* Node = nullptr;

		float NodeMovementForce = 0.0;
		float scale = get_visual_scale(giant);
		
		auto Data = Transient::GetSingleton().GetData(giant);

		if (Data) {

			NiPoint3& DataCoordinates_LL = Data->POS_Last_Leg_L;
			NiPoint3& DataCoordinates_RL = Data->POS_Last_Leg_R;
			NiPoint3& DataCoordinates_LH = Data->POS_Last_Hand_L;
			NiPoint3& DataCoordinates_RH = Data->POS_Last_Hand_R;

			switch (Type) {
				case NodeMovementType::Movement_LeftLeg: {
					//log::info("-------for Left Leg: ");
					Node = find_node(giant, "NPC L Foot [Lft ]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_LL, Data);
					break;
				}
				case NodeMovementType::Movement_RightLeg: {
					//log::info("-------for Right Leg: ");
					Node = find_node(giant, "NPC R Foot [Rft ]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_RL, Data);
					break;
				}
				case NodeMovementType::Movement_LeftHand: 
					Node = find_node(giant, "NPC L Hand [LHnd]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_LH, Data);
				break;
				case NodeMovementType::Movement_RightHand: 
					Node = find_node(giant, "NPC R Hand [RHnd]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_RH, Data);
				break;
			}
		}
		
		if (NodeMovementForce > 0) {
			NodeMovementForce /= 10.0;
			//log::info("movement force / 10: {}", NodeMovementForce);
			float NodeMovementForce_Clamped = std::clamp(NodeMovementForce, 0.0f, 1.0f);
			//log::info("Clamped movement force: {}", NodeMovementForce_Clamped);
			return NodeMovementForce;
		}
		return 0.0;
	}
}