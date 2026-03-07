#pragma once

struct ResourceBinding_t
{
	void* m_pData;
};

template <typename T>
class CStrongHandle
{
public:
	operator T* () const
	{
		if (m_pBinding == nullptr)
			return nullptr;

		return static_cast<T*>(m_pBinding->m_pData);
	}

	T* operator->() const
	{
		if (m_pBinding == nullptr)
			return nullptr;

		return static_cast<T*>(m_pBinding->m_pData);
	}

	const ResourceBinding_t* m_pBinding;
};
