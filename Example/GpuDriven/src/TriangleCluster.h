#pragma once
#include <GEngine.h>

#define TRIANGLES_COUNT_PER_CLUSTER 64
#define VERTEX_COUNT_PER_CLUSTER TRIANGLES_COUNT_PER_CLUSTER * 3

using namespace GEngine;

struct InstanceData
{
	Matrix4x4 Model;  // 模型矩阵
};

struct VisiableData
{
	int InstanceIndex; // 指向InstanceBuffer的索引
	int ClusterIndex;  // 指向ClusterData的索引
};

struct ClusterData
{
	int VertexIndices[VERTEX_COUNT_PER_CLUSTER]; // 指向全局顶点缓冲区的索引
};

class TriangleCluster
{
public:
	TriangleCluster() = default;
	~TriangleCluster() = default;


	void AddMesh(Mesh& mesh);

	void SetInstanceData(InstanceData& data, int meshIndex);

	std::vector<InstanceData>& GetInstanceData() { return m_InstanceData; }
	std::vector<std::vector<ClusterData>>& GetClusterData() { return m_ClusterData; }
	std::vector<Vertex>& GetVertexData() { return m_VertexData; }
private:
	std::vector<ClusterData>	ProcessMesh(Mesh& mesh); // 处理网格数据
private:
	std::vector<InstanceData>				m_InstanceData; // 实例数据
	std::vector<std::vector<ClusterData>>	m_ClusterData; // 每个网格的集群数据
	std::vector<Vertex>						m_VertexData; // 全局顶点缓冲区
};

