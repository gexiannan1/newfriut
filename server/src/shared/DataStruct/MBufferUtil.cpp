#include "MBufferUtil.h"
#include <assert.h>

void* MBufferUtil::memSwap(void *dest, const void *source, size_t count)
{
	assert((nullptr != dest) && (nullptr != source));

	char *tmp_source, *tmp_dest;
	char tmpChar;

	tmp_source = (char *)source;
	tmp_dest = (char *)dest;

	// ���û���ص�����
	while (count--)
	{
		tmpChar = *tmp_dest;
		*tmp_dest = *tmp_source;
		*tmp_source = tmpChar;
		++tmp_dest;
		++tmp_source;
	}

	return dest;
}
