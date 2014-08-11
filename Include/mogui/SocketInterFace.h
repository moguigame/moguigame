// �Ż���
//	connect��ʱ��������úûص�
//	IConnectPool ������linsten

#pragma once

#include <string>

namespace Mogui
{
	//�������� �ͻ��� ͨ�����ӳ�
	class IConnectPool;				//���ӳؽӿڣ����ñ��ӿڵ�Start����֮�����߳̽�������������socket�¼���IConnectPool���ṩ��ʱ������
	class IConnectPoolCallback;		//IConnectPool�Ļص��ӿڣ�������������Ҫʵ�ֱ��ӿ���ʵ�ֶ�ʱ�����첽��Ϣ�ȹ��ܡ�

	//������ģʽ
	class IConnect;					//���ӳش������ӵĽӿڣ��ṩ�˷������ݰ����Ͽ����ӵĽӿڡ�
	class IConnectCallback;			//IConnect�ص���ʵ�ָýӿڿ��Բ���Socket�Ͽ����յ����ݰ���Ϣ��	

	// �������ӳض���
	IConnectPool*	CreateConnectPool( void );

	// �������ӳض���
	void			DestoryConnectPool( IConnectPool* ppool );

	// ��ȡ�������İ汾 
	std::string		GetSocketBaseVersion( void );


	//�����Ǳ��ⲿ���õ���Ľӿ�
	class IConnectPool
	{
	public:
		virtual ~IConnectPool( void ) { }

		// ע��ص�
		virtual void SetCallback( IConnectPoolCallback* callback ) = 0;

		// ��������
		virtual bool Start( int port, int clientcnt, int connectcnt ) = 0;

		// ���ӽ�ֹ�����IP, ip==0��Ч����, �ո�ȷ���Ҫ����trim��
		virtual void AddForbidIP( const char* ip ) = 0;

		// ɾ����ֹ�����IP, ip==0ɾ��ȫ��, �ո�ȷ���Ҫ����trim��
		virtual void DelForbidIP( const char* ip ) = 0;

		// ֹͣ����
		virtual void Stop( void ) = 0;

		// ���ӳ�ȥ
		virtual IConnect* Connect(const char* ip, int port) = 0;
	};

	// IConnectPool �ص�
	class IConnectPoolCallback
	{
	public:
		virtual ~IConnectPoolCallback( void ) { };

		// �����¼�������true����ʾ���������¼�
		virtual bool OnPriorityEvent( void );

		// ��ʱ������������Ϊ1��
		virtual void OnTimer( void );

		// û���κ���Ϣ��ʱ��Ĭ�ϴ���sleep 1����
		virtual void OnIdle( void );

		// ���յ�����
		virtual void OnAccept( IConnect* connect ) = 0;

		// û������callback��IConnect�رյ�ʱ���ص�����¼�
		virtual void OnClose( IConnect* nocallbackconnect, bool bactive ) = 0;
	};

	// ���� 
	class IConnect
	{
	public:
		virtual ~IConnect( void ) { }

		// �����رպ� �ص�IConnectCallback::OnClose�¼�, bactive=true
		virtual void Close( void ) = 0;

		// ����Э��
		virtual bool Send( const char* buf, int len ) = 0;

		// ���ûص�
		virtual void SetCallback( IConnectCallback * callback ) = 0;

		// ȡ��ip
		virtual std::string GetPeerStringIp( void ) = 0;

		// ȡ��ip
		virtual long GetPeerLongIp( void ) = 0;

		// ����ÿ��send��󳤶�, ���Ը��������������, ��ΧΪ >=512 <=4096, Ĭ��4096
		virtual void SetSendLength( unsigned short length ) = 0;
	};

	// ISocket �ص��ӿ�
	class IConnectCallback
	{
	public:
		virtual ~IConnectCallback( void ) { }

		// ���ûص���ϣ����ӽ����ɹ�
		virtual void OnConnect( void ) = 0;

		// �ر�
		virtual void OnClose( bool bactive ) = 0;

		// �յ���Ϣ ����ֵΪ������ĳ��ȣ�
		// ���ʹ���߷���<=0����ô�ڲ������ٻص�OnMsg
		// �ڲ��Ỻ��δ������������ݣ�������ڲ������ʣ��<=0 ��ô�ڲ������ٻص�OnMsg
		// ���������ݴ���һ���ٽ�ֵ����ǰΪ4k���ͻ�������4k������
		// ��Ҫ�������4k����Ϣ������ʹ�����Խ�����
		virtual int OnMsg( const char* buf, int len ) = 0;
	};
}