#pragma once
#include "UI/DebugAPI.hpp"
#include "data/world.hpp"

#include <windows.h>

using namespace Gts;

std::vector<DebugAPILine*> DebugAPI::LinesToDraw;

bool DebugAPI::CachedMenuData;

float DebugAPI::ScreenResX;
float DebugAPI::ScreenResY;

std::vector<std::string> DebugOverlayMenu::Hidden_Sources;

DebugAPILine::DebugAPILine(glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness, unsigned __int64 destroyTickCount)
{
	From = from;
	To = to;
	Color = color;
	fColor = DebugAPI::RGBToHex(color);
	Alpha = color.a * 100.0f;
	LineThickness = lineThickness;
	DestroyTickCount = destroyTickCount;
}

void DebugAPI::DrawLineForMS(const glm::vec3& from, const glm::vec3& to, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	DebugAPILine* oldLine = GetExistingLine(from, to, color, lineThickness);
	if (oldLine) {
		oldLine->From = from;
		oldLine->To = to;
		oldLine->DestroyTickCount = GetTickCount64() + liftetimeMS;
		oldLine->LineThickness = lineThickness;
		return;
	}

	DebugAPILine* newLine = new DebugAPILine(from, to, color, lineThickness, GetTickCount64() + liftetimeMS);
	LinesToDraw.push_back(newLine);
}

void DebugAPI::Update()
{
	auto hud = GetHUD();
	if (!hud || !hud->uiMovie) {
		return;
	}

	CacheMenuData();
	ClearLines2D(hud->uiMovie);

	for (int i = 0; i < LinesToDraw.size(); i++)
	{
		DebugAPILine* line = LinesToDraw[i];

		DrawLine3D(hud->uiMovie, line->From, line->To, line->fColor, line->LineThickness, line->Alpha);

		if (GetTickCount64() > line->DestroyTickCount) {
			LinesToDraw.erase(LinesToDraw.begin() + i);
			delete line;

			i--;
			continue;
		}
	}
}

