#ifndef __CircleQueue_H__
#define __CircleQueue_H__

#include <mutex>

const	int		DEFAULT_QUEUE_SIZE = 8;						//默认队列长度
const	int		MAX_FRAME_SIZE = (1920*1080*4 + 4096*128);	//最大帧size

class CCycleQueue
{
public:

	CCycleQueue();
	virtual	~CCycleQueue();

	bool	InitQueue(int nFrameSize, int nQueueSize = DEFAULT_QUEUE_SIZE);
    bool	PushBack(unsigned char* pFrameData, int nFrameSize,int& nRs);
	bool	GetFront(unsigned char* pFrameData, int& nFrameSize);
	bool	IsEmpty();
	void	Clear();
	int		Size();

protected:
	void	DestroyQueue();

private:
	const	int		m_nFrameSize;
	int				m_nQueueSize;

	unsigned char	(*m_ptrFrameQueue)[MAX_FRAME_SIZE];
	int		m_nFront;
	int		m_nTail;


    std::mutex    m_mutex;
};

#endif // !__FileDataStream_H__
