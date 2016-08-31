#pragma once

#include "Lock.h"

namespace Mogui
{
	class CIOCP;
	class CDispatcher;
	class CConnect;

	class CConnectPool : public IConnectPool,public boost::noncopyable
	{
	public:
		enum ConnectPoolState{ CPS_NONE=0, CPS_CALLBACK, CPS_START };

		CConnectPool( void );
		virtual ~CConnectPool( void );

		// ע��ص�
		virtual void SetCallback( IConnectPoolCallback* callback );

		// ��������
		virtual bool Start( int port, int clientcnt, int connectcnt );

		// ���ӽ�ֹ�����IP
		virtual void AddForbidIP( const char* ip );

		// ɾ����ֹ�����IP, ip==0ɾ��ȫ��
		virtual void DelForbidIP( const char* ip );

		// ֹͣ����
		virtual void Stop( void );

		// ���ӳ�ȥ
		virtual IConnect* Connect(const char* ip, int port);

		// �����¼�������true����ʾ���������¼�
		virtual bool OnPriorityEvent( void );

		// ��ʱ������������Ϊ1��
		virtual void OnTimer( void );

		// ���յ�����
		virtual void OnAccept( CConnect* connect );

		// û������callback��IConnect�رյ�ʱ���ص�����¼�,�� nocallback ��ʶ
		virtual void OnClose( CConnect* connect, bool bactive, bool nocallback );

		// û���κ���Ϣ��ʱ��
		void OnIdle( void );

	private:
		CLock                   m_poolLock;
		IConnectPoolCallback*	m_callback;
		CDispatcher*			m_dispatcher;
		CIOCP*					m_iocp;
		int						m_status;
	};
}