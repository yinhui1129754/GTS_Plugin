#include "managers/TES.hpp"

#include "RE/B/bhkPickData.h"
#include "RE/G/GridCellArray.h"
#include "RE/N/NiAVObject.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TESWorldSpace.h"

/// Copy-pasted from alandtse/CommonLibNG
/// since with default CommonLib it crashes on AE...

namespace RE
{
	TES_Fix* TES_Fix::GetSingleton()
	{
		REL::Relocation<TES_Fix**> singleton{ Offset::TES_Fix::Singleton };
		return *singleton;
	}

	void TES_Fix::ForEachReference(std::function<BSContainer::ForEachResult(TESObjectREFR* a_ref)> a_callback)
	{
		if (interiorCell) {
			interiorCell->ForEachReference([&](TESObjectREFR* a_ref) {
				return a_callback(a_ref);
			});
		} else {
			if (const auto gridLength = gridCells ? gridCells->length : 0; gridLength > 0) {
				std::uint32_t x = 0;
				do {
					std::uint32_t y = 0;
					do {
						if (const auto cell = gridCells->GetCell(x, y); cell && cell->IsAttached()) {
							cell->ForEachReference([&](TESObjectREFR* a_ref) {
								return a_callback(a_ref);
							});
						}
						++y;
					} while (y < gridLength);
					++x;
				} while (x < gridLength);
			}
		}
		if (const auto skyCell = worldSpace ? worldSpace->GetSkyCell() : nullptr; skyCell) {
			skyCell->ForEachReference([&](TESObjectREFR* a_ref) {
				return a_callback(a_ref);
			});
		}
	}

	void TES_Fix::ForEachReferenceInRange(TESObjectREFR* a_origin, float a_radius, std::function<BSContainer::ForEachResult(TESObjectREFR* a_ref)> a_callback)
	{
		if (a_origin && a_radius > 0.0f) {
			const auto originPos = a_origin->GetPosition();

			if (interiorCell) {
				interiorCell->ForEachReferenceInRange(originPos, a_radius, [&](TESObjectREFR* a_ref) {
					return a_callback(a_ref);
				});
			} else {
				if (const auto gridLength = gridCells ? gridCells->length : 0; gridLength > 0) {
					const float yPlus = originPos.y + a_radius;
					const float yMinus = originPos.y - a_radius;
					const float xPlus = originPos.x + a_radius;
					const float xMinus = originPos.x - a_radius;

					std::uint32_t x = 0;
					do {
						std::uint32_t y = 0;
						do {
							if (const auto cell = gridCells->GetCell(x, y); cell && cell->IsAttached()) {
								if (const auto cellCoords = cell->GetCoordinates(); cellCoords) {
									const NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
									if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus && (worldPos.y + 4096.0f) > yMinus) {
										cell->ForEachReferenceInRange(originPos, a_radius, [&](TESObjectREFR* a_ref) {
											return a_callback(a_ref);
										});
									}
								}
							}
							++y;
						} while (y < gridLength);
						++x;
					} while (x < gridLength);
				}
			}

			if (const auto skyCell = worldSpace ? worldSpace->GetSkyCell() : nullptr; skyCell) {
				skyCell->ForEachReferenceInRange(originPos, a_radius, [&](TESObjectREFR* a_ref) {
					return a_callback(a_ref);
				});
			}
		} else {
			ForEachReference([&](TESObjectREFR* a_ref) {
				return a_callback(a_ref);
			});
		}
	}

	TESObjectCELL* TES_Fix::GetCell(const NiPoint3& a_position) const
	{
		using func_t = decltype(&TES_Fix::GetCell);
		REL::Relocation<func_t> func{ RELOCATION_ID(13177, 13322) };
		return func(this, a_position);
	}

	MATERIAL_ID TES_Fix::GetLandMaterialType(const NiPoint3& a_position) const
	{
		using func_t = decltype(&TES_Fix::GetLandMaterialType);
		REL::Relocation<func_t> func{ RELOCATION_ID(13203, 13349) };
		return func(this, a_position);
	}

	bool TES_Fix::GetLandHeight(const NiPoint3& a_positionIn, float& a_heightOut)
	{
		using func_t = decltype(&TES_Fix::GetLandHeight);
		REL::Relocation<func_t> func{ RELOCATION_ID(13198, 13344) };
		return func(this, a_positionIn, a_heightOut);
	}

	TESLandTexture* TES_Fix::GetLandTexture(const NiPoint3& a_position) const
	{
		using func_t = decltype(&TES_Fix::GetLandTexture);
		REL::Relocation<func_t> func{ RELOCATION_ID(13202, 13348) };
		return func(this, a_position);
	}

	float TES_Fix::GetWaterHeight(const NiPoint3& a_pos, TESObjectCELL* a_cell) const
	{
		using func_t = decltype(&TES_Fix::GetWaterHeight);
		REL::Relocation<func_t> func{ RELOCATION_ID(13212, 13358) };
		return func(this, a_pos, a_cell);
	}

	NiAVObject* TES_Fix::Pick(bhkPickData& a_pickData)
	{
		using func_t = decltype(&TES_Fix::Pick);
		REL::Relocation<func_t> func{ RELOCATION_ID(13221, 13371) };
		return func(this, a_pickData);
	}

	void TES_Fix::PurgeBufferedCells()
	{
		using func_t = decltype(&TES_Fix::PurgeBufferedCells);
		REL::Relocation<func_t> func{ RELOCATION_ID(13159, 13299) };
		return func(this);
	}
}
