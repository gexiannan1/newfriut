#ifndef __MCLIENTTHREADSAFEDATA_H
#define __MCLIENTTHREADSAFEDATA_H

class MByteBuffer;
class MDynBuffer;
class MMsgBuffer;

/**
 * @brief �����̰߳�ȫ�� socket buffer ���ݣ�ͬһ���̹߳�����Щ����
 */
class MClientThreadSafeData
{
public:
	MByteBuffer* m_pHeaderBA;		// ��Ҫ����������ͷ�Ĵ�С���ĸ��ֽ�.
	MByteBuffer* m_pMsgBA;			// ���ص��ֽ����飬û����Ϣ���ȵ�һ����������Ϣ.

	// ���������ݿ����е��߳�ֻ��Ҫһ�����ݣ���˵�����ʼ��
	MDynBuffer* m_recvSocketDynBuffer;	// ������ʱ����
	//MByteBuffer* m_sendClientBA;		// ��Ž�Ҫ���͵���ʱ���ݣ���Ҫ�ŵ� m_sendClientBuffer ��ȥ
	MMsgBuffer* m_sendClient2SocketBuffer;	// Client �� Socket ���ɣ��̹߳���

public:
	MClientThreadSafeData();
	~MClientThreadSafeData();

	//void newRecvSocketDynBuffer();
	//void newSendClientBA();
};

#endif