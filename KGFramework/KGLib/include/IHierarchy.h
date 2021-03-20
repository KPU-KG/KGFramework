#pragma once
#include <functional>
namespace KG::Core
{
	class BaseHierarchy
	{
	private:
		BaseHierarchy* parent = nullptr;
		BaseHierarchy* child = nullptr;
		BaseHierarchy* nextSibling = nullptr;
		BaseHierarchy* prevSibling = nullptr;

	protected:
		template<typename Ty>
		void FunctionChild(const std::function<void(Ty*)>& func)
		{
			if ( this->child != nullptr )
			{
				this->child->FunctionSibiling(func);
				this->child->FunctionChild(func);
			}
			func(static_cast<Ty*>(this));
		}

		template<typename Ty>
		void FunctionSibiling(const std::function<void(Ty*)>& func)
		{
			if ( this->nextSibling != nullptr )
			{
				this->nextSibling->FunctionChild(func);
				this->nextSibling->FunctionSibiling(func);
			}
			func(static_cast<Ty*>(this));
		}

		void SetParent(BaseHierarchy* parent)
		{
			this->parent = parent;
		}

		BaseHierarchy* GetParent() const
		{
			return this->parent;
		}

		BaseHierarchy* GetPrevsibiling() const
		{
			return this->nextSibling;
		}

		BaseHierarchy* GetNextsibiling() const
		{
			return this->nextSibling;
		}
		BaseHierarchy* GetChild() const
		{
			return this->child;
		}
		void AddChild(BaseHierarchy* obj)
		{
			obj->SetParent(this);
			if ( this->hasChild() )
			{
				this->child->AddSibiling(obj);
			}
			else
			{
				this->child = obj;
			}
		}
		void AddSibiling(BaseHierarchy* obj)
		{
			obj->SetParent(this->parent);
			BaseHierarchy* csr = this;
			while ( true )
			{
				if ( csr->hasNextSibiling() )
				{
					csr = csr->nextSibling;
				}
				else
				{
					csr->nextSibling = obj;
					obj->SetPrevSibiling(csr);
					break;
				}
			}
		}

		void SetPrevSibiling(BaseHierarchy* obj)
		{
			this->prevSibling = obj;
			if( obj )
				obj->nextSibling = this;
		}

		void SetNextSibiling(BaseHierarchy* obj)
		{
			this->nextSibling = obj;
			if( obj )
				obj->prevSibling = this;
		}
		void SetChild(BaseHierarchy* obj)
		{
			this->child = obj;
		}

	public:
		bool hasSibiling() const
		{
			return this->nextSibling != nullptr || this->prevSibling != nullptr;
		}

		bool hasNextSibiling() const
		{
			return this->nextSibling != nullptr;
		}

		bool hasChild() const
		{
			return this->child != nullptr;
		}

		void ExtractThisNode()
		{
			if ( this->parent && this->parent->GetChild() == (BaseHierarchy*)this )
			{
				this->parent->SetChild(this->nextSibling);
			}
			else if ( this->prevSibling )
			{
				this->prevSibling->SetNextSibiling(this->nextSibling);
			}
			this->parent = nullptr;
			this->nextSibling = nullptr;
			this->prevSibling = nullptr;
		}

	};

	template<typename ChildTy>
	class IHierarchy : public BaseHierarchy
	{
	public:
		void FunctionChild(const std::function<void(ChildTy*)>& func)
		{
			BaseHierarchy::FunctionChild<ChildTy>(func);
		}

		template<typename ChildTy>
		void FunctionSibiling(const std::function<void(ChildTy*)>& func)
		{
			BaseHierarchy::FunctionSibiling<ChildTy>(func);
		}

		void SetParent(ChildTy* parent)
		{
			BaseHierarchy::SetParent(static_cast<BaseHierarchy*>(parent));
		}

		ChildTy* GetParent() const
		{
			return static_cast<ChildTy*>(BaseHierarchy::GetParent());
		}

		ChildTy* GetPrevsibiling() const
		{
			return static_cast<ChildTy*>(BaseHierarchy::GetPrevsibiling());
		}

		ChildTy* GetNextsibiling() const
		{
			return static_cast<ChildTy*>(BaseHierarchy::GetNextsibiling());
		}
		ChildTy* GetChild() const
		{
			return static_cast<ChildTy*>(BaseHierarchy::GetChild());
		}
		void AddChild(ChildTy* obj)
		{
			BaseHierarchy::AddChild(static_cast<BaseHierarchy*>(obj));
		}
		void AddSibiling(ChildTy* obj)
		{
			BaseHierarchy::AddSibiling(static_cast<BaseHierarchy*>(obj));
		}

		void SetPrevSibiling(ChildTy* obj)
		{
			BaseHierarchy::SetPrevSibiling(static_cast<BaseHierarchy*>(obj));
		}

		void SetNextSibiling(ChildTy* obj)
		{
			BaseHierarchy::SetNextSibiling(static_cast<BaseHierarchy*>(obj));
		}

		void SetChild(ChildTy* obj)
		{
			BaseHierarchy::SetChild(static_cast<BaseHierarchy*>(obj));
		}
	};


