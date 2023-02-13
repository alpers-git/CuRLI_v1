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
			DrawSceneObjectsList();
			DrawCameraController();
		}

	private:
		entt::entity selectedSceneObject;
		void DrawSceneObjectsList()
		{
			if (ImGui::CollapsingHeader("Scene Objects"))
			{
				if (ImGui::BeginListBox("##1", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
				{
					int n = 0;
					for (std::map<std::string, entt::entity>::iterator
						it = scene->sceneObjectsBegin(); it != scene->sceneObjectsEnd(); ++it)
					{
						const bool is_selected = (selectedSceneObject == it->second);
						if (ImGui::Selectable(it->first.c_str(), is_selected))
							selectedSceneObject = it->second;

						if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}

						n++;
					}
					ImGui::EndListBox();
				}
				DrawSceneObjectEditor();
				ImGui::Separator();
			}
		}
		void DrawSceneObjectEditor()
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("Components", tab_bar_flags))
			{
				//Draw transform tab
				scene->registry.view<CTransform>()
					.each([&](auto e,  auto& t)
					{
							if (e== selectedSceneObject && ImGui::BeginTabItem("Transform"))
							{
								glm::vec3 p = t.GetPosition();
								if (ImGui::DragFloat3("Position", &p[0]))
									t.SetPosition(p);
								glm::vec3 r = t.GetRotation();
								if (ImGui::DragFloat3("Rotation", &r[0]))
									t.SetEulerRotation(r);
								glm::vec3 s = t.GetScale();
								if (ImGui::DragFloat3("Scale", &s[0]))
									t.SetScale(s);
								glm::vec3 l = t.GetPivot();
								if (ImGui::DragFloat3("Pivot", &l[0]))
									t.SetPivot(l);
								ImGui::EndTabItem();
							}
					});

				//Draw Material tab
				scene->registry.view<CPhongMaterial>()
					.each([&](auto e, auto& m)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("Material"))
							{
								ImGui::ColorEdit3("Ambient", &(m.ambient[0]));
								ImGui::ColorEdit3("Diffuse", &(m.diffuse[0]));
								ImGui::ColorEdit3("Specular", &(m.specular[0]));
								ImGui::DragFloat("Shininess", &m.shininess, 0.01f, 0.0f);
								ImGui::EndTabItem();
							}
						});
				//Draw Light tab
				scene->registry.view<CLight>()
					.each([&](auto e, auto& l)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("Light"))
							{
								ImGui::Text(l.GetLightType() == LightType::POINT ? "Point" :
									(l.GetLightType() == LightType::DIRECTIONAL ? "Directional" : "Spot"));
								ImGui::DragFloat("Intensity", &l.intensity, 0.01f, 0.0f, 1.0f);
								ImGui::ColorEdit3("Color", &(l.color[0]));
								if (l.GetLightType() == LightType::POINT ||
									l.GetLightType() == LightType::SPOT)
								{
									ImGui::DragFloat3("Position", &(l.position[0]));

								}
								if (l.GetLightType() == LightType::DIRECTIONAL ||
									l.GetLightType() == LightType::SPOT)
								{
									ImGui::DragFloat3("Direction", &(l.position[0]));
								}
								if (l.GetLightType() == LightType::SPOT)
								{
									ImGui::DragFloat3("Inner Cutoff", &(l.innerCutOff));
									ImGui::DragFloat3("Outter Cutoff", &(l.outerCutoff));
								}
								ImGui::EndTabItem();
							}
						});
				//Draw Rigidbody tab
				scene->registry.view<CRigidBody>()
					.each([&](auto e, auto& r)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("RigidBody"))
							{
								ImGui::DragFloat("Mass", &r.mass, 0.01f, 0.0f);
								ImGui::DragFloat("Drag", &r.drag, 0.01f, 0.0f);
								ImGui::BeginDisabled();
								ImGui::DragFloat3("Position", &r.position[0]);
								ImGui::DragFloat3("Velocity", &r.velocity[0]);
								ImGui::DragFloat3("Acceleration", &r.acceleration[0]);
								ImGui::DragFloat3("Rotation", &r.rotation[0]);
								ImGui::EndDisabled();
								ImGui::EndTabItem();
							}
						});

				//Draw CTriMesh tab
				scene->registry.view<CTriMesh>()
					.each([&](auto e, auto& m)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("Mesh"))
							{
								ImGui::Text("Mesh info");
								ImGui::SameLine();
								if (ImGui::SmallButton("Calculate Normals"))
									m.ComputeNormals();
								ImGui::Text("# Vertices:");
								ImGui::SameLine();
								ImGui::TextColored(ImColor(0.6f,0.7f,0.8f), "%d", m.GetNumVertices());
								ImGui::Text("# Faces:");
								ImGui::SameLine();
								ImGui::TextColored(ImColor(0.6f, 0.7f, 0.8f), "%d", m.GetNumFaces());
								ImGui::Text("# Normal:");
								ImGui::SameLine();
								ImGui::TextColored(ImColor(0.4f, 0.3f, 0.8f), "%d", m.GetNumNormals());
								ImGui::Text("# UVs:");
								ImGui::SameLine();
								ImGui::TextColored(ImColor(0.4f, 0.3f, 0.8f), "%d", m.GetNumTextureVertices());
								ImGui::Text("Bounds:");
								ImGui::SameLine();
								ImGui::TextColored(ImColor(0.4f, 0.3f, 0.8f), "Min: %.2f %.2f %.2f,\nMax: %.2f %.2f %.2f",
									m.GetBoundingBoxMin().x, m.GetBoundingBoxMin().y, m.GetBoundingBoxMin().z,
									m.GetBoundingBoxMax().x, m.GetBoundingBoxMax().y, m.GetBoundingBoxMax().z);
								ImGui::EndTabItem();
							}
						});
				//Draw Rigidbody tab
				scene->registry.view<CVertexArrayObject>()
					.each([&](auto e, auto& v)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("VAO"))
							{
								ImGui::Text("Vertex Array Object info");
								ImGui::Checkbox("Visible", &v.visible);

								const char* modes[] = { "GL_POINTS", "GL_LINES", "GL_LINE_STRIP", "GL_LINE_LOOP",
									"GL_TRIANGLES", "GL_TRIANGLE_STRIP", "GL_TRIANGLE_FAN"};
								int item_current_idx = v.GetDrawMode();
								const char* combo_preview_value = modes[item_current_idx];
								if (ImGui::BeginCombo("Modes", combo_preview_value, 0))
								{
									for (int n = 0; n < IM_ARRAYSIZE(modes); n++)
									{
										const bool is_selected = (item_current_idx == n);
										if (ImGui::Selectable(modes[n], is_selected))
										{
											item_current_idx = n;
											v.SetDrawMode(n);
										}

										if (is_selected)
											ImGui::SetItemDefaultFocus();
									}
									ImGui::EndCombo();
								}

								for (size_t i = 0; i < v.GetNumVBOs(); i++)
								{
									const auto& vbo = v.GetVBO(i);
									ImGui::Text("Attribute:");
									ImGui::SameLine();
									ImGui::TextColored(ImColor(0.6f, 0.2f, 0.6f), "%s (layout = %d)", vbo.attribName.c_str(), vbo.glID);
									ImGui::Text("Count:");
									ImGui::SameLine();
									ImGui::TextColored(ImColor(0.6f, 0.7f, 0.8f), "%d", vbo.dataSize);
									ImGui::Text("Attrib Size:");
									ImGui::SameLine();
									ImGui::TextColored(ImColor(0.6f, 0.7f, 0.8f), "%d", vbo.attribSize);
									ImGui::Text("Stride:");
									ImGui::SameLine();
									ImGui::TextColored(ImColor(0.4f, 0.3f, 0.8f), "%d", vbo.stride);
									ImGui::Text("Offset:");
									ImGui::SameLine();
									ImGui::TextColored(ImColor(0.4f, 0.3f, 0.8f), "%d", vbo.offset);

									if(i+1 != v.GetNumVBOs())
										ImGui::Separator();

								}
								ImGui::EndTabItem();
							}
						});

				ImGui::EndTabBar();
			}
		}
		void DrawCameraController()
		{
			if (ImGui::CollapsingHeader("Camera"))
			{
				auto& camera = scene->camera;
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
					if (ImGui::DragFloat("Radius", &d, 0.01f, 0.0001f))
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
				ImGui::Separator();
			}
		}
	};
}