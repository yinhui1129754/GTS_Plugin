#include "utils/DeathReport.hpp"
#include "managers/highheel.hpp"

using namespace RE;
using namespace Gts;

namespace {
	const std::string_view lFoot = "NPC L Foot [Lft ]";
	const std::string_view rFoot = "NPC R Foot [Rft ]";
	const std::string_view rCalf = "NPC R Calf [RClf]";
	const std::string_view lCalf = "NPC L Calf [LClf]";
	const std::string_view rHand = "NPC R Finger20 [RF20]";
	const std::string_view lHand = "NPC L Finger20 [LF20]";
	const std::string_view rThigh = "NPC R FrontThigh";
	const std::string_view breast = "NPC Spine2 [Spn2]";
	const std::string_view booty = "NPC Spine [Spn0]";
	const std::string_view none = "None";
}


namespace Gts {

	std::string_view GetDeathNodeName(DamageSource cause) {
		if (cause == DamageSource::HandCrawlRight||cause == DamageSource::HandSwipeRight||cause == DamageSource::HandSlamRight||cause == DamageSource::HandDropRight) {
			return rHand;
		} else if (cause == DamageSource::HandCrawlLeft||cause == DamageSource::HandSwipeLeft||cause == DamageSource::HandSlamLeft 
			|| cause == DamageSource::HandDropLeft || cause == DamageSource::HandCrushed) {
			return lHand;
		} else if (cause == DamageSource::KickedRight||cause == DamageSource::CrushedRight||cause == DamageSource::FootGrindedRight) {
			return rFoot;
		} else if (cause == DamageSource::KickedLeft||cause == DamageSource::CrushedLeft||cause == DamageSource::FootGrindedLeft) {
			return lFoot;
		} else if (cause == DamageSource::KneeRight || cause == DamageSource::KneeDropRight) {
			return rCalf;
		} else if (cause == DamageSource::KneeLeft || cause == DamageSource::KneeDropLeft) {
			return lCalf;
		} else if (cause == DamageSource::BodyCrush||cause == DamageSource::Hugs||cause == DamageSource::Breast||cause == DamageSource::BreastImpact) {
			return breast;
		} else if (cause == DamageSource::Booty) {
			return booty;
		} else if (cause == DamageSource::ThighSandwiched||cause == DamageSource::ThighCrushed) {
			return rThigh;
		}
		return none;
	}

