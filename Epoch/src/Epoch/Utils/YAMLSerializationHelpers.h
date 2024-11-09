#pragma once
#pragma warning(push, 0)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)
#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Gradient.h>
#include "Epoch/Core/UUID.h"

namespace YAML
{
	template<>
	struct convert<Epoch::UUID>
	{
		static Node encode(const Epoch::UUID& aRhs)
		{
			Node node;
			node.push_back((uint64_t)aRhs);
			return node;
		}

		static bool decode(const Node& node, Epoch::UUID& aRhs)
		{
			aRhs = (Epoch::UUID)node.as<uint64_t>(0);
			return true;
		}
	};

	template<>
	struct convert<CU::Vector2f>
	{
		static Node encode(const CU::Vector2f& aRhs)
		{
			Node node;
			node.push_back(aRhs.x);
			node.push_back(aRhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CU::Vector2f& aRhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			aRhs.x = node[0].as<float>();
			aRhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<CU::Vector3f>
	{
		static Node encode(const CU::Vector3f& aRhs)
		{
			Node node;
			node.push_back(aRhs.x);
			node.push_back(aRhs.y);
			node.push_back(aRhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CU::Vector3f& aRhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			aRhs.x = node[0].as<float>();
			aRhs.y = node[1].as<float>();
			aRhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<CU::Vector4f>
	{
		static Node encode(const CU::Vector4f& aRhs)
		{
			Node node;
			node.push_back(aRhs.x);
			node.push_back(aRhs.y);
			node.push_back(aRhs.z);
			node.push_back(aRhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& aNode, CU::Vector4f& aRhs)
		{
			if (!aNode.IsSequence() || aNode.size() != 4)
				return false;

			aRhs.x = aNode[0].as<float>();
			aRhs.y = aNode[1].as<float>();
			aRhs.z = aNode[2].as<float>();
			aRhs.w = aNode[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<CU::Gradient>
	{
		static bool decode(const Node& node, CU::Gradient& aGradient)
		{
			if (!node.IsMap())
				return false;

			aGradient = CU::Gradient(false);

			YAML::Node colorKeys = node["ColorKeys"];
			if (colorKeys)
			{
				for (auto key : colorKeys)
				{
					CU::Color color = key["Color"].as<CU::Vector4f>(CU::Color::White.GetVector4());
					float time = key["Time"].as<float>(0.0f);
					aGradient.AddColorKey(time, color);
				}
			}

			YAML::Node alphaKeys = node["AlphaKeys"];
			if (alphaKeys)
			{
				for (auto key : alphaKeys)
				{
					float alpha = key["Alpha"].as<float>(1.0f);
					float time = key["Time"].as<float>(0.0f);
					aGradient.AddAlphaKey(time, alpha);
				}
			}

			return true;
		}
	};
}

namespace Epoch
{
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Vector2f& aVector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aVector.x << aVector.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Vector3f& aVector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aVector.x << aVector.y << aVector.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Vector4f& aVector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aVector.x << aVector.y << aVector.z << aVector.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Gradient& aGradient)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "ColorKeys" << YAML::Value << YAML::BeginSeq;
		for (const std::shared_ptr<CU::Gradient::Key> key : aGradient.colorKeys)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Color" << YAML::Value << key->color.GetVector4();
			out << YAML::Key << "Time" << YAML::Value << key->time;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "AlphaKeys" << YAML::Value << YAML::BeginSeq;
		for (const std::shared_ptr<CU::Gradient::Key> key : aGradient.alphaKeys)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Alpha" << YAML::Value << key->alpha;
			out << YAML::Key << "Time" << YAML::Value << key->time;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;
		return out;
	}
}
