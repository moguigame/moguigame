#include "MoGuiEncrypt.h"

#include <string>
extern "C"
{
#include "openssl/aes.h"
#include "openssl/des.h"
#include "openssl/md5.h"
}
#pragma comment(lib,"libeay32.lib")

const unsigned char MoGuiEncrypt::_gAesIV[16] = {0x15,0xFF,0x01,0x00,0x34,0xAB,0x4C,0xD3,0x55,0xFE,0xA1,0x22,0x08,0x4F,0x13,0x07};
const unsigned char MoGuiEncrypt::_gKey[32] =   {0xF3,0x62,0x12,0x05,0x13,0xE3,0x89,0xFF,0x23,0x11,0xD7,0x36,0x01,0x23,0x10,0x07,0x05,0xA2,0x10,0x00,0x7A,0xCC,0x02,0x3C,0x39,0x01,0xDA,0x2E,0xCB,0x12,0x44,0x8B };

MoGuiEncrypt::MoGuiEncrypt(void)
{
	reset();
}
MoGuiEncrypt::~MoGuiEncrypt(void)
{
}
void MoGuiEncrypt::reset()
{
	memcpy(m_AesKey,_gKey,32);
	m_bSetKey = false;
	m_KeySize = 32;
}
bool MoGuiEncrypt::setAesKey(const BYTE* key, short size)
{
	if( size <= 32 )
	{
		m_bSetKey = true;
		m_KeySize = size;
		memcpy(m_AesKey, key, size);
		return true;
	}
	return false;
}

bool MoGuiEncrypt::encrypt(BYTE* in, BYTE* out, int size, const BYTE* iv, int num)
{
	BYTE ivec[16];
	memcpy(ivec, iv, sizeof(ivec));

	//std::cout<<"encrypt pre "<<Tool::MemoryToString(in,size)<<endl;

	AES_KEY key;
	AES_set_encrypt_key(m_AesKey, m_KeySize<<3, &key);
	AES_cfb128_encrypt(in, out, size, &key, ivec, &num, AES_ENCRYPT);

	//std::cout<<"encrypt end "<<Tool::MemoryToString(out,size)<<endl;

	return true;
}

bool MoGuiEncrypt::decrypt(BYTE* in, BYTE* out, int size, const BYTE* iv, int num)
{
	BYTE ivec[16];
	memcpy(ivec, iv, sizeof(ivec));

	//std::cout<<"decrypt pre "<<Tool::MemoryToString(in,size)<<endl;

	AES_KEY key;
	AES_set_encrypt_key(m_AesKey, m_KeySize<<3, &key);
	AES_cfb128_encrypt(in, out, size, &key, ivec, &num, AES_DECRYPT);

	//std::cout<<"decrypt end "<<Tool::MemoryToString(out,size)<<endl;

	return true;
}