	void ReportDeath(Actor* giant, Actor* tiny, DamageSource cause) {
		int random = rand()% 8;

		std::string_view TinyName = tiny->GetDisplayFullName();
		std::string_view GiantName = giant->GetDisplayFullName();

		if (cause == DamageSource::CrushedLeft||cause == DamageSource::CrushedRight||cause == DamageSource::WalkRight||cause == DamageSource::WalkLeft) { // Default crush by the feet
			if (!HighHeelManager::IsWearingHH(giant)) {
				if (random < 2) {
					Cprint("{} became a bloody stain under {} foot.", TinyName, GiantName);
				} else if (random == 2) {
					Cprint("{} was crushed by the feet of {}", TinyName, GiantName);
				} else if (random == 3) {
					Cprint("Feet of {} crushed {} into nothing", GiantName, TinyName);
				} else if (random == 4) {
					Cprint("{} stept on {} too hard", GiantName, TinyName);
				} else if (random == 5) {
					Cprint("{} got crushed by {}", TinyName, GiantName);
				} else if (random == 6) {
					Cprint("{} ended up being crushed by the {}", TinyName, GiantName);
				} else if (random >= 7) {
					Cprint("{} relentlessly crushed {}", GiantName, TinyName);
				}
				return;
			} else {
				if (random < 2) {
					Cprint("{} became a bloody stain under {} heel.", TinyName, GiantName);
				} else if (random == 2) {
					Cprint("{} was crushed under the heel of {}", TinyName, GiantName);
				} else if (random == 3) {
					Cprint("High Heels of {} obliterated {}", GiantName, TinyName);
				} else if (random == 4) {
					Cprint("{} stept on {} too hard", GiantName, TinyName);
				} else if (random == 5) {
					Cprint("{} got crushed under the heels of {}", TinyName, GiantName);
				} else if (random == 6) {
					Cprint("{} ended up being crushed by heels of {}", TinyName, GiantName);
				} else if (random >= 7) {
					Cprint("{} turned {} into bloody mist", GiantName, TinyName);
				}
				return;
			}
		} else if (cause == DamageSource::HandCrushed) { // when Grab -> Crush happens
			if (random == 1) {
				Cprint("{} life was squeezed out in {} grip", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} Squeezed her palm, unexpectedly crushing {}", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} was transformed into bloody mist by the tight grip of {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("{} has been crushed inside the hand of {}", TinyName, GiantName);
			} else if (random >= 6) {
				Cprint("{} applied too much pressure to her hand, crushing {}", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} was turned into pulp by the palm of {}", TinyName, GiantName);
			}
			return;
		} else if (cause == DamageSource::Collision) { // Through Collision
			if (random <= 3) {
				Cprint("{} exploded after colliding with {}", TinyName, GiantName);
			} else {
				Cprint("{} was instantly turned into mush by the body of {}", TinyName, GiantName);
			}
		} else if (cause == DamageSource::Shrinked) { // Shrink to nothing
			if (random <= 2) {
				Cprint("{} greedily absorbed {}", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} completely absorbed {}", GiantName, TinyName);
			} else if (random == 4) {
				Cprint("{} was absorbed by {}", TinyName, GiantName);
			} else if (random == 5) {
				Cprint("{} was shrinkned to nothing by {}", TinyName, GiantName);
			} else if (random == 6) {
				Cprint("{} size was completely drained by {}", TinyName, GiantName);
			} else if (random >= 7) {
				Cprint("{} stole all the size from {}, exploding {}", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::Vored) {
			///nothing for now
			return;
		} else if (cause == DamageSource::ThighCrushed) { // During thigh crush
			if (random == 1) {
				Cprint("{} was crushed to death between {} thighs.", TinyName, GiantName);
			} else if (random <= 3) {
				Cprint("{} crushed {} during leg stretch", GiantName, TinyName);
			} else if (random == 4) {
				Cprint("{} ended life of {} between legs", GiantName, TinyName);
			} else if (random >= 5) {
				Cprint("Thighs of {} took the life of {}", GiantName, TinyName);
			} 
			return;
		} else if (cause == DamageSource::BodyCrush) { // During Body Crush
			if (random == 1) {
				Cprint("{} was crushed by the body of {}", TinyName, GiantName);
			} else if (random <= 3) {
				Cprint("{} body obliterated {}", GiantName, TinyName);
			} else if (random == 4) {
				Cprint("{} let her body do the job", GiantName);
			} else if (random == 5) {
				Cprint("{} dropped her body onto {}", GiantName, TinyName);
			} else if (random >= 6) {
				Cprint("{} was turned into mush by the body of {}", TinyName, GiantName);
			}
			return;
		} else if (cause == DamageSource::ThighSandwiched) { // Self explanatory
			if (random <= 3) {
				Cprint("{} was crushed by the thighs of {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("Thighs of {} gently crushed {}", GiantName, TinyName);
			} else if (random == 5) {
				Cprint("{} has disappeared between the thighs of {}", TinyName, GiantName);
			} else if (random == 6) {
				Cprint("{} was smothered to nothing between the thighs of {}", TinyName, GiantName);
			} else if (random >= 7) {
				Cprint("Thighs of {} sandwiched {} to nothing", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::Overkill) {  // When we hit actor with too much weapon damage while being huge
			if (random == 1) {
				Cprint("{} body exploded because of massive size difference with {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} hit {} with so much force that {} exploded", GiantName, TinyName, TinyName);
			} else if (random == 3) {
				Cprint("{} was pulverized into nothing by {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("All that's left from {} after being hit by {} is a bloody pulp", TinyName, GiantName);
			} else if (random >= 6) {
				Cprint("{} couldn't handle enormous hit from {}", TinyName, GiantName);
			} else if (random >= 7) {
				Cprint("{} put so much force into attack that {} turned into red mush", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::HitSteal) { // Hit Growth perk
			if (random <= 2) {
				Cprint("{} body exploded after trying to hit {}", TinyName, GiantName);
			} else if (random == 3) {
				Cprint("Protective magic of {} made {} absorb {}", GiantName, GiantName, TinyName);
			} else if (random > 3) {
				Cprint("{} Tried to kill {}, but ended up being absorbed by the size magic of {}", TinyName, GiantName, GiantName);
			}
			return;
		} else if (cause == DamageSource::Explode) { // Poison Of Shrinking
			if (random <= 2) {
				Cprint("{} exploded into bloody dust", TinyName);
			} else if (random == 3) {
				Cprint("{} suddenly exploded", TinyName);
			} else if (random > 3) {
				Cprint("{} was turned into nothing", TinyName);
			}
			return;
		} else if (cause == DamageSource::BlockDamage) { // WHen tiny in hand receives too much damage
			if (random == 1) {
				Cprint("{} received too much damage and was automatically crushed in the hands of {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} was crushed from receiving too much damage {}", TinyName, GiantName);
			} else if (random == 3) {
				Cprint("{} stopped to be useful, so he was turned into bloody mist in the hands of {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("{} took extreme amounts of damage and exploded inside the hands of {}", TinyName, GiantName);
			} else if (random >= 6) {
				Cprint("{} took a little more damage than intended, so her fingers ended up squeezing {} into nothing", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} blocked too much damage and was squeezed into bloody stain by {}", TinyName, GiantName);
			}
			return;
		} else if (cause == DamageSource::FootGrindedLeft || cause == DamageSource::FootGrindedRight) { // Grinded by the foot. Currently doesn't exist. It is planned to add it.
			if (random < 2) {
				Cprint("{} became a bloody stain under {} foot.", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} was crushed by the feet of {}", TinyName, GiantName);
			} else if (random == 3 || random == 4) {
				Cprint("Feet of {} crushed {} into nothing", GiantName, TinyName);
			} else if (random == 5 || random == 6) {
				Cprint("{} got crushed by {}", TinyName, GiantName);
			} else if (random >= 7) {
				Cprint("{} relentlessly crushed {}", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::Melted) { // Melted by tongue. Currently doesn't exist. Possible Vore variation with melting actors with tongue instead.
			if (random < 2) {
				Cprint("{} was melted by the tongue of {}.", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} got absorbed by the tongue of {}", TinyName, GiantName);
			} else if (random == 3 || random == 4) {
				Cprint("Hot tongue of {} melted {} like a candy", GiantName, TinyName);
			} else if (random == 5 || random == 6) {
				Cprint("{} was (un)forunate enough to be melted by the tongue of {} ", TinyName, GiantName);
			} else if (random >= 7) {
				Cprint("Tongue of {} sucked all life out of {}", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::Breast) { // Someone died between breasts during Grabbing
			if (random == 1) {
				Cprint("{} was weakened and got accidentally crushed by {} breasts", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} got unintentionally crushed by the breasts of {}", TinyName, GiantName);
			} else if (random == 3) {
				Cprint("{} left this world by being crushed between the cleavage of {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("Breasts of {} squeezed all life out of {}", GiantName, TinyName);
			} else if (random >= 6) {
				Cprint("{} took some damage and ended up crushing {} between her breasts", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} got smothered by soft breasts of {}", TinyName, GiantName);
			}
			return;
		} else if (cause == DamageSource::BreastImpact) { // Someone died under cleavage
			if (random == 1) {
				Cprint("{} was crushed under the soft breasts of {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} ended up being crushed under the cleavage of {}", TinyName, GiantName);
			} else if (random == 3) {
				Cprint("{} was murdered beneath the breasts of {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("Breasts of {} squeezed all life out of {}", GiantName, TinyName);
			} else if (random >= 6) {
				Cprint("Cleavage of {} annihilated {}", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} got smothered under the soft breasts of {}", TinyName, GiantName);
			}
			return;
		} else if (cause == DamageSource::Booty) { // Butt Crushed. Currently doesn't exist. It is planned though.
			if (random < 2) {
				Cprint("{} got crushed by the butt of {}.", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} was overwhelmed by the booty of {}", TinyName, GiantName);
			} else if (random == 3) {
				Cprint("Bootie of {} completely pulverized {}", GiantName, TinyName);
			} else if (random == 4) {
				Cprint("Booty of {} completely pulverized {}", GiantName, TinyName);
			} else if (random == 5) {
				Cprint("{} has been squashed by butt attack of {}", TinyName, GiantName);
			} else if (random == 6) {
				Cprint("{} cheeks dropped on {}, turning {} into red paste", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} relentlessly crushed {} with butt attack", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::Hugs) { // For Hug Crush
			if (random < 2) {
				Cprint("{} was hugged to death by {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} got too carried away hugging {}, crushing {} as a result", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} applied too much force to the hugs, killing {}", GiantName, TinyName);
			} else if (random == 4) {
				Cprint("{} couldn't resist hug crushing {}", GiantName, TinyName);
			} else if (random == 5) {
				Cprint("{} failed to escape hugs of death with {}", TinyName, GiantName);
			} else if (random == 6) {
				Cprint("{} got greedy and stole all size from {} through hugs", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} gently hug crushed {}", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::KneeLeft || cause == DamageSource::KneeRight || cause == DamageSource::KneeDropLeft || cause == DamageSource::KneeDropRight) { // Crushed by knee
			if (random < 2) {
				Cprint("{} got crushed under the knee of {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("Knee of {} turned {} into nothing", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} was unlucky to be under {} knee", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("{} couldn't survive being under the knee of {}", TinyName, GiantName);
			} else if (random == 5) {
				Cprint("For some reason something has crunched under {} knee", GiantName);
			} else if (random == 6) {
				Cprint("{} was obliterated by {} knee", TinyName, GiantName);
			} else if (random >= 7) {
				Cprint("{} didn't realize that it's extremely dangerous to be under {} knees", TinyName, GiantName);
			}
			return;
		} else if (cause == DamageSource::HandCrawlLeft || cause == DamageSource::HandCrawlRight || cause == DamageSource::HandDropRight || cause == DamageSource::HandDropLeft) { // By hand
			if (random < 2) {
				Cprint("{} was accidentally crushed by the hand of {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} accidentally transformed {} into red jam with under her hands", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} died under the hand of {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("{} death wasn't even noticed by {}", TinyName, GiantName);
			} else if (random == 5) {
				Cprint("For some reason something has crunched under {} hands", GiantName);
			} else if (random == 6) {
				Cprint("{} wonders why her hands feel warm and wet in some places", GiantName);
			} else if (random >= 7) {
				Cprint("{} heard some crunching around her hands", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::HandSlamLeft || cause == DamageSource::HandSlamRight) { // By hand
			if (random < 2) {
				Cprint("Hand of {} aggressively crushed {}", GiantName, TinyName);
			} else if (random == 2) {
				Cprint("{} killed {} with her own hands", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} was turned into red jam by {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("Last thing {} saw is a huge hand coming from above", TinyName);
			} else if (random == 5) {
				Cprint("{} slammed {} into bloody mist", GiantName, TinyName);
			} else if (random == 6) {
				Cprint("{} decided to test endurance of {}", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} smashed {} with too much force", GiantName, TinyName);
			}
			return;
		} else if (cause == DamageSource::RightFinger || cause == DamageSource::LeftFinger) { // By Finger
			if (random < 2) {
				Cprint("{} got crushed by the finger of {}", TinyName, GiantName);
			} else if (random > 2) {
				Cprint("{} obliteated {} with her index finger", GiantName, TinyName);
			} 
			return;
		}
		
		else if (cause == DamageSource::HandSwipeLeft || cause == DamageSource::HandSwipeRight) { // By hand
			if (random < 2) {
				Cprint("{} was sent flying by the {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} turned {} into bloody mush with her hand", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} annoyed {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("{} death wasn't even noticed by {}", TinyName, GiantName);
			} else if (random == 5) {
				Cprint("{} applied too much force, crushing {} with her palm", GiantName, TinyName);
			} else if (random == 6) {
				Cprint("{} hit {} with her hand so hard that {} exploded", GiantName, TinyName, TinyName);
			} else if (random >= 7) {
				Cprint("{} wanted to push {} away, but ended up crushing {} instead", GiantName, TinyName, TinyName);
			}
			return;
		} else if (cause == DamageSource::KickedLeft || cause == DamageSource::KickedRight) { // By being kicked
			if (random < 2) {
				Cprint("{} was sent flying by the kick of {}", TinyName, GiantName);
			} else if (random == 2) {
				Cprint("{} attempted to launch {} into the sky", GiantName, TinyName);
			} else if (random == 3) {
				Cprint("{} tried to learn how to fly from {}", TinyName, GiantName);
			} else if (random == 4) {
				Cprint("{} met the mighty kick of {}", TinyName, GiantName);
			} else if (random == 5) {
				Cprint("{} kicked {} to death", GiantName, TinyName);
			} else if (random == 6) {
				Cprint("{} kick was too strong for {} to handle", GiantName, TinyName);
			} else if (random >= 7) {
				Cprint("{} obliterated {} with a kick", GiantName, TinyName);
			}
			return;
		}
	}
}
