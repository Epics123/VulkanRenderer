#pragma once

#include "../Device.h"
#include "../FrameInfo.h"
#include "../Enums.h"
#include "../GameObject.h"

#include <vector>
#include <memory>
#include <string.h>

class ImGuiSystem
{
public:
	ImGuiSystem(Device& device);
	~ImGuiSystem();

	ImGuiSystem(const ImGuiSystem&) = delete;
	ImGuiSystem& operator=(const ImGuiSystem&) = delete;

	struct ViewportInfo
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
	};

	void drawViewport();

	void drawImGui(FrameInfo& frameInfo);
	void drawDebugWindow();
	void drawRenderModeText(RenderMode mode);
	void drawFrameInfo(float framerate, float frameTime);
	void drawDeviceSpecs();
	void drawSceneInfo(FrameInfo& frameInfo);
	void drawShowGridText(FrameInfo& frameInfo);
	void drawGizmos(FrameInfo& frameInfo);

	GameObject& getSelectedObject(FrameInfo& frameInfo);

	void setViewportInfo(float x, float y, float width, float height);

	static void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawVec3Control(const char* label, glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawFloatControl(const char* label, float& value, float resetValue = 1.0f, float columnWidth = 100.0f, float min = 0.0f, float max = 0.0f, bool shouldClamp = false);

private:
	Device& device;

	uint32_t selectionContext = 0;

	std::string cpuInfo;

	ViewportInfo viewportInfo{};
};