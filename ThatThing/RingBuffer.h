#pragma once
#include <vector>
#include <algorithm>


template<typename T>
class RingBuffer
{
private:
	std::vector<T> _vec;
	typename std::vector<T>::iterator _lastwright;
	std::size_t _size;//vector does not guarantee, that reserve will reserve exactly size elements. So we have to keep that number by ourselfs
public:
	RingBuffer(std::size_t size) : _vec.reserve(size), _lastwright(typename _vec.begin()){}
	RingBuffer(RingBuffer<T>&) = delete;//No copy constructor
	RingBuffer(RingBuffer<T>&& other)
	{
		_vec = std::move(other._vec);
	}
	void push_back(T&& Element)
	{
		if (_vec.size() < _size)//If it's the first time we're filling this
		{
			_vec.push_back(Element);
			_lastwright++;
		}
		else
		{
			if (++_lastwright == _vec.end())
			{
				_lastwright = _vec.begin();
			}
			std::swap((*_lastwright),Element)
		}
		
	}
	void push_back(T Element)
	{
		if (_vec.size() < _size)//If it's the first time we're filling this
		{
			_vec.push_back(Element);
			_lastwright++;
		}
		else
		{
			if (++_lastwright == _vec.end())
			{
				_lastwright = _vec.begin();
			}
			std::swap((*_lastwright), Element);
		}

	}
};