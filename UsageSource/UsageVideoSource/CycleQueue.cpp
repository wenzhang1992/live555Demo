
#include "CycleQueue.h"
CCycleQueue::CCycleQueue()
    : m_nFrameSize(0)
{
    m_nQueueSize = 0;
    
    m_ptrFrameQueue = nullptr;
    
    m_nFront = 0;
    m_nTail = 0;
}

CCycleQueue::~CCycleQueue()
{
    DestroyQueue();
}

/*
 *初始化队列
 */
bool	CCycleQueue::InitQueue(int nFrameSize, int nQueueSize)
{
    DestroyQueue();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_nQueueSize = (nQueueSize > 0) ? (nQueueSize + 1) : (DEFAULT_QUEUE_SIZE + 1);		//循环队列，有一个数据块为空，用于存储队列指针
    const_cast<int&>(m_nFrameSize) = nFrameSize;
    
    if(m_nFrameSize > MAX_FRAME_SIZE)
    {
        return false;
    }
    
    m_ptrFrameQueue = new unsigned char[m_nQueueSize][MAX_FRAME_SIZE];
    
    if(nullptr == m_ptrFrameQueue)
    {
        return false;
    }
    
    return true;
}

/*
 *数据插入队列尾部
 */
bool	CCycleQueue::PushBack(unsigned char* pFrameData, int nFrameSize,int& nRs)
{
    nRs=0;
    if (m_nFrameSize != nFrameSize)
    {
        return false;
    }
    
	std::lock_guard<std::mutex> lock(m_mutex);
    
    //如果队尾与队首相同，说明队列已满，队首指针后移一位
    if ((m_nTail + 1) % m_nQueueSize == m_nFront)
    {
        m_nFront++;
        m_nFront %= m_nQueueSize;
        nRs = 1;
    }
    
    //数据入队
    memcpy(m_ptrFrameQueue[m_nTail], pFrameData, nFrameSize);
    
    m_nTail++;
    m_nTail %= m_nQueueSize;
    
   // nRs = 0;
    
    return true;
}

/*
 *从队列头部获取数据
 */
bool	CCycleQueue::GetFront(unsigned char* pFrameData, int& nFrameSize)
{
    if (IsEmpty() )
    {
        return false;
    }    
    
	std::lock_guard<std::mutex> lock(m_mutex);
    
    nFrameSize = m_nFrameSize;
    
    memcpy(pFrameData, m_ptrFrameQueue[m_nFront], nFrameSize);
    
    m_nFront++;
    m_nFront %= m_nQueueSize;
    
    return true;
}

/*
 *判断队列是否为空
 */
bool	CCycleQueue::IsEmpty()
{
    return !(Size() > 0);
}

/*
 *清空队列
 */
void	CCycleQueue::Clear()
{    
	std::lock_guard<std::mutex> lock(m_mutex);
    
    m_nFront = 0;
    m_nTail = 0;
}

/*
 *获取队列大小(有效数据个数)
 */
int		CCycleQueue::Size()
{
    int nSize = 0;    
    
	std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_nTail > m_nFront)
    {
        nSize = m_nTail - m_nFront;
    }
    else if (m_nTail < m_nFront)
    {
        nSize = m_nQueueSize + m_nTail - m_nFront;
    }
    else
    {
        nSize = 0;
    }
   // qDebug()<<"Front:"<<m_nFront;
   // qDebug()<<"Tail:"<<m_nTail;
    return nSize;
}

/*
 *销毁队列
 */
void	CCycleQueue::DestroyQueue()
{    
	std::lock_guard<std::mutex> lock(m_mutex);
    
    if (nullptr != m_ptrFrameQueue)
    {
        delete [] m_ptrFrameQueue;
        m_ptrFrameQueue = nullptr;
    }
    
    m_nFront = 0;
    m_nTail = 0;  
}
