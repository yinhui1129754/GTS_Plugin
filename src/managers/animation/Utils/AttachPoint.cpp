#include "managers/animation/Utils/AttachPoint.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;

namespace {
	void TestActorAttaches() {
		Actor* testActorPtr = PlayerCharacter::GetSingleton();
		Actor& testActorRef = *testActorPtr;
		ActorHandle testHandle = testActorPtr->CreateRefHandle();
		FormID testFormID = 0x14;

		AttachToObjectA(testActorPtr, testActorPtr);
		AttachToObjectA(testActorRef, testActorPtr);
		AttachToObjectA(testHandle, testActorPtr);
		AttachToObjectA(testFormID, testActorPtr);
	}
}