void DebugAPI::DrawBoundsForMS(ObjectBound objectBound, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	auto boundRight = objectBound.GetBoundRightVectorRotated();
	auto boundForward = objectBound.GetBoundForwardVectorRotated();
	auto boundUp = objectBound.GetBoundUpVectorRotated();

	auto objectLocation = objectBound.worldBoundMin;

	// x axis
	glm::vec3 glmXStart1(objectLocation.x, objectLocation.y, objectLocation.z);
	glm::vec3 glmXEnd1(glmXStart1.x + boundRight.x, glmXStart1.y + boundRight.y, glmXStart1.z + boundRight.z);
	// + y
	glm::vec3 glmXStart2(objectLocation.x + boundUp.x, objectLocation.y + boundUp.y, objectLocation.z + boundUp.z);
	glm::vec3 glmXEnd2(glmXStart2.x + boundRight.x, glmXStart2.y + boundRight.y, glmXStart2.z + boundRight.z);
	// + z
	glm::vec3 glmXStart3(objectLocation.x + boundForward.x, objectLocation.y + boundForward.y, objectLocation.z + boundForward.z);
	glm::vec3 glmXEnd3(glmXStart3.x + boundRight.x, glmXStart3.y + boundRight.y, glmXStart3.z + boundRight.z);
	// + y + z
	glm::vec3 glmXStart4(objectLocation.x + boundUp.x + boundForward.x, objectLocation.y + boundUp.y + boundForward.y, objectLocation.z + boundUp.z + boundForward.z);
	glm::vec3 glmXEnd4(glmXStart4.x + boundRight.x, glmXStart4.y + boundRight.y, glmXStart4.z + boundRight.z);

	// y axis
	glm::vec3 glmYStart1(objectLocation.x, objectLocation.y, objectLocation.z);
	glm::vec3 glmYEnd1(glmYStart1.x + boundForward.x, glmYStart1.y + boundForward.y, glmYStart1.z + boundForward.z);
	// + z
	glm::vec3 glmYStart2(objectLocation.x + boundUp.x, objectLocation.y + boundUp.y, objectLocation.z + boundUp.z);
	glm::vec3 glmYEnd2(glmYStart2.x + boundForward.x, glmYStart2.y + boundForward.y, glmYStart2.z + boundForward.z);
	// + x
	glm::vec3 glmYStart3(objectLocation.x + boundRight.x, objectLocation.y + boundRight.y, objectLocation.z + boundRight.z);
	glm::vec3 glmYEnd3(glmYStart3.x + boundForward.x, glmYStart3.y + boundForward.y, glmYStart3.z + boundForward.z);
	// + z + x
	glm::vec3 glmYStart4(objectLocation.x + boundUp.x + boundRight.x, objectLocation.y + boundUp.y + boundRight.y, objectLocation.z + boundUp.z + boundRight.z);
	glm::vec3 glmYEnd4(glmYStart4.x + boundForward.x, glmYStart4.y + boundForward.y, glmYStart4.z + boundForward.z);

	// z axis
	glm::vec3 glmZStart1(objectLocation.x, objectLocation.y, objectLocation.z);
	glm::vec3 glmZEnd1(glmZStart1.x + boundUp.x, glmZStart1.y + boundUp.y, glmZStart1.z + boundUp.z);
	// + x
	glm::vec3 glmZStart2(objectLocation.x + boundRight.x, objectLocation.y + boundRight.y, objectLocation.z + boundRight.z);
	glm::vec3 glmZEnd2(glmZStart2.x + boundUp.x, glmZStart2.y + boundUp.y, glmZStart2.z + boundUp.z);
	// + y
	glm::vec3 glmZStart3(objectLocation.x + boundForward.x, objectLocation.y + boundForward.y, objectLocation.z + boundForward.z);
	glm::vec3 glmZEnd3(glmZStart3.x + boundUp.x, glmZStart3.y + boundUp.y, glmZStart3.z + boundUp.z);
	// + x + y
	glm::vec3 glmZStart4(objectLocation.x + boundRight.x + boundForward.x, objectLocation.y + boundRight.y + boundForward.y, objectLocation.z + boundRight.z + boundForward.z);
	glm::vec3 glmZEnd4(glmZStart4.x + boundUp.x, glmZStart4.y + boundUp.y, glmZStart4.z + boundUp.z);

	DebugAPI::DrawLineForMS(glmZStart1, glmZEnd1, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmZStart2, glmZEnd2, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmZStart3, glmZEnd3, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmZStart4, glmZEnd4, liftetimeMS, color, lineThickness);

	DebugAPI::DrawLineForMS(glmYStart1, glmYEnd1, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmYStart2, glmYEnd2, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmYStart3, glmYEnd3, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmYStart4, glmYEnd4, liftetimeMS, color, lineThickness);

	DebugAPI::DrawLineForMS(glmXStart1, glmXEnd1, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmXStart2, glmXEnd2, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmXStart3, glmXEnd3, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmXStart4, glmXEnd4, liftetimeMS, color, lineThickness);
}

void DebugAPI::DrawSphere(glm::vec3 origin, float radius, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	DrawCircle(origin, radius, glm::vec3(0.0f, 0.0f, 0.0f), liftetimeMS, color, lineThickness);
	DrawCircle(origin, radius, glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f), liftetimeMS, color, lineThickness);
}

void DebugAPI::DrawCircle(glm::vec3 origin, float radius, glm::vec3 eulerAngles, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	glm::vec3 lastEndPos = GetPointOnRotatedCircle(origin, radius, CIRCLE_NUM_SEGMENTS, (float)(CIRCLE_NUM_SEGMENTS - 1), eulerAngles);

	for (int i = 0; i <= CIRCLE_NUM_SEGMENTS; i++)
	{
		glm::vec3 currEndPos = GetPointOnRotatedCircle(origin, radius, (float)i, (float)(CIRCLE_NUM_SEGMENTS - 1), eulerAngles);

		DrawLineForMS(
			lastEndPos,
			currEndPos,
			liftetimeMS,
			color,
			lineThickness);

		lastEndPos = currEndPos;
	}
}

void DebugAPI::DrawHalfCircle(glm::vec3 origin, float radius, glm::vec3 eulerAngles, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	glm::vec3 lastEndPos = GetPointOnRotatedCircle(origin, radius, CIRCLE_NUM_SEGMENTS/2, (float)(CIRCLE_NUM_SEGMENTS - 1), eulerAngles);

	for (int i = 0; i <= CIRCLE_NUM_SEGMENTS/2; i++)
	{
		glm::vec3 currEndPos = GetPointOnRotatedCircle(origin, radius, (float)i, (float)(CIRCLE_NUM_SEGMENTS - 1), eulerAngles);

		DrawLineForMS(
			lastEndPos,
			currEndPos,
			liftetimeMS,
			color,
			lineThickness);

		lastEndPos = currEndPos;
	}
}

