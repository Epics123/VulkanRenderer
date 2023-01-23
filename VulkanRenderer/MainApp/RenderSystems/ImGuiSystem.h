#pragma once

#include "../Device.h"
#include "../FrameInfo.h"
#include "../Enums.h"

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

	void drawImGui(FrameInfo& frameInfo);
	void drawDebugWindow();
	void drawRenderModeText(RenderMode mode);
	void drawFrameInfo(float framerate, float frameTime);
	void drawDeviceSpecs();
	void drawSceneInfo(FrameInfo& frameInfo);
	void drawShowGridText(FrameInfo& frameInfo);

	static void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawVec3Control(const char* label, glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawFloatControl(const char* label, float& value, float resetValue = 1.0f, float columnWidth = 100.0f, float min = 0.0f, float max = 0.0f, bool shouldClamp = false);

private:
	Device& device;

	std::string cpuInfo;
};