	//template<typename Ty>
	//class IHierarchy
	//{
	//	using CurrentType = IHierarchy<Ty>;
	//	using ThisType = Ty;
	//protected:
	//	ThisType* parent = nullptr;
	//	ThisType* child = nullptr;
	//	ThisType* sibiling = nullptr;


	//	ThisType* sibilingPrev = nullptr;

	//public:
	//	bool isEraseSubNode = false;
	//	virtual void OnDestroy()
	//	{

	//	}
	//	void FunctionChild(const std::function<void(ThisType*)>& func)
	//	{
	//		if ( this->child != nullptr )
	//		{
	//			this->child->FunctionSibiling(func);
	//			this->child->FunctionChild(func);
	//		}
	//		func(static_cast<ThisType*>(this));
	//	}
	//	void FunctionSibiling(const std::function<void(ThisType*)>& func)
	//	{
	//		if ( this->sibiling != nullptr )
	//		{
	//			this->sibiling->FunctionChild(func);
	//			this->sibiling->FunctionSibiling(func);
	//		}
	//		func(static_cast<ThisType*>(this));
	//	}
	//	void SetParent(ThisType* parent)
	//	{
	//		this->parent = parent;
	//	}
	//	ThisType* GetParent() const
	//	{
	//		return this->parent;
	//	}

	//	ThisType* GetPrevsibiling() const
	//	{
	//		return this->sibiling;
	//	}

	//	ThisType* GetNextsibiling() const
	//	{
	//		return this->sibiling;
	//	}
	//	ThisType* GetChild() const
	//	{
	//		return this->child;
	//	}
	//	void AddChild(ThisType* obj)
	//	{
	//		obj->SetParent((ThisType*)this);
	//		if ( this->hasChild() )
	//		{
	//			this->child->AddSibiling(obj);
	//		}
	//		else
	//		{
	//			this->child = obj;
	//		}
	//	}
	//	void AddSibiling(ThisType* obj)
	//	{
	//		CurrentType* csr = this;
	//		obj->SetParent((ThisType*)this->parent);
	//		while ( true )
	//		{
	//			if ( csr->hasSibiling() )
	//			{
	//				csr = csr->sibiling;
	//			}
	//			else
	//			{
	//				csr->sibiling = obj;
	//				obj->SetPrevSibiling((ThisType*)csr);
	//				break;
	//			}
	//		}
	//	}

	//	void SetPrevSibiling(ThisType* obj)
	//	{
	//		this->sibilingPrev = obj;
	//	}

	//	void SetNextSibiling(ThisType* obj)
	//	{
	//		this->sibiling = obj;
	//	}
	//	void SetChild(ThisType* obj)
	//	{
	//		this->child = obj;
	//	}

	//	bool hasSibiling()
	//	{
	//		return this->sibiling != nullptr;
	//	}

	//	bool hasChild()
	//	{
	//		return this->child != nullptr;
	//	}

	//	void DeleteAllChild()
	//	{
	//		if ( isEraseSubNode )
	//		{
	//			return;
	//		}
	//		else
	//		{
	//			this->isEraseSubNode = true;
	//		}
	//		if ( this->child != nullptr )
	//		{
	//			this->child->DeleteAllChildSibilling();
	//		}
	//		this->child = nullptr;
	//	}

	//	void DeleteAllChildSibilling()
	//	{
	//		if ( isEraseSubNode )
	//		{
	//			return;
	//		}
	//		else
	//		{
	//			this->isEraseSubNode = true;
	//		}
	//		if ( this->child != nullptr )
	//		{
	//			this->child->DeleteAllChildSibilling();
	//		}
	//		if ( this->sibiling != nullptr )
	//		{
	//			this->sibiling->DeleteAllChildSibilling();
	//		}
	//		this->child = nullptr;
	//		this->sibiling = nullptr;
	//	}

	//	void DeleteChild(ThisType* ptr)
	//	{

	//		if ( this->child == ptr )
	//		{
	//			this->child->DeleteAllChild();
	//			this->child = this->child->GetNextsibiling();
	//		}
	//		else
	//		{
	//			if ( this->hasChild() && this->child->hasSibiling() )
	//			{
	//				this->child->DeleteSibiling(ptr);
	//			}
	//		}
	//	}

	//	void DeleteSibiling(ThisType* ptr)
	//	{
	//		if ( this->sibiling == ptr )
	//		{
	//			this->sibiling->DeleteAllChild();
	//			this->sibiling = this->sibiling->GetNextsibiling();
	//		}
	//		else
	//		{
	//			if ( this->hasSibiling() )
	//			{
	//				this->GetNextsibiling()->DeleteSibiling(ptr);
	//			}
	//		}

	//	}

	//	void ExtractThisNode()
	//	{
	//		if ( this->parent && this->parent->GetChild() == (ThisType*)this )
	//		{
	//			this->parent->SetChild(this->sibiling);
	//		}
	//		else if ( this->sibilingPrev )
	//		{
	//			this->sibilingPrev->SetNextSibiling(this->sibiling);
	//		}
	//		this->parent = nullptr;
	//		this->sibiling = nullptr;
	//	}

	//};
};