#ifndef __MCLIENTPROCESSDATA_H
#define __MCLIENTPROCESSDATA_H

class MByteBuffer;
class MDynBuffer;

/**
 * @brief ���ڽ��� socket buffer ���ݣ�ͬһ�����̹�����Щ����
 */
class MClientProcessData
{
public:
	MByteBuffer* m_pHeaderBA;		// ��Ҫ����������ͷ�Ĵ�С���ĸ��ֽ�.
	MByteBuffer* m_pMsgBA;			// ���ص��ֽ����飬û����Ϣ���ȵ�һ����������Ϣ.

	MByteBuffer* m_sendClientBA;		// ��Ž�Ҫ���͵���ʱ���ݣ���Ҫ�ŵ� m_sendClientBuffer ��ȥ

public:
	MClientProcessData();
	~MClientProcessData();
};

#endif