void DebugAPI::DrawCapsule(glm::vec3 start, glm::vec3 end, float radius, glm::mat4 transform, int liftetimeMS, const glm::vec4& color, float lineThickness) {
	// From https://gamedev.stackexchange.com/questions/162426/how-to-draw-a-3d-capsule
	// Local basis
	float pi = glm::pi<float>();
	glm::vec3 axis = end - start;
	float length = glm::length(axis);
	glm::vec3 localZ = axis/length;
	glm::vec3 localX = GetAnyPerpendicularUnitVector(localZ);
	glm::vec3 localY = glm::cross(localZ, localX);
	localX = glm::cross(localY, localZ); // Reorthonalize

	auto shaft_point = [&](float u, float v) {
				   return start
				          + localX * glm::cos(2 * pi * u) * radius
				          + localY * glm::sin(2 * pi * u) * radius
				          + localZ * v * length;
			   };
	auto start_hemi_point = [&](float u, float v) {
					float latitude = (pi/2) * (v - 1);
					return start
					       + localX * cos(2 * pi * u) * cos(latitude) * radius
					       + localY * sin(2 * pi * u) * cos(latitude) * radius
					       + localZ * sin(latitude) * radius;
				};
	auto end_hemi_point = [&](float u, float v) {
				      float latitude = (pi/2) * v;
				      return end
				             + localX * cos(2 * pi * u) * cos(latitude) * radius
				             + localY * sin(2 * pi * u) * cos(latitude) * radius
				             + localZ * sin(latitude) * radius;
			      };
	auto apply_transform = [](glm::vec3 vec, glm::mat4 mat) {
				       return glm::vec3(mat * glm::vec4(vec, 1.0));
			       };
	// Loop1
	DrawLineForMS(
		apply_transform(shaft_point(0.0, 0.0), transform),
		apply_transform(shaft_point(0.0, 1.0), transform),
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		apply_transform(shaft_point(0.5, 0.0), transform),
		apply_transform(shaft_point(0.5, 1.0), transform),
		liftetimeMS,
		color,
		lineThickness);
	// Loop2
	DrawLineForMS(
		apply_transform(shaft_point(0.25, 0.0), transform),
		apply_transform(shaft_point(0.25, 1.0), transform),
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		apply_transform(shaft_point(0.75, 0.0), transform),
		apply_transform(shaft_point(0.75, 1.0), transform),
		liftetimeMS,
		color,
		lineThickness);

	// Start hemi
	const int STEPS = 20;
	glm::vec3 prev_point = apply_transform(start_hemi_point(0.0, 0.0), transform);
	for (int i = 1; i<STEPS; i++) {
		float x = (1.0/STEPS*i);

		float v = glm::sin(x*pi);
		float u = (x<0.5) ? 0.0 : 0.5;
		glm::vec3 next_point = apply_transform(start_hemi_point(u, v), transform);
		DrawLineForMS(
			prev_point,
			next_point,
			liftetimeMS,
			color,
			lineThickness);
		prev_point = next_point;
	}
	prev_point = apply_transform(start_hemi_point(0.0, 0.25), transform);
	for (int i = 1; i<STEPS; i++) {
		float x = (1.0/STEPS*i);
		float v = glm::sin(x*pi);
		float u = (x<0.5) ? 0.25 : 0.75;
		glm::vec3 next_point = apply_transform(start_hemi_point(u, v), transform);
		DrawLineForMS(
			prev_point,
			next_point,
			liftetimeMS,
			color,
			lineThickness);
		prev_point = next_point;
	}

	// end hemi
	prev_point = apply_transform(end_hemi_point(0.0, 0.0), transform);
	for (int i = 1; i<STEPS; i++) {
		float x = (1.0/STEPS*i);
		float v = glm::sin(x*pi);
		float u = (x<0.5) ? 0.0 : 0.5;
		glm::vec3 next_point = apply_transform(end_hemi_point(u, v), transform);
		DrawLineForMS(
			prev_point,
			next_point,
			liftetimeMS,
			color,
			lineThickness);
		prev_point = next_point;
	}
	prev_point = apply_transform(end_hemi_point(0.0, 0.25), transform);
	for (int i = 1; i<STEPS; i++) {
		float x = (1.0/STEPS*i);
		float v = glm::sin(x*pi);
		float u = (x<0.5) ? 0.25 : 0.75;
		glm::vec3 next_point = apply_transform(end_hemi_point(u, v), transform);
		DrawLineForMS(
			prev_point,
			next_point,
			liftetimeMS,
			color,
			lineThickness);
		prev_point = next_point;
	}
}

