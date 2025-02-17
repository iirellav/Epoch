#include "epch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Epoch/Utils/YAMLSerializationHelpers.h"
#include "Epoch/Script/ScriptEngine.h"
#include "Epoch/Script/ScriptUtils.h"
#include "Epoch/Assets/AssetManager.h"

namespace Epoch
{
	void SceneSerializer::Serialize(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		YAML::Emitter out;
		SerializeToYAML(out);

		std::ofstream fout(aFilepath);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		std::ifstream stream(aFilepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		myScene->myName = aFilepath.stem().string();

		return DeserializeFromYAML(strStream.str());
	}

	void SceneSerializer::SerializeEntities(YAML::Emitter& aOut)
	{
		EPOCH_PROFILE_FUNC();

		auto idComponentView = myScene->myRegistry.view<IDComponent>();
		for (auto ent : idComponentView)
		{
			Entity entity = { ent, myScene.get() };

			EPOCH_ASSERT(entity.HasComponent<IDComponent>(), "Entity has no ID!");

			aOut << YAML::BeginMap;
			aOut << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

			if (entity.HasComponent<ActiveComponent>())
			{
				aOut << YAML::Key << "ActiveComponent";
				aOut << YAML::BeginMap;

				bool state = entity.GetComponent<ActiveComponent>().isActive;
				aOut << YAML::Key << "IsActive" << YAML::Value << state;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<NameComponent>())
			{
				aOut << YAML::Key << "NameComponent";
				aOut << YAML::BeginMap;

				const std::string& name = entity.GetComponent<NameComponent>().name;
				aOut << YAML::Key << "Name" << YAML::Value << name;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<RelationshipComponent>())
			{
				aOut << YAML::Key << "RelationshipComponent";
				aOut << YAML::BeginMap;

				const RelationshipComponent& rc = entity.GetComponent<RelationshipComponent>();
				aOut << YAML::Key << "Parent" << YAML::Value << rc.parentHandle;

				aOut << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
				for (auto child : rc.children)
				{
					aOut << YAML::BeginMap;
					aOut << YAML::Key << "Handle" << YAML::Value << child;
					aOut << YAML::EndMap;
				}
				aOut << YAML::EndSeq;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<PrefabComponent>())
			{
				aOut << YAML::Key << "PrefabComponent";
				aOut << YAML::BeginMap;

				const PrefabComponent& pc = entity.GetComponent<PrefabComponent>();
				aOut << YAML::Key << "PrefabID" << YAML::Value << pc.prefabID;
				aOut << YAML::Key << "EntityID" << YAML::Value << pc.entityID;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<TransformComponent>())
			{
				aOut << YAML::Key << "TransformComponent";
				aOut << YAML::BeginMap;

				const TransformComponent& tc = entity.GetComponent<TransformComponent>();
				aOut << YAML::Key << "Translation" << YAML::Value << tc.transform.GetTranslation();
				aOut << YAML::Key << "Rotation" << YAML::Value << tc.transform.GetRotation();
				aOut << YAML::Key << "Scale" << YAML::Value << tc.transform.GetScale();

				aOut << YAML::EndMap;
			}
			
			if (entity.HasComponent<RectComponent>())
			{
				aOut << YAML::Key << "RectComponent";
				aOut << YAML::BeginMap;

				const RectComponent& rc = entity.GetComponent<RectComponent>();
				aOut << YAML::Key << "Size" << YAML::Value << rc.size;
				aOut << YAML::Key << "Pivot" << YAML::Value << rc.pivot;
				aOut << YAML::Key << "Anchor" << YAML::Value << rc.anchor;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<ScriptComponent>())
			{
				aOut << YAML::Key << "ScriptComponent";
				aOut << YAML::BeginMap;

				const ScriptComponent& sc = entity.GetComponent<ScriptComponent>();
				ManagedClass* scriptClass = ScriptCache::GetManagedClassByID(ScriptEngine::GetScriptClassIDFromComponent(sc));
				aOut << YAML::Key << "ScriptClassHandle" << YAML::Value << sc.scriptClassHandle;
				aOut << YAML::Key << "Name" << YAML::Value << (scriptClass ? scriptClass->fullName : "Null");

				if (sc.fieldIDs.size() > 0)
				{
					aOut << YAML::Key << "StoredFields" << YAML::Value;
					aOut << YAML::BeginSeq;

					for (auto fieldID : sc.fieldIDs)
					{
						FieldInfo* fieldInfo = ScriptCache::GetFieldByID(fieldID);

						if (!fieldInfo->IsWritable())
						{
							continue;
						}

						std::shared_ptr<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(entity, fieldID);

						if (!storage)
						{
							continue;
						}

						aOut << YAML::BeginMap; // Field
						aOut << YAML::Key << "ID" << YAML::Value << fieldInfo->id;
						aOut << YAML::Key << "Name" << YAML::Value << fieldInfo->name; // This is only here for the sake of debugging. All we need is the ID
						aOut << YAML::Key << "Type" << YAML::Value << FieldUtils::FieldTypeToString(fieldInfo->type);

						aOut << YAML::Key << "Data" << YAML::Value;

						if (fieldInfo->IsArray())
						{
							aOut << YAML::BeginSeq;

							std::shared_ptr<ArrayFieldStorage> arrayStorage = std::dynamic_pointer_cast<ArrayFieldStorage>(storage);
							for (uint32_t i = 0; i < uint32_t(arrayStorage->GetLength()); i++)
							{
								switch (fieldInfo->type)
								{
								case FieldType::Bool:
								{
									aOut << arrayStorage->GetValue<bool>(i);
									break;
								}
								case FieldType::Int8:
								{
									aOut << arrayStorage->GetValue<int8_t>(i);
									break;
								}
								case FieldType::Int16:
								{
									aOut << arrayStorage->GetValue<int16_t>(i);
									break;
								}
								case FieldType::Int32:
								{
									aOut << arrayStorage->GetValue<int32_t>(i);
									break;
								}
								case FieldType::Int64:
								{
									aOut << arrayStorage->GetValue<int64_t>(i);
									break;
								}
								case FieldType::UInt8:
								{
									aOut << arrayStorage->GetValue<uint8_t>(i);
									break;
								}
								case FieldType::UInt16:
								{
									aOut << arrayStorage->GetValue<uint16_t>(i);
									break;
								}
								case FieldType::UInt32:
								{
									aOut << arrayStorage->GetValue<uint32_t>(i);
									break;
								}
								case FieldType::UInt64:
								{
									aOut << arrayStorage->GetValue<uint64_t>(i);
									break;
								}
								case FieldType::Float:
								{
									aOut << arrayStorage->GetValue<float>(i);
									break;
								}
								case FieldType::Double:
								{
									aOut << arrayStorage->GetValue<double>(i);
									break;
								}
								case FieldType::String:
								{
									aOut << arrayStorage->GetValue<std::string>(i);
									break;
								}
								case FieldType::Vector2:
								{
									aOut << arrayStorage->GetValue<CU::Vector2f>(i);
									break;
								}
								case FieldType::Vector3:
								{
									aOut << arrayStorage->GetValue<CU::Vector3f>(i);
									break;
								}
								case FieldType::Color:
								{
									aOut << arrayStorage->GetValue<CU::Vector4f>(i);
									break;
								}
								case FieldType::Scene:
								case FieldType::Entity:
								case FieldType::Prefab:
								case FieldType::Material:
								case FieldType::Mesh:
								case FieldType::Texture2D:
								{
									aOut << arrayStorage->GetValue<UUID>(i);
									break;
								}
								default: EPOCH_ASSERT(false, "Field failed to be serialized!");
								}
							}

							aOut << YAML::EndSeq;
						}
						else
						{
							std::shared_ptr<FieldStorage> fieldStorage = std::dynamic_pointer_cast<FieldStorage>(storage);
							switch (fieldInfo->type)
							{
							case FieldType::Bool:
							{
								aOut << fieldStorage->GetValue<bool>();
								break;
							}
							case FieldType::Int8:
							{
								aOut << fieldStorage->GetValue<int8_t>();
								break;
							}
							case FieldType::Int16:
							{
								aOut << fieldStorage->GetValue<int16_t>();
								break;
							}
							case FieldType::Int32:
							{
								aOut << fieldStorage->GetValue<int32_t>();
								break;
							}
							case FieldType::Int64:
							{
								aOut << fieldStorage->GetValue<int64_t>();
								break;
							}
							case FieldType::UInt8:
							{
								aOut << fieldStorage->GetValue<uint8_t>();
								break;
							}
							case FieldType::UInt16:
							{
								aOut << fieldStorage->GetValue<uint16_t>();
								break;
							}
							case FieldType::LayerMask:
							case FieldType::UInt32:
							{
								aOut << fieldStorage->GetValue<uint32_t>();
								break;
							}
							case FieldType::UInt64:
							{
								aOut << fieldStorage->GetValue<uint64_t>();
								break;
							}
							case FieldType::Float:
							{
								aOut << fieldStorage->GetValue<float>();
								break;
							}
							case FieldType::Double:
							{
								aOut << fieldStorage->GetValue<double>();
								break;
							}
							case FieldType::String:
							{
								aOut << fieldStorage->GetValue<std::string>();
								break;
							}
							case FieldType::Vector2:
							{
								aOut << fieldStorage->GetValue<CU::Vector2f>();
								break;
							}
							case FieldType::Vector3:
							{
								aOut << fieldStorage->GetValue<CU::Vector3f>();
								break;
							}
							case FieldType::Color:
							{
								aOut << fieldStorage->GetValue<CU::Vector4f>();
								break;
							}
							case FieldType::Scene:
							case FieldType::Entity:
							case FieldType::Prefab:
							case FieldType::Material:
							case FieldType::Mesh:
							{
								aOut << fieldStorage->GetValue<UUID>();
								break;
							}
							default: EPOCH_ASSERT(false, "Field failed to be serialized!");
							}
						}

						aOut << YAML::EndMap; // Field
					}

					aOut << YAML::EndSeq;
				}

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<MeshRendererComponent>())
			{
				aOut << YAML::Key << "MeshRendererComponent";
				aOut << YAML::BeginMap;

				const MeshRendererComponent& mr = entity.GetComponent<MeshRendererComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << mr.isActive;
				aOut << YAML::Key << "Mesh" << YAML::Value << mr.mesh;
				aOut << YAML::Key << "CastsShadows" << YAML::Value << mr.castsShadows;

				auto materialTable = mr.materialTable;
				if (materialTable->GetMaterialCount() > 0)
				{
					aOut << YAML::Key << "MaterialTable" << YAML::Value << YAML::BeginSeq; // MaterialTable

					for (uint32_t i = 0; i < materialTable->GetMaterialCount(); i++)
					{
						aOut << YAML::Value << materialTable->GetMaterial(i);
					}

					aOut << YAML::EndSeq; // MaterialTable
				}

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<SkinnedMeshRendererComponent>())
			{
				aOut << YAML::Key << "SkinnedMeshRendererComponent";
				aOut << YAML::BeginMap;

				const SkinnedMeshRendererComponent& smr = entity.GetComponent<SkinnedMeshRendererComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << smr.isActive;
				aOut << YAML::Key << "Mesh" << YAML::Value << smr.mesh;
				aOut << YAML::Key << "CastsShadows" << YAML::Value << smr.castsShadows;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<SpriteRendererComponent>())
			{
				aOut << YAML::Key << "SpriteRendererComponent";
				aOut << YAML::BeginMap;

				const SpriteRendererComponent& sr = entity.GetComponent<SpriteRendererComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << sr.isActive;
				aOut << YAML::Key << "Sprite" << YAML::Value << sr.texture;
				aOut << YAML::Key << "Tint" << YAML::Value << sr.tint.GetVector4();
				aOut << YAML::Key << "FlipX" << YAML::Value << sr.flipX;
				aOut << YAML::Key << "FlipY" << YAML::Value << sr.flipY;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<ImageComponent>())
			{
				aOut << YAML::Key << "ImageComponent";
				aOut << YAML::BeginMap;

				const ImageComponent& i = entity.GetComponent<ImageComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << i.isActive;
				aOut << YAML::Key << "Sprite" << YAML::Value << i.texture;
				aOut << YAML::Key << "Tint" << YAML::Value << i.tint;
				aOut << YAML::Key << "FlipX" << YAML::Value << i.flipX;
				aOut << YAML::Key << "FlipY" << YAML::Value << i.flipY;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<ButtonComponent>())
			{
				aOut << YAML::Key << "ButtonComponent";
				aOut << YAML::BeginMap;

				const ButtonComponent& b = entity.GetComponent<ButtonComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << b.isActive;
				aOut << YAML::Key << "DefaultColor" << YAML::Value << b.colorGroup.defaultColor;
				aOut << YAML::Key << "HoveredColor" << YAML::Value << b.colorGroup.hoveredColor;
				aOut << YAML::Key << "PressedColor" << YAML::Value << b.colorGroup.pressedColor;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<CheckboxComponent>())
			{
				aOut << YAML::Key << "CheckboxComponent";
				aOut << YAML::BeginMap;

				const CheckboxComponent& c = entity.GetComponent<CheckboxComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << c.isActive;
				aOut << YAML::Key << "DefaultColor" << YAML::Value << c.colorGroup.defaultColor;
				aOut << YAML::Key << "HoveredColor" << YAML::Value << c.colorGroup.hoveredColor;
				aOut << YAML::Key << "PressedColor" << YAML::Value << c.colorGroup.pressedColor;

				aOut << YAML::Key << "IsOn" << YAML::Value << c.isOn;
				aOut << YAML::Key << "Checkmark" << YAML::Value << c.checkmark;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<VideoPlayerComponent>())
			{
				aOut << YAML::Key << "VideoPlayerComponent";
				aOut << YAML::BeginMap;

				const VideoPlayerComponent& vp = entity.GetComponent<VideoPlayerComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << vp.isActive;
				aOut << YAML::Key << "Video" << YAML::Value << vp.video;
				aOut << YAML::Key << "PlayOnAwake" << YAML::Value << vp.playOnAwake;
				aOut << YAML::Key << "Loop" << YAML::Value << vp.loop;
				aOut << YAML::Key << "Play Back Speed" << YAML::Value << vp.playbackSpeed;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<TextRendererComponent>())
			{
				aOut << YAML::Key << "TextRendererComponent";
				aOut << YAML::BeginMap;

				const TextRendererComponent& tr = entity.GetComponent<TextRendererComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << tr.isActive;
				aOut << YAML::Key << "Text" << YAML::Value << tr.text;
				aOut << YAML::Key << "Font" << YAML::Value << tr.font;
				aOut << YAML::Key << "Color" << YAML::Value << tr.color.GetVector4();

				aOut << YAML::Key << "LetterSpacing" << YAML::Value << tr.letterSpacing;
				aOut << YAML::Key << "LineSpacing" << YAML::Value << tr.lineSpacing;
				aOut << YAML::Key << "LineWidth" << YAML::Value << tr.maxWidth;
				aOut << YAML::Key << "Billboard" << YAML::Value << tr.billboard;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<SkyLightComponent>())
			{
				aOut << YAML::Key << "SkyLightComponent";
				aOut << YAML::BeginMap;

				const SkyLightComponent& sl = entity.GetComponent<SkyLightComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << sl.isActive;
				aOut << YAML::Key << "Environment" << YAML::Value << (uint64_t)sl.environment;
				aOut << YAML::Key << "Intensity" << YAML::Value << sl.intensity;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<DirectionalLightComponent>())
			{
				aOut << YAML::Key << "DirectionalLightComponent";
				aOut << YAML::BeginMap;

				const DirectionalLightComponent& dl = entity.GetComponent<DirectionalLightComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << dl.isActive;
				aOut << YAML::Key << "Color" << YAML::Value << dl.color.GetVector3();
				aOut << YAML::Key << "Intensity" << YAML::Value << dl.intensity;
				aOut << YAML::Key << "CastsShadows" << YAML::Value << dl.castsShadows;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<SpotlightComponent>())
			{
				aOut << YAML::Key << "SpotlightComponent";
				aOut << YAML::BeginMap;

				const SpotlightComponent& sl = entity.GetComponent<SpotlightComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << sl.isActive;
				aOut << YAML::Key << "Color" << YAML::Value << sl.color.GetVector3();
				aOut << YAML::Key << "Intensity" << YAML::Value << sl.intensity;
				aOut << YAML::Key << "Range" << YAML::Value << sl.range;
				aOut << YAML::Key << "OuterSpotAngle" << YAML::Value << sl.outerSpotAngle;
				aOut << YAML::Key << "InnerSpotAngle" << YAML::Value << sl.innerSpotAngle;
				aOut << YAML::Key << "CastsShadows" << YAML::Value << sl.castsShadows;
				aOut << YAML::Key << "Cookie" << YAML::Value << sl.cookieTexture;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<PointLightComponent>())
			{
				aOut << YAML::Key << "PointLightComponent";
				aOut << YAML::BeginMap;

				const PointLightComponent& pl = entity.GetComponent<PointLightComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << pl.isActive;
				aOut << YAML::Key << "Color" << YAML::Value << pl.color.GetVector3();
				aOut << YAML::Key << "Intensity" << YAML::Value << pl.intensity;
				aOut << YAML::Key << "Range" << YAML::Value << pl.range;
				aOut << YAML::Key << "CastsShadows" << YAML::Value << pl.castsShadows;
				aOut << YAML::Key << "Cookie" << YAML::Value << pl.cookieTexture;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<CameraComponent>())
			{
				aOut << YAML::Key << "CameraComponent";
				aOut << YAML::BeginMap;

				const CameraComponent& cc = entity.GetComponent<CameraComponent>();
				const SceneCamera& c = cc.camera;

				aOut << YAML::Key << "IsActive" << YAML::Value << cc.isActive;

				aOut << YAML::Key << "Camera" << YAML::Value;
				aOut << YAML::BeginMap;

				aOut << YAML::Key << "ProjectionType" << YAML::Value << (int)c.GetProjectionType();
				aOut << YAML::Key << "PerspectiveFOV" << YAML::Value << c.GetPerspectiveFOV();
				aOut << YAML::Key << "PerspectiveNear" << YAML::Value << c.GetPerspectiveNearPlane();
				aOut << YAML::Key << "PerspectiveFar" << YAML::Value << c.GetPerspectiveFarPlane();
				aOut << YAML::Key << "OrthographicSize" << YAML::Value << c.GetOrthographicSize();
				aOut << YAML::Key << "OrthographicNear" << YAML::Value << c.GetOrthographicNearPlane();
				aOut << YAML::Key << "OrthographicFar" << YAML::Value << c.GetOrthographicFarPlane();
				aOut << YAML::EndMap;

				aOut << YAML::Key << "Primary" << YAML::Value << cc.primary;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<ParticleSystemComponent>())
			{
				aOut << YAML::Key << "ParticleSystemComponent";
				aOut << YAML::BeginMap;

				const ParticleSystemComponent& ps = entity.GetComponent<ParticleSystemComponent>();

				//Base
				{
					aOut << YAML::Key << "IsActive" << YAML::Value << ps.isActive;

					aOut << YAML::Key << "Duration" << YAML::Value << ps.duration;
					aOut << YAML::Key << "Looping" << YAML::Value << ps.looping;
					aOut << YAML::Key << "Prewarm" << YAML::Value << ps.prewarm;

					aOut << YAML::Key << "StartLifetime" << YAML::Value << ps.startLifetime;
					aOut << YAML::Key << "StartSpeed" << YAML::Value << ps.startSpeed;
					aOut << YAML::Key << "StartSize" << YAML::Value << ps.startSize;
					aOut << YAML::Key << "StartColor" << YAML::Value << ps.startColor.GetVector4();

					aOut << YAML::Key << "GravityMultiplier" << YAML::Value << ps.gravityMultiplier;
					aOut << YAML::Key << "PlayOnAwake" << YAML::Value << ps.playOnAwake;
				}

				//Emission
				{
					aOut << YAML::Key << "Emission";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Rate" << YAML::Value << ps.emission.rate;
					//aOut << YAML::Key << "Bursts" << YAML::Value << ps.emission.bursts;

					aOut << YAML::EndMap;
				}

				//Shape
				{
					aOut << YAML::Key << "Shape";
					aOut << YAML::BeginMap;

					//aOut << YAML::Key << "Shape" << YAML::Value << ps.shape.shape;
					aOut << YAML::Key << "Angle" << YAML::Value << ps.shape.angle;
					aOut << YAML::Key << "Radius" << YAML::Value << ps.shape.radius;

					aOut << YAML::Key << "Position" << YAML::Value << ps.shape.position;
					aOut << YAML::Key << "Rotation" << YAML::Value << ps.shape.rotation;
					aOut << YAML::Key << "Scale" << YAML::Value << ps.shape.scale;

					aOut << YAML::EndMap;
				}

				//Velocity over Lifetime
				{
					aOut << YAML::Key << "VelocityOverLifetime";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Linear" << YAML::Value << ps.velocityOverLifetime.linear;

					aOut << YAML::EndMap;
				}

				//Color over Lifetime
				{
					aOut << YAML::Key << "ColorOverLifetime";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Gradient" << YAML::Value << ps.colorOverLifetime.colorGradient;

					aOut << YAML::EndMap;
				}

				//Size over Lifetime
				{
					aOut << YAML::Key << "SizeOverLifetime";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "StartSize" << YAML::Value << ps.sizeOverLifetime.start;
					aOut << YAML::Key << "EndSize" << YAML::Value << ps.sizeOverLifetime.end;

					aOut << YAML::EndMap;
				}

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<VolumeComponent>())
			{
				aOut << YAML::Key << "VolumeComponent";
				aOut << YAML::BeginMap;

				const VolumeComponent& v = entity.GetComponent<VolumeComponent>();
				aOut << YAML::Key << "IsActive" << YAML::Value << v.isActive;

				//Color Grading
				{
					aOut << YAML::Key << "Tonemapping";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Enabled" << YAML::Value << v.tonemapping.enabled;
					aOut << YAML::Key << "Tonemap" << YAML::Value << (int)v.tonemapping.tonemap;
					
					aOut << YAML::EndMap;
				}

				//Color Grading
				{
					aOut << YAML::Key << "ColorGrading";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Enabled" << YAML::Value << v.colorGrading.enabled;
					aOut << YAML::Key << "LUT" << YAML::Value << v.colorGrading.lut;
					
					aOut << YAML::EndMap;
				}

				//Vignette
				{
					aOut << YAML::Key << "Vignette";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Enabled" << YAML::Value << v.vignette.enabled;
					aOut << YAML::Key << "Color" << YAML::Value << v.vignette.color;
					aOut << YAML::Key << "Center" << YAML::Value << v.vignette.center;
					aOut << YAML::Key << "Intensity" << YAML::Value << v.vignette.intensity;
					aOut << YAML::Key << "Size" << YAML::Value << v.vignette.size;
					aOut << YAML::Key << "Smoothness" << YAML::Value << v.vignette.smoothness;
					
					aOut << YAML::EndMap;
				}

				//Distance Fog
				{
					aOut << YAML::Key << "DistanceFog";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Enabled" << YAML::Value << v.distanceFog.enabled;
					aOut << YAML::Key << "Color" << YAML::Value << v.distanceFog.color.GetVector3();
					aOut << YAML::Key << "Density" << YAML::Value << v.distanceFog.density;
					aOut << YAML::Key << "Offset" << YAML::Value << v.distanceFog.offset;

					aOut << YAML::EndMap;
				}

				//Posterization
				{
					aOut << YAML::Key << "Posterization";
					aOut << YAML::BeginMap;

					aOut << YAML::Key << "Enabled" << YAML::Value << v.posterization.enabled;
					aOut << YAML::Key << "Steps" << YAML::Value << v.posterization.steps;

					aOut << YAML::EndMap;
				}

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<RigidbodyComponent>())
			{
				aOut << YAML::Key << "RigidbodyComponent";
				aOut << YAML::BeginMap;

				const RigidbodyComponent& rb = entity.GetComponent<RigidbodyComponent>();
				aOut << YAML::Key << "Mass" << YAML::Value << rb.mass;
				aOut << YAML::Key << "LinearDrag" << YAML::Value << rb.linearDrag;
				aOut << YAML::Key << "AngularDrag" << YAML::Value << rb.angularDrag;
				aOut << YAML::Key << "UseGravity" << YAML::Value << rb.useGravity;
				aOut << YAML::Key << "IsKinematic" << YAML::Value << rb.isKinematic;

				aOut << YAML::Key << "Constraints" << YAML::Value << (uint32_t)(uint8_t)rb.constraints;

				aOut << YAML::Key << "InitialLinearVelocity" << YAML::Value << rb.initialLinearVelocity;
				aOut << YAML::Key << "InitialAngularVelocity" << YAML::Value << rb.initialAngularVelocity;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<BoxColliderComponent>())
			{
				aOut << YAML::Key << "BoxColliderComponent";
				aOut << YAML::BeginMap;

				const BoxColliderComponent& bc = entity.GetComponent<BoxColliderComponent>();
				aOut << YAML::Key << "HalfSize" << YAML::Value << bc.halfSize;
				aOut << YAML::Key << "Offset" << YAML::Value << bc.offset;
				aOut << YAML::Key << "Layer" << YAML::Value << bc.layerID;
				aOut << YAML::Key << "PhysicsMaterial" << YAML::Value << bc.physicsMaterial;
				aOut << YAML::Key << "IsTrigger" << YAML::Value << bc.isTrigger;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<SphereColliderComponent>())
			{
				aOut << YAML::Key << "SphereColliderComponent";
				aOut << YAML::BeginMap;

				const SphereColliderComponent& sc = entity.GetComponent<SphereColliderComponent>();
				aOut << YAML::Key << "Radius" << YAML::Value << sc.radius;
				aOut << YAML::Key << "Offset" << YAML::Value << sc.offset;
				aOut << YAML::Key << "Layer" << YAML::Value << sc.layerID;
				aOut << YAML::Key << "PhysicsMaterial" << YAML::Value << sc.physicsMaterial;
				aOut << YAML::Key << "IsTrigger" << YAML::Value << sc.isTrigger;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<CapsuleColliderComponent>())
			{
				aOut << YAML::Key << "CapsuleColliderComponent";
				aOut << YAML::BeginMap;

				const CapsuleColliderComponent& cc = entity.GetComponent<CapsuleColliderComponent>();
				aOut << YAML::Key << "Radius" << YAML::Value << cc.radius;
				aOut << YAML::Key << "Height" << YAML::Value << cc.height;
				aOut << YAML::Key << "Offset" << YAML::Value << cc.offset;
				aOut << YAML::Key << "Layer" << YAML::Value << cc.layerID;
				aOut << YAML::Key << "PhysicsMaterial" << YAML::Value << cc.physicsMaterial;
				aOut << YAML::Key << "IsTrigger" << YAML::Value << cc.isTrigger;

				aOut << YAML::EndMap;
			}

			if (entity.HasComponent<CharacterControllerComponent>())
			{
				aOut << YAML::Key << "CharacterControllerComponent";
				aOut << YAML::BeginMap;

				const CharacterControllerComponent& cc = entity.GetComponent<CharacterControllerComponent>();
				aOut << YAML::Key << "SlopeLimit" << YAML::Value << cc.slopeLimit;
				aOut << YAML::Key << "StepOffset" << YAML::Value << cc.stepOffset;
				aOut << YAML::Key << "Radius" << YAML::Value << cc.radius;
				aOut << YAML::Key << "Height" << YAML::Value << cc.height;
				aOut << YAML::Key << "Offset" << YAML::Value << cc.offset;
				aOut << YAML::Key << "Layer" << YAML::Value << cc.layerID;

				aOut << YAML::EndMap;
			}

			aOut << YAML::EndMap;
		}
	}

	void SceneSerializer::DeserializeEntities(YAML::Node& aEntitiesNode)
	{
		EPOCH_PROFILE_FUNC();

		for (auto entity : aEntitiesNode)
		{
			UUID uuid = (UUID)entity["Entity"].as<uint64_t>();

			std::string name;
			YAML::Node nameComponent = entity["NameComponent"];
			if (nameComponent)
			{
				name = nameComponent["Name"].as<std::string>();
			}

			Entity deserializedEntity = myScene->CreateEntityWithUUID(uuid, name);

			YAML::Node activeComponent = entity["ActiveComponent"];
			if (activeComponent)
			{
				if (deserializedEntity.HasComponent<ActiveComponent>())
				{
					ActiveComponent& ac = deserializedEntity.GetComponent<ActiveComponent>();
					ac.isActive = activeComponent["IsActive"].as<bool>(true);
				}
				else
				{
					ActiveComponent& ac = deserializedEntity.AddComponent<ActiveComponent>();
					ac.isActive = activeComponent["IsActive"].as<bool>(true);
				}
			}

			YAML::Node relationshipComponent = entity["RelationshipComponent"];
			if (relationshipComponent)
			{
				RelationshipComponent& rc = deserializedEntity.GetComponent<RelationshipComponent>();
				uint64_t parentHandle = relationshipComponent["Parent"] ? relationshipComponent["Parent"].as<uint64_t>() : 0;
				rc.parentHandle = parentHandle;

				auto children = relationshipComponent["Children"];
				if (children)
				{
					for (auto child : children)
					{
						uint64_t childHandle = child["Handle"].as<uint64_t>();
						rc.children.push_back(childHandle);
					}
				}
			}

			YAML::Node prefabComponent = entity["PrefabComponent"];
			if (prefabComponent)
			{
				PrefabComponent& pc = deserializedEntity.AddComponent<PrefabComponent>();

				pc.prefabID = prefabComponent["PrefabID"].as<uint64_t>((uint64_t)0);
				pc.entityID = prefabComponent["EntityID"].as<uint64_t>((uint64_t)0);
			}

			YAML::Node transformComponent = entity["TransformComponent"];
			if (transformComponent)
			{
				TransformComponent& tc = deserializedEntity.GetComponent<TransformComponent>();
				tc.transform.SetTranslation(transformComponent["Translation"].as<CU::Vector3f>());
				tc.transform.SetRotation(transformComponent["Rotation"].as<CU::Vector3f>());
				tc.transform.SetScale(transformComponent["Scale"].as<CU::Vector3f>());
			}

			YAML::Node rectComponent = entity["RectComponent"];
			if (rectComponent)
			{
				RectComponent& rc = deserializedEntity.AddComponent<RectComponent>();
				rc.size = rectComponent["Size"].as<CU::Vector2ui>(CU::Vector2ui(100, 100));
				rc.pivot = rectComponent["Pivot"].as<CU::Vector2f>(CU::Vector2f(0.5f, 0.5f));
				rc.anchor = rectComponent["Anchor"].as<CU::Vector2f>(CU::Vector2f(0.5f, 0.5f));
			}

			YAML::Node scriptComponent = entity["ScriptComponent"];
			if (scriptComponent)
			{
				ScriptComponent& sc = deserializedEntity.AddComponent<ScriptComponent>();

				AssetHandle scriptAssetHandle = scriptComponent["ScriptClassHandle"].as<uint64_t>((uint64_t)0);
				std::string name = scriptComponent["Name"].as<std::string>("");

				if (scriptAssetHandle != 0)
				{
					sc.scriptClassHandle = scriptAssetHandle;
					ScriptEngine::InitializeScriptEntity(deserializedEntity);

					if (sc.fieldIDs.size() > 0)
					{
						auto storedFields = scriptComponent["StoredFields"];
						if (storedFields)
						{
							for (auto field : storedFields)
							{
								uint32_t id = field["ID"].as<uint32_t>(0);
								std::string fullName = field["Name"].as<std::string>();
								std::string name = CU::SubStr(fullName, fullName.find(':') + 1);
								std::string typeStr = field["Type"].as<std::string>("");
								FieldInfo* fieldData = ScriptCache::GetFieldByID(id);
								std::shared_ptr<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(deserializedEntity, id);

								if (fieldData == nullptr || storage == nullptr)
								{
									id = (uint32_t)Hash::GenerateFNVHash(name);
									fieldData = ScriptCache::GetFieldByID(id);
									storage = ScriptEngine::GetFieldStorage(deserializedEntity, id);
								}

								if (storage == nullptr)
								{
									CONSOLE_LOG_WARN("Serialized C# field '{}' which doesn't exist in script cache! This could be because the script field no longer exists or because it's been renamed.", name);
								}
								else
								{
									auto dataNode = field["Data"];

									if (fieldData->IsArray() && dataNode.IsSequence())
									{
										std::shared_ptr<ArrayFieldStorage> arrayStorage = std::dynamic_pointer_cast<ArrayFieldStorage>(storage);
										arrayStorage->Resize(uint32_t(dataNode.size()));
										
										for (uint32_t i = 0; i < uint32_t(dataNode.size()); i++)
										{
											switch (fieldData->type)
											{
												case FieldType::Bool:
												{
													arrayStorage->SetValue(i, dataNode[i].as<bool>());
													break;
												}
												case FieldType::Int8:
												{
													arrayStorage->SetValue(i, static_cast<int8_t>(dataNode[i].as<int16_t>()));
													break;
												}
												case FieldType::Int16:
												{
													arrayStorage->SetValue(i, dataNode[i].as<int16_t>());
													break;
												}
												case FieldType::Int32:
												{
													arrayStorage->SetValue(i, dataNode[i].as<int32_t>());
													break;
												}
												case FieldType::Int64:
												{
													arrayStorage->SetValue(i, dataNode[i].as<int64_t>());
													break;
												}
												case FieldType::UInt8:
												{
													arrayStorage->SetValue(i, dataNode[i].as<uint8_t>());
													break;
												}
												case FieldType::UInt16:
												{
													arrayStorage->SetValue(i, dataNode[i].as<uint16_t>());
													break;
												}
												case FieldType::UInt32:
												{
													arrayStorage->SetValue(i, dataNode[i].as<uint32_t>());
													break;
												}
												case FieldType::UInt64:
												{
													arrayStorage->SetValue(i, dataNode[i].as<uint64_t>());
													break;
												}
												case FieldType::Float:
												{
													arrayStorage->SetValue(i, dataNode[i].as<float>());
													break;
												}
												case FieldType::Double:
												{
													arrayStorage->SetValue(i, dataNode[i].as<double>());
													break;
												}
												case FieldType::String:
												{
													arrayStorage->SetValue(i, dataNode[i].as<std::string>());
													break;
												}
												case FieldType::Vector2:
												{
													arrayStorage->SetValue(i, dataNode[i].as<CU::Vector2f>());
													break;
												}
												case FieldType::Vector3:
												{
													arrayStorage->SetValue(i, dataNode[i].as<CU::Vector3f>());
													break;
												}
												case FieldType::Color:
												{
													arrayStorage->SetValue(i, dataNode[i].as<CU::Vector4f>());
													break;
												}
												case FieldType::Scene:
												case FieldType::Entity:
												case FieldType::Prefab:
												case FieldType::Material:
												case FieldType::Mesh:
												{
													arrayStorage->SetValue(i, dataNode[i].as<UUID>());
													break;
												}
												default: EPOCH_ASSERT(false, "Field failed to be deserialized!");
											}
										}
									}
									else
									{
										std::shared_ptr<FieldStorage> fieldStorage = std::dynamic_pointer_cast<FieldStorage>(storage);
										switch (fieldData->type)
										{
										case FieldType::Bool:
										{
											fieldStorage->SetValue(dataNode.as<bool>());
											break;
										}
										case FieldType::Int8:
										{
											fieldStorage->SetValue(static_cast<int8_t>(dataNode.as<int16_t>()));
											break;
										}
										case FieldType::Int16:
										{
											fieldStorage->SetValue(dataNode.as<int16_t>());
											break;
										}
										case FieldType::Int32:
										{
											fieldStorage->SetValue(dataNode.as<int32_t>());
											break;
										}
										case FieldType::Int64:
										{
											fieldStorage->SetValue(dataNode.as<int64_t>());
											break;
										}
										case FieldType::UInt8:
										{
											fieldStorage->SetValue(dataNode.as<uint8_t>());
											break;
										}
										case FieldType::UInt16:
										{
											fieldStorage->SetValue(dataNode.as<uint16_t>());
											break;
										}
										case FieldType::LayerMask:
										case FieldType::UInt32:
										{
											fieldStorage->SetValue(dataNode.as<uint32_t>());
											break;
										}
										case FieldType::UInt64:
										{
											fieldStorage->SetValue(dataNode.as<uint64_t>());
											break;
										}
										case FieldType::Float:
										{
											fieldStorage->SetValue(dataNode.as<float>());
											break;
										}
										case FieldType::Double:
										{
											fieldStorage->SetValue(dataNode.as<double>());
											break;
										}
										case FieldType::String:
										{
											fieldStorage->SetValue(dataNode.as<std::string>());
											break;
										}
										case FieldType::Vector2:
										{
											fieldStorage->SetValue(dataNode.as<CU::Vector2f>());
											break;
										}
										case FieldType::Vector3:
										{
											fieldStorage->SetValue(dataNode.as<CU::Vector3f>());
											break;
										}
										case FieldType::Color:
										{
											fieldStorage->SetValue(dataNode.as<CU::Vector4f>());
											break;
										}
										case FieldType::Scene:
										case FieldType::Entity:
										case FieldType::Prefab:
										case FieldType::Material:
										case FieldType::Mesh:
										case FieldType::Texture2D:
										{
											fieldStorage->SetValue(dataNode.as<UUID>());
											break;
										}
										default: EPOCH_ASSERT(false, "Field failed to be deserialized!");
										}
									}
								}
							}
						}
					}
				}
			}

			YAML::Node meshRendererComponent = entity["MeshRendererComponent"];
			if (meshRendererComponent)
			{
				MeshRendererComponent& mr = deserializedEntity.AddComponent<MeshRendererComponent>();

				mr.isActive = meshRendererComponent["IsActive"].as<bool>(true);
				mr.mesh = meshRendererComponent["Mesh"].as<UUID>(UUID(0));
				mr.castsShadows = meshRendererComponent["CastsShadows"].as<bool>();

				if (meshRendererComponent["MaterialTable"])
				{
					auto materialTableNode = meshRendererComponent["MaterialTable"];
					mr.materialTable->RemoveMaterial();
					for (auto material : materialTableNode)
					{
						AssetHandle materialAsset = material.as<UUID>(UUID(0));
						mr.materialTable->AddMaterial(materialAsset);
						//if (materialAsset && AssetManager::IsAssetHandleValid(materialAsset))
						//{
						//}
					}
				}
			}

			YAML::Node skinnedMeshRendererComponent = entity["SkinnedMeshRendererComponent"];
			if (skinnedMeshRendererComponent)
			{
				SkinnedMeshRendererComponent& smr = deserializedEntity.AddComponent<SkinnedMeshRendererComponent>();

				smr.isActive = skinnedMeshRendererComponent["IsActive"].as<bool>(true);
				smr.mesh = skinnedMeshRendererComponent["Mesh"].as<UUID>(UUID(0));
				smr.castsShadows = skinnedMeshRendererComponent["CastsShadows"].as<bool>();
			}

			YAML::Node spriteRendererComponent = entity["SpriteRendererComponent"];
			if (spriteRendererComponent)
			{
				SpriteRendererComponent& sr = deserializedEntity.AddComponent<SpriteRendererComponent>();

				sr.isActive = spriteRendererComponent["IsActive"].as<bool>(true);
				sr.texture = spriteRendererComponent["Sprite"].as<UUID>(UUID(0));
				sr.tint = spriteRendererComponent["Tint"].as<CU::Color>(CU::Color::White);
				sr.flipX = spriteRendererComponent["FlipX"].as<bool>();
				sr.flipY = spriteRendererComponent["FlipY"].as<bool>();
			}

			YAML::Node imageComponent = entity["ImageComponent"];
			if (imageComponent)
			{
				ImageComponent& i = deserializedEntity.AddComponent<ImageComponent>();

				i.isActive = imageComponent["IsActive"].as<bool>(true);
				i.texture = imageComponent["Sprite"].as<UUID>(UUID(0));
				i.tint = imageComponent["Tint"].as<CU::Color>(CU::Color::White);
				i.flipX = imageComponent["FlipX"].as<bool>();
				i.flipY = imageComponent["FlipY"].as<bool>();
			}

			YAML::Node buttonComponent = entity["ButtonComponent"];
			if (buttonComponent)
			{
				ButtonComponent& b = deserializedEntity.AddComponent<ButtonComponent>();

				b.isActive = buttonComponent["IsActive"].as<bool>(true);
				b.colorGroup.defaultColor = buttonComponent["DefaultColor"].as<CU::Color>(b.colorGroup.defaultColor);
				b.colorGroup.hoveredColor = buttonComponent["HoveredColor"].as<CU::Color>(b.colorGroup.hoveredColor);
				b.colorGroup.pressedColor = buttonComponent["PressedColor"].as<CU::Color>(b.colorGroup.pressedColor);
			}

			YAML::Node checkboxComponent = entity["CheckboxComponent"];
			if (checkboxComponent)
			{
				CheckboxComponent& c = deserializedEntity.AddComponent<CheckboxComponent>();

				c.isActive = checkboxComponent["IsActive"].as<bool>(true);
				c.colorGroup.defaultColor = checkboxComponent["DefaultColor"].as<CU::Color>(c.colorGroup.defaultColor);
				c.colorGroup.hoveredColor = checkboxComponent["HoveredColor"].as<CU::Color>(c.colorGroup.hoveredColor);
				c.colorGroup.pressedColor = checkboxComponent["PressedColor"].as<CU::Color>(c.colorGroup.pressedColor);

				c.isOn = checkboxComponent["IsOn"].as<bool>(true);
				c.checkmark = checkboxComponent["Checkmark"].as<UUID>(0);
			}

			YAML::Node videoPlayerComponent = entity["VideoPlayerComponent"];
			if (videoPlayerComponent)
			{
				VideoPlayerComponent& vp = deserializedEntity.AddComponent<VideoPlayerComponent>();

				vp.isActive = videoPlayerComponent["IsActive"].as<bool>(true);
				vp.video = videoPlayerComponent["Video"].as<UUID>(UUID(0));
				vp.playOnAwake = videoPlayerComponent["PlayOnAwake"].as<bool>(true);
				vp.loop = videoPlayerComponent["Loop"].as<bool>(false);
				vp.playbackSpeed = videoPlayerComponent["PlaybackSpeed"].as<float>(1.0f);
			}

			YAML::Node textRendererComponent = entity["TextRendererComponent"];
			if (textRendererComponent)
			{
				TextRendererComponent& tr = deserializedEntity.AddComponent<TextRendererComponent>();

				tr.isActive = textRendererComponent["IsActive"].as<bool>(true);
				tr.text = textRendererComponent["Text"].as<std::string>();
				tr.font = textRendererComponent["Font"].as<UUID>(UUID(0));
				tr.color = CU::Color(textRendererComponent["Color"].as<CU::Vector4f>());

				tr.letterSpacing = textRendererComponent["LetterSpacing"].as<float>();
				tr.lineSpacing = textRendererComponent["LineSpacing"].as<float>();
				tr.maxWidth = textRendererComponent["LineWidth"].as<float>(10.0f);
				tr.billboard = textRendererComponent["Billboard"].as<bool>(false);
			}

			YAML::Node skyLightComponent = entity["SkyLightComponent"];
			if (skyLightComponent)
			{
				SkyLightComponent& sl = deserializedEntity.AddComponent<SkyLightComponent>();

				sl.isActive = skyLightComponent["IsActive"].as<bool>(true);
				sl.environment = skyLightComponent["Environment"].as<UUID>(UUID(0));
				sl.intensity = skyLightComponent["Intensity"].as<float>();
			}

			YAML::Node directionalLightComponent = entity["DirectionalLightComponent"];
			if (directionalLightComponent)
			{
				DirectionalLightComponent& dl = deserializedEntity.AddComponent<DirectionalLightComponent>();

				dl.isActive = directionalLightComponent["IsActive"].as<bool>(true);
				dl.color = CU::Color(directionalLightComponent["Color"].as<CU::Vector3f>(), 1.0f);
				dl.intensity = directionalLightComponent["Intensity"].as<float>();
				dl.castsShadows = directionalLightComponent["CastsShadows"].as<bool>();
			}

			YAML::Node spotlightComponent = entity["SpotlightComponent"];
			if (spotlightComponent)
			{
				SpotlightComponent& sl = deserializedEntity.AddComponent<SpotlightComponent>();

				sl.isActive = spotlightComponent["IsActive"].as<bool>(true);
				sl.color = CU::Color(spotlightComponent["Color"].as<CU::Vector3f>(), 1.0f);
				sl.intensity = spotlightComponent["Intensity"].as<float>();
				sl.range = spotlightComponent["Range"].as<float>();
				sl.outerSpotAngle = spotlightComponent["OuterSpotAngle"].as<float>();
				sl.innerSpotAngle = spotlightComponent["InnerSpotAngle"].as<float>();
				sl.castsShadows = spotlightComponent["CastsShadows"].as<bool>();
				sl.cookieTexture = spotlightComponent["Cookie"].as<UUID>(UUID(0));
			}

			YAML::Node pointLightComponent = entity["PointLightComponent"];
			if (pointLightComponent)
			{
				PointLightComponent& pl = deserializedEntity.AddComponent<PointLightComponent>();

				pl.isActive = pointLightComponent["IsActive"].as<bool>(true);
				pl.color = CU::Color(pointLightComponent["Color"].as<CU::Vector3f>(), 1.0f);
				pl.intensity = pointLightComponent["Intensity"].as<float>();
				pl.range = pointLightComponent["Range"].as<float>();
				pl.castsShadows = pointLightComponent["CastsShadows"].as<bool>();
				pl.cookieTexture = pointLightComponent["Cookie"].as<UUID>(UUID(0));
			}

			YAML::Node cameraComponent = entity["CameraComponent"];
			if (cameraComponent)
			{
				CameraComponent& cc = deserializedEntity.AddComponent<CameraComponent>();

				cc.isActive = cameraComponent["IsActive"].as<bool>(true);

				YAML::Node cameraProps = cameraComponent["Camera"];
				cc.camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				cc.camera.SetPerspectiveFOV(cameraProps["PerspectiveFOV"].as<float>());
				cc.camera.SetPerspectiveNearPlane(cameraProps["PerspectiveNear"].as<float>());
				cc.camera.SetPerspectiveFarPlane(cameraProps["PerspectiveFar"].as<float>());

				cc.camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.camera.SetOrthographicNearPlane(cameraProps["OrthographicNear"].as<float>());
				cc.camera.SetOrthographicFarPlane(cameraProps["OrthographicFar"].as<float>());

				cc.primary = cameraComponent["Primary"].as<bool>();
			}

			YAML::Node particleSystemComponent = entity["ParticleSystemComponent"];
			if (particleSystemComponent)
			{
				ParticleSystemComponent& pc = deserializedEntity.AddComponent<ParticleSystemComponent>();

				//Base
				{
					pc.isActive = particleSystemComponent["IsActive"].as<bool>(true);

					pc.duration = particleSystemComponent["Duration"].as<float>(1.0f);
					pc.looping = particleSystemComponent["Looping"].as<bool>(true);
					pc.prewarm = particleSystemComponent["Prewarm"].as<bool>(false);

					pc.startLifetime = particleSystemComponent["StartLifetime"].as<float>(1.0f);
					pc.startSpeed = particleSystemComponent["StartSpeed"].as<float>(1.0f);
					pc.startSize = particleSystemComponent["StartSize"].as<float>(1.0f);
					pc.startColor = CU::Color(particleSystemComponent["StartColor"].as<CU::Vector4f>(CU::Vector4f(1.0f, 1.0f, 1.0f, 1.0f)));

					pc.gravityMultiplier = particleSystemComponent["GravityMultiplier"].as<float>(1.0f);
					pc.playOnAwake = particleSystemComponent["PlayOnAwake"].as<bool>(false);
				}

				//Emission
				{
					YAML::Node emissionData = particleSystemComponent["Emission"];

					pc.emission.rate = emissionData["Rate"].as<float>(10.0f);
					//aOut << YAML::Key << "Bursts" << YAML::Value << ps.emission.bursts;
				}

				//Shape
				{
					YAML::Node shapeData = particleSystemComponent["Shape"];

					//aOut << YAML::Key << "Shape" << YAML::Value << ps.shape.shape;
					pc.shape.angle = shapeData["Angle"].as<float>(20.0f);
					pc.shape.radius = shapeData["Radius"].as<float>(50.0f);

					pc.shape.position = shapeData["Position"].as<CU::Vector3f>(CU::Vector3f::Zero);
					pc.shape.rotation = shapeData["Rotation"].as<CU::Vector3f>(CU::Vector3f::Zero);
					pc.shape.scale = shapeData["Scale"].as<CU::Vector3f>(CU::Vector3f::One);
				}

				//Velocity Over Lifetime
				{
					YAML::Node velocityOverLifetimeData = particleSystemComponent["VelocityOverLifetime"];

					pc.velocityOverLifetime.linear = velocityOverLifetimeData["Linear"].as<CU::Vector3f>(CU::Vector3f::Zero);
				}

				//Color Over Lifetime
				{
					YAML::Node colorOverLifetimeData = particleSystemComponent["ColorOverLifetime"];

					pc.colorOverLifetime.colorGradient = colorOverLifetimeData["Gradient"].as<CU::Gradient>();
				}

				//Size Over Lifetime
				{
					YAML::Node sizeOverLifetimeData = particleSystemComponent["SizeOverLifetime"];

					pc.sizeOverLifetime.start = sizeOverLifetimeData["StartSize"].as<float>(1.0f);
					pc.sizeOverLifetime.end = sizeOverLifetimeData["EndSize"].as<float>(1.0f);
				}
			}

			YAML::Node volumeComponent = entity["VolumeComponent"];
			if (volumeComponent)
			{
				VolumeComponent& vc = deserializedEntity.AddComponent<VolumeComponent>();
				
				vc.isActive = volumeComponent["IsActive"].as<bool>(true);
				
				//Tonemapping
				{
					YAML::Node tonemappingData = volumeComponent["Tonemapping"];

					vc.tonemapping.enabled = tonemappingData["Enabled"].as<bool>(true);
					vc.tonemapping.tonemap = (PostProcessing::Tonemap)tonemappingData["Tonemap"].as<int>((int)PostProcessing::Tonemap::Unreal);
				}

				//Color Grading
				{
					YAML::Node colorGradingData = volumeComponent["ColorGrading"];

					vc.colorGrading.enabled = colorGradingData["Enabled"].as<bool>(false);
					vc.colorGrading.lut = colorGradingData["LUT"].as<UUID>(UUID(0));
				}
				
				//Vignette
				{
					YAML::Node vignetteData = volumeComponent["Vignette"];
					
					vc.vignette.enabled = vignetteData["Enabled"].as<bool>(false);
					vc.vignette.color = vignetteData["Color"].as<CU::Vector3f>(CU::Color::Black.GetVector3());
					vc.vignette.center = vignetteData["Center"].as<CU::Vector2f>(CU::Vector2f(0.5f, 0.5f));
					vc.vignette.intensity = vignetteData["Intensity"].as<float>(1.0f);
					vc.vignette.size = vignetteData["Size"].as<float>(1.0f);
					vc.vignette.smoothness = vignetteData["Smoothness"].as<float>(1.0f);
				}

				//Distance Fog
				{
					YAML::Node vignetteData = volumeComponent["DistanceFog"];

					vc.distanceFog.enabled = vignetteData["Enabled"].as<bool>(false);
					vc.distanceFog.color = vignetteData["Color"].as<CU::Vector3f>(CU::Color::White.GetVector3());
					vc.distanceFog.density = vignetteData["Density"].as<float>(0.3f);
					vc.distanceFog.offset = vignetteData["Offset"].as<float>(0.0f);
				}

				//Posterization
				{
					YAML::Node posterizationData = volumeComponent["Posterization"];

					vc.posterization.enabled = posterizationData["Enabled"].as<bool>(false);
					vc.posterization.steps = posterizationData["Steps"].as<uint32_t>(0);
				}
			}

			YAML::Node rigidbodyComponent = entity["RigidbodyComponent"];
			if (rigidbodyComponent)
			{
				RigidbodyComponent& rb = deserializedEntity.AddComponent<RigidbodyComponent>();
				rb.mass = rigidbodyComponent["Mass"].as<float>(1);
				rb.linearDrag = rigidbodyComponent["LinearDrag"].as<float>(0.01f);
				rb.angularDrag = rigidbodyComponent["AngularDrag"].as<float>(0.05f);
				rb.useGravity = rigidbodyComponent["UseGravity"].as<bool>(true);
				rb.isKinematic = rigidbodyComponent["IsKinematic"].as<bool>(false);

				rb.constraints = (Physics::Axis)rigidbodyComponent["Constraints"].as<uint32_t>((uint32_t)(uint8_t)Physics::Axis::None);

				rb.initialLinearVelocity = rigidbodyComponent["InitialLinearVelocity"].as<CU::Vector3f>();
				rb.initialAngularVelocity = rigidbodyComponent["InitialAngularVelocity"].as<CU::Vector3f>();
			}

			YAML::Node boxColliderComponent = entity["BoxColliderComponent"];
			if (boxColliderComponent)
			{
				BoxColliderComponent& bc = deserializedEntity.AddComponent<BoxColliderComponent>();
				bc.halfSize = boxColliderComponent["HalfSize"].as<CU::Vector3f>(CU::Vector3f(50.0f));
				bc.offset = boxColliderComponent["Offset"].as<CU::Vector3f>(CU::Vector3f::Zero);
				bc.layerID = boxColliderComponent["Layer"].as<uint32_t>(0);
				bc.physicsMaterial = boxColliderComponent["PhysicsMaterial"].as<AssetHandle>(AssetHandle(0));
				bc.isTrigger = boxColliderComponent["IsTrigger"].as<bool>(false);
			}

			YAML::Node sphereColliderComponent = entity["SphereColliderComponent"];
			if (sphereColliderComponent)
			{
				SphereColliderComponent& sc = deserializedEntity.AddComponent<SphereColliderComponent>();
				sc.radius = sphereColliderComponent["Radius"].as<float>(50.0f);
				sc.offset = sphereColliderComponent["Offset"].as<CU::Vector3f>(CU::Vector3f::Zero);
				sc.layerID = sphereColliderComponent["Layer"].as<uint32_t>(0);
				sc.physicsMaterial = sphereColliderComponent["PhysicsMaterial"].as<AssetHandle>(AssetHandle(0));
				sc.isTrigger = sphereColliderComponent["IsTrigger"].as<bool>(false);
			}

			YAML::Node capsuleColliderComponent = entity["CapsuleColliderComponent"];
			if (capsuleColliderComponent)
			{
				CapsuleColliderComponent& cc = deserializedEntity.AddComponent<CapsuleColliderComponent>();
				cc.radius = capsuleColliderComponent["Radius"].as<float>(50.0f);
				cc.height = capsuleColliderComponent["Height"].as<float>(200.0f);
				cc.offset = capsuleColliderComponent["Offset"].as<CU::Vector3f>(CU::Vector3f::Zero);
				cc.layerID = capsuleColliderComponent["Layer"].as<uint32_t>(0);
				cc.physicsMaterial = capsuleColliderComponent["PhysicsMaterial"].as<AssetHandle>(AssetHandle(0));
				cc.isTrigger = capsuleColliderComponent["IsTrigger"].as<bool>(false);
			}

			YAML::Node characterControllerComponent = entity["CharacterControllerComponent"];
			if (characterControllerComponent)
			{
				CharacterControllerComponent& cc = deserializedEntity.AddComponent<CharacterControllerComponent>();
				cc.slopeLimit = characterControllerComponent["SlopeLimit"].as<float>();
				cc.stepOffset = characterControllerComponent["StepOffset"].as<float>();
				cc.radius = characterControllerComponent["Radius"].as<float>(50.0f);
				cc.height = characterControllerComponent["Height"].as<float>(200.0f);
				cc.offset = characterControllerComponent["Offset"].as<CU::Vector3f>(CU::Vector3f::Zero);
				cc.layerID = characterControllerComponent["Layer"].as<uint32_t>(0);
			}
		}
	}

	bool SceneSerializer::SerializeToAssetPack(FileStreamWriter& aStream, AssetSerializationInfo& outInfo)
	{
		YAML::Emitter out;
		SerializeToYAML(out);

		outInfo.offset = aStream.GetStreamPosition();
		std::string yamlString = out.c_str();
		aStream.WriteString(yamlString);
		outInfo.size = aStream.GetStreamPosition() - outInfo.offset;
		return outInfo.size > 0;
	}

	bool SceneSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::SceneInfo& aSceneInfo)
	{
		aStream.SetStreamPosition(aSceneInfo.packedOffset);
		std::string sceneYAML;
		aStream.ReadString(sceneYAML);

		return DeserializeFromYAML(sceneYAML);
	}
	
	void SceneSerializer::SerializeToYAML(YAML::Emitter& out)
	{
		LOG_DEBUG("Serializing scene '{}'", myScene->myName);

		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << myScene->myName;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		SerializeEntities(out);

		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	
	bool SceneSerializer::DeserializeFromYAML(const std::string& aYamlString)
	{
		YAML::Node data = YAML::Load(aYamlString);

		if (!data["Scene"])
		{
			return false;
		}

		LOG_DEBUG("Deserializing scene '{}'", myScene->myName);

		YAML::Node entities = data["Entities"];
		if (entities)
		{
			DeserializeEntities(entities);
		}

		return true;
	}
}