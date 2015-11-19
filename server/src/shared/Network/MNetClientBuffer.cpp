#include "MNetClientBuffer.h"
#include "MMsgBuffer.h"
#include "MDynBuffer.h"
#include "MByteBuffer.h"
#include "MCircularBuffer.h"
#include "MBufferDefaultValue.h"
#include "MClientThreadSafeData.h"
#include "MClientProcessData.h"

MNetClientBuffer::MNetClientBuffer()
{
	m_recvSocketBuffer = new MMsgBuffer();
	m_recvClientBuffer = new MMsgBuffer();
	//m_recvSocketDynBuffer = new MDynBuffer(INIT_CAPACITY);	// ͬһ���̹߳��������

	m_sendClientBuffer = new MByteBuffer(INIT_CAPACITY);
	m_sendSocketBuffer = new MByteBuffer(INIT_CAPACITY);
	//m_sendClientBA = new MByteBuffer(INIT_CAPACITY);	// ͬһ���̹߳��������

	m_unCompressHeaderBA = new MByteBuffer(MSG_HEADER_SIZE);
	m_pHeaderBA = new MByteBuffer(MSG_HEADER_SIZE);

	m_pSendMutex = new boost::mutex();
	m_pRevMutex = new boost::mutex();
}

MNetClientBuffer::~MNetClientBuffer()
{
	delete m_recvSocketBuffer;
	delete m_recvClientBuffer;
	//delete m_recvSocketDynBuffer;

	delete m_sendClientBuffer;
	delete m_sendSocketBuffer;
	//delete m_sendClientBA;

	delete m_unCompressHeaderBA;

	delete m_pSendMutex;
	delete m_pRevMutex;
}

void MNetClientBuffer::setRecvMsgSize(size_t len)
{
	m_recvSocketDynBuffer->setCapacity(len);
}

void MNetClientBuffer::moveRecvSocketDyn2RecvSocket(size_t dynLen)
{
	m_recvSocketDynBuffer->setSize(dynLen);
	m_recvSocketBuffer->m_pMCircularBuffer->pushBack(m_recvSocketDynBuffer->getStorage(), 0, m_recvSocketDynBuffer->size());

	//moveRecvSocket2RecvClient();
}

// �п���һ�����ݰ��ж����Ϣ������ط�û�д�������ж����Ϣ����Ҫ��������ᶪʧ��Ϣ
void MNetClientBuffer::moveRecvSocket2RecvClient()
{
	while (m_recvSocketBuffer->checkHasMsg())  // ���������
	{
		UnCompressAndDecryptEveryOne();
	}
}

MByteBuffer* MNetClientBuffer::getMsg()
{
	if (m_recvClientBuffer->checkHasMsg())
	{
		m_pRevMutex->lock();
		m_recvClientBuffer->moveOutOneMsg();
		m_pRevMutex->unlock();
		return m_recvClientBuffer->m_pMsgBA;
	}

	return nullptr;
}

void MNetClientBuffer::onReadComplete(size_t dynLen)
{
	moveRecvSocketDyn2RecvSocket(dynLen);		// ���������Ϣ��������
	moveRecvSocket2RecvClient();
}

void MNetClientBuffer::sendMsg(MByteBuffer* sendClientBA)
{
	m_pHeaderBA->clear();
	m_pHeaderBA->writeUnsignedInt32(sendClientBA->size());      // ��䳤��

	m_pSendMutex->lock();

	m_sendClientBuffer->writeUnsignedInt32(sendClientBA->size());
	m_sendClientBuffer->writeBytes((char*)sendClientBA->getStorage(), 0, sendClientBA->size());

	m_pSendMutex->unlock();

	sendClientBA->clear();
}

