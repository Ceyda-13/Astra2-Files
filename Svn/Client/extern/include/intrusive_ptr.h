#pragma once
#include <cstddef>

template <typename T>
void intrusive_ptr_add_ref(T* p)
{
	p->AddRef();
}
 
template <typename T>
void intrusive_ptr_release(T* p)
{
	p->Release();
}

template <typename T>
class intrusive_ptr
{
	protected:
		template <typename U> friend class intrusive_ptr;
		typedef intrusive_ptr<T> this_type;

		T* mpObject;

	public:
		typedef T element_type;
		intrusive_ptr()
			: mpObject(NULL)
		{
			
		}

		intrusive_ptr(T* p, bool bAddRef = true)
			: mpObject(p) 
		{
			if (mpObject && bAddRef)
				intrusive_ptr_add_ref(mpObject);
		} 


		intrusive_ptr(const intrusive_ptr& ip) : mpObject(ip.mpObject) 
		{
			if (mpObject)
				intrusive_ptr_add_ref(mpObject);
		}

		intrusive_ptr(intrusive_ptr&& ip) : mpObject(nullptr)
		{
			swap(ip);
		}

		template <typename U>
		intrusive_ptr(const intrusive_ptr<U>& ip) : mpObject(ip.mpObject) 
		{
			if (mpObject)
				intrusive_ptr_add_ref(mpObject);
		}

		~intrusive_ptr() 
		{
			if (mpObject)
				intrusive_ptr_release(mpObject);
		}


		intrusive_ptr& operator=(const intrusive_ptr& ip)
		{
			return operator=(ip.mpObject);
		}

		intrusive_ptr& operator=(intrusive_ptr&& ip)
		{
			swap(ip);
			return *this;
		}

		template <typename U>
		intrusive_ptr& operator=(const intrusive_ptr<U>& ip)       
		{
			return operator=(ip.mpObject);
		}

		intrusive_ptr& operator=(T* pObject)
		{
			if (pObject != mpObject)
			{
				T* const pTemp = mpObject;
				if (pObject)
					intrusive_ptr_add_ref(pObject);
				mpObject = pObject;
				if (pTemp)
					intrusive_ptr_release(pTemp);
			}
			return *this;
		}

		T& operator *() const 
		{
			return *mpObject;
		}

		T* operator ->() const
		{
			return mpObject;
		}

		T* get() const
		{
			return mpObject;
		}

		void reset() 
		{
			T* const pTemp = mpObject;
			mpObject = NULL;
			if (pTemp)
				intrusive_ptr_release(pTemp);
		}

		void swap(this_type& ip)
		{
			T* const pTemp = mpObject;
			mpObject = ip.mpObject;
			ip.mpObject = pTemp;
		}

		void attach(T* pObject)
		{
			T* const pTemp = mpObject;
			mpObject = pObject;
			if (pTemp)
				intrusive_ptr_release(pTemp);
		}

		T* detach()
		{
			T* const pTemp = mpObject;
			mpObject = NULL;
			return pTemp;
		}

		typedef T* (this_type::*bool_)() const;
		operator bool_() const
		{
			if (mpObject)
				return &this_type::get;
			return NULL;
		}

		bool operator!() const
		{
			return (mpObject == NULL);
		}

};

template <typename T>
inline T* get_pointer(const intrusive_ptr<T>& intrusivePtr)
{
	return intrusivePtr.get();
}

template <typename T>
inline void swap(intrusive_ptr<T>& intrusivePtr1, intrusive_ptr<T>& intrusivePtr2)
{
	intrusivePtr1.swap(intrusivePtr2);
}


template <typename T, typename U>
bool operator==(intrusive_ptr<T> const& iPtr1, intrusive_ptr<U> const& iPtr2)
{
	return (iPtr1.get() == iPtr2.get());
}

template <typename T, typename U>
bool operator!=(intrusive_ptr<T> const& iPtr1, intrusive_ptr<U> const& iPtr2)
{
	return (iPtr1.get() != iPtr2.get());
}

template <typename T>
bool operator==(intrusive_ptr<T> const& iPtr1, T* p)
{
	return (iPtr1.get() == p);
}

template <typename T>
bool operator!=(intrusive_ptr<T> const& iPtr1, T* p)
{
	return (iPtr1.get() != p);
}

template <typename T>
bool operator==(T* p, intrusive_ptr<T> const& iPtr2)
{
	return (p == iPtr2.get());
}

template <typename T>
bool operator!=(T* p, intrusive_ptr<T> const& iPtr2)
{
	return (p != iPtr2.get());
}

template <typename T, typename U>
bool operator<(intrusive_ptr<T> const& iPtr1, intrusive_ptr<U> const& iPtr2)
{
	return ((uintptr_t)iPtr1.get() < (uintptr_t)iPtr2.get());
}

template <class T, class U>
intrusive_ptr<T> static_pointer_cast(const intrusive_ptr<U>& intrusivePtr)
{
	return static_cast<T*>(intrusivePtr.get());
}

template <class T, class U>
intrusive_ptr<T> dynamic_pointer_cast(const intrusive_ptr<U>& intrusivePtr)
{
	return dynamic_cast<T*>(intrusivePtr.get());
}

