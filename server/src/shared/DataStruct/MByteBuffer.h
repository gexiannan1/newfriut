#ifndef __BYTEBUFFER_H
#define __BYTEBUFFER_H

#include <vector>
#include <list>
#include <string>
#include <map>

#include "MByteConverter.h"
#include "MSystemEndian.h"
#include "Platform/Define.h"
#include "MDynBufResizePolicy.h"
#include "MStorageBuffer.h"

class MByteBufferException
{
public:
	MByteBufferException(bool _add, size_t _pos, size_t _esize, size_t _size)
		: add(_add), pos(_pos), esize(_esize), size(_size)
	{
		PrintPosError();
	}

	void PrintPosError() const;
private:
	bool add;
	size_t pos;
	size_t esize;
	size_t size;
};

template<class T>
struct MUnused
{
	MUnused() {}
};

class MByteBuffer
{
protected:
	SysEndian m_sysEndian;

public:
	// constructor
	MByteBuffer(size_t len);
	~MByteBuffer();

	void setEndian(SysEndian endian);
	void clear();

	// �����ֵһ����ϵͳ��С��һ����
	template <typename T>
	void put(size_t pos, T value);

	MByteBuffer& writeUnsignedInt8(uint8 value);
	MByteBuffer& writeUnsignedInt16(uint16 value);
	MByteBuffer& writeUnsignedInt32(uint32 value);
	MByteBuffer& writeUnsignedInt64(uint64 value);
	// signed as in 2e complement
	MByteBuffer& writeInt8(int8 value);
	MByteBuffer& writeInt16(int16 value);
	MByteBuffer& writeInt32(int32 value);
	MByteBuffer& writeInt64(int64 value);
	// floating points
	MByteBuffer& writeFloat(float value);
	MByteBuffer& writeDouble(double value);
	// д�� UTF-8 �ַ����������ַ������� '\0' ���Լ����õ������
	MByteBuffer& writeMultiByte(const std::string& value, size_t len);
	//MByteBuffer& writeMultiByte(const char* str, size_t len)
	MByteBuffer& writeBytes(const char* str, size_t startPos, size_t len);
	MByteBuffer& readBoolean(bool& value);
	MByteBuffer& readUnsignedInt8(uint8& value);
	MByteBuffer& readUnsignedInt16(uint16& value);
	MByteBuffer& readUnsignedInt32(uint32& value);
	MByteBuffer& readUnsignedInt64(uint64& value);
	// signed as in 2e complement
	MByteBuffer& readInt8(int8& value);
	MByteBuffer& readInt16(int16& value);
	MByteBuffer& readInt32(int32& value);
	MByteBuffer& readInt64(int64& value);
	MByteBuffer& readFloat(float& value);
	MByteBuffer& readDouble(double& value);
	MByteBuffer& readMultiByte(std::string& value, size_t len);
	MByteBuffer& readMultiByte(char* value, size_t len);

	template<class T>
	MByteBuffer& readUnused(MUnused<T> const&);

	uint8 operator[](size_t pos) const;
	size_t pos() const;
	size_t pos(size_t pos_);

	// ������������
	template<typename T>
	void read_skip();

	// ���ݴ�С����
	void read_skip(size_t skip);

	template<typename T>
	void write_skip();

	// ���ݴ�С����
	void write_skip(size_t skip);

	template <typename T>
	T read();

	// ��ȡ������һ���Ǻ�ϵͳ��С��һ����
	template <typename T>
	T read(size_t pos) const;

	void read(uint8* dest, size_t len);
	const uint8* getStorage() const;
	size_t size() const;
	void setSize(size_t len);
	bool empty() const;
	size_t capacity();
	void readAddPos(int delta);
	void writeAddPos(int delta);
	void setCapacity(std::size_t newCapacity);

	/**
	*@brief �ܷ���� num ���ȵ�����
	*/
	bool canAddData(uint32 num);
	// ��������
	void append(const std::string& str);
	void append(const char* src, size_t cnt);

	template<class T>
	void append(const T* src, size_t cnt);

	void append(const uint8* src, size_t cnt);
	// �����ƶ�дָ�룬�����������
	void append(size_t cnt);
	void append(const MByteBuffer& buffer);
	void put(size_t pos, const uint8* src, size_t cnt);
	char* getCurPtr();				// ��ȡ��ǰ
	size_t avaliableBytes();
	void print_storage() const;
	void textlike() const;
	void hexlike() const;
	void writeFile(FILE* file);

	void compress(size_t startPos = 0, size_t len_ = 0);
	void uncompress(size_t startPos = 0, size_t len_ = 0);
	void encrypt(size_t startPos = 0, size_t len_ = 0);
	void decrype(size_t startPos = 0, size_t len_ = 0);

private:
	// ��ӵ�һ���Ǻ�ϵͳ��С����ͬ��
	// limited for internal use because can "append" any unexpected type (like pointer and etc) with hard detection problem
	template <typename T>
	void append(T value);

protected:
	size_t m_pos;		// ��ȡд��λ��
	MStorageBuffer* m_pStorageBuffer;
	char m_encryptKey[8];
	char m_decryptKey[8];
};


#endif