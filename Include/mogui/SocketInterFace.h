// 优化：
//	connect的时候最好设置好回调
//	IConnectPool 能自由linsten

#pragma once

#include <string>

namespace Mogui
{
	//服务器端 客户端 通用连接池
	class IConnectPool;				//连接池接口，调用本接口的Start方法之后，主线程将被阻塞，处理socket事件。IConnectPool还提供定时器服务。
	class IConnectPoolCallback;		//IConnectPool的回调接口，服务器程序需要实现本接口来实现定时器，异步消息等功能。

	//服务器模式
	class IConnect;					//连接池处理连接的接口，提供了发送数据包、断开连接的接口。
	class IConnectCallback;			//IConnect回调，实现该接口可以捕获Socket断开和收到数据包消息。	

	// 创建连接池对象
	IConnectPool*	CreateConnectPool( void );

	// 销毁连接池对象
	void			DestoryConnectPool( IConnectPool* ppool );

	// 获取服务器的版本 
	std::string		GetSocketBaseVersion( void );


	//以下是被外部调用的类的接口
	class IConnectPool
	{
	public:
		virtual ~IConnectPool( void ) { }

		// 注册回调
		virtual void SetCallback( IConnectPoolCallback* callback ) = 0;

		// 启动服务
		virtual bool Start( int port, int clientcnt, int connectcnt ) = 0;

		// 增加禁止进入的IP, ip==0无效加入, 空格等符号要首先trim掉
		virtual void AddForbidIP( const char* ip ) = 0;

		// 删除禁止进入的IP, ip==0删除全部, 空格等符号要首先trim掉
		virtual void DelForbidIP( const char* ip ) = 0;

		// 停止服务
		virtual void Stop( void ) = 0;

		// 连接出去
		virtual IConnect* Connect(const char* ip, int port) = 0;
	};

	// IConnectPool 回调
	class IConnectPoolCallback
	{
	public:
		virtual ~IConnectPoolCallback( void ) { };

		// 优先事件，返回true，表示存在优先事件
		virtual bool OnPriorityEvent( void );

		// 定时器触发，周期为1秒
		virtual void OnTimer( void );

		// 没有任何消息的时候，默认处理sleep 1毫秒
		virtual void OnIdle( void );

		// 接收到连接
		virtual void OnAccept( IConnect* connect ) = 0;

		// 没有设置callback的IConnect关闭的时候会回调这个事件
		virtual void OnClose( IConnect* nocallbackconnect, bool bactive ) = 0;
	};

	// 连接 
	class IConnect
	{
	public:
		virtual ~IConnect( void ) { }

		// 主动关闭后 回调IConnectCallback::OnClose事件, bactive=true
		virtual void Close( void ) = 0;

		// 发送协议
		virtual bool Send( const char* buf, int len ) = 0;

		// 设置回调
		virtual void SetCallback( IConnectCallback * callback ) = 0;

		// 取得ip
		virtual std::string GetPeerStringIp( void ) = 0;

		// 取得ip
		virtual long GetPeerLongIp( void ) = 0;

		// 设置每次send最大长度, 可以根据网络情况设置, 范围为 >=512 <=4096, 默认4096
		virtual void SetSendLength( unsigned short length ) = 0;
	};

	// ISocket 回调接口
	class IConnectCallback
	{
	public:
		virtual ~IConnectCallback( void ) { }

		// 设置回调完毕，连接建立成功
		virtual void OnConnect( void ) = 0;

		// 关闭
		virtual void OnClose( bool bactive ) = 0;

		// 收到消息 返回值为处理掉的长度，
		// 如果使用者返回<=0，那么内部将不再回调OnMsg
		// 内部会缓存未被处理掉的数据，但如果内部缓存的剩余<=0 那么内部将不再回调OnMsg
		// 若缓存数据大于一个临界值，当前为4k，就会抛弃这4k的数据
		// 若要处理大于4k的消息包，则使用者自建缓存
		virtual int OnMsg( const char* buf, int len ) = 0;
	};
}