#pragma once
#include <imgui.h>
#include <ImguiHelpers.h>
#include <GLFWHandler.h>
#include <Scene.h>
#include <windows.h>
#include <string>
#include <ApplicationState.h>

class ApplicationState;

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
			DrawStatPanel();
			DrawTopMenu();
			DrawSceneObjectsList();
			DrawCameraController(scene->camera);
		}

	private:
		entt::entity selectedSceneObject;
		bool openComponentsPopup = false;
		bool openCreateEntityPopup = false;
		
		inline bool openFilePicker(std::string &filePath)
		{
			OPENFILENAME ofn = { 0 };
			TCHAR szFile[260] = { 0 };
			// Initialize remaining fields of OPENFILENAME structure
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			
			bool result = (GetOpenFileName(&ofn) == TRUE);
			filePath = std::string(ofn.lpstrFile);
			return result;

		}
		
		void DrawStatPanel()
		{
			static bool first = true;
			if (first)
			{
				first = false;
				return;
			}
			const ImGuiViewport* stats_viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(ImVec2(stats_viewport->WorkSize.x / 8, 0));
			
			ImGui::SetNextWindowPos(ImVec2(5, stats_viewport->WorkSize.y - ImGui::GetCursorPos().y -20));
			ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::End();
			
		}
		void DrawTopMenu()
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Import .obj from file")) 
					{
						std::string filename;
						if (openFilePicker(filename))
						{
							printf("Loading: %s\n", filename.c_str());
							//check if name ends with .obj
							std::string::size_type idx;
							idx = filename.rfind('.');

							if (idx != std::string::npos)
							{
								std::string extension = filename.substr(idx + 1);
								if (extension == "obj")
								{
									//load obj
									scene->CreateModelObject(filename);
								}
								else
									printf("File extension not supported\n");
							}
							else
								printf("File extension not supported\n");
						}
					}
					ImGui::EndMenu();
				}
				//----------------------------------------------------------------------
				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::MenuItem("Create Entity", "CTRL+e"))
					{
						openCreateEntityPopup = true;
					}
					if (ImGui::MenuItem("Delete Entity", "del"))
					{
						scene->RemoveSceneObject(selectedSceneObject);
					}
					ImGui::Separator();
					if (ImGui::BeginMenu("Create Light", ""))
					{
						if (ImGui::MenuItem("Point"))
						{
							auto light = scene->CreateSceneObject("Point Light");
							scene->registry.emplace<CLight>(light, LightType::POINT, 
								glm::vec3( 1,1,1 ), 1.0f, glm::vec3( 0,30,0), glm::vec3( 0,0,0 ), 0);
						}
						if (ImGui::MenuItem("Diretional"))
						{
							auto light = scene->CreateSceneObject("Directional Light");
							scene->registry.emplace<CLight>(light, LightType::DIRECTIONAL,
								glm::vec3(1, 1, 1), 1.0f, glm::vec3(0, 0, 0), glm::vec3(0, -10, 0), 0);
						}
						if (ImGui::MenuItem("Spot"))
						{
							auto light = scene->CreateSceneObject("Spot Light");
							scene->registry.emplace<CLight>(light, LightType::SPOT,
								glm::vec3(1, 1, 1), 1.0f, glm::vec3(0, 20, 0), glm::vec3(0, -10, 0), glm::radians(50.f));
						}
						ImGui::EndMenu();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Attach/Detach Component", "CTRL+Space")) 
					{
						openComponentsPopup = true;
					}
					ImGui::Separator();
					if (ImGui::BeginMenu("Tangencial ForceField2D", ""))
					{
						if(ImGui::MenuItem("XY"))
						{
							auto forceF = scene->CreateSceneObject("forceField");
							scene->registry.emplace<CForceField2D>(forceF, [](glm::vec2 pos)
								{
									float x = pos.x;
									float y = pos.y;
									float radius = sqrt(x * x + y * y);
									float angle = atan2(y, x) + 0.5f * glm::pi<float>();
									return glm::vec2(-radius * cos(angle), - radius * sin(angle));
								}, FieldPlane::XY).scaling = 1.5f;
						}
						if(ImGui::MenuItem("YZ"))
						{
							auto forceF = scene->CreateSceneObject("forceField");
							scene->registry.emplace<CForceField2D>(forceF, [](glm::vec2 pos)
								{
									float x = pos.x;
									float y = pos.y;
									float radius = sqrt(x * x + y * y);
									float angle = atan2(y, x) + 0.5f * glm::pi<float>();
									return glm::vec2(-radius * cos(angle), - radius * sin(angle));
								}, FieldPlane::YZ).scaling = 1.5f;
						}
						if (ImGui::MenuItem("XZ"))
						{
							auto forceF = scene->CreateSceneObject("forceField");
							scene->registry.emplace<CForceField2D>(forceF, [](glm::vec2 pos)
								{
									float x = pos.x;
									float y = pos.y;
									float radius = sqrt(x * x + y * y);
									float angle = atan2(y, x) + 0.5f * glm::pi<float>();
									return glm::vec2(-radius * cos(angle), -radius * sin(angle));
								}, FieldPlane::XZ).scaling = 1.5f;
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("To Right VelocityField2D", ""))
					{
						if(ImGui::MenuItem("XY"))
						{
							auto velocityF = scene->CreateSceneObject("velocityField");
							scene->registry.emplace<CVelocityField2D>(velocityF, [](glm::vec2 pos)
								{
									float x = pos.x;
									float y = pos.y;
									return glm::vec2(
										1.0f, -y*0.3f
									);
								}, FieldPlane::XY).scaling = 1.5f;
						}
						if (ImGui::MenuItem("YZ"))
						{
							auto velocityF = scene->CreateSceneObject("velocityField");
							scene->registry.emplace<CVelocityField2D>(velocityF, [](glm::vec2 pos)
								{
									float x = pos.x;
									float y = pos.y;
									return glm::vec2(
										1.0f, -y * 0.3f
									);
								}, FieldPlane::YZ).scaling = 1.5f;
						}
						if(ImGui::MenuItem("XZ"))
						{
							auto velocityF = scene->CreateSceneObject("velocityField");
							scene->registry.emplace<CVelocityField2D>(velocityF, [](glm::vec2 pos)
								{
									float x = pos.x;
									float y = pos.y;
									return glm::vec2(
										1.0f, -y*0.3f
									);
								}, FieldPlane::XZ).scaling = 1.5f;
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				//---------------------------------------------------------------------------------
				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Wireframes", "", &ApplicationState::GetInstance().renderingWireframe);
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			//===============================================================
			if (openComponentsPopup)
			{
				ImGui::OpenPopup("componentAdder");
				openComponentsPopup = false;
			}
			if (ImGui::BeginPopup("componentAdder"))
			{
				const char* components[] = {
						"Transform", "TriMesh",
						"PhongMaterial", "Image Map", "Light", 
						"Skybox", "Physics Bounds",
						"VelocityField2D", "ForceField2D",
						"RigidBody", "Box Collider"};
				if (ImGui::BeginListBox("Add Component", 
					ImVec2(0, 5 * ImGui::GetTextLineHeightWithSpacing())))
				{
					for (int n = 0; n < IM_ARRAYSIZE(components); n++)
					{
						if (!scene->EntityHas(selectedSceneObject, ((CType)n)) && ImGui::Selectable(components[n]))
						{
							switch (((CType) n))
							{
							case CType::Transform:
								scene->registry.emplace_or_replace<CTransform>(selectedSceneObject);
								break;
							case CType::TriMesh:
								scene->registry.emplace_or_replace<CTriMesh>(selectedSceneObject);
								break;
							case CType::PhongMaterial:
								scene->registry.emplace_or_replace<CPhongMaterial>(selectedSceneObject);
								break;
							case CType::ImageMaps:
								scene->registry.emplace_or_replace<CImageMaps>(selectedSceneObject);
								break;
							case CType::Light:
								scene->registry.emplace_or_replace<CLight>(selectedSceneObject, LightType::POINT, 
									glm::vec3(1.0f), 1.0f, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f);
								break;
							case CType::PhysicsBounds:
								scene->registry.emplace_or_replace<CPhysicsBounds>(selectedSceneObject, glm::vec3(-10.0f), glm::vec3(10.0f));
								break;
							case CType::VelocityField2D:
								scene->registry.emplace_or_replace<CVelocityField2D>(selectedSceneObject, [](glm::vec2 pos)
									{
										float x = pos.x;
										float y = pos.y;
										return glm::vec2(
											1.0f, -y * 0.3f
										);
									},FieldPlane::XZ).scaling = 0.5f;
								break;
							case CType::ForceField2D:
								scene->registry.emplace_or_replace<CForceField2D>(selectedSceneObject, [](glm::vec2 pos)
									{
										float x = pos.x;
										float y = pos.y;
										float radius = sqrt(x * x + y * y);
										float angle = atan2(y, x) + 0.5f * glm::pi<float>();
										return glm::vec2(-radius * cos(angle), -radius * sin(angle));
									}, FieldPlane::XZ).scaling = 0.5f;
								break;
							case CType::RigidBody:
								scene->registry.emplace_or_replace<CRigidBody>(selectedSceneObject);
								break;
							case CType::BoxCollider:
							{
								auto* mesh = scene->registry.try_get<CTriMesh>(selectedSceneObject);
								if (mesh)
									scene->registry.emplace_or_replace<CBoxCollider>(selectedSceneObject, 
															mesh->GetBoundingBoxMin(), mesh->GetBoundingBoxMax());
								else
									scene->registry.emplace_or_replace<CBoxCollider>(selectedSceneObject, glm::vec3(0.0f), glm::vec3(0.0f));
							}
								break;
							case CType::Count:
								break;
							default:
								break;
							}
						}
					}
				ImGui::EndListBox();
				}
				if (ImGui::BeginListBox("Remove Component",
					ImVec2(0, 5 * ImGui::GetTextLineHeightWithSpacing())))
				{
					for (int n = 0; n < IM_ARRAYSIZE(components); n++)
					{
						if (scene->EntityHas(selectedSceneObject, ((CType)n)) && ImGui::Selectable(components[n]))
						{
							switch (((CType)n))
							{
							case CType::Transform:
								scene->registry.erase<CTransform>(selectedSceneObject);
								break;
							case CType::TriMesh:
								scene->registry.erase<CTriMesh>(selectedSceneObject);
								break;
							case CType::PhongMaterial:
								scene->registry.erase<CPhongMaterial>(selectedSceneObject);
								break;
							case CType::ImageMaps:
								scene->registry.erase<CImageMaps>(selectedSceneObject);
								break;
							case CType::Light:
								scene->registry.erase<CLight>(selectedSceneObject);
								break;
							case CType::PhysicsBounds:
								scene->registry.erase<CPhysicsBounds>(selectedSceneObject);
								break;
							case CType::VelocityField2D:
								scene->registry.erase<CVelocityField2D>(selectedSceneObject);
								break;
							case CType::ForceField2D:
								scene->registry.erase<CForceField2D>(selectedSceneObject);
								break;
							case CType::RigidBody:
								scene->registry.erase<CRigidBody>(selectedSceneObject);
								break;
							case CType::BoxCollider:
								scene->registry.erase<CBoxCollider>(selectedSceneObject);
								break;
							case CType::Count:
								break;
							default:
								break;
							}
						}
					}
					ImGui::EndListBox();
				}
				ImGui::EndPopup();
			}
			//---------------------------------------------------------------
			if (openCreateEntityPopup)
			{
				ImGui::OpenPopup("entitycreator");
				openCreateEntityPopup = false;
			}
			if (ImGui::BeginPopup("entitycreator"))
			{
				ImGui::Text("Create Entity");
				ImGui::Separator();
				static char buf[128] = "";
				ImGui::InputText("Name", buf, IM_ARRAYSIZE(buf));
				if (ImGui::Button("Create"))
				{
					scene->CreateSceneObject(buf);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		void DrawSceneObjectsList()
		{
			if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::BeginListBox("##1", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
				{
					int n = 0;
					for (std::unordered_map<std::string, entt::entity>::iterator
						it = scene->sceneObjectsBegin(); it != scene->sceneObjectsEnd(); ++it)
					{
						const bool is_selected = (selectedSceneObject == it->second);
						if(is_selected)
							ImGui::PushStyleColor(ImGuiCol_Header,
								ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
						if (ImGui::Selectable(it->first.c_str(), is_selected))
						{
							selectedSceneObject = it->second;
							ApplicationState::GetInstance().selectedObject = selectedSceneObject;
						}

						if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
							ImGui::PopStyleColor();
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
				static bool openParentSelector = false;
				//Draw transform tab
				scene->registry.view<CTransform>()
					.each([&](auto e,  auto& t)
				{
							if (e== selectedSceneObject && ImGui::BeginTabItem("Transform"))
							{
								glm::vec3 p = t.GetPosition();
								if (ImGui::DragFloat3("Position", &p[0]))
									t.SetPosition(p);
								glm::vec3 r = glm::degrees(t.GetRotation());
								if (ImGui::DragFloat3("Rotation", &r[0]))
									t.SetEulerRotation(glm::radians(r));
								glm::vec3 s = t.GetScale();
								if (ImGui::DragFloat3("Scale", &s[0]))
									t.SetScale(s);
								glm::vec3 l = t.GetPivot();
								if (ImGui::DragFloat3("Pivot", &l[0]))
									t.SetPivot(l);
								
								if (t.GetParent())
									ImGui::Text("Parent: %s", t.GetParent()->entityName.c_str());
								else 
									ImGui::Text("Parent: None");
								ImGui::SameLine();
								
								if (ImGui::Button("Set Parent"))
								{
									//create a popup to select the parent
									openParentSelector = true;
								}
								ImGui::SameLine();
								if (ImGui::Button("Unset Parent"))
									t.SetParent(nullptr);
								if (ImGui::Button("Reset"))
								{
									auto* rb = scene->registry.try_get<CRigidBody>(e);
									if (rb)
										rb->ResetToRest();
									t.Reset();
								}
								ImGui::EndTabItem();
							}
				});
				if (openParentSelector)
				{
					ImGui::OpenPopup("parentselector");
					openParentSelector = false;
				}
				if (ImGui::BeginPopup("parentselector"))
				{
					static bool close = false;
					for (std::unordered_map<std::string, entt::entity>::iterator
						it = scene->sceneObjectsBegin(); it != scene->sceneObjectsEnd(); ++it)
					{
						auto* parentTrPtr = scene->registry.try_get<CTransform>(it->second);
						if (parentTrPtr && it->second != selectedSceneObject && 
							ImGui::Selectable(it->first.c_str(), false))
						{
							scene->registry.try_get<CTransform>(selectedSceneObject)->
								SetParent(parentTrPtr);
							parentTrPtr->entityName = it->first;
							break;
							close = true;
						}
					}
					if (close)
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}

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
								ImGui::Text("Type: "); ImGui::SameLine();
								ImGui::TextColored(ImVec4(0.84f, 0.58f, 1.0f, 1.0f),
									l.GetLightType() == LightType::POINT ? "Point" :
									(l.GetLightType() == LightType::DIRECTIONAL ? "Directional" : "Spot"));
								
								if ((l.GetLightType() == LightType::POINT ||
									l.GetLightType() == LightType::SPOT) && ImGui::Checkbox("Show", &l.show))
								{
									Event event;
									event.type = Event::Type::GeometryChange;
									event.geometryChange.e = e;
									event.geometryChange.toBeRemoved = !l.show;
									GLFWHandler::GetInstance().QueueEvent(event);
									ImGui::SameLine();
								}
								ImGui::Text("Casts Shadow"); ImGui::SameLine();
								bool shadow = l.IsCastingShadows();
								if(ImGui::ToggleButton("shadow", &shadow))
									l.SetCastingShadows(shadow);
								if (shadow && l.GetLightType() != LightType::POINT)
								{
									ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
									ImGui::Image((void*)(intptr_t) l.glID,
										ImVec2(viewportPanelSize.x / 2, viewportPanelSize.x / 2));
								}
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
									ImGui::DragFloat3("Direction", &(l.direction[0]));
								}
								if (l.GetLightType() == LightType::SPOT)
								{
									auto cutoff = glm::degrees(l.cutoff);
									if (ImGui::DragFloat("cutoff", &cutoff, 0.01f, 0.0f, 90.0f))
										l.cutoff = glm::radians(cutoff);
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
								if (ImGui::DragFloat("Mass", &r.mass, 0.01f, 0.0f))
								{
									r.mass = max(0.f, r.mass);
									auto* tr = scene->registry.try_get<CTransform>(e);
									const auto* mesh = scene->registry.try_get<CTriMesh>(e);
									if (tr && mesh)
									{
										r.SetMassMatrix();
										//r.SetInteriaMatrix(mesh, tr);
									}
								}
								ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
								ImGui::PushItemWidth(viewportPanelSize.x/4);
								ImGui::DragFloat("Drag", &r.drag, 0.01f, 0.0f);
								ImGui::SameLine();
								ImGui::DragFloat("Gravity", &r.gravity, 0.1f, 0.0f);
								ImGui::PopItemWidth();
								ImGui::BeginDisabled();
								ImGui::DragFloat3("Position", &r.position[0]);
								ImGui::DragFloat3("Linear Momentum", &r.linearMomentum[0]);
								ImGui::DragFloat3("Angular Momentum", &r.angularMomentum[0]);
								/*ImGui::DragFloat3("Velocity", &r.velocity[0]);
								ImGui::DragFloat3("Acceleration", &r.acceleration[0]);
								ImGui::DragFloat3("Rotation", &r.rotation[0]);*/
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
								if (ImGui::SmallButton("Load from file"))
								{
									std::string filename;
									if (openFilePicker(filename))
									{
										printf("Loading: %s\n", filename.c_str());
										//check if name ends with .obj
										std::string::size_type idx;
										idx = filename.rfind('.');

										if (idx != std::string::npos)
										{
											std::string extension = filename.substr(idx + 1);
											if (extension == "obj")
											{
												scene->registry.erase<CTriMesh>(e);
												scene->registry.emplace<CTriMesh>(e, filename);
											}
											else
												printf("File extension not supported\n");
										}
										else
											printf("File extension not supported\n");
									}
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("Calculate Normals"))
								{
									m.ComputeNormals();
									Event event;
									event.type = Event::Type::GeometryChange;
									event.geometryChange.e = e;
									GLFWHandler::GetInstance().QueueEvent(event);
								}
								
								ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
								ImGui::PushItemWidth(viewportPanelSize.x * 0.66f);
								if (ImGui::InputInt("Tesselation lvl", &m.tessellationLevel, 1, 5, ImGuiInputTextFlags_EnterReturnsTrue))
									m.tessellationLevel = glm::clamp(m.tessellationLevel, 1, 64);
								ImGui::PopItemWidth();
								
								ImGui::Checkbox("Visible", &(m.visible));
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
				//Draw CBoundingBox tab
				scene->registry.view<CPhysicsBounds>()
					.each([&](auto e, auto& b)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("Physics Bounds"))
							{
								glm::vec3 min = b.GetMin();
								glm::vec3 max = b.GetMax();
								if (ImGui::DragFloat3("Min", &min[0], 0.01f)) b.SetMin(min);
								if (ImGui::DragFloat3("Max", &max[0], 0.01f)) b.SetMax(max);
								ImGui::EndTabItem();
							}
						});
				
				//Draw CTextures tab
				scene->registry.view<CImageMaps>()
					.each([&](auto e, auto& t)
						{	
							if (e == selectedSceneObject && ImGui::BeginTabItem("Image Maps"))
							{
								static ImageMap::BindingSlot textureBinding;
								static const char* current_item = NULL;
								//Texturebinding picker combo box
								const char* items[] = {
									"Ambient", "Diffuse", "Specular", "Normal Map", "Displacement Map", "Environment Map"};
								if (ImGui::BeginCombo("##combo", current_item))
								{
									for (int n = 0; n < IM_ARRAYSIZE(items); n++)
									{
										bool is_selected = (current_item == items[n]);
										if (ImGui::Selectable(items[n], is_selected))
										{
											current_item = items[n];
											textureBinding = (ImageMap::BindingSlot)n;
										}
										if (is_selected)
											ImGui::SetItemDefaultFocus();
									}
									ImGui::EndCombo();
								}
								//ImGui::SameLine();
								if (ImGui::Button(textureBinding == ImageMap::BindingSlot::ENV_MAP ?
									"from Img. 6 files" : "from img") && current_item != NULL)
								{
									if (textureBinding == ImageMap::BindingSlot::ENV_MAP)
									{
										std::string paths[6];
										if (openFilePicker(paths[0]))
										{
											if (openFilePicker(paths[1]))
											{
												if (openFilePicker(paths[2]))
												{
													if (openFilePicker(paths[3]))
													{
														if (openFilePicker(paths[4]))
														{
															if (openFilePicker(paths[5]))
															{
																t.AddImageMap(textureBinding, paths);
															}
														}
													}
												}
											}
										}
										
									}
									else
									{
										std::string path;
										if (openFilePicker(path))
										{
											//Get extension
											std::string extension = path.substr(path.find_last_of(".") + 1);
											//Check if extension is supported
											if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "bmp")
											{
												t.AddImageMap(textureBinding, path); 
											}
											else
											{
												std::cout << "extension not supported:" << extension << std::endl;
											}
										}
									}
								}
								
								ImGui::SameLine();
								if (ImGui::Button("from reflections") && current_item != NULL)
								{
									auto* mesh = scene->registry.try_get<CTriMesh>(e);
									if (mesh);
									//TODO: find a way to extract dims from mesh
									if (textureBinding == ImageMap::BindingSlot::ENV_MAP)
										t.AddImageMap(textureBinding, glm::uvec2(1000, 1000), ImageMap::RenderImageMode::CUSTOM);
									else
										t.AddImageMap(textureBinding, glm::uvec2(1000,1000), ImageMap::RenderImageMode::REFLECTION );
								}
								
								ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
								int j = 0;
								for (auto it = t.mapsBegin(); it != t.mapsEnd(); it++)
								{
									//get viewport size
									ImGui::PushID(j);
									ImGui::BeginGroup();
									ImGui::Text("As %s", it->second.GetSlotName().c_str());
									if (it->second.GetBindingSlot() != ImageMap::BindingSlot::ENV_MAP)
										ImGui::Image((void*)(intptr_t)it->second.glID,
											ImVec2(viewportPanelSize.x / 2, viewportPanelSize.x / 2));
									if (ImGui::Button("Remove"))
									{
										t.RemoveImageMap(it->second.GetBindingSlot());
										ImGui::EndGroup();
										ImGui::PopID();
										break;
									}
									if (it->first == ImageMap::BindingSlot::DISPLACEMENT)
									{
										ImGui::PushItemWidth(viewportPanelSize.x / 4);
										ImGui::DragFloat("multiplier", &it->second.dispMultiplier);
										ImGui::PopItemWidth();
									}
									if (it->second.IsRenderedImage())
									{
										ImGui::Spacing();
										ImGui::Text("Rendered from camera");
										ImGui::PushID("renderedCamera");
										DrawCameraController(it->second.GetRenderedImageCamera());
										ImGui::PopID();
									}
									
									ImGui::Separator();
									ImGui::EndGroup();
									ImGui::PopID();
									if (j % 2 == 0)
										ImGui::SameLine();
									j++;
								}
								ImGui::EndTabItem();
							}
						});

				//Draw CSkybox tab
				scene->registry.view<CSkyBox>()
					.each([&](auto e, auto& b)
						{
							ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
							if (e == selectedSceneObject && ImGui::BeginTabItem("Skybox"))
							{
								ImGui::Text("Yeah I exist");
								
								ImGui::EndTabItem();
							}
						});

				//Draw CBoxCollider tab
				scene->registry.view<CBoxCollider>()
					.each([&](auto e, auto& c)
						{
							if (e == selectedSceneObject && ImGui::BeginTabItem("Box Collider"))
							{
								glm::vec3 min = c.GetMin();
								glm::vec3 max = c.GetMax();
								if (ImGui::DragFloat3("Min", &min[0], 0.01f)) c.SetMin(min);
								if (ImGui::DragFloat3("Max", &max[0], 0.01f)) c.SetMax(max);
								ImGui::DragFloat("Elasticity", &c.elasticity, 0.0001f, 0.0f);
								ImGui::EndTabItem();
							}
						});

				ImGui::EndTabBar();
			}
		}
		void DrawCameraController(Camera& camera)
		{
			if (ImGui::CollapsingHeader("Camera"))
			{
				//auto& camera = scene->camera;
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