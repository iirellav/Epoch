#include "StatisticsPanel.h"
#include <CommonUtilities/Timer.h>
#include <CommonUtilities/StringUtils.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/Core/GraphicsEngine.h> //TODO: Remove
#include <Epoch/Debug/Timer.h>
#include <Epoch/Scene/SceneRenderer.h>
#include <Epoch/Rendering/Renderer.h>
#include <Epoch/Rendering/DebugRenderer.h>
#include <Epoch/Rendering/Font.h>
#include "../EditorResources.h"

namespace Epoch
{
	StatisticsPanel::StatisticsPanel(const std::string& aName) : EditorPanel(aName)
	{
	}

	void StatisticsPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		bool open = ImGui::Begin(myName.c_str(), &aIsOpen);

		if (!open)
		{
			ImGui::End();
			return;
		}

		//Viewport
		{
			const auto [width, height] = mySceneRendererReference.lock()->GetViewportSize();
			ImGui::Text("Viewport Size: %i x %i", width, height);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//FPS
		{
			static double time = 0.0;
			static unsigned frames = 0;

			time += CU::Timer::GetDeltaTime();
			frames++;

			static float frameTimeMs = 0.0f;
			static uint32_t frameRate = 0;

			if (time > 0.2)
			{
				const double avgFrameTime = time / frames;
				frameTimeMs = (float)((avgFrameTime * 1000.0) * 0.5 * 2);
				frameRate = (uint32_t)(1.0 / avgFrameTime);

				time = 0.0;
				frames = 0;
			}

			ImGui::Text("%i FPS (%.3fms)", frameRate, frameTimeMs);
			ImGui::Text("VSync Enabled:");
			ImGui::SameLine();
			ImGui::Checkbox("##VSync Enabled", &GraphicsEngine::Get().GetVSyncBool());
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//Scripts
		{
			static double time = 0.0;
			static double scriptUpdateTime = 0.0;
			static double scriptLateUpdateTime = 0.0;
			static double scriptFixedUpdateTime = 0.0;
			static unsigned frames = 0;

			time += CU::Timer::GetDeltaTime();
			frames++;

			scriptUpdateTime += mySceneContext->GetPerformanceTimers().scriptUpdate;
			scriptLateUpdateTime += mySceneContext->GetPerformanceTimers().scriptLateUpdate;
			scriptFixedUpdateTime += mySceneContext->GetPerformanceTimers().scriptFixedUpdate;

			static float scriptUpdateTimeMs = 0.0f;
			static float scriptLateUpdateTimeMs = 0.0f;
			static float scriptFixedUpdateTimeMs = 0.0f;

			if (time > 0.2)
			{
				scriptUpdateTimeMs = (float)(scriptUpdateTime / frames);
				scriptLateUpdateTimeMs = (float)(scriptLateUpdateTime / frames);
				scriptFixedUpdateTimeMs = (float)(scriptFixedUpdateTime / frames);

				time = 0.0;
				frames = 0;

				scriptUpdateTime = 0.0f;
				scriptLateUpdateTime = 0.0f;
				scriptFixedUpdateTime = 0.0f;
			}

			ImGui::Text(("Script Entities: " + CU::NumberFormat(mySceneContext->GetAllEntitiesWith<ScriptComponent>().size())).c_str());
			ImGui::Spacing();
			ImGui::Text("C# Update: %.3fms", scriptUpdateTimeMs);
			ImGui::Text("C# Late Update: %.3fms", scriptLateUpdateTimeMs);
			ImGui::Text("C# Fixed Update: %.3fms", scriptFixedUpdateTimeMs);
		}
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//Physics
		{
			static double time = 0.0;
			static std::vector<float> simTimes;

			time += CU::Timer::GetDeltaTime();

			if (mySceneContext->GetPerformanceTimers().physicsSimulation > 0.0f)
			{
				simTimes.emplace_back(mySceneContext->GetPerformanceTimers().physicsSimulation);
			}

			static float frameTimeMs = 0.0f;
			if (time > 0.2)
			{
				frameTimeMs = 0.0f;
				for (float simTime : simTimes)
				{
					frameTimeMs += simTime;
				}
				if (frameTimeMs > 0.0f)
				{
					frameTimeMs /= simTimes.size();
				}

				simTimes.clear();
				time = 0.0;
				//simTime = 0.0;
				//frames = 0;
			}

			auto [staticCount, dynamicCount] = mySceneContext->GetPhysicsBodyCount();
			ImGui::Text(("Static Physics Bodies: " + CU::NumberFormat(staticCount)).c_str());
			ImGui::Text(("Dynamic Physics Bodies: " + CU::NumberFormat(dynamicCount)).c_str());
			ImGui::Spacing();
			//ImGui::Text("Physics Simulation: %.3fms", mySceneContext->GetPerformanceTimers().physicsSimulation);
			ImGui::Text("Physics Simulation: %.3fms", frameTimeMs);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//Scene
		{
			//Mesh
			{
				uint32_t counter = 0;
				uint32_t casterCounter = 0;
				auto entities = mySceneContext->GetAllEntitiesWith<MeshRendererComponent>();
				for (auto id : entities)
				{
					Entity entity = Entity(id, mySceneContext.get());
					if (!entity.IsActive()) continue;

					const auto& component = entities.get<MeshRendererComponent>(id);
					if (!component.isActive) continue;

					counter++;

					if (!component.castsShadows) continue;
					casterCounter++;
				}
				ImGui::Text(("Active Mesh Entities: " + CU::NumberFormat(counter)).c_str());
				ImGui::Text(("Active Shadow Casting Mesh Entities: " + CU::NumberFormat(casterCounter)).c_str());
			}

			ImGui::Spacing();

			//Point Light
			{
				uint32_t counter = 0;
				auto entities = mySceneContext->GetAllEntitiesWith<PointLightComponent>();
				for (auto id : entities)
				{
					Entity entity = Entity(id, mySceneContext.get());
					if (!entity.IsActive()) continue;

					const auto& component = entities.get<PointLightComponent>(id);
					if (!component.isActive || !component.castsShadows) continue;

					counter++;
				}
				ImGui::Text(("Active Shadow Casters (Point Light): " + CU::NumberFormat(counter)).c_str());
			}

			//Spotlight
			{
				uint32_t counter = 0;
				auto entities = mySceneContext->GetAllEntitiesWith<SpotlightComponent>();
				for (auto id : entities)
				{
					Entity entity = Entity(id, mySceneContext.get());
					if (!entity.IsActive()) continue;

					const auto& component = entities.get<SpotlightComponent>(id);
					if (!component.isActive || !component.castsShadows) continue;

					counter++;
				}
				ImGui::Text(("Active Shadow Casters (Spotlight): " + CU::NumberFormat(counter)).c_str());
			}

			ImGui::Spacing();

			//Text
			{
				uint32_t counter = 0;
				auto entities = mySceneContext->GetAllEntitiesWith<TextRendererComponent>();
				for (auto id : entities)
				{
					Entity entity = Entity(id, mySceneContext.get());
					if (!entity.IsActive()) continue;

					const auto& component = entities.get<TextRendererComponent>(id);
					if (!component.isActive) continue;

					counter++;
				}
				ImGui::Text(("Active Text Entities: " + CU::NumberFormat(counter)).c_str());
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//Renderer Stats
		{
			const auto& stats = mySceneRendererReference.lock()->GetStats();

			ImGui::Text(("Render Commands: " + CU::NumberFormat(Renderer::GetRenderCommandCount())).c_str());

			UI::Spacing();
			
			ImGui::Text(("Draw Calls: " + CU::NumberFormat(stats.drawCalls)).c_str());
			ImGui::Text(("Saved Draws: " + CU::NumberFormat(stats.savedDraws)).c_str());

			UI::Spacing();

			if (UI::PropertyGridHeader("Detailed", true))
			{
				ImGui::Text(("Instances: " + CU::NumberFormat(stats.instances)).c_str());
				ImGui::Text(("Batched: " + CU::NumberFormat(stats.batched)).c_str());

				UI::Spacing();

				ImGui::Text(("Vertices: " + CU::NumberFormat(stats.vertices)).c_str());
				ImGui::Text(("Indices: " + CU::NumberFormat(stats.indices)).c_str());
				ImGui::Text(("Triangles: " + CU::NumberFormat(stats.triangles)).c_str());

				UI::Spacing();
				
				ImGui::Text(("Meshes: " + CU::NumberFormat(stats.meshes)).c_str());
				ImGui::Text(("Sub meshes: " + CU::NumberFormat(stats.submeshes)).c_str());

				ImGui::TreePop();
			}

			//UI::Spacing(2);

			//if (UI::PropertyGridHeader("Render Statistics", true))
			//{
			//	//TODO: Get GPU times
			//
			//	ImGui::TreePop();
			//}

			//if (UI::PropertyGridHeader("Debug Renderer", false))
			//{
			//	const auto& debugStats = myDebugRendererReference.lock()->GetStats();
			//
			//	ImGui::Text("Draw Calls: %u", debugStats.drawCalls);
			//	ImGui::Text("Vertices: %u", debugStats.vertices);
			//	ImGui::Text("Indices: %u", debugStats.indices);
			//
			//	ImGui::TreePop();
			//}
		}

#if 0

		UI::Spacing();
		ImGui::Separator();
		UI::Spacing();

		if (UI::PropertyGridHeader("Font Awesome", false))
		{
			ImGui::TextUnformatted(EP_ICON_GLASS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MUSIC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SEARCH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ENVELOPE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HEART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STAR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STAR_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILM);

			ImGui::TextUnformatted(EP_ICON_TH_LARGE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TH_LIST);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHECK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TIMES);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SEARCH_PLUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SEARCH_MINUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_POWER_OFF);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SIGNAL);

			ImGui::TextUnformatted(EP_ICON_COG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRASH_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOME);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CLOCK_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ROAD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DOWNLOAD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_O_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_O_UP);

			ImGui::TextUnformatted(EP_ICON_INBOX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLAY_CIRCLE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REPEAT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REFRESH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LIST_ALT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LOCK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FLAG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HEADPHONES);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VOLUME_OFF);

			ImGui::TextUnformatted(EP_ICON_VOLUME_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VOLUME_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_QRCODE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BARCODE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TAG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TAGS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BOOK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BOOKMARK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PRINT);

			ImGui::TextUnformatted(EP_ICON_CAMERA);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FONT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BOLD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ITALIC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TEXT_HEIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TEXT_WIDTH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ALIGN_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ALIGN_CENTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ALIGN_RIGHT);

