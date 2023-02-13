#pragma once
#include <imgui.h>
#include <ImguiHelpers.h>
#include <GLFWHandler.h>
#include <Scene.h>
#include <string>

namespace gui
{
	template <class T>
	class GUIManager
	{
	public:
		GUIManager(std::shared_ptr<Scene> scene)
			:scene(scene) 
		{}

		void Initialize(GLFWwindow* window)
		{
			gui::InitImgui(window);
			static_cast<T*>(this)->Start();
		}
		
		void DrawGUI()
		{
			gui::GetNewImguiFrame();
			SetViewSizeAndPos();
			ImGui::Begin(windowName.c_str(), 0, windowFlags);
			static_cast<T*>(this)->Draw();
			ImGui::End();
			gui::RenderImgui();
		}

		void Terminate()
		{
			static_cast<T*>(this)->End();
			gui::TerminateImgui();
		}

	protected:
		/*
		* Called before application loop
		*/
		void Start() {};

		/*
		* Called in application loop
		*/
		virtual void Draw() = 0;

		/*
		* Called after render loop
		*/
		void End() {};

		void SetViewSizeAndPos()
		{
			const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x / 5, main_viewport->WorkSize.y / 2));
			ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x - main_viewport->WorkSize.x / 5 - 5, main_viewport->WorkPos.y + 5));
		};

		std::shared_ptr<Scene> scene;
		ImGuiWindowFlags windowFlags;
		std::string windowName = "GUI";
	};

	class ControlPanel : public GUIManager<ControlPanel>
	{
	public:
		ControlPanel(std::shared_ptr<Scene> scene) : GUIManager(scene){}
		
		void Start() {
			windowName = "Control Panel";
			windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
		};

		void Draw() {
			if (ImGui::CollapsingHeader("Scene Objects"))
			{
				if (ImGui::BeginListBox("##1", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
				{
					int n = 0;
					for (std::map<std::string, entt::entity>::iterator
						it = scene->sceneObjectsBegin(); it != scene->sceneObjectsEnd(); ++it)
					{
						const bool is_selected = (selectedSceneObjectIndex == n);
						if (ImGui::Selectable(it->first.c_str(), is_selected))
							selectedSceneObjectIndex = n;

						if (is_selected)
							ImGui::SetItemDefaultFocus();

						n++;
					}
					ImGui::EndListBox();
				}
				ImGui::Separator();
			}
			DrawCameraController();
		}

	private:
		int selectedSceneObjectIndex = -1;
		void DrawCameraController()
		{
			if (ImGui::CollapsingHeader("Camera"))
			{
				auto camera = scene->camera;
				static bool orbit = false;
				ImGui::ToggleButton("type", &orbit);
				ImGui::SameLine();
				ImGui::Text(orbit ? "Orbital" : "Look at");
				glm::vec3 c = camera.GetCenter();
				ImGui::SameLine();
				if (ImGui::Button("Reset Camera"))
				{
					scene->camera.SetOrbitDistance(50.f);
					scene->camera.SetCenter({ 0,0,0 });
					scene->camera.SetOrbitAngles({ 90,0,0 });
				}
				if (ImGui::DragFloat3("Center", &c[0], 0.01f))
				{
					camera.SetCenter(c);
				}
				if (orbit)
				{
					glm::vec3 a = glm::degrees(camera.GetOrbitAngles());
					if (ImGui::DragFloat3("Angles", &a[0], 0.01f))
					{
						camera.SetOrbitAngles(a);
					}
					float d = camera.GetOrbitDistance();
					if (ImGui::DragFloat("Radius", &d, 0.01f))
					{
						camera.SetOrbitDistance(d);
					}
				}
				else
				{
					glm::vec3 p = camera.GetLookAtEye();
					if (ImGui::DragFloat3("Position", &p[0], 0.01f))
					{
						camera.SetLookAtEye(p);
					}
					glm::vec3 u = camera.GetLookAtUp();
					if (ImGui::DragFloat3("Up", &u[0], 0.01f))
					{
						camera.SetLookAtUp(u);
					}
				}
			}
			ImGui::Separator();
		}
	};
}