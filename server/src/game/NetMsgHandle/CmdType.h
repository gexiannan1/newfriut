#ifndef _CMD_TYPE_H_
#define _CMD_TYPE_H_

namespace Cmd
{
	enum EnByCmd
	{
		eNULL_USERCMD,			/// ��ָ��
		eOBJECT_USERCMD,		/// ����ָ��
		eHERO_CARD_USERCMD,		/// ����ָ��

		eByCmdTotal
	};

	enum EnObjectByParam
	{
		eOBJECT_BASIC_USERCMD,		/// ���߻���ָ��
		eObjectByParamTotal
	};
}

#endif