// ��ȡ���ݣ�Ȼ��ѹ������
void MNetClientBuffer::moveSendClient2SendSocket()
{
	m_sendSocketBuffer->clear(); // �����������λ������ֿ��Դ� 0 ������ʼ��
	m_sendClient2SocketBuffer->m_pMCircularBuffer->clear();

	m_pSendMutex->lock();

	m_sendClient2SocketBuffer->m_pMCircularBuffer->pushBack((char*)m_sendClientBuffer->getStorage(), 0, m_sendClientBuffer->size());
	m_sendClientBuffer->clear();	// �����������λ������ֿ��Դ� 0 ������ʼ��

	m_pSendMutex->unlock();

	m_sendSocketBuffer->writeBytes(m_sendClient2SocketBuffer->m_pMCircularBuffer->getStorage(), 0, m_sendClient2SocketBuffer->m_pMCircularBuffer->size());
	m_sendSocketBuffer->pos(0);				// ����λ��Ϊ��ʼλ��
}

bool MNetClientBuffer::startAsyncSend()
{
	if (m_sendClientBuffer->size() > 0 && m_sendSocketBuffer->size() == 0)		// ֻ�л�����Ϊ�յ�ʱ����ܿ������ݣ������Ϊ�գ��������������ڷ���
	{
		moveSendClient2SendSocket();		// ������Ϣ���ݣ��ȴ�����
	}

	if (m_sendSocketBuffer->size() == 0)		// �������û������
	{
		return false;
	}

	return true;
}

void MNetClientBuffer::onWriteComplete(size_t len)
{
	if (len < m_sendSocketBuffer->avaliableBytes())		// �������û�з������
	{
		m_sendSocketBuffer->pos(len);
	}
	else				// ���ȫ���������
	{
		m_sendSocketBuffer->clear();	// ��������
	}
}

// ������ж���߳�ȥ���ղ�ͬ�� socket ����
void MNetClientBuffer::setRecvSocketBufferTSData(MClientThreadSafeData* tsData)
{
	m_recvSocketBuffer->setHeaderBATSData(tsData);
	m_recvSocketBuffer->setMsgBATSData(tsData);
	
	m_recvSocketDynBuffer = tsData->m_recvSocketDynBuffer;
	m_sendClient2SocketBuffer = tsData->m_sendClient2SocketBuffer;
}

// �����������һ�����̷߳�����Ϣ
void MNetClientBuffer::setRecvClientProcessData(MClientProcessData* pMClientProcessData)
{
	m_recvClientBuffer->setHeaderBAProcessData(pMClientProcessData);
	m_recvClientBuffer->setMsgBAProcessData(pMClientProcessData);

	//m_sendClientBA = tsData->m_sendClientBA;
}

void MNetClientBuffer::UnCompressAndDecryptEveryOne()
{
	m_recvSocketBuffer->moveOutOneMsg();
	//m_recvSocketBuffer->m_pMsgBA->uncompress();
	m_unCompressHeaderBA->clear();
	m_unCompressHeaderBA->writeUnsignedInt32(m_recvSocketBuffer->m_pMsgBA->size());
	m_unCompressHeaderBA->pos(0);

	m_pRevMutex->lock();

	m_recvClientBuffer->m_pMCircularBuffer->pushBack((char*)m_unCompressHeaderBA->getStorage(), 0, MSG_HEADER_SIZE);             // ������Ϣ��С�ֶ�
	m_recvClientBuffer->m_pMCircularBuffer->pushBack((char*)m_recvSocketBuffer->m_pMsgBA->getStorage(), 0, m_recvSocketBuffer->m_pMsgBA->size());      // ������Ϣ��С�ֶ�

	m_pRevMutex->unlock();
}

bool MNetClientBuffer::canSendData()
{
	if (m_sendSocketBuffer->size() > 0)		// ֻ�л�����Ϊ�յ�ʱ����ܿ������ݣ������Ϊ�գ��������������ڷ���
	{
		return false;
	}

	return true;
}

bool MNetClientBuffer::hasSendData()
{
	if (m_sendClientBuffer->size() == 0)		// ���Ϊ 0 ��˵��û�п��Է��͵�����
	{
		return false;
	}

	return true;
}