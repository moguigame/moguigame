#pragma once

#include "MemoryPool.h"
#include "SocketDefine.h"

namespace Mogui{

	class CConnect;

	class CPacket : public CMemoryPool_Public<CPacket>{
	public:
		enum PacketType{
			PT_DATA	= 0,
			PT_ACCEPT,
			PT_CONNECT,
			PT_CLOSE_ACTIVE,
			PT_CLOSE_PASSIVE,
			PT_SOCKET_REUSE,
		};

		CPacket( void ) : m_next( 0 ), m_socket( 0 ), m_used( 0 ), m_type( 0 )
			, m_callback( 0 ),m_starttick(0){		
			memset(m_buffer,0,sizeof(m_buffer));
		}

		CPacket*		    m_next;
		CConnect*		    m_socket;
		unsigned int	    m_used;
		int				    m_type;
		IConnectCallback*   m_callback;
		char			    m_buffer[_MAX_BUFFER_LENGTH];
		long long           m_starttick;
	};

	class CPacketQueue
	{
	public:
		CPacketQueue( void ) : m_head( 0 ), m_tail( 0 ), m_InPacket( 0 ), m_OutPacket( 0 ){
		}
		~CPacketQueue( void ){
			Clear( );
			Init();
		}

		void Init(){
			m_InPacket = 0;
			m_OutPacket = 0;

			m_head = 0;
			m_tail = 0;
		}

		void PushPacket( CPacket* packet ){
			if ( packet ){
				packet->m_next      = 0;
				packet->m_starttick = GetTickCount64();

				if ( m_tail ){
					m_tail->m_next = packet;
					m_tail         = packet;
				}
				else{
					m_head = m_tail = packet;
				}
				m_InPacket++;
			}
		}

		void PushQueue( CPacketQueue& queue ){
			if ( queue.IsEmpty() ){
				return;
			}

			if ( m_tail ){
				m_tail->m_next = queue.m_head;
				m_tail		   = queue.m_tail;
			}
			else{
				m_head	= queue.m_head;;
				m_tail  = queue.m_tail;
			}

			m_InPacket += queue.Size();
			queue.Init();
		}

		CPacket* PopPacket( void ){
			if ( m_head==0 ){
				return 0;
			}

			CPacket* packet = m_head;
			m_head	= packet->m_next;
			packet->m_next	= 0;
			m_OutPacket++;

			if( m_tail==packet ){
				m_tail = 0;
			}
			return packet;
		}

		bool IsEmpty( void ){
			return m_head==0;
		}

		void DeleteClearAll( void ){
			CPacket* packet = 0;
			while ( (packet=PopPacket()) ){
				delete packet;
			}
			assert( m_InPacket == m_OutPacket);
		}

		void Clear( void ){
			CPacket* packet = 0;
			while ( (packet=PopPacket()) ){
				delete packet;
			}
			assert( m_InPacket == m_OutPacket);
		}

		int Size( void ){
			assert(m_InPacket - m_OutPacket>=0);
			return int(m_InPacket - m_OutPacket);
		}

		long long GetInPakcet(){ return m_InPacket; }
		long long GetOutPacket(){ return m_OutPacket; }

	private:
		CPacket*	m_head;
		CPacket*	m_tail;

		long long   m_InPacket;
		long long   m_OutPacket;
	};
}

