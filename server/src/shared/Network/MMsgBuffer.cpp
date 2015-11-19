#include "MMsgBuffer.h"
#include "MCircularBuffer.h"
#include "MByteBuffer.h"
#include "MBufferDefaultValue.h"
#include "MClientThreadSafeData.h"
#include "MClientProcessData.h"

MMsgBuffer::MMsgBuffer()
{
	m_pMCircularBuffer = new MCircularBuffer(INIT_CAPACITY);
	//m_pHeaderBA = new MByteBuffer(MSG_HEADER_SIZE);		// �������ֶ���Ҫ�̹߳���
	//m_pMsgBA = new MByteBuffer(INIT_CAPACITY);			// �������ֶ���Ҫ�̹߳���
}

MMsgBuffer::~MMsgBuffer()
{
	delete m_pMCircularBuffer;
	//delete m_pHeaderBA;
	//delete m_pMsgBA;
}

bool MMsgBuffer::checkHasMsg()
{
	if (m_pMCircularBuffer->size() <= MSG_HEADER_SIZE)
	{
		return false;
	}

	m_pHeaderBA->clear();
	m_pMCircularBuffer->front((char*)m_pHeaderBA->getStorage(), 0, MSG_HEADER_SIZE);
	m_pHeaderBA->setSize(MSG_HEADER_SIZE);

	//m_pHeaderBA->readUnsignedInt32(m_msgLen);
	getHeaderSize(m_msgLen);
	if (m_msgLen + MSG_HEADER_SIZE <= m_pMCircularBuffer->size())
	{
		return true;
	}

	return false;
}

// �Ƴ�һ����Ϣ
void MMsgBuffer::moveOutOneMsg()
{
	m_pMCircularBuffer->popFrontLenNoData(MSG_HEADER_SIZE);

	m_pMsgBA->setCapacity(m_msgLen);
	m_pMsgBA->clear();

	m_pMCircularBuffer->popFront((char*)m_pMsgBA->getStorage(), 0, m_msgLen);
	m_pMsgBA->setSize(m_msgLen);
}

void MMsgBuffer::setHeaderBATSData(MClientThreadSafeData* tsData)
{
	m_pHeaderBA = tsData->m_pHeaderBA;
}

void MMsgBuffer::setMsgBATSData(MClientThreadSafeData* tsData)
{
	m_pMsgBA = tsData->m_pMsgBA;
}

void MMsgBuffer::setHeaderBAProcessData(MClientProcessData* pMClientProcessData)
{
	m_pHeaderBA = pMClientProcessData->m_pHeaderBA;
}

void MMsgBuffer::setMsgBAProcessData(MClientProcessData* pMClientProcessData)
{
	m_pMsgBA = pMClientProcessData->m_pMsgBA;
}

void MMsgBuffer::getHeaderSize(uint32& msgLen)
{
	m_pHeaderBA->readUnsignedInt32(msgLen);
#if MSG_COMPRESS
	if ((msglen & PACKET_ZIP) > 0)         // �����ѹ����־
	{
		msglen &= (~PACKET_ZIP);         // ȥ��ѹ����־λ
	}
#endif
}