void DebugAPI::DrawBox(glm::vec3 origin, glm::vec3 halfExtents, glm::mat4 transform, int liftetimeMS, const glm::vec4& color, float lineThickness) {
	glm::vec3 p000 = ApplyTransform(origin + CompMult(glm::vec3(-1.,-1.,-1.), halfExtents), transform);
	glm::vec3 p100 = ApplyTransform(origin + CompMult(glm::vec3( 1.,-1.,-1.), halfExtents), transform);
	glm::vec3 p101 = ApplyTransform(origin + CompMult(glm::vec3( 1.,-1., 1.), halfExtents), transform);
	glm::vec3 p001 = ApplyTransform(origin + CompMult(glm::vec3(-1.,-1., 1.), halfExtents), transform);
	glm::vec3 p010 = ApplyTransform(origin + CompMult(glm::vec3(-1., 1.,-1.), halfExtents), transform);
	glm::vec3 p110 = ApplyTransform(origin + CompMult(glm::vec3( 1., 1.,-1.), halfExtents), transform);
	glm::vec3 p111 = ApplyTransform(origin + CompMult(glm::vec3( 1., 1., 1.), halfExtents), transform);
	glm::vec3 p011 = ApplyTransform(origin + CompMult(glm::vec3(-1., 1., 1.), halfExtents), transform);
	DrawLineForMS(
		p000,
		p100,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p100,
		p101,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p101,
		p001,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p001,
		p000,
		liftetimeMS,
		color,
		lineThickness);

	DrawLineForMS(
		p010,
		p110,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p110,
		p111,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p111,
		p011,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p011,
		p010,
		liftetimeMS,
		color,
		lineThickness);

	DrawLineForMS(
		p000,
		p010,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p001,
		p011,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p101,
		p111,
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		p100,
		p110,
		liftetimeMS,
		color,
		lineThickness);
}

void DebugAPI::DrawTriangle(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::mat4 transform, int liftetimeMS, const glm::vec4& color, float lineThickness) {
	auto apply_transform = [](glm::vec3 vec, glm::mat4 mat) {
				       return glm::vec3(mat * glm::vec4(vec, 1.0));
			       };

	DrawLineForMS(
		apply_transform(pointA, transform),
		apply_transform(pointB, transform),
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		apply_transform(pointB, transform),
		apply_transform(pointC, transform),
		liftetimeMS,
		color,
		lineThickness);
	DrawLineForMS(
		apply_transform(pointC, transform),
		apply_transform(pointA, transform),
		liftetimeMS,
		color,
		lineThickness);
}


DebugAPILine* DebugAPI::GetExistingLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color, float lineThickness)
{
	for (int i = 0; i < LinesToDraw.size(); i++)
	{
		DebugAPILine* line = LinesToDraw[i];

		if (
			IsRoughlyEqual(from.x, line->From.x, DRAW_LOC_MAX_DIF) &&
			IsRoughlyEqual(from.y, line->From.y, DRAW_LOC_MAX_DIF) &&
			IsRoughlyEqual(from.z, line->From.z, DRAW_LOC_MAX_DIF) &&
			IsRoughlyEqual(to.x, line->To.x, DRAW_LOC_MAX_DIF) &&
			IsRoughlyEqual(to.y, line->To.y, DRAW_LOC_MAX_DIF) &&
			IsRoughlyEqual(to.z, line->To.z, DRAW_LOC_MAX_DIF) &&
			IsRoughlyEqual(lineThickness, line->LineThickness, DRAW_LOC_MAX_DIF) &&
			color == line->Color) {
			return line;
		}
	}

	return nullptr;
}