			ImGui::TextUnformatted(EP_ICON_ALIGN_JUSTIFY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LIST);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OUTDENT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_INDENT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VIDEO_CAMERA);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PICTURE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PENCIL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAP_MARKER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ADJUST);

			ImGui::TextUnformatted(EP_ICON_TINT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PENCIL_SQUARE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHARE_SQUARE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHECK_SQUARE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROWS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STEP_BACKWARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FAST_BACKWARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BACKWARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLAY);

			ImGui::TextUnformatted(EP_ICON_PAUSE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STOP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FORWARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FAST_FORWARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STEP_FORWARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EJECT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLUS_CIRCLE);

			ImGui::TextUnformatted(EP_ICON_MINUS_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TIMES_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHECK_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_QUESTION_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_INFO_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CROSSHAIRS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TIMES_CIRCLE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHECK_CIRCLE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BAN);

			ImGui::TextUnformatted(EP_ICON_ARROW_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXPAND);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMPRESS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MINUS);

			ImGui::TextUnformatted(EP_ICON_ASTERISK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXCLAMATION_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GIFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LEAF);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FIRE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EYE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EYE_SLASH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXCLAMATION_TRIANGLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLANE);

			ImGui::TextUnformatted(EP_ICON_CALENDAR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RANDOM);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMMENT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAGNET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RETWEET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHOPPING_CART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FOLDER);

			ImGui::TextUnformatted(EP_ICON_FOLDER_OPEN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROWS_V);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROWS_H);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BAR_CHART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TWITTER_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FACEBOOK_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CAMERA_RETRO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_KEY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COGS);

			ImGui::TextUnformatted(EP_ICON_COMMENTS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_THUMBS_O_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_THUMBS_O_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STAR_HALF);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HEART_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SIGN_OUT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LINKEDIN_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_THUMB_TACK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXTERNAL_LINK);

			ImGui::TextUnformatted(EP_ICON_SIGN_IN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TROPHY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GITHUB_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UPLOAD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LEMON_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PHONE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SQUARE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BOOKMARK_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PHONE_SQUARE);

			ImGui::TextUnformatted(EP_ICON_TWITTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FACEBOOK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GITHUB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UNLOCK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CREDIT_CARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RSS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HDD_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BULLHORN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BELL);

			ImGui::TextUnformatted(EP_ICON_CERTIFICATE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_O_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_O_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_O_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_O_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_DOWN);

			ImGui::TextUnformatted(EP_ICON_GLOBE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GLOBE_E);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GLOBE_W);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WRENCH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TASKS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BRIEFCASE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROWS_ALT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USERS);

			ImGui::TextUnformatted(EP_ICON_LINK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CLOUD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FLASK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SCISSORS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILES_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAPERCLIP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FLOPPY_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BARS);

			ImGui::TextUnformatted(EP_ICON_LIST_UL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LIST_OL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STRIKETHROUGH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UNDERLINE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TABLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAGIC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRUCK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PINTEREST);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PINTEREST_SQUARE);

			ImGui::TextUnformatted(EP_ICON_GOOGLE_PLUS_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GOOGLE_PLUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MONEY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COLUMNS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT);

			ImGui::TextUnformatted(EP_ICON_SORT_DESC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_ASC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ENVELOPE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LINKEDIN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UNDO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GAVEL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TACHOMETER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMMENT_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMMENTS_O);

			ImGui::TextUnformatted(EP_ICON_BOLT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SITEMAP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UMBRELLA);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CLIPBOARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LIGHTBULB_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXCHANGE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CLOUD_DOWNLOAD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CLOUD_UPLOAD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USER_MD);

			ImGui::TextUnformatted(EP_ICON_STETHOSCOPE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SUITCASE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BELL_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COFFEE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CUTLERY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_TEXT_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BUILDING_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOSPITAL_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_AMBULANCE);

			ImGui::TextUnformatted(EP_ICON_MEDKIT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FIGHTER_JET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BEER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_H_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLUS_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_DOUBLE_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_DOUBLE_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_DOUBLE_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_DOUBLE_DOWN);

			ImGui::TextUnformatted(EP_ICON_ANGLE_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGLE_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DESKTOP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LAPTOP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TABLET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MOBILE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CIRCLE_O);

			ImGui::TextUnformatted(EP_ICON_QUOTE_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_QUOTE_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SPINNER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REPLY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GITHUB_ALT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FOLDER_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FOLDER_OPEN_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SMILE_O);

			ImGui::TextUnformatted(EP_ICON_FROWN_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MEH_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GAMEPAD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_KEYBOARD_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FLAG_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FLAG_CHECKERED);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TERMINAL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CODE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REPLY_ALL);

			ImGui::TextUnformatted(EP_ICON_STAR_HALF_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LOCATION_ARROW);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CROP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CODE_FORK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHAIN_BROKEN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_QUESTION);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_INFO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXCLAMATION);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SUPERSCRIPT);

			ImGui::TextUnformatted(EP_ICON_SUBSCRIPT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ERASER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PUZZLE_PIECE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MICROPHONE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MICROPHONE_SLASH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHIELD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CALENDAR_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FIRE_EXTINGUISHER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ROCKET);

			ImGui::TextUnformatted(EP_ICON_MAXCDN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_CIRCLE_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_CIRCLE_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_CIRCLE_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHEVRON_CIRCLE_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HTML5);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CSS3);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANCHOR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UNLOCK_ALT);

			ImGui::TextUnformatted(EP_ICON_BULLSEYE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ELLIPSIS_H);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ELLIPSIS_V);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RSS_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLAY_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TICKET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MINUS_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MINUS_SQUARE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LEVEL_UP);

			ImGui::TextUnformatted(EP_ICON_LEVEL_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHECK_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PENCIL_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXTERNAL_LINK_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHARE_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMPASS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_SQUARE_O_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_SQUARE_O_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_SQUARE_O_RIGHT);

			ImGui::TextUnformatted(EP_ICON_EUR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GBP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_INR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_JPY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RUB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_KRW);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BTC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE);

			ImGui::TextUnformatted(EP_ICON_FILE_TEXT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_ALPHA_ASC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_ALPHA_DESC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_AMOUNT_ASC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_AMOUNT_DESC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_NUMERIC_ASC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SORT_NUMERIC_DESC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_THUMBS_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_THUMBS_DOWN);

			ImGui::TextUnformatted(EP_ICON_YOUTUBE_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_YOUTUBE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_XING);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_XING_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_YOUTUBE_PLAY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DROPBOX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STACK_OVERFLOW);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_INSTAGRAM);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FLICKR);

			ImGui::TextUnformatted(EP_ICON_ADN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BITBUCKET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BITBUCKET_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TUMBLR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TUMBLR_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LONG_ARROW_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LONG_ARROW_UP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LONG_ARROW_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LONG_ARROW_RIGHT);
			
			ImGui::TextUnformatted(EP_ICON_APPLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WINDOWS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANDROID);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LINUX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DRIBBBLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SKYPE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FOURSQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRELLO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FEMALE);

			ImGui::TextUnformatted(EP_ICON_MALE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GRATIPAY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SUN_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MOON_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARCHIVE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BUG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WEIBO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RENREN);

			ImGui::TextUnformatted(EP_ICON_PAGELINES);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STACK_EXCHANGE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_O_RIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ARROW_CIRCLE_O_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CARET_SQUARE_O_LEFT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DOT_CIRCLE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WHEELCHAIR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VIMEO_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRY);

			ImGui::TextUnformatted(EP_ICON_PLUS_SQUARE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SPACE_SHUTTLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SLACK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ENVELOPE_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WORDPRESS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OPENID);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_UNIVERSITY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GRADUATION_CAP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_YAHOO);

			ImGui::TextUnformatted(EP_ICON_GOOGLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REDDIT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REDDIT_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STUMBLEUPON_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STUMBLEUPON);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DELICIOUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DIGG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DRUPAL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_JOOMLA);

			ImGui::TextUnformatted(EP_ICON_LANGUAGE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FAX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BUILDING);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHILD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAW);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SPOON);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CUBE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CUBES);

			ImGui::TextUnformatted(EP_ICON_BEHANCE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BEHANCE_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STEAM);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STEAM_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_RECYCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CAR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TAXI);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TREE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SPOTIFY);

			ImGui::TextUnformatted(EP_ICON_DEVIANTART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SOUNDCLOUD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DATABASE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_PDF_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_WORD_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_EXCEL_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_POWERPOINT_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_IMAGE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_ARCHIVE_O);

			ImGui::TextUnformatted(EP_ICON_FILE_AUDIO_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_VIDEO_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FILE_CODE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VINE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CODEPEN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_JSFIDDLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LIFE_RING);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CIRCLE_O_NOTCH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REBEL);

			ImGui::TextUnformatted(EP_ICON_EMPIRE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GIT_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GIT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HACKER_NEWS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TENCENT_WEIBO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_QQ);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WEIXIN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAPER_PLANE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAPER_PLANE_O);

			ImGui::TextUnformatted(EP_ICON_HISTORY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CIRCLE_THIN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HEADER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PARAGRAPH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SLIDERS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHARE_ALT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHARE_ALT_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BOMB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FUTBOL_O);

			ImGui::TextUnformatted(EP_ICON_TTY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BINOCULARS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PLUG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SLIDESHARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TWITCH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_YELP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_NEWSPAPER_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WIFI);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CALCULATOR);

			ImGui::TextUnformatted(EP_ICON_PAYPAL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GOOGLE_WALLET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_VISA);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_MASTERCARD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_DISCOVER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_AMEX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_PAYPAL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_STRIPE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BELL_SLASH);

			ImGui::TextUnformatted(EP_ICON_BELL_SLASH_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRASH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COPYRIGHT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_AT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EYEDROPPER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAINT_BRUSH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BIRTHDAY_CAKE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_AREA_CHART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PIE_CHART);

			ImGui::TextUnformatted(EP_ICON_LINE_CHART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LASTFM);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LASTFM_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TOGGLE_OFF);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TOGGLE_ON);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BICYCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_IOXHOST);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ANGELLIST);

			ImGui::TextUnformatted(EP_ICON_CC);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ILS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MEANPATH);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BUYSELLADS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CONNECTDEVELOP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DASHCUBE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FORUMBEE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_LEANPUB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SELLSY);

			ImGui::TextUnformatted(EP_ICON_SHIRTSINBULK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SIMPLYBUILT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SKYATLAS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CART_PLUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CART_ARROW_DOWN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_DIAMOND);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHIP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USER_SECRET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MOTORCYCLE);

			ImGui::TextUnformatted(EP_ICON_STREET_VIEW);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HEARTBEAT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VENUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MARS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MERCURY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRANSGENDER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRANSGENDER_ALT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VENUS_DOUBLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MARS_DOUBLE);

			ImGui::TextUnformatted(EP_ICON_VENUS_MARS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MARS_STROKE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MARS_STROKE_V);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MARS_STROKE_H);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_NEUTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GENDERLESS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FACEBOOK_OFFICIAL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PINTEREST_P);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WHATSAPP);

			ImGui::TextUnformatted(EP_ICON_SERVER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USER_PLUS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_USER_TIMES);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BED);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_VIACOIN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRAIN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SUBWAY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MEDIUM);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MEDIUM_SQUARE);

			ImGui::TextUnformatted(EP_ICON_Y_COMBINATOR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OPTIN_MONSTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OPENCART);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EXPEDITEDSSL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BATTERY_FULL);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BATTERY_THREE_QUARTERS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BATTERY_HALF);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BATTERY_QUARTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BATTERY_EMPTY);

			ImGui::TextUnformatted(EP_ICON_MOUSE_POINTER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_I_CURSOR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OBJECT_GROUP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OBJECT_UNGROUP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STICKY_NOTE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STICKY_NOTE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_JCB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CC_DINERS_CLUB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CLONE);

			ImGui::TextUnformatted(EP_ICON_BALANCE_SCALE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOURGLASS_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOURGLASS_START);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOURGLASS_HALF);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOURGLASS_END);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOURGLASS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_ROCK_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_PAPER_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_SCISSORS_O);

			ImGui::TextUnformatted(EP_ICON_HAND_LIZARD_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_SPOCK_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_POINTER_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HAND_PEACE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TRADEMARK);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REGISTERED);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CREATIVE_COMMONS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GG);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GG_CIRCLE);

			ImGui::TextUnformatted(EP_ICON_TRIPADVISOR);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ODNOKLASSNIKI);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_ODNOKLASSNIKI_SQUARE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_GET_POCKET);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_WIKIPEDIA_W);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SAFARI);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CHROME);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FIREFOX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_OPERA);

			ImGui::TextUnformatted(EP_ICON_INTERNET_EXPLORER);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TELEVISION);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_TELEVISION);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CONTAO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_500PX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_AMAZON);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CALENDAR_PLUS_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CALENDAR_MINUS_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CALENDAR_TIMES_O);

			ImGui::TextUnformatted(EP_ICON_CALENDAR_CHECK_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_INDUSTRY);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAP_PIN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAP_SIGNS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAP_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MAP);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMMENTING);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_COMMENTING_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_HOUZZ);

			ImGui::TextUnformatted(EP_ICON_VIMEO);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_BLACK_TIE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FONTICONS);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_REDDIT_ALIEN);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_EDGE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CREDIT_CARD_ALT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_CODIEPIE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MODX);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_FORT_AWESOME);

			ImGui::TextUnformatted(EP_ICON_USB);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PRODUCT_HUNT);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_MIXCLOUD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SCRIBD);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAUSE_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_PAUSE_CIRCLE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STOP_CIRCLE);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_STOP_CIRCLE_O);
			ImGui::SameLine(); ImGui::TextUnformatted(EP_ICON_SHOPPING_BAG);

			ImGui::TextUnformatted(EP_ICON_SHOPPING_BASKET);
			ImGui::TextUnformatted(EP_ICON_HASHTAG);
			ImGui::TextUnformatted(EP_ICON_BLUETOOTH);
			ImGui::TextUnformatted(EP_ICON_BLUETOOTH_B);
			ImGui::TextUnformatted(EP_ICON_PERCENT);
			ImGui::TextUnformatted(EP_ICON_GITLAB);
			ImGui::TextUnformatted(EP_ICON_WPBEGINNER);
			ImGui::TextUnformatted(EP_ICON_WPFORMS);

			ImGui::TreePop();
		}
		
#endif

		ImGui::End();
	}
}
