#include "../include/Acceptor.hpp"

Acceptor::Acceptor(EventLoop* loop, const std::string& ip, const uint16_t port) : _M_loop_ptr(loop)
{
    // 初始化serv_sock的地址
    InetAddress serv_addr(ip, port);

    // 初始化内部serv_sock为非阻塞
    _M_serv_sock_ptr = new Socket(create_non_blocking_fd());

    // 设置serv_sock的属性
    _M_serv_sock_ptr->set_keep_alive(true);
    _M_serv_sock_ptr->set_reuse_addr(true);
    _M_serv_sock_ptr->set_reuse_port(true);
    _M_serv_sock_ptr->set_tcp_nodelay(true);

    // 绑定并开始监听
    _M_serv_sock_ptr->bind(serv_addr);
    _M_serv_sock_ptr->listen();


    // 使用acceptor_channel_ptr将serv_fd和ep绑定在一起
    _M_acceptor_channel_ptr = new Channel(_M_loop_ptr, _M_serv_sock_ptr->get_fd());
    // 添加读事件, 并且监听
    _M_acceptor_channel_ptr->set_read_events();
    // 设置acceptor_channel_ptr的执行函数为new_connection
    _M_acceptor_channel_ptr->set_read_callback(std::bind(&Acceptor::new_connection, this));

}

void Acceptor::set_create_connection_callback(std::function<void(Socket*)> func) {
    create_connection_callback = func;
} 

void Acceptor::new_connection()
{
    InetAddress clnt_addr;
    Socket* clnt_sock_ptr = new Socket(_M_serv_sock_ptr->accept(clnt_addr));

    // 通过回调函数将创建好的clnt_sock传递给TcpServer, 让TcpServer创建Connection对象
    create_connection_callback(clnt_sock_ptr);
}

Acceptor::~Acceptor()
{
    delete _M_serv_sock_ptr;
    delete _M_acceptor_channel_ptr;
}