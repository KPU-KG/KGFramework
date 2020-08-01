#pragma once
#include "ResourceContainer.h"
#include <map>
#include <set>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include "KGShader.h"
#include "KGGeometry.h"
#include "ShaderData.h"
#include "KGGraphicBuffer.h"
namespace KG::Renderer
{
	//���� ������ -> �ѹ��� DrawCall
	//���������� ���� -> ���� �޽� + ���� PSO -> 
	//�������� PSO������ ����
	//���� ->�������̴����� -> ���� ���̴����� ���� ������Ʈ��
	//

	//���۴� ���� 1¥�� ������ / 5¥�� ������ / 10¥�� ������ / 50¥�� ������ / 100¥�� ������ �ؼ� �̸� �Ҵ��� ���� 
	//������Ʈ ����� �°� �ݳ� / �뿩 �ϱ�


	//�갡 �ؾ��Ұ�
	// ������Ʈ�κ��� ���� �޾Ƽ� �ν��Ͻ� ����
	// �׷��� ���� �ؾ��Ұ�
	// GPU ���� ����
	// 
	struct KGRenderJob
	{
		IShader* shader;
		Geometry* geometry;
		int objectSize = 0;
		int visibleSize = 0;
		int updateCount = 0;
		PooledBuffer<ObjectData>* objectBuffer;

		//static �Ҷ� ó�� �߰� �ʿ� // �� ������ ���ε��� �ʿ䰡 ����.
		void OnObjectAdd(bool isVisible)
		{
			this->objectSize += 1;
			if (isVisible) OnVisibleAdd();
		}
		void OnObjectRemove(bool isVisible)
		{
			this->objectSize -= 1;
			if (isVisible) OnVisibleRemove();
		}
		void OnVisibleAdd()
		{
			this->visibleSize += 1;
		}
		void OnVisibleRemove()
		{
			this->visibleSize -= 1;
		}
		int GetUpdateCount()
		{
			auto result = this->updateCount;
			this->updateCount++;
			return result;
		}
		void ClearCount()
		{
			this->updateCount = 0;
		}

		void Render(ID3D12GraphicsCommandList* cmdList, IShader*& prevShader)
		{
			if (prevShader != this->shader)
			{
				prevShader = this->shader;
				this->shader->Set(cmdList);
				cmdList->SetGraphicsRootShaderResourceView(0,
					this->objectBuffer->buffer.resource->GetGPUVirtualAddress()
				);
				this->geometry->Render(cmdList, this->visibleSize);
			}
		}

		static bool ShaderCompare(const KGRenderJob& a, const KGRenderJob& b)
		{
			return a.shader < b.shader;
		}

		static bool GeometryCompare(const KGRenderJob& a, const KGRenderJob& b)
		{
			return a.geometry < b.geometry;
		}
	};

	class KGRenderEngine
	{
		IShader* currentShader = nullptr;
		Geometry* currentGeometry = nullptr;
		ResourceContainer container;
		bool renderJobsDirty = false;
		std::vector<KGRenderJob> renderJobs; // �ϴ� ���� �˰����� ���� / �ѹ� ������ �����̸� 0�� ����� �ٽ� ���� ���ɼ��� �ִٰ� ����
	public:
		void CreateRenderJob(const KGRenderJob& job)
		{
			this->renderJobsDirty = true;
			this->renderJobs.push_back(job);
		}
		KGRenderJob* FindRenderJob(IShader* shader, Geometry* geometry)
		{
			KGRenderJob target;
			target.shader = shader;
			target.geometry = geometry;
			auto [start, end] = std::equal_range(renderJobs.begin(), renderJobs.end(), target, KGRenderJob::ShaderCompare);
			return &*std::find_if(start, end, [=](const KGRenderJob& job) { return job.geometry == geometry; });
		}

		void Render(ID3D12GraphicsCommandList* cmdList)
		{
			for (KGRenderJob& job : this->renderJobs)
			{
				job.Render(cmdList, this->currentShader);
			}
		}

		void ClearJobs()
		{
			this->renderJobs.clear();
		}

		void SortJobs()
		{
			//std::sort(renderJobs.begin(), renderJobs.end(), KGRenderJob::GeometryCompare);
			std::sort(renderJobs.begin(), renderJobs.end(), KGRenderJob::ShaderCompare);
		}
	};
}
