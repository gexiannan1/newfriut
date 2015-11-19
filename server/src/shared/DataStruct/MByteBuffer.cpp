#include "MByteBuffer.h"
#include "MCompress.h"
#include "Encrypt.h"
#include "MemOp.h"

#include <sstream>

void MByteBufferException::PrintPosError() const
{
	
}

// constructor
MByteBuffer::MByteBuffer(size_t len) : m_pos(0)
{
	m_pStorageBuffer = new MStorageBuffer(len);
	m_sysEndian = eSys_LITTLE_ENDIAN;		// Ĭ����С��
}

MByteBuffer::~MByteBuffer()
{
	if (m_pStorageBuffer)
	{
		delete m_pStorageBuffer;
	}
}

void MByteBuffer::setEndian(SysEndian endian)
{
	m_sysEndian = endian;
}

void MByteBuffer::clear()
{
	m_pStorageBuffer->m_size = 0;
	m_pos = 0;
}

// �����ֵһ����ϵͳ��С��һ����
template <typename T>
void MByteBuffer::put(size_t pos, T value)
{
	if (sSysEndian != m_sysEndian)
	{
		EndianConvert(value);
	}
	put(pos, (uint8*)&value, sizeof(value));
}

MByteBuffer& MByteBuffer::writeUnsignedInt8(uint8 value)
{
	append<uint8>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeUnsignedInt16(uint16 value)
{
	append<uint16>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeUnsignedInt32(uint32 value)
{
	append<uint32>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeUnsignedInt64(uint64 value)
{
	append<uint64>(value);
	return *this;
}

// signed as in 2e complement
MByteBuffer& MByteBuffer::writeInt8(int8 value)
{
	append<int8>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeInt16(int16 value)
{
	append<int16>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeInt32(int32 value)
{
	append<int32>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeInt64(int64 value)
{
	append<int64>(value);
	return *this;
}

// floating points
MByteBuffer& MByteBuffer::writeFloat(float value)
{
	append<float>(value);
	return *this;
}

MByteBuffer& MByteBuffer::writeDouble(double value)
{
	append<double>(value);
	return *this;
}

// д�� UTF-8 �ַ����������ַ������� '\0' ���Լ����õ������
MByteBuffer& MByteBuffer::writeMultiByte(const std::string& value, size_t len)
{
	if (len > value.length())
	{
		append((uint8 const*)value.c_str(), value.length());
		append(len - value.length());
	}
	else
	{
		append((uint8 const*)value.c_str(), len);
	}
	return *this;
}

MByteBuffer& MByteBuffer::writeBytes(const char* str, size_t startPos, size_t len)
{
	if (str)
	{
		size_t charLen = strlen(str) / sizeof(char);
		if (len > charLen)
		{
			append(str, charLen);
			append(len - charLen);
		}
		else
		{
			append(str, len);
		}
	}
	else
	{
		append(len);
	}
	return *this;
}

MByteBuffer& MByteBuffer::readBoolean(bool& value)
{
	value = read<char>() > 0 ? true : false;
	return *this;
}

MByteBuffer& MByteBuffer::readUnsignedInt8(uint8& value)
{
	value = read<uint8>();
	return *this;
}

MByteBuffer& MByteBuffer::readUnsignedInt16(uint16& value)
{
	value = read<uint16>();
	return *this;
}

MByteBuffer& MByteBuffer::readUnsignedInt32(uint32& value)
{
	value = read<uint32>();
	return *this;
}

MByteBuffer& MByteBuffer::readUnsignedInt64(uint64& value)
{
	value = read<uint64>();
	return *this;
}

// signed as in 2e complement
MByteBuffer& MByteBuffer::readInt8(int8& value)
{
	value = read<int8>();
	return *this;
}

MByteBuffer& MByteBuffer::readInt16(int16& value)
{
	value = read<int16>();
	return *this;
}

MByteBuffer& MByteBuffer::readInt32(int32& value)
{
	value = read<int32>();
	return *this;
}

MByteBuffer& MByteBuffer::readInt64(int64& value)
{
	value = read<int64>();
	return *this;
}

MByteBuffer& MByteBuffer::readFloat(float& value)
{
	value = read<float>();
	return *this;
}

MByteBuffer& MByteBuffer::readDouble(double& value)
{
	value = read<double>();
	return *this;
}

MByteBuffer& MByteBuffer::readMultiByte(std::string& value, size_t len)
{
	value.clear();
	if (len)		// �����Ϊ 0 ���Ͷ�ȡָ������
	{
		size_t readNum = 0;	// �Ѿ���ȡ������
		while (pos() < size() && readNum < len) // prevent crash at wrong string format in packet
		{
			char c = read<char>();
			value += c;
			++readNum;
		}
	}
	else				// ���Ϊ 0 ����һֱ��ȡ��ֱ��������һ�� '\0'
	{
		while (pos() < size()) // prevent crash at wrong string format in packet
		{
			char c = read<char>();
			if (c == 0)
				break;
			value += c;
		}
	}

	return *this;
}

MByteBuffer& MByteBuffer::readMultiByte(char* value, size_t len)
{
	size_t readNum = 0;	// �Ѿ���ȡ������
	char c = 0;
	if (len)		// �����Ϊ 0 ���Ͷ�ȡָ������
	{
		while (pos() < size() && readNum < len)                         // prevent crash at wrong string format in packet
		{
			c = read<char>();
			value[readNum] = c;
			++readNum;
		}
	}
	else				// ���Ϊ 0 ����һֱ��ȡ��ֱ��������һ�� '\0'
	{
		while (pos() < size()) // prevent crash at wrong string format in packet
		{
			c = read<char>();
			if (c == 0)
			{
				break;
			}
			value[readNum] = c;
			++readNum;
		}
	}

	return *this;
}

template<class T>
MByteBuffer& MByteBuffer::readUnused(MUnused<T> const&)
{
	read_skip<T>();
	return *this;
}

uint8 MByteBuffer::operator[](size_t pos) const
{
	return read<uint8>(pos);
}

size_t MByteBuffer::pos() const 
{ 
	return m_pos; 
}

size_t MByteBuffer::pos(size_t pos_)
{
	m_pos = pos_;
	return m_pos;
}

// ���ݴ�С����
void MByteBuffer::read_skip(size_t skip)
{
	if (m_pos + skip > size())
	{
		throw MByteBufferException(false, m_pos, skip, size());
	}
	m_pos += skip;
}

// ���ݴ�С����
void MByteBuffer::write_skip(size_t skip)
{
	append(skip);
}

// ������������
template<typename T>
void MByteBuffer::read_skip()
{
	read_skip(sizeof(T));
}

template<typename T>
void MByteBuffer::write_skip()
{
	write_skip(sizeof(T));
}

template <typename T>
T MByteBuffer::read()
{
	T r = read<T>(m_pos);
	readAddPos(sizeof(T));
	return r;
}

// ��ȡ������һ���Ǻ�ϵͳ��С��һ����
template <typename T>
T MByteBuffer::read(size_t pos) const
{
	if (pos + sizeof(T) > size())
		throw MByteBufferException(false, pos, sizeof(T), size());
	T val = *((T const*)&m_pStorageBuffer->m_storage[pos]);
	if (sSysEndian != m_sysEndian)
	{
		MEndianConvert(val);
	}
	return val;
}

void MByteBuffer::read(uint8* dest, size_t len)
{
	if (m_pos + len > size())
	{
		throw MByteBufferException(false, m_pos, len, size());
	}
	memcpy(dest, &m_pStorageBuffer->m_storage[m_pos], len);
	readAddPos(len);
}

const uint8* MByteBuffer::getStorage() const
{
	return (uint8*)m_pStorageBuffer->m_storage;
}

size_t MByteBuffer::size() const
{
	if (m_pStorageBuffer != nullptr)
	{
		return m_pStorageBuffer->m_size;
	}

	return 0;
}

void MByteBuffer::setSize(size_t len)
{
	m_pStorageBuffer->setSize(len);
}

bool MByteBuffer::empty() const
{
	return m_pStorageBuffer->m_size == 0;
}

size_t MByteBuffer::capacity()
{
	return m_pStorageBuffer->m_iCapacity;
}

void MByteBuffer::readAddPos(int delta)
{
	m_pos += delta;
}

void MByteBuffer::writeAddPos(int delta)
{
	m_pos += delta;
	m_pStorageBuffer->m_size += delta;
}

void MByteBuffer::setCapacity(std::size_t newCapacity)
{
	m_pStorageBuffer->setCapacity(newCapacity);
}

/**
*@brief �ܷ���� num ���ȵ�����
*/
bool MByteBuffer::canAddData(uint32 num)
{
	return m_pStorageBuffer->canAddData(num);
}

// ��������
void MByteBuffer::append(const std::string& str)
{
	append((uint8 const*)str.c_str(), str.size() + 1);
}

void MByteBuffer::append(const char* src, size_t cnt)
{
	return append((const uint8*)src, cnt);
}

template<class T>
void MByteBuffer::append(const T* src, size_t cnt)
{
	return append((const uint8*)src, cnt * sizeof(T));
}

void MByteBuffer::append(const uint8* src, size_t cnt)
{
	if (!cnt)
	{
		return;
	}

	if (!canAddData(cnt))
	{
		uint32 closeSize = MDynBufResizePolicy::getCloseSize(cnt + size(), capacity());
		setCapacity(closeSize);
	}
	//memcpy(&m_pStorageBuffer->m_storage[m_pos], src, cnt);
	memmove(&m_pStorageBuffer->m_storage[m_pos], src, cnt);	// ͬһ���ڴ淽ʽ����
	writeAddPos(cnt);
}

// �����ƶ�дָ�룬�����������
void MByteBuffer::append(size_t cnt)
{
	if (!cnt)
	{
		return;
	}

	if (!canAddData(cnt))
	{
		uint32 closeSize = MDynBufResizePolicy::getCloseSize(cnt + size(), capacity());
		setCapacity(closeSize);
	}
	writeAddPos(cnt);
}

void MByteBuffer::append(const MByteBuffer& buffer)
{
	if (buffer.pos())
	{
		append(buffer.getStorage(), buffer.size());
	}
}

void MByteBuffer::put(size_t pos, const uint8* src, size_t cnt)
{
	if (!canAddData(cnt))
	{
		throw MByteBufferException(true, pos, cnt, size());
	}
	memcpy(&m_pStorageBuffer->m_storage[pos], src, cnt);
}

char* MByteBuffer::getCurPtr()
{
	return m_pStorageBuffer->m_storage + m_pos;
}

size_t MByteBuffer::avaliableBytes()
{
	return m_pStorageBuffer->m_size - m_pos;
}

void MByteBuffer::print_storage() const
{
    std::ostringstream ss;
    ss <<  "STORAGE_SIZE: " << size() << "\n";
    ss << "         ";

    for (size_t i = 0; i < size(); ++i)
        ss << uint32(read<uint8>(i)) << " - ";
}

void MByteBuffer::textlike() const
{
    std::ostringstream ss;
    ss <<  "STORAGE_SIZE: " << size() << "\n";

    ss << "         ";

    for (size_t i = 0; i < size(); ++i)
        ss << read<uint8>(i);
}

void MByteBuffer::hexlike() const
{
    std::ostringstream ss;
    ss <<  "STORAGE_SIZE: " << size() << "\n";

    ss << "         ";

    size_t j = 1, k = 1;

    for (size_t i = 0; i < size(); ++i)
    {
        if ((i == (j * 8)) && ((i != (k * 16))))
        {
            ss << "| ";
            ++j;
        }
        else if (i == (k * 16))
        {
            ss << "\n";

            ss << "         ";

            ++k;
            ++j;
        }

        char buf[4];
        _snprintf(buf, 4, "%02X", read<uint8>(i));
        ss << buf << " ";
    }
}

void MByteBuffer::writeFile(FILE* file)
{
	fwrite((void*)&m_pStorageBuffer->m_storage[0], sizeof(char), this->size(), file);
}

template <typename T>
void MByteBuffer::append(T value)
{
	if (sSysEndian != m_sysEndian)
	{
		MEndianConvert(value);
	}
	append((uint8*)&value, sizeof(value));
}

template<>
inline void MByteBuffer::read_skip<char*>()
{
	std::string temp;
	this->readMultiByte(temp, 0);
}

template<>
inline void MByteBuffer::read_skip<char const*>()
{
	read_skip<char*>();
}

template<>
inline void MByteBuffer::read_skip<std::string>()
{
	read_skip<char*>();
}

void MByteBuffer::compress(size_t startPos, size_t len_)
{
	uint32 destLen = 0;
	char** destBytes = nullptr;
	CompressUtil::mcompress(m_pStorageBuffer->m_storage + startPos, len_ ? len_ : size(), destBytes, &destLen);

	clear();
	setSize(startPos + destLen + (len_ ? size() - startPos - len_ : 0));
	if (len_)
	{
		pos(startPos + destLen);
		writeBytes(m_pStorageBuffer->m_storage, startPos + len_, size() - startPos - len_);	   // ���Լ��ռ��п��������ܵ������ݱ�����
	}
	pos(startPos);
	writeBytes((*destBytes), 0, destLen);
	MemOp::freeMem((*destBytes));
}

void MByteBuffer::uncompress(size_t startPos, size_t len_)
{
	uint32 destLen = 0;
	char** destBytes = nullptr;
	CompressUtil::muncompress(m_pStorageBuffer->m_storage + startPos, len_ ? len_ : size(), destBytes, &destLen);

	clear();
	setSize(startPos + destLen + (len_ ? size() - startPos - len_ : 0));
	if (len_)
	{
		pos(startPos + destLen);
		writeBytes(m_pStorageBuffer->m_storage, startPos + len_, size() - startPos - len_);	   // ���Լ��ռ��п��������ܵ������ݱ�����
	}
	pos(startPos);
	writeBytes((*destBytes), 0, destLen);
	MemOp::freeMem((*destBytes));
}

void MByteBuffer::encrypt(size_t startPos, size_t len_)
{
	EncryptDecryptUtil::encrypt(m_pStorageBuffer->m_storage + startPos, len_ ? len_ : size(), m_pStorageBuffer->m_storage + startPos, m_encryptKey);
}

void MByteBuffer::decrype(size_t startPos, size_t len_)
{
	EncryptDecryptUtil::decrypt(m_pStorageBuffer->m_storage + startPos, len_ ? len_ : size(), m_pStorageBuffer->m_storage + startPos, m_decryptKey);
}