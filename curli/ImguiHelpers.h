#pragma once
#include <imgui.h>
#include <glad/glad.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace gui
{
	void StyleColorsAlteredDracula()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.05f, 0.05f, 0.085f, .8f };
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Border
		colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.99f };
		colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

		// Text
		colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.94f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };
		colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

		// Popups
		colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.94f };
		colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13, 0.17, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.24, 0.24f, 0.32f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
		colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

		// Seperator
		colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
		colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
		colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
		colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
		colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

		// Docking
		//colors[ImGuiCol_DockingPreview] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

		auto& style = ImGui::GetStyle();
		style.TabRounding = 4;
		style.ScrollbarRounding = 9;
		style.WindowRounding = 7;
		style.GrabRounding = 3;
		style.FrameRounding = 3;
		style.PopupRounding = 4;
		style.ChildRounding = 4;
	}
	
	void InitImgui(GLFWwindow* window)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		
		//ImGui::StyleColorsDark();
		StyleColorsAlteredDracula();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 130");
	}

	void GetNewImguiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void RenderImgui()
	{
		ImGui::Render();
		ImGuiIO& io = ImGui::GetIO();
		glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void TerminateImgui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	
}

namespace ImGui
{
	void ToggleButton(const char* str_id, bool* v)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();
		float width = height * 1.55f;
		float radius = height * 0.50f;

		if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
			*v = !*v;
		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = *v ? IM_COL32(0.74f*255.f, 0.58f*255.f, 0.98f*255.f, 255) : IM_COL32(0.24*255.f, 0.24f*255.f, 0.32f*255.f, 255);
		else
			col_bg = *v ? IM_COL32(0.44f*255.f, 0.37f*255.f, 0.61f*255.f, 255) : IM_COL32(0.16f*255.f, 0.16f*255.f, 0.21f*255.f, 255);

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}
}