void DebugAPI::DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, float color, float lineThickness, float alpha)
{
	if (IsPosBehindPlayerCamera(from) && IsPosBehindPlayerCamera(to)) {
		return;
	}

	glm::vec2 screenLocFrom = WorldToScreenLoc(movie, from);
	glm::vec2 screenLocTo = WorldToScreenLoc(movie, to);

	DrawLine2D(movie, screenLocFrom, screenLocTo, color, lineThickness, alpha);
}

void DebugAPI::DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness)
{
	DrawLine3D(movie, from, to, RGBToHex(glm::vec3(color.r, color.g, color.b)), lineThickness, color.a * 100.0f);
}

void DebugAPI::DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, float color, float lineThickness, float alpha)
{
	// all parts of the line are off screen - don't need to draw them
	if (!IsOnScreen(from, to)) {
		return;
	}

	FastClampToScreen(from);
	FastClampToScreen(to);

	// lineStyle(thickness:Number = NaN, color : uint = 0, alpha : Number = 1.0, pixelHinting : Boolean = false,
	// scaleMode : String = "normal", caps : String = null, joints : String = null, miterLimit : Number = 3) :void
	//
	// CapsStyle values: 'NONE', 'ROUND', 'SQUARE'
	// const char* capsStyle = "NONE";

	RE::GFxValue argsLineStyle[3]{ lineThickness, color, alpha};
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsStartPos[2]{ from.x, from.y };
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	RE::GFxValue argsEndPos[2]{ to.x, to.y };
	movie->Invoke("lineTo", nullptr, argsEndPos, 2);

	movie->Invoke("endFill", nullptr, nullptr, 0);
}

void DebugAPI::DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, glm::vec4 color, float lineThickness)
{
	DrawLine2D(movie, from, to, RGBToHex(glm::vec3(color.r, color.g, color.b)), lineThickness, color.a * 100.0f);
}

void DebugAPI::ClearLines2D(RE::GPtr<RE::GFxMovieView> movie)
{
	movie->Invoke("clear", nullptr, nullptr, 0);
}

RE::GPtr<RE::IMenu> DebugAPI::GetHUD()
{
	RE::GPtr<RE::IMenu> hud = RE::UI::GetSingleton()->GetMenu(DebugOverlayMenu::MENU_NAME);
	return hud;
}

float DebugAPI::RGBToHex(glm::vec3 rgb)
{
	return ConvertComponentR(rgb.r * 255) + ConvertComponentG(rgb.g * 255) + ConvertComponentB(rgb.b * 255);
}

// if drawing outside the screen rect, at some point Scaleform seems to start resizing the rect internally, without
// increasing resolution. This will cause all line draws to become more and more pixelated and increase in thickness
// the farther off screen even one line draw goes. I'm allowing some leeway, then I just clamp the
// coordinates to the screen rect.
//
// this is inaccurate. A more accurate solution would require finding the sub vector that overshoots the screen rect between
// two points and scale the vector accordingly. Might implement that at some point, but the inaccuracy is barely noticeable
const float CLAMP_MAX_OVERSHOOT = 10000.0f;
void DebugAPI::FastClampToScreen(glm::vec2& point)
{
	if (point.x < 0.0) {
		float overshootX = abs(point.x);
		if (overshootX > CLAMP_MAX_OVERSHOOT) {
			point.x += overshootX - CLAMP_MAX_OVERSHOOT;
		}
	} else if (point.x > ScreenResX) {
		float overshootX = point.x - ScreenResX;
		if (overshootX > CLAMP_MAX_OVERSHOOT) {
			point.x -= overshootX - CLAMP_MAX_OVERSHOOT;
		}
	}

	if (point.y < 0.0) {
		float overshootY = abs(point.y);
		if (overshootY > CLAMP_MAX_OVERSHOOT) {
			point.y += overshootY - CLAMP_MAX_OVERSHOOT;
		}
	} else if (point.y > ScreenResY) {
		float overshootY = point.y - ScreenResY;
		if (overshootY > CLAMP_MAX_OVERSHOOT) {
			point.y -= overshootY - CLAMP_MAX_OVERSHOOT;
		}
	}
}

float DebugAPI::ConvertComponentR(float value)
{
	return (value * 0xffff) + value;
}

float DebugAPI::ConvertComponentG(float value)
{
	return (value * 0xff) + value;
}

float DebugAPI::ConvertComponentB(float value)
{
	return value;
}

