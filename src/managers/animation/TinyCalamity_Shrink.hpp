

    
using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
    struct CalamityData {
        public:
        CalamityData(Actor* tiny, float until);
        Actor* tiny;
        float until; // Shrink until x size difference
	};

	class Animation_TinyCalamity : public EventListener {
		public:

            [[nodiscard]] static Animation_TinyCalamity& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void RegisterEvents();
            static void RegisterTriggers();

            virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

            static void AddToData(Actor* giant, Actor* tiny, float until);
            static void Remove(Actor* giant);

            static Actor* GetShrinkActor(Actor* giant);
            static float GetShrinkUntil(Actor* giant);

            std::unordered_map<Actor*, CalamityData> data;
        };
    }

