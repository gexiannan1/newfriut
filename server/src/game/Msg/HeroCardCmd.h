#pragma once

#include "BaseCmd.h"
#include <string>

namespace Cmd
{
	enum EHeroCardCmd
	{
		eRET_MAGIC_POINT_INFO_CMD = 21,
		eADD_BATTLE_CARD_PROPERTY_CMD = 26,
	};

	struct t_MagicPoint
	{
	public:
		uint32 mp;
		uint32 maxmp;
		uint32 forbid;

		t_MagicPoint()
		{
			mp = 0;
			maxmp = 0;
			forbid = 0;
		}

		void serialize(MByteBuffer& pMsgBU)
		{
			pMsgBU.writeUnsignedInt32(mp);
			pMsgBU.writeUnsignedInt32(maxmp);
			pMsgBU.writeUnsignedInt32(forbid);
		}
	};

	class stObjectLocation
	{
	public:
		uint32 dwLocation;
		uint32 dwTableID;
		uint16 x;
		uint16 y;

		virtual void serialize(MByteBuffer& pMsgBU)
		{
			pMsgBU.writeUnsignedInt32(dwLocation);
			pMsgBU.writeUnsignedInt32(dwTableID);
			pMsgBU.writeUnsignedInt16(x);
			pMsgBU.writeUnsignedInt16(y);
		}
	};

	class t_Card
	{
	public:
		uint32 qwThisID;		    //��ƷΨһid
		uint32 dwObjectID;		    //��Ʒ���еı��
		stObjectLocation pos;	// λ��

		uint32 dwNum;		//���������Ҫɾ��
		std::string strName; //����
		uint32 mpcost;		    //����
		uint32 damage;		    //������
		uint32 hp;		        //Ѫ��
		uint32 maxhp;		    //Ѫ������
		uint32 dur;		    //�;ö�
		uint32 maxdur;		    //�;ö�����

		uint8 magicDamAdd;	//�����˺�����(X)
		uint8 overload;		//����(num)
		uint32 armor;          //����ֵ

		char* state;

		virtual void serialize(MByteBuffer& pMsgBU)
		{
			pMsgBU.writeUnsignedInt32(qwThisID);
			pMsgBU.writeUnsignedInt32(dwObjectID);
			pos.serialize(pMsgBU);

			pMsgBU.writeUnsignedInt32(dwNum);
			pMsgBU.writeMultiByte(strName, strName.length());
			pMsgBU.writeUnsignedInt32(mpcost);
			pMsgBU.writeUnsignedInt32(damage);
			pMsgBU.writeUnsignedInt32(hp);
			pMsgBU.writeUnsignedInt32(maxhp);
			pMsgBU.writeUnsignedInt32(dur);
			pMsgBU.writeUnsignedInt32(maxdur);

			pMsgBU.writeUnsignedInt8(magicDamAdd);
			pMsgBU.writeUnsignedInt8(overload);
			pMsgBU.writeUnsignedInt32(armor);

			uint32 len = (8 + 7) / 8;
			state = new char[len];
			pMsgBU.writeBytes(state, 0, len);
			delete state;
		}
	};

	class stRetMagicPointInfoUserCmd : public stHeroCardCmd
	{
	public:
		t_MagicPoint self;
		t_MagicPoint other;

	public:
		stRetMagicPointInfoUserCmd()
		{
			byParam = eRET_MAGIC_POINT_INFO_CMD;
		}

		virtual void serialize(MByteBuffer& pMsgBU)
		{
			stHeroCardCmd::serialize(pMsgBU);
			self.serialize(pMsgBU);
			other.serialize(pMsgBU);
		}
	};

	class stAddBattleCardPropertyUserCmd : public stHeroCardCmd
	{
	public:
		uint8 slot;	    //�ĸ���
		uint8 who;	    //1,�Լ� 2,�Է�
		uint8 byActionType;  //
		t_Card mobject;

		uint8 attackType;    //��������
		// ֻ�й���ˢ�����Ե�ʱ��
		uint32 pAttThisID;   //������
		uint32 pDefThisID;   //������

		stAddBattleCardPropertyUserCmd()
		{
			byParam = eADD_BATTLE_CARD_PROPERTY_CMD;
			slot = 0;
			who = 0;
			byActionType = 0;
		}

		virtual void serialize(MByteBuffer& pMsgBU)
		{
			stHeroCardCmd::serialize(pMsgBU);
			
			pMsgBU.writeUnsignedInt8(slot);
			pMsgBU.writeUnsignedInt8(who);
			pMsgBU.writeUnsignedInt8(byActionType);
			mobject.serialize(pMsgBU);

			pMsgBU.writeUnsignedInt8(attackType);
			pMsgBU.writeUnsignedInt32(pAttThisID);
			pMsgBU.writeUnsignedInt32(pDefThisID);
		}
	};
}