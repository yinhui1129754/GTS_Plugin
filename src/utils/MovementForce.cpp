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
	NodeMovementType Convert_To_MovementType(DamageSource Source) {
		NodeMovementType Type = NodeMovementType::Movement_None;
		switch (Source) {
			case DamageSource::HandDropRight:
			case DamageSource::HandSwipeRight:
			case DamageSource::HandSlamRight:
			case DamageSource::HandCrawlRight:
				Type = NodeMovementType::Movement_RightHand;
			break;	
			case DamageSource::HandDropLeft:
			case DamageSource::HandSwipeLeft:
			case DamageSource::HandSlamLeft:
			case DamageSource::HandCrawlLeft:
				Type = NodeMovementType::Movement_LeftHand;
			break;
			case DamageSource::WalkRight:
			case DamageSource::CrushedRight:
			case DamageSource::FootGrindedRight:
			case DamageSource::KickedRight:
			case DamageSource::KneeRight:
				Type = NodeMovementType::Movement_RightLeg;
			break;	
			case DamageSource::WalkLeft:
			case DamageSource::CrushedLeft:
			case DamageSource::FootGrindedLeft:
			case DamageSource::KickedLeft:
			case DamageSource::KneeLeft:
				Type = NodeMovementType::Movement_LeftLeg;
			break;	
			default: {
				return NodeMovementType::Movement_None;
			break;
			}
		}
		return Type;
	}

	void Record_Node_Coordinates(NiAVObject* Node, NiPoint3& coords_out) {
		if (Node) {
			NiPoint3 coords_in = Node->world.translate;
			
			if (coords_in == coords_out) { // We don't want to apply it on the same frame in that case, will result in 0
				log::info("Coords are the same");
				return;
			} else {
				log::info("Coords are different: {} : {}", Vector2Str(coords_in), Vector2Str(coords_out));
				coords_out = coords_in; // Else Record new pos of bone
			}
		}
	}

	float Calculate_Movement(NiPoint3& input, NiPoint3& output) {
		float movement = 0.0;
		log::info("Input: {}, Output: {}", Vector2Str(input), Vector2Str(output));
		if (input.Length() > 0 && output.Length() > 0) {
			movement = (input - output).Length();
			// ^ Compare values, get movement force of Node X over 1 frame
		}
		return movement;
	}
}

namespace Gts {
	void Update_Movement_Data(Actor* giant) {

		auto Data = Transient::GetSingleton().GetData(giant);
		if (Data) {
			NiPoint3& DataCoordinates_LL = Data->POS_Last_Leg_L;
			NiPoint3& DataCoordinates_RL = Data->POS_Last_Leg_R;

			NiAVObject* Node_LL = find_node(giant, "NPC L Foot [Lft ]");
			NiAVObject* Node_RL = find_node(giant, "NPC R Foot [Rft ]");

			Record_Node_Coordinates(Node_LL, DataCoordinates_LL);
			Record_Node_Coordinates(Node_RL, DataCoordinates_RL);

			if (IsCrawling(giant)) {
				NiPoint3& DataCoordinates_LH = Data->POS_Last_Hand_L;
				NiPoint3& DataCoordinates_RH = Data->POS_Last_Hand_R;

				NiAVObject* Node_LH = find_node(giant, "NPC L Hand [LHnd]");
				NiAVObject* Node_RH = find_node(giant, "NPC R Hand [RHnd]");

				Record_Node_Coordinates(Node_RH, DataCoordinates_RH);
				Record_Node_Coordinates(Node_LH, DataCoordinates_LH);
			}
		}
	}

	float Get_Bone_Movement_Speed(Actor* giant, NodeMovementType Type) {
		auto profiler = Profilers::Profile("NodeMovement");
		NiAVObject* Node = nullptr;

		float NodeMovementForce = 0.0;
		float scale = get_visual_scale(giant);
		
		auto Data = Transient::GetSingleton().GetData(giant);
		NiPoint3 coordinates = NiPoint3(0.0, 0.0, 0.0);

		if (Data) {
			//log::info("Movement Owner: {}", giant->GetDisplayFullName());
			NiPoint3& DataCoordinates_LL = Data->POS_Last_Leg_L;
			NiPoint3& DataCoordinates_RL = Data->POS_Last_Leg_R;
			NiPoint3& DataCoordinates_LH = Data->POS_Last_Hand_L;
			NiPoint3& DataCoordinates_RH = Data->POS_Last_Hand_R;

			switch (Type) {
				case NodeMovementType::Movement_LeftLeg: {
					Node = find_node(giant, "NPC L Foot [Lft ]");
					if (Node) {
						NiPoint3 NodeCoords = Node->world.translate;
						NodeMovementForce = Calculate_Movement(NodeCoords, DataCoordinates_LL);
					}
					break;
				}
				case NodeMovementType::Movement_RightLeg: {
					Node = find_node(giant, "NPC R Foot [Rft ]");
					if (Node) {
						NiPoint3 NodeCoords = Node->world.translate;
						NodeMovementForce = Calculate_Movement(NodeCoords, DataCoordinates_RL);
					}
					break;
				}
				case NodeMovementType::Movement_LeftHand: 
					Node = find_node(giant, "NPC L Hand [LHnd]");
					if (Node) {
						NiPoint3 NodeCoords = Node->world.translate;
						NodeMovementForce = Calculate_Movement(NodeCoords, DataCoordinates_LH);
					}
				break;
				case NodeMovementType::Movement_RightHand: 
					Node = find_node(giant, "NPC R Hand [RHnd]");
					if (Node) {
						NiPoint3 NodeCoords = Node->world.translate;
						NodeMovementForce = Calculate_Movement(NodeCoords, DataCoordinates_RH);
					}
				break;
				case NodeMovementType::Movement_None:
					return 1.0; // Always allow for actions that are supposed to stagger always
				break;
			}
		}
		
		if (NodeMovementForce > 0) {
			log::info("movement force: {}", NodeMovementForce);
			float NodeMovementForce_Clamped = std::clamp(NodeMovementForce / 10.0f, 0.0f, 1.0f);
			//log::info("Clamped movement force: {}", NodeMovementForce_Clamped);
			return NodeMovementForce_Clamped;
		}
		return 0.0;
	}

	float Get_Bone_Movement_Speed(Actor* giant, DamageSource Source) {
		auto profiler = Profilers::Profile("ConvertMovement");
		NodeMovementType Type = Convert_To_MovementType(Source);

		return Get_Bone_Movement_Speed(giant, Type);
	}
}