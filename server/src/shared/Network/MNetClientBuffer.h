#ifndef __NETCLIENTBUFFER_H
#define __NETCLIENTBUFFER_H

class MMsgBuffer;
class MDynBuffer;
class MByteBuffer;
class MCircularBuffer;
class Mutex;
class MClientThreadSafeData;
class MClientProcessData;

#include <boost/thread/thread.hpp>

/**
* @brief ��Ϣ������
*/
class MNetClientBuffer
{
public:
	// ���յ� Buffer
	MMsgBuffer* m_recvSocketBuffer;		// ֱ�Ӵӷ��������յ���ԭʼ�����ݣ�����ѹ���ͼ��ܹ����߳���ִ��
	MMsgBuffer* m_recvClientBuffer;		// ��ѹ���ܺ����ʹ�õĻ����������̻߳���ĳ�����߳���ִ��
	MDynBuffer* m_recvSocketDynBuffer;	// ���յ�����ʱ���ݣ���Ҫ�ŵ� m_recvRawBuffer ��ȥ��ͬһ���߳������� MNetClientBuffer ����ͬһ���������

	// ���͵� Buffer
	MByteBuffer* m_sendClientBuffer;		// ������ʱ�����������͵����ݶ���ʱ��������
	MMsgBuffer* m_sendClient2SocketBuffer;		// Client �� Socket ����
	MByteBuffer* m_sendSocketBuffer;		// ���ͻ�������ѹ�����߼��ܹ���
	//MByteBuffer* m_sendClientBA;			// ��Ž�Ҫ���͵���ʱ���ݣ���Ҫ�ŵ� m_sendClientBuffer ��ȥ

	MByteBuffer* m_unCompressHeaderBA;  // ��Ž�ѹ���ͷ�ĳ���
	MByteBuffer* m_pHeaderBA;			// д���ĸ��ֽ�ͷ��

	boost::mutex* m_pSendMutex;
	boost::mutex* m_pRevMutex;
	//bool m_canSend;						// �첽���͵Ĺ����У���ǰ���͵������Ƿ������

public:
	MNetClientBuffer();
	~MNetClientBuffer();

	// ����
	void moveRecvSocketDyn2RecvSocket(size_t dynLen);
	void moveRecvSocket2RecvClient();
	MByteBuffer* getMsg();
	void onReadComplete(size_t dynLen);

	// ����
	void sendMsg(MByteBuffer* sendClientBA);
	void moveSendClient2SendSocket();
	void setRecvMsgSize(size_t len);
	bool startAsyncSend();
	void onWriteComplete(size_t len);

	// �����̰߳�ȫ������
	void setRecvSocketBufferTSData(MClientThreadSafeData* tsData);
	void setRecvClientProcessData(MClientProcessData* pMClientProcessData);
	void MNetClientBuffer::UnCompressAndDecryptEveryOne();
	bool canSendData();		// ��ǰ�Ƿ���Դ� socket ��������
	bool hasSendData();		// ��ǰ�Ƿ��з��͵�����
};

#endif				// __NETCLIENTBUFFER_H