glm::vec2 DebugAPI::WorldToScreenLoc(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 worldLoc)
{
	glm::vec2 screenLocOut;
	RE::NiPoint3 niWorldLoc(worldLoc.x, worldLoc.y, worldLoc.z);

	float zVal;

	RE::NiCamera::WorldPtToScreenPt3(World::WorldToCamera().data, World::ViewPort(), niWorldLoc, screenLocOut.x, screenLocOut.y, zVal, 1e-5f);
	RE::GRectF rect = movie->GetVisibleFrameRect();

	screenLocOut.x = rect.left + (rect.right - rect.left) * screenLocOut.x;
	screenLocOut.y = 1.0f - screenLocOut.y;  // Flip y for Flash coordinate system
	screenLocOut.y = rect.top + (rect.bottom - rect.top) * screenLocOut.y;

	return screenLocOut;
}

DebugOverlayMenu::DebugOverlayMenu()
{
}
void DebugOverlayMenu::Init() {
	if (this->inited) {
		return;
	}
	auto scaleformManager = RE::BSScaleformManager::GetSingleton();
	if (!scaleformManager) {
		log::error("Gts: failed to initialize DebugOverlayMenu - ScaleformManager not found");
		return;
	}

	depthPriority = 0;

	menuFlags.set(RE::UI_MENU_FLAGS::kRequiresUpdate);
	menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);
	menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);

	inputContext = Context::kNone;

	scaleformManager->LoadMovieEx(this, MENU_PATH, [](RE::GFxMovieDef* a_def) -> void
	{
		a_def->SetState(RE::GFxState::StateType::kLog,
		                RE::make_gptr<Logger>().get());
	});
	this->inited = true;

	log::error("Gts: initialize scale forms");
}

DebugOverlayMenu& DebugOverlayMenu::GetSingleton() noexcept {
	static DebugOverlayMenu instance;
	return instance;
}

std::string DebugOverlayMenu::DebugName() {
	return "DebugOverlayMenu";
}

void DebugOverlayMenu::DataReady()
{
	log::info("Gts: registering DebugOverlayMenu...");

	auto ui = RE::UI::GetSingleton();
	if (ui) {
		ui->Register(MENU_NAME, Creator);
		DebugOverlayMenu::Start();

		log::info("Gts: successfully registered DebugOverlayMenu");
	} else {
		log::error("Gts: failed to register DebugOverlayMenu");
	}
}

void DebugOverlayMenu::Start()
{
	auto msgQ = RE::UIMessageQueue::GetSingleton();
	if (msgQ) {
		msgQ->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	} else {
		log::warn("Gts: failed to show DebugOverlayMenu");
	}
}

void DebugOverlayMenu::Update()
{
	DebugAPI::Update();
}

void DebugOverlayMenu::Unload()
{
	auto msgQ = RE::UIMessageQueue::GetSingleton();
	if (msgQ) {
		msgQ->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	} else {
		log::warn("Gts: failed to hide DebugOverlayMenu");
	}
}

void DebugOverlayMenu::Show(std::string source)
{
	auto sourceIdx = std::find(Hidden_Sources.begin(), Hidden_Sources.end(), source);
	if (sourceIdx != Hidden_Sources.end()) {
		Hidden_Sources.erase(sourceIdx);
	}

	if (Hidden_Sources.empty()) {
		ToggleVisibility(true);
	}
}

void DebugOverlayMenu::Hide(std::string source)
{
	auto sourceIdx = std::find(Hidden_Sources.begin(), Hidden_Sources.end(), source);
	if (sourceIdx == Hidden_Sources.end()) {
		Hidden_Sources.push_back(source);
	}

	if (!Hidden_Sources.empty()) {
		ToggleVisibility(false);
	}
}

