#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

template <typename _Type_>
class RingBuffer
{
public:
	RingBuffer(uint32_t capacity);
	RingBuffer(uint32_t capacity, const _Type_& fill);
	RingBuffer(const RingBuffer&) = delete;
	RingBuffer(RingBuffer&&) = delete;
	~RingBuffer();

	RingBuffer& operator=(const RingBuffer&) = delete;
	RingBuffer& operator=(RingBuffer&&) = delete;

	void     Reserve(uint32_t capacity);
	uint32_t GetCapacity() const;

	void PushBack(const _Type_& element);
	void PushBack(_Type_&& element);
	void PopFront();

	uint32_t GetSize() const;
	bool     IsFull() const;

	void Fill(const _Type_& element);

	_Type_*  GetRawData() const;
	uint32_t GetRawIndex(uint32_t index) const;

	uint32_t GetFirstSegmentSize() const;
	_Type_*  GetFirstSegmentData() const;

	uint32_t GetSecondSegmentSize() const;
	_Type_*  GetSecondSegmentData() const;

private:
	uint32_t _capacity = 0;
	uint32_t _size = 0;
	uint32_t _head = 0;

	_Type_* _data = nullptr;
};

template <typename _Type_>
RingBuffer<_Type_>::RingBuffer(uint32_t capacity)
{
	Reserve(capacity);
}

template <typename _Type_>
RingBuffer<_Type_>::RingBuffer(uint32_t capacity, const _Type_& fill)
{
	Reserve(capacity);
	Fill(fill);
}

template <typename _Type_>
RingBuffer<_Type_>::~RingBuffer()
{
	if (_data != nullptr)
	{
		if constexpr (!std::is_fundamental_v<_Type_>)
		{
			for (uint32_t i = 0; i < _size; ++i)
			{
				_data[GetRawIndex(i)].~_Type_();
			}
		}

		free(_data);
	}
}

template <typename _Type_>
inline uint32_t RingBuffer<_Type_>::GetRawIndex(uint32_t index) const
{
	return (_head + index) % _capacity;
}

template <typename _Type_>
inline uint32_t RingBuffer<_Type_>::GetCapacity() const
{
	return _capacity;
}

template <typename _Type_>
inline uint32_t RingBuffer<_Type_>::GetSize() const
{
	return _size;
}

template <typename _Type_>
inline bool RingBuffer<_Type_>::IsFull() const
{
	return _size == _capacity;
}

template <typename _Type_>
void RingBuffer<_Type_>::Reserve(uint32_t capacity)
{
	if (_capacity < capacity)
	{
		_Type_* newData = (_Type_*)malloc(sizeof(_Type_) * capacity);

		if (_data != nullptr)
		{
			if (_size > 0)
			{
				if constexpr (std::is_fundamental_v<_Type_>)
				{
					uint32_t firstSegmentSize = _size - _head;
					std::memcpy(newData, _data + _head, firstSegmentSize * sizeof(_Type_));
					if (_head != 0)
					{
						uint32_t secondSegmentSize = _size - firstSegmentSize;
						std::memcpy(newData + firstSegmentSize, _data, secondSegmentSize * sizeof(_Type_));
					}
				}
				else
				{
					for (uint32_t i = 0; i < _size; ++i)
					{
						uint32_t rawIndex = GetRawIndex(i);

						if constexpr (std::is_move_constructible_v<_Type_>)
						{
							new (newData + i) _Type_(static_cast<_Type_&&>(_data[rawIndex]));
						}
						else
						{
							new (newData + i) _Type_(_data[rawIndex]);
						}

						_data[rawIndex].~_Type_();
					}
				}
			}

			free(_data);
		}

		_data = newData;
		_capacity = capacity;
	}
}

template <typename _Type_>
void RingBuffer<_Type_>::PushBack(const _Type_& element)
{
	if (IsFull())
	{
		_data[_head].~_Type_();
	}
	else
	{
		++_size;
	}
	new (_data + _head) _Type_(element);
	_head = (_head + 1) % _capacity;
}

template <typename _Type_>
void RingBuffer<_Type_>::PushBack(_Type_&& element)
{
	if (IsFull())
	{
		_data[_head].~_Type_();
	}
	else
	{
		++_size;
	}
	new (_data + _head) _Type_(std::move(element));
	_head = (_head + 1) % _capacity;
}

template <typename _Type_>
void RingBuffer<_Type_>::PopFront()
{
	if (_size > 0)
	{
		_data[_head].~_Type_();
		if (_head == 0)
		{
			_head = _capacity - 1;
		}
		else
		{
			--_head;
		}
		--_size;
	}
}

template <typename _Type_>
inline _Type_* RingBuffer<_Type_>::GetRawData() const
{
	return _data;
}

template <typename _Type_>
inline uint32_t RingBuffer<_Type_>::GetFirstSegmentSize() const
{
	return _size - _head;
}

template <typename _Type_>
inline _Type_* RingBuffer<_Type_>::GetFirstSegmentData() const
{
	return _data + _head;
}

template <typename _Type_>
inline uint32_t RingBuffer<_Type_>::GetSecondSegmentSize() const
{
	if (_head == 0)
	{
		return 0;
	}
	else
	{
		return _size - GetFirstSegmentSize();
	}
}

template <typename _Type_>
inline _Type_* RingBuffer<_Type_>::GetSecondSegmentData() const
{
	if (_head == 0)
	{
		return nullptr;
	}
	else
	{
		return _data;
	}
}

template <typename _Type_>
void RingBuffer<_Type_>::Fill(const _Type_& element)
{
	for (uint32_t index = 0; index < _capacity; ++index)
	{
		PushBack(element);
	}
}
