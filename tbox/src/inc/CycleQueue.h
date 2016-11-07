#ifndef GUARD_CycleQueue_h__
#define GUARD_CycleQueue_h__

template <typename T>
class CycleQueue : public BCMemory
{
private:
	unsigned int m_size;
	int m_front;
	int m_rear;
	T*  m_data;
public:
	CycleQueue(unsigned size)
		:m_size(size),
		m_front(0),
		m_rear(0)
	{
		m_data = ::new T[size];
	}

	~CycleQueue()
	{
		::delete[] m_data;
	}

	bool isEmpty()
	{
		return m_front == m_rear;
	}

	bool isFull()
	{
		return m_front == (m_rear + 1) % m_size;
	}

	bool push(T& ele)
	{
		if (isFull())
		{
			return false;
		}
		m_data[m_rear] = ele;
		m_rear = (m_rear + 1) % m_size;
		return true;
	}

	bool pop(T& t)
	{
		if (isEmpty())
		{
			return false;
		}
		t = m_data[m_front];
		m_front = (m_front + 1) % m_size;
		return true;
	}
	T getDataAt(unsigned int idx) {
		if (idx >= (m_size + m_rear - m_front) % m_size)return NULL;
		return m_data[(idx + m_front) % m_size];
	}
};
#endif // GUARD_CycleQueue_h__
