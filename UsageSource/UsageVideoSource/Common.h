#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__
#include <vector>
#include <queue>
#include <share.h>
#include <mutex>
#define ImageWidth 1280
#define ImageHeight 720
#define ImagePixelSize 2
#define ImageSize ImageHeight*ImageWidth*ImagePixelSize
#define ImageSizeYUV ImageHeight *ImageWidth * 1.5

template <typename T>
class Queue_S
{
public :
	Queue_S()
	{

	}

	~Queue_S()
	{
		while (m_sQueue.size() != 0)
		{
			T* item = m_sQueue.front();

			m_sQueue.pop();

			delete item;
		}
	}

	void Push(T* item)
	{
		std::unique_lock<std::mutex> lock(m_lock);

		m_sQueue.push(item);
	}

	int Size()
	{
		return m_sQueue.size();
	}

	T* Get()
	{
		if (Size() == 0)
		{
			return nullptr;
		}
		else
		{
			std::unique_lock<std::mutex> lock(m_lock);

			T* temp = m_sQueue.front();

			m_sQueue.pop();

			return temp;
		}
	}
private:
	std::queue<T*> m_sQueue;

	std::mutex m_lock;
};

#endif