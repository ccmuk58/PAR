#include "Mesh.h"

#include "Vertex.h"
#include "D3DUtil.h"
#include "Device.h"
namespace
{
	const Vertex cubeVertices[] = {
	{ { -0.5f,-0.5f,-0.5f },{ 0,0,0 } }, // 0
	{ { -0.5f, 0.5f,-0.5f },{ 0,1,0 } }, // 1
	{ { 0.5f, 0.5f,-0.5f },{ 1,1,0 } }, // 2
	{ { 0.5f,-0.5f,-0.5f },{ 1,0,0 } }, // 3
	{ { -0.5f,-0.5f, 0.5f },{ 0,0,1 } }, // 4
	{ { -0.5f, 0.5f, 0.5f },{ 0,1,1 } }, // 5
	{ { 0.5f, 0.5f, 0.5f },{ 1,1,1 } }, // 6
	{ { 0.5f,-0.5f, 0.5f },{ 1,0,1 } }, // 7
	};
	const uint32_t cubeIndices[] = {
		0,1,2, 0,2,3,   // 앞 (-Z)
		7,6,5, 7,5,4,   // 뒤 (+Z)
		4,5,1, 4,1,0,   // 왼 (-X)
		3,2,6, 3,6,7,   // 오른 (+X)
		1,5,6, 1,6,2,   // 위 (+Y)
		3,7,4, 3,4,0,   // 아래 (-Y)
	};

} // namespace

bool Mesh::Init(Device& device, std::span<const Vertex> vertices, std::span<const uint32_t> indices)
{
	ID3D11Device* dev = device.Get();
	// vertex buffer 생성
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth =  static_cast<UINT>(vertices.size_bytes());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA init{};
	init.pSysMem = vertices.data();
	if (!Check(dev->CreateBuffer(&bd, &init, &m_vertexBuffer), "CreateBuffer(vertex)"))
		return false;

	// index buffer 생성
	D3D11_BUFFER_DESC ibd{};
	ibd.ByteWidth = static_cast<UINT>(indices.size_bytes());
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA iinit{};
	iinit.pSysMem = indices.data();
	if (!Check(dev->CreateBuffer(&ibd, &iinit, &m_indexBuffer), "CreateBuffer(index)"))
		return false;

	m_indexCount = static_cast<UINT>(indices.size());
	return true;
}

bool Mesh::InitCube(Device& device)
{
	return Init(device, cubeVertices, cubeIndices);
}

void Mesh::Bind(ID3D11DeviceContext* ctx) const
{
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
