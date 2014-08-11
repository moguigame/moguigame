#pragma once

#include <boost\noncopyable.hpp>

class MoGuiEncrypt  : public boost::noncopyable 
{
public:
	MoGuiEncrypt(void);
	~MoGuiEncrypt(void);

	typedef unsigned char BYTE;

	enum{ KeyLen_16=16,KeyLen_24=24,KeyLen_32=32 };

public:
	void reset();	//重置属性

	bool setAesKey(const BYTE* key, short size);	//size只能取16、24或32
	bool encrypt(BYTE* in, BYTE* out, int size, const BYTE* iv = _gAesIV, int num = 0);	//in,out可以选择用同一个指针
	bool decrypt(BYTE* in, BYTE* out, int size, const BYTE* iv = _gAesIV, int num = 0);	//in,out可以选择用同一个指针

	static const BYTE _gAesIV[16];
	static const BYTE _gKey[32];

private:
	BYTE  m_AesKey[32];
	short m_KeySize;
	bool  m_bSetKey;                    //是否是初始密码
};

