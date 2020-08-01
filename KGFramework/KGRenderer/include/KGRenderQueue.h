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
	//대충 렌더잡 -> 한번의 DrawCall
	//렌더잡으로 구분 -> 같은 메쉬 + 같은 PSO -> 
	//렌더잡은 PSO순으로 구분
	//대충 ->같은쉐이더끼리 -> 같은 쉐이더에서 같은 지오메트리
	//

	//버퍼는 대충 1짜리 여러개 / 5짜리 여러개 / 10짜리 여러개 / 50짜리 여러개 / 100짜리 여러개 해서 미리 할당한 다음 
	//오브젝트 사이즈에 맞게 반납 / 대여 하기


	//얘가 해야할거
	// 컴포넌트로부터 정보 받아서 인스턴싱 엮기
	// 그러기 위해 해야할거
	// GPU 버퍼 관리
	// 
	struct KGRenderJob
	{
		IShader* shader;
		Geometry* geometry;
		int objectSize = 0;
		int visibleSize = 0;
		int updateCount = 0;
		PooledBuffer<ObjectData>* objectBuffer;

		//static 할때 처리 추가 필요 // 매 프레임 업로드할 필요가 없다.
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
		std::vector<KGRenderJob> renderJobs; // 일단 제거 알고리즘은 없음 / 한번 생성된 조합이면 0이 됬더라도 다시 생길 가능성이 있다고 생각
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
