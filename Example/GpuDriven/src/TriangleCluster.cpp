#include "TriangleCluster.h"

void TriangleCluster::AddMesh(Mesh& mesh)
{
	m_InstanceData.push_back(InstanceData());

	auto clusters = ProcessMesh(mesh);

	m_ClusterData.push_back(clusters); // 将集群数据添加到集群数据列表中

	for (int i = 0; i < mesh.m_Vertices.size(); i++)
	{
		m_VertexData.push_back(mesh.m_Vertices[i]);
	}
}

void TriangleCluster::SetInstanceData(InstanceData& data, int meshIndex)
{
	m_InstanceData.at(meshIndex) = data; // 设置实例数据
}

std::vector<ClusterData> TriangleCluster::ProcessMesh(Mesh& mesh)
{
	std::vector < ClusterData> clusters;

	int vtsCount = 0;
	int vtsIndexOffset = m_VertexData.size(); // 顶点索引偏移量

	ClusterData cluster;

	for (int i = 0; i < mesh.m_Indices.size(); i++)
	{
		if (vtsCount >= VERTEX_COUNT_PER_CLUSTER)
		{
			clusters.push_back(cluster);
			cluster = ClusterData();
			vtsCount = 0;
		}

		cluster.VertexIndices[vtsCount] = mesh.m_Indices[i] + vtsIndexOffset; // 将索引添加到集群数据中

		vtsCount++;
	}

	if (vtsCount > 0 && vtsCount < VERTEX_COUNT_PER_CLUSTER)
	{
		while (vtsCount < VERTEX_COUNT_PER_CLUSTER)
		{
			cluster.VertexIndices[vtsCount] = -1; // 填充剩余的索引
			vtsCount++;
		}

		clusters.push_back(cluster); // 添加最后一个集群
	}

	return clusters;
}
