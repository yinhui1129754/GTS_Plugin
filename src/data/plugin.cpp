#include "data/plugin.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	bool Plugin::Enabled() {
		return Plugin::GetSingleton().enabled.load();
	}

	bool Plugin::InGame() {
		auto ui = RE::UI::GetSingleton();
		if (!ui) {
			return false;
		}
		if (ui->IsMenuOpen(MainMenu::MENU_NAME)) {
			return false;
		}

		return Plugin::GetSingleton().ingame.load();
	}
	void Plugin::SetInGame(bool value) {
		Plugin::GetSingleton().ingame.store(value);
	}

	bool Plugin::Ready() {
		if (Plugin::InGame()) {
			// We are not loading or in the mainmenu
			auto player_char = RE::PlayerCharacter::GetSingleton();
			if (player_char) {
				if (player_char->Is3DLoaded()) {
					// Player is loaded
					return true;
				}
			}
		}
		return false;
	}

	bool Plugin::Live() {
		if (Plugin::Ready()) {
			auto ui = RE::UI::GetSingleton();
			if (!ui->GameIsPaused()) {
				// Not paused
				return true;
			}
		}
		return false;
	}

	bool Plugin::OnMainThread() {
		return Plugin::GetSingleton().onmainthread.load();
	}
	void Plugin::SetOnMainThread(bool value) {
		Plugin::GetSingleton().onmainthread.store(value);
	}

	Plugin& Plugin::GetSingleton() {
		static Plugin instance;
		return instance;
	}
}