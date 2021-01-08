#pragma once
#include <functional>
namespace KG::Core
{
	//Delete 쪽 변경 필요
	template<typename Ty>
	class IHierarchy
	{
		using ThisType = Ty;
	protected:
		ThisType* parent = nullptr;
		ThisType* child = nullptr;
		ThisType* sibiling = nullptr;

		ThisType* sibilingFirst = nullptr;

	public:
		bool isEraseSubNode = false;
		virtual void OnDestroy()
		{

		}
		void FunctionChild(const std::function<void(ThisType*)>& func)
		{
			if (this->child != nullptr)
			{
				this->child->FunctionSibiling(func);
				this->child->FunctionChild(func);
			}
			func(static_cast<ThisType*>(this));
		}
		void FunctionSibiling(const std::function<void(ThisType*)>& func)
		{
			if (this->sibiling != nullptr)
			{
				this->sibiling->FunctionChild(func);
				this->sibiling->FunctionSibiling(func);
			}
			func(static_cast<ThisType*>(this));
		}
		void SetParent(ThisType* parent)
		{
			this->parent = parent;
		}
		ThisType* GetParent() const
		{
			return this->parent;
		}
		ThisType* GetNextsibiling() const
		{
			return this->sibiling;
		}
		ThisType* GetChild() const
		{
			return this->child;
		}
		void AddChild(ThisType* obj)
		{
			obj->SetParent((Ty*)this);
			if (this->hasChild())
			{
				this->child->AddSibiling(obj);
			}
			else
			{
				this->child = obj;
			}
		}
		void AddSibiling(ThisType* obj)
		{
			IHierarchy<Ty>* csr = this;
			obj->SetParent( (Ty*)this->parent );
			while (true)
			{
				if (csr->hasSibiling())
				{
					csr = csr->sibiling;
				}
				else
				{
					csr->sibiling = obj;
					break;
				}
			}
		}

		bool hasSibiling()
		{
			return this->sibiling != nullptr;
		}

		bool hasChild()
		{
			return this->child != nullptr;
		}

		void DeleteAllChild()
		{
			if (isEraseSubNode) { return; }
			else { this->isEraseSubNode = true; }
			if (this->child != nullptr)
			{
				this->child->DeleteAllChildSibilling();
			}
			this->child = nullptr;
		}

		void DeleteAllChildSibilling()
		{
			if (isEraseSubNode) { return; }
			else { this->isEraseSubNode = true; }
			if (this->child != nullptr)
			{
				this->child->DeleteAllChildSibilling();
			}
			if (this->sibiling != nullptr)
			{
				this->sibiling->DeleteAllChildSibilling();
			}
			this->child = nullptr;
			this->sibiling = nullptr;
		}

		void DeleteChild(ThisType* ptr)
		{

			if (this->child == ptr)
			{
				this->child->DeleteAllChild();
				this->child = this->child->GetNextsibiling();
			}
			else
			{
				if (this->hasChild() && this->child->hasSibiling())
				{
					this->child->DeleteSibiling(ptr);
				}
			}
		}

		void DeleteSibiling(ThisType* ptr)
		{
			if (this->sibiling == ptr)
			{
				this->sibiling->DeleteAllChild();
				this->sibiling = this->sibiling->GetNextsibiling();
			}
			else
			{
				if (this->hasSibiling())
				{
					this->GetNextsibiling()->DeleteSibiling(ptr);
				}
			}

		}

	};
};