#pragma once
#include "events.hpp"
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "data/world.hpp"

using namespace Gts;

namespace Util {

	inline glm::vec3 CompMult(const glm::vec3& a, const glm::vec3& b) {
		return glm::vec3(a[0]*b[0], a[1]*b[1], a[2]*b[2]);
	}
	inline glm::vec3 HkVecToGlmVec(const RE::hkVector4 &vec) {
		return glm::vec3(vec.quad.m128_f32[0], vec.quad.m128_f32[1], vec.quad.m128_f32[2]);
	}

	inline glm::vec3 HkToGlm(const RE::hkVector4 &vec) {
		return HkVecToGlmVec(vec)  * World::WorldScaleInverse();
	}
	inline glm::mat3 HkToGlm(const RE::hkRotation &mat) {
		return glm::mat3(
			HkVecToGlmVec(mat.col0),
			HkVecToGlmVec(mat.col1),
			HkVecToGlmVec(mat.col2)
			);
	}
	inline glm::mat4 HkToGlm(const RE::hkTransform &transform) {
		return glm::mat4(
			glm::vec4(HkVecToGlmVec(transform.rotation.col0), 0.0),
			glm::vec4(HkVecToGlmVec(transform.rotation.col1), 0.0),
			glm::vec4(HkVecToGlmVec(transform.rotation.col2), 0.0),
			glm::vec4(HkToGlm(transform.translation), 1.0)
			);
	}
	inline glm::mat4 HkToGlm(const RE::hkQsTransform &transform) {
		glm::quat quat = glm::quat(HkVecToGlmVec(transform.rotation.vec));
		glm::mat4 rotMat = glm::mat4_cast(quat);
		return glm::translate(
			(rotMat *
			 glm::scale(
				 glm::mat4(1.0),
				 HkVecToGlmVec(transform.scale)
				 )
			),
			HkToGlm(transform.translation)
			);
	}

	inline glm::vec3 ApplyTransform(glm::vec4 vec, glm::mat4 mat) {
		return glm::vec3(mat * vec);
	}

	inline glm::vec3 ApplyTransform(glm::vec3 vec, glm::mat4 mat) {
		return glm::vec3(mat * glm::vec4(vec, 1.0));
	}

	inline bool IsRoughlyEqual(float first, float second, float maxDif)
	{
		return abs(first - second) <= maxDif;
	}

	inline glm::vec3 RotateVector(glm::quat quatIn, glm::vec3 vecIn) {
		float num = quatIn.x * 2.0f;
		float num2 = quatIn.y * 2.0f;
		float num3 = quatIn.z * 2.0f;
		float num4 = quatIn.x * num;
		float num5 = quatIn.y * num2;
		float num6 = quatIn.z * num3;
		float num7 = quatIn.x * num2;
		float num8 = quatIn.x * num3;
		float num9 = quatIn.y * num3;
		float num10 = quatIn.w * num;
		float num11 = quatIn.w * num2;
		float num12 = quatIn.w * num3;
		glm::vec3 result;
		result.x = (1.0f - (num5 + num6)) * vecIn.x + (num7 - num12) * vecIn.y + (num8 + num11) * vecIn.z;
		result.y = (num7 + num12) * vecIn.x + (1.0f - (num4 + num6)) * vecIn.y + (num9 - num10) * vecIn.z;
		result.z = (num8 - num11) * vecIn.x + (num9 + num10) * vecIn.y + (1.0f - (num4 + num5)) * vecIn.z;
		return result;
	}

	inline glm::vec3 GetCameraPos()
	{
		auto playerCam = RE::PlayerCamera::GetSingleton();
		return glm::vec3(playerCam->pos.x, playerCam->pos.y, playerCam->pos.z);
	}

	inline glm::quat GetCameraRot()
	{
		auto playerCam = RE::PlayerCamera::GetSingleton();

		auto cameraState = playerCam->currentState.get();
		if (!cameraState) {
			return glm::quat();
		}

		RE::NiQuaternion niRotation;
		cameraState->GetRotation(niRotation);

		return glm::quat(niRotation.w, niRotation.x, niRotation.y, niRotation.z);
	}

	inline glm::vec3 NormalizeVector(glm::vec3 p)
	{
		return glm::normalize(p);
	}

	inline glm::vec3 GetForwardVector(glm::quat quatIn)
	{
		// rotate Skyrim's base forward vector (positive Y forward) by quaternion
		return RotateVector(quatIn, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	inline bool IsPosBehindPlayerCamera(glm::vec3 pos)
	{
		auto cameraPos = GetCameraPos();
		auto cameraRot = GetCameraRot();

		auto toTarget = NormalizeVector(pos - cameraPos);
		auto cameraForward = NormalizeVector(GetForwardVector(cameraRot));

		auto angleDif = abs( glm::length(toTarget - cameraForward) );

		// root_two is the diagonal length of a 1x1 square. When comparing normalized forward
		// vectors, this accepts an angle of 90 degrees in all directions
		return angleDif > glm::root_two<float>();
	}

	inline glm::vec3 GetPointOnRotatedCircle(glm::vec3 origin, float radius, float i, float maxI, glm::vec3 eulerAngles) {
		float currAngle = (i / maxI) * glm::two_pi<float>();

		glm::vec3 targetPos(
			(radius * cos(currAngle)),
			(radius * sin(currAngle)),
			0.0f
			);

		auto targetPosRotated = RotateVector(eulerAngles, targetPos);

		return glm::vec3(targetPosRotated.x + origin.x, targetPosRotated.y + origin.y, targetPosRotated.z + origin.z);
	}

	inline glm::vec3 GetAnyPerpendicularUnitVector(const glm::vec3& vec)
	{
		if (vec.y != 0.0f || vec.z != 0.0f) {
			return glm::vec3(1, 0, 0);
		} else {
			return glm::vec3(0, 1, 0);
		}
	}
}

using namespace Util;

namespace Gts {
	inline RE::NiPoint3 Glm2Ni(const glm::vec3 &position) {
		return RE::NiPoint3(position[0], position[1], position[2]);
	}

	inline glm::vec3 Ni2Glm(const RE::NiPoint3 &position) {
		return glm::vec3{position.x, position.y, position.z};
	}

	inline glm::mat4 Ni2Glm(const RE::NiTransform &transform) {
		return glm::mat4(
			glm::vec4(transform.rotate.entry[0][0], transform.rotate.entry[0][1], transform.rotate.entry[0][2], 0.0),
			glm::vec4(transform.rotate.entry[1][0], transform.rotate.entry[1][1], transform.rotate.entry[1][2], 0.0),
			glm::vec4(transform.rotate.entry[2][0], transform.rotate.entry[2][1], transform.rotate.entry[2][2], 0.0),
			glm::vec4(transform.translate[0], transform.translate[1], transform.translate[2], 1.0)
			);
	}
}

struct ObjectBound
{
	ObjectBound()
	{
		boundMin = glm::vec3();
		boundMax = glm::vec3();
		worldBoundMin = glm::vec3();
		worldBoundMax = glm::vec3();
		rotation = glm::vec3();
	}

	ObjectBound(glm::vec3 pBoundMin, glm::vec3 pBoundMax, glm::vec3 pWorldBoundMin, glm::vec3 pWorldBoundMax, glm::vec3 pRotation)
	{
		boundMin = pBoundMin;
		boundMax = pBoundMax;
		worldBoundMin = pWorldBoundMin;
		worldBoundMax = pWorldBoundMax;
		rotation = pRotation;
	}

	inline glm::vec3 GetBoundRightVectorRotated()
	{
		glm::vec3 bound(abs(boundMin.x - boundMax.x), 0.0f, 0.0f);
		auto boundRotated = RotateVector(rotation, bound);

		return boundRotated;
	}

	inline glm::vec3 GetBoundForwardVectorRotated()
	{
		glm::vec3 bound(0.0f, abs(boundMin.y - boundMax.y), 0.0f);
		auto boundRotated = RotateVector(rotation, bound);

		return boundRotated;
	}

	inline glm::vec3 GetBoundUpVectorRotated()
	{
		glm::vec3 bound(0.0f, 0.0f, abs(boundMin.z - boundMax.z));
		auto boundRotated = RotateVector(rotation, bound);

		return boundRotated;
	}

	glm::vec3 boundMin;
	glm::vec3 boundMax;
	glm::vec3 worldBoundMin;
	glm::vec3 worldBoundMax;
	glm::vec3 rotation;
};

class DebugAPILine
{
	public:
		DebugAPILine(glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness, unsigned __int64 destroyTickCount);

		glm::vec3 From;
		glm::vec3 To;
		glm::vec4 Color;
		float fColor;
		float Alpha;
		float LineThickness;

		unsigned __int64 DestroyTickCount;
};

class DebugAPI
{
	public:
		static void Update();

		static RE::GPtr<RE::IMenu> GetHUD();

		static void DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, float color, float lineThickness, float alpha);
		static void DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, glm::vec4 color, float lineThickness);
		static void DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, float color, float lineThickness, float alpha);
		static void DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness);
		static void ClearLines2D(RE::GPtr<RE::GFxMovieView> movie);

		static void DrawLineForMS(const glm::vec3& from, const glm::vec3& to, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawBoundsForMS(ObjectBound objectBound, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawSphere(glm::vec3, float radius, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawCircle(glm::vec3, float radius, glm::vec3 eulerAngles, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawHalfCircle(glm::vec3, float radius, glm::vec3 eulerAngles, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawCapsule(glm::vec3 start, glm::vec3 end, float radius, glm::mat4 transform, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawTriangle(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::mat4 transform, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
		static void DrawBox(glm::vec3 origin, glm::vec3 halfExtents, glm::mat4 transform, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);

		static std::vector<DebugAPILine*> LinesToDraw;

		static bool DEBUG_API_REGISTERED;

		static constexpr int CIRCLE_NUM_SEGMENTS = 32;

		static constexpr float DRAW_LOC_MAX_DIF = 1.0f;

		static glm::vec2 WorldToScreenLoc(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 worldLoc);
		static float RGBToHex(glm::vec3 rgb);

		static void FastClampToScreen(glm::vec2& point);

		// 	static void ClampVectorToScreen(glm::vec2& from, glm::vec2& to);
		// 	static void ClampPointToScreen(glm::vec2& point, float lineAngle);

		static bool IsOnScreen(glm::vec2 from, glm::vec2 to);
		static bool IsOnScreen(glm::vec2 point);

		static void CacheMenuData();

		static bool CachedMenuData;

		static float ScreenResX;
		static float ScreenResY;

	private:
		static float ConvertComponentR(float value);
		static float ConvertComponentG(float value);
		static float ConvertComponentB(float value);
		// returns true if there is already a line with the same color at around the same from and to position
		// with some leniency to bundle together lines in roughly the same spot (see DRAW_LOC_MAX_DIF)
		static DebugAPILine* GetExistingLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color, float lineThickness);
};

class DebugOverlayMenu : RE::IMenu, public Gts::EventListener
{
	public:
		static constexpr const char* MENU_PATH = "GTS_Plugin/GTS_overlay_menu";
		static constexpr const char* MENU_NAME = "GTS Ovelay Menu";

		DebugOverlayMenu();

		[[nodiscard]] static DebugOverlayMenu& GetSingleton() noexcept;

		std::string DebugName() override;
		void DataReady() override;
		void Start() override;
		void Update() override;
		void MenuChange(const MenuOpenCloseEvent* a_event) override;

		static std::vector<std::string> Hidden_Sources;


		static void Unload();

		static void Show(std::string source);
		static void Hide(std::string source);
		static void ToggleVisibility(bool mode);

		static RE::stl::owner<RE::IMenu*> Creator() {
			auto& instance = GetSingleton();
			instance.Init();
			return &instance;
		}

		void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override;

	private:
		void Init();
		bool inited = false;

		class Logger : public RE::GFxLog
		{
			public:
				void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
				{
					std::string fmt(a_fmt ? a_fmt : "");
					while (!fmt.empty() && fmt.back() == '\n') {
						fmt.pop_back();
					}

					std::va_list args;
					va_copy(args, a_argList);
					std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
					std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
					va_end(args);

					log::info("{}"sv, buf.data());
				}
		};
};
