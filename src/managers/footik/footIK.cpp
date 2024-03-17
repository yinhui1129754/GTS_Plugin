// Module that handles footsteps
#include "managers/footik/footIK.hpp"
#include "colliders/RE.hpp"
#include "util.hpp"

namespace Gts {
	/*
	FootIkData::FootIkData() {
		this->ik = nullptr;
	}
	FootIkData::FootIkData(hkbFootIkDriver* ik) {
		this->ik = nullptr;
		this->UpdateColliders(ik);
	}
	FootIkData::~FootIkData() {
		if (this->ik) {
			this->ik->RemoveReference();
		}
	}

	void FootIkData::UpdateColliders(hkbFootIkDriver* ik) {
		if (this->ik != ik) {
			if (this->ik) {
				this->ik->RemoveReference();
			}
			this->ik = ik;
			if (this->ik) {
				this->ik->AddReference();
			}
		}
		if (this->ik) {
			for (auto& leg: ik->m_internalLegData) {
				auto solver = leg.m_footIkSolver;
				this->AddSolver(solver);
			}
		}
	}

	void FootIkData::ApplyScale(const float& new_scale, const hkVector4& vecScale) {
		if (!this->ik) {
			return;
		}
		this->UpdateColliders(this->ik);

		for (auto& leg: this->ik->m_internalLegData) {
			auto solver = leg.m_footIkSolver;
			try {
				auto& data = this->solver_data.at(solver);
				data.ApplyScale(new_scale, vecScale);
			} catch (std::out_of_range& e) {
				continue;
			}
		}
	}

	void FootIkData::PruneColliders(Actor* actor) {
		if (!this->ik) {
			return;
		}
		for (auto i = this->solver_data.begin(); i != this->solver_data.end();) {
			auto& data = (*i);
			auto solver = data.second.solver;
			bool found = false;
			for (auto& leg: ik->m_internalLegData) {
				if (solver == leg.m_footIkSolver) {
					found = true;
					break;
				}
			}
			if (!found) {
				i = this->solver_data.erase(i);
			} else {
				++i;
			}
		}
	}

	void FootIkData::AddSolver(hkaFootPlacementIkSolver* solver) {
		if (solver) {
			//log::info("Solver: {}, RawName: {}", reinterpret_cast<std::uintptr_t>(solver), GetRawName(solver));
			this->solver_data.try_emplace(solver, solver);
		}
	}*/
}