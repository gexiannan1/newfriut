#include "MDynBufResizePolicy.h"
#include "MBufferDefaultValue.h"

// ��ȡһ������Ĵ�С
uint32 MDynBufResizePolicy::getCloseSize(uint32 needSize, uint32 capacity)
{
	uint32 ret = 0;

	if (needSize <= MAX_CAPACITY)
	{
		if (capacity > needSize)		// ʹ�� > �������� >= ���˷�һ���Լ��������ж�
		{
			ret = capacity;
		}
		else
		{
			ret = 2 * capacity;
			while (ret < needSize && ret < MAX_CAPACITY)
			{
				ret *= 2;
			}

			if (ret > MAX_CAPACITY)
			{
				ret = MAX_CAPACITY;
			}
		}
	}
	else	// ��Ҫ�����ݱ���������������Ĵ�С����
	{
		// ��־����Ҫ�Ĵ洢�ռ�̫��
	}

	return ret;
}