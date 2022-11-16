#include "ImGuiSystem.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"

#include "../Pipeline.h"
#include "../Camera.h"
#include "../GameObject.h"

ImGuiSystem::ImGuiSystem(Device& device)
	:device {device}
{

}

void ImGuiSystem::drawImGui(FrameInfo& frameInfo)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawDebugWindow();
	drawFrameInfo(frameInfo.framerate, frameInfo.frameTime);

	ImGui::NewLine();

	drawRenderModeText(frameInfo.renderMode);

	ImGui::NewLine();

	drawSceneInfo(frameInfo);

	ImGui::End();
	ImGui::Render();
}

void ImGuiSystem::drawDebugWindow()
{
	bool debugActive = true;
	ImGui::Begin("Debug Info", &debugActive, ImGuiWindowFlags_MenuBar);
}

void ImGuiSystem::drawRenderModeText(RenderMode mode)
{
	ImGui::Text("Current Render Mode:");
	ImGui::SameLine();
	switch (mode)
	{
	case DEFAULT_LIT:
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "Default Lit");
		break;
	case WIREFRAME:
		ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f), "Wireframe");
		break;
	}
}

void ImGuiSystem::drawFrameInfo(float framerate, float frameTime)
{
	ImGui::Text("FPS: %f /", framerate);
	ImGui::SameLine();
	ImGui::Text("%f ms", frameTime);
}

void ImGuiSystem::drawSceneInfo(FrameInfo& frameInfo)
{
	if (ImGui::CollapsingHeader("Scene"))
	{
		if (ImGui::TreeNode("Camera"))
		{
			DrawVec3Control("Position", frameInfo.camera.position, 0.0f, 120.0f);
			DrawVec3Control("Rotation", frameInfo.camera.rotation, 0.0f, 120.0f);

			ImGui::TreePop();
		}

		for (auto& keyValue : frameInfo.gameObjects)
		{
			GameObject& obj = keyValue.second;

			if (ImGui::TreeNode(obj.getObjectName().c_str()))
			{
				DrawVec3Control("Position", obj.transform.translation, 0.0f, 120.0f);
				DrawVec3Control("Rotation", obj.transform.rotation, 0.0f, 120.0f);
				DrawVec3Control("Scale", obj.transform.scale, 1.0f, 120.0f);

				if (obj.pointLight)
				{
					DrawFloatControl("Intensity", obj.pointLight->intensity, 1.0f, 120.0f, 0.0f, 20.0f, true);
					ImGui::NewLine();
				}

				ImGui::TreePop();
			}
		}
	}
}

void ImGuiSystem::DrawVec3Control(const char* label, glm::vec3& values, float resetValue /*= 0.0f*/, float columnWidth /*= 100.0f*/)
{
	ImGui::PushID(label);

	ImGui::Columns(2);

	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
	if (ImGui::Button("Y", buttonSize))
		values.y = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize))
		values.z = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

void ImGuiSystem::DrawVec3Control(const char* label, glm::vec4& values, float resetValue /*= 0.0f*/, float columnWidth /*= 100.0f*/)
{
	ImGui::PushID(label);

	ImGui::Columns(2);

	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
	if (ImGui::Button("Y", buttonSize))
		values.y = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize))
		values.z = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

void ImGuiSystem::DrawFloatControl(const char* label, float& value, float resetValue, float columnWidth, float min, float max, bool shouldClamp)
{
	ImGui::PushID(label);

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("", buttonSize))
		value = resetValue;
	ImGui::PopStyleColor(3);

	if (shouldClamp)
	{
		value = glm::clamp(value, min, max);
	}

	ImGui::SameLine();
	ImGui::DragFloat("##", &value, 0.1f, min, max, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopItemWidth();
	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

ImGuiSystem::~ImGuiSystem()
{
}