void DebugOverlayMenu::MenuChange(const MenuOpenCloseEvent* a_event) {
	auto mName = a_event->menuName;
	if (
		mName == RE::JournalMenu::MENU_NAME ||
		mName == RE::InventoryMenu::MENU_NAME ||
		mName == RE::MapMenu::MENU_NAME ||
		mName == RE::BookMenu::MENU_NAME ||
		mName == RE::LockpickingMenu::MENU_NAME ||
		mName == RE::MagicMenu::MENU_NAME ||
		mName == RE::RaceSexMenu::MENU_NAME ||
		mName == RE::CraftingMenu::MENU_NAME ||
		mName == RE::SleepWaitMenu::MENU_NAME ||
		mName == RE::TrainingMenu::MENU_NAME ||
		mName == RE::BarterMenu::MENU_NAME ||
		mName == RE::FavoritesMenu::MENU_NAME ||
		mName == RE::GiftMenu::MENU_NAME ||
		mName == RE::StatsMenu::MENU_NAME ||
		mName == RE::ContainerMenu::MENU_NAME ||
		mName == RE::DialogueMenu::MENU_NAME ||
		mName == RE::MessageBoxMenu::MENU_NAME ||
		mName == RE::TweenMenu::MENU_NAME || // tab menu
		mName == RE::MainMenu::MENU_NAME ||
		mName == "CustomMenu") { // papyrus custom menues go here
		if (a_event->opening) {
			DebugOverlayMenu::Hide(mName.c_str());
		} else {
			DebugOverlayMenu::Show(mName.c_str());
		}
	}
	// for some reason, after each cell change, the menu is hidden and needs to be shown again
	// using a UI message kShow
	else if (mName == RE::LoadingMenu::MENU_NAME) {
		if (!a_event->opening) {
			DebugOverlayMenu::Start();
		}
	}
}

void DebugOverlayMenu::ToggleVisibility(bool mode)
{
	auto ui = RE::UI::GetSingleton();
	if (!ui) {
		return;
	}

	auto overlayMenu = ui->GetMenu(DebugOverlayMenu::MENU_NAME);
	if (!overlayMenu || !overlayMenu->uiMovie) {
		return;
	}

	overlayMenu->uiMovie->SetVisible(mode);
}

void DebugAPI::CacheMenuData()
{
	if (CachedMenuData) {
		return;
	}

	RE::GPtr<RE::IMenu> menu = RE::UI::GetSingleton()->GetMenu(DebugOverlayMenu::MENU_NAME);
	if (!menu || !menu->uiMovie) {
		return;
	}

	RE::GRectF rect = menu->uiMovie->GetVisibleFrameRect();

	ScreenResX = abs(rect.left - rect.right);
	ScreenResY = abs(rect.top - rect.bottom);

	CachedMenuData = true;
	log::info("Gts: DebugAPI::CacheMenuData");

}

// void DebugAPI::ClampVectorToScreen(glm::vec2& from, glm::vec2& to)
// {
// 	glm::vec2 between = to - from;
// 	float m = to.y / from.y;
//
// 	float angle = glm::atan(m);
//
// 	ClampPointToScreen(from, angle);
// 	ClampPointToScreen(to, angle);
// }
//
// void DebugAPI::ClampPointToScreen(glm::vec2& point, float lineAngle)
// {
// 	if (point.y < 0.0)
// 	{
// 		float overshootY = point.y;
// 		float overshootX = glm::tan(lineAngle) * overshootY;
//
// 		point.y += overshootY;
// 		point.x += overshootX;
// 	}
// 	else if (point.y > ScreenResY)
// 	{
// 		float overshootY = point.y - ScreenResY;
// 		float overshootX = glm::tan(lineAngle) * overshootY;
//
// 		point.y -= overshootY;
// 		point.x -= overshootX;
// 	}
//
// 	if (point.x < 0.0)
// 	{
// 		float overshootX = point.x;
// 		float overshootY = glm::tan(lineAngle) * overshootX;
//
// 		point.y += overshootY;
// 		point.x += overshootX;
// 	}
// 	else if (point.x > ScreenResX)
// 	{
// 		float overshootX = point.x - ScreenResX;
// 		float overshootY = glm::tan(lineAngle) * overshootX;
//
// 		point.y -= overshootY;
// 		point.x -= overshootX;
// 	}
// }

bool DebugAPI::IsOnScreen(glm::vec2 from, glm::vec2 to)
{
	return IsOnScreen(from) || IsOnScreen(to);
}

bool DebugAPI::IsOnScreen(glm::vec2 point)
{
	return (point.x <= ScreenResX && point.x >= 0.0 && point.y <= ScreenResY && point.y >= 0.0);
}

void DebugOverlayMenu::AdvanceMovie(float a_interval, std::uint32_t a_currentTime)
{
	RE::IMenu::AdvanceMovie(a_interval, a_currentTime);

	DebugAPI::Update();
}
