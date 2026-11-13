#include "MeshLoader.h"

#include <charconv>
#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace DirectX;

namespace
{
	struct VertexKey
	{
		int pos = -1;
		int uv = -1;
		int normal = -1;
		bool operator==(const VertexKey&) const = default;
	};

	struct VertexKeyHash
	{
		size_t operator()(const VertexKey& key) const
		{
			size_t h = std::hash<int>{}(key.pos);
			h ^= std::hash<int>{}(key.uv) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>{}(key.normal) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};

	bool IsSpace(char c)
	{
		return c == ' ' || c == '\t' || c == '\r';
	}

	std::string_view NextToken(std::string_view& s)
	{
		while (!s.empty() && IsSpace(s.front()))
			s.remove_prefix(1);

		size_t len = 0;
		while (len < s.size() && !IsSpace(s[len]))
			++len;

		const std::string_view token = s.substr(0, len);
		s.remove_prefix(len);
		return token;
	}

	float ParseFloat(std::string_view& s)
	{
		const std::string_view token = NextToken(s);
		float value = 0.0f;
		std::from_chars(token.data(), token.data() + token.size(), value);
		return value;
	}

	// 1-based, 음수는 끝 기준 -> 0-based, 없으면 -1
	int ResolveIndex(int index, size_t count)
	{
		const int n = static_cast<int>(count);
		const int resolved = index > 0 ? index - 1 : n + index;
		return (index != 0 && resolved >= 0 && resolved < n) ? resolved : -1;
	}

	// v, v/vt, v//vn, v/vt/vn
	VertexKey ParseFaceVertex(std::string_view token, size_t posCount, size_t uvCount, size_t normalCount)
	{
		int raw[3] = {};
		for (int& value : raw)
		{
			const size_t slash = token.find('/');
			const std::string_view part = token.substr(0, slash);
			std::from_chars(part.data(), part.data() + part.size(), value);
			if (slash == std::string_view::npos)
				break;
			token.remove_prefix(slash + 1);
		}
		return { ResolveIndex(raw[0], posCount), ResolveIndex(raw[1], uvCount), ResolveIndex(raw[2], normalCount) };
	}

} // namespace

bool LoadObj(const std::filesystem::path& file, MeshData& out)
{
	std::ifstream in(file);
	if (!in)
	{
		std::fprintf(stderr, "Mesh load failed: %ls\n", file.c_str());
		return false;
	}

	out.vertices.clear();
	out.indices.clear();

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> uvs;
	std::vector<VertexKey> keys;
	std::unordered_map<VertexKey, uint32_t, VertexKeyHash> lookup;
	std::vector<uint32_t> polygon;
	bool missingNormals = false;

	std::string line;
	for (int lineNumber = 1; std::getline(in, line); ++lineNumber)
	{
		std::string_view s = line;
		const std::string_view tag = NextToken(s);

		// 오른손 -> 왼손 좌표계: z 반전
		if (tag == "v")
		{
			const float x = ParseFloat(s);
			const float y = ParseFloat(s);
			const float z = ParseFloat(s);
			positions.push_back({ x, y, -z });
		}
		else if (tag == "vn")
		{
			const float x = ParseFloat(s);
			const float y = ParseFloat(s);
			const float z = ParseFloat(s);
			normals.push_back({ x, y, -z });
		}
		else if (tag == "vt")
		{
			const float u = ParseFloat(s);
			const float v = ParseFloat(s);
			uvs.push_back({ u, 1.0f - v });
		}
		else if (tag == "f")
		{
			polygon.clear();
			for (std::string_view token = NextToken(s); !token.empty(); token = NextToken(s))
			{
				const VertexKey key = ParseFaceVertex(token, positions.size(), uvs.size(), normals.size());
				if (key.pos < 0)
				{
					std::fprintf(stderr, "Mesh load failed: %ls(%d): invalid face index\n", file.c_str(), lineNumber);
					return false;
				}

				const auto [it, inserted] = lookup.try_emplace(key, static_cast<uint32_t>(out.vertices.size()));
				if (inserted)
				{
					Vertex vertex{};
					vertex.pos = positions[key.pos];
					if (key.uv >= 0)
						vertex.uv = uvs[key.uv];
					if (key.normal >= 0)
						vertex.normal = normals[key.normal];
					else
						missingNormals = true;
					out.vertices.push_back(vertex);
					keys.push_back(key);
				}
				polygon.push_back(it->second);
			}

			// fan 삼각화, winding 반전 (CCW -> CW)
			for (size_t i = 2; i < polygon.size(); ++i)
			{
				out.indices.push_back(polygon[0]);
				out.indices.push_back(polygon[i]);
				out.indices.push_back(polygon[i - 1]);
			}
		}
	}

	if (out.indices.empty())
	{
		std::fprintf(stderr, "Mesh load failed: %ls: no faces\n", file.c_str());
		return false;
	}

	// vn 없는 정점: 면 법선들의 평균으로 정점 법선 생성
	if (missingNormals)
	{
		std::vector<XMFLOAT3> accumulated(positions.size(), XMFLOAT3{ 0.0f, 0.0f, 0.0f });
		for (size_t i = 0; i + 2 < out.indices.size(); i += 3)
		{
			const int ids[3] = { keys[out.indices[i]].pos, keys[out.indices[i + 1]].pos, keys[out.indices[i + 2]].pos };
			const XMVECTOR p0 = XMLoadFloat3(&positions[ids[0]]);
			const XMVECTOR p1 = XMLoadFloat3(&positions[ids[1]]);
			const XMVECTOR p2 = XMLoadFloat3(&positions[ids[2]]);
			const XMVECTOR faceNormal = XMVector3Cross(p1 - p0, p2 - p0);
			for (const int id : ids)
				XMStoreFloat3(&accumulated[id], XMLoadFloat3(&accumulated[id]) + faceNormal);
		}

		for (size_t i = 0; i < out.vertices.size(); ++i)
		{
			if (keys[i].normal < 0)
				XMStoreFloat3(&out.vertices[i].normal, XMVector3Normalize(XMLoadFloat3(&accumulated[keys[i].pos])));
		}
	}

	return true;
}
