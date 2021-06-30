#include "net.hh"
extern gsky::util::firewall *gsky::data::firewall;

gsky::net::net::net(int port,int number_of_thread) :
    started_(false),
    listened_(false),
    port_(port),
    number_of_thread_(number_of_thread),
    base_eventloop_(new eventloop) {
    base_eventloop_->set_name("base_eventloop");
}

gsky::net::net::net() :
    started_(false),
    listened_(false),
    base_eventloop_(new eventloop){

}
gsky::net::net::~net() {
    //std::cout << "~net()";
    delete base_eventloop_;
};

void gsky::net::net::init(int port, int number_of_thread) {
    port_ = port;
    number_of_thread_ = number_of_thread;
}

void gsky::net::net::start() {
    listen_fd = listen();
    if(listen_fd == -1) {
        d_cout << "net init fail\n";
        logger() << "sys: net init fail\n";
        abort();
    }
    listened_ = true;
    util::ignore_sigpipe();
    if(!util::set_fd_nonblocking(listen_fd)) {
        d_cout << "set fd nonblocking error\n";
        logger() << "sys: set fd nonblocking error\n";
        abort();
    }
    //base_eventloop_thread_ = sp_eventloop_thread(new eventloop_thread);

    accept_channel_ = sp_channel(new channel(base_eventloop_));
    accept_channel_->set_fd(listen_fd);
    up_eventloop_threadpool_ = std::unique_ptr<eventloop_threadpool>(new eventloop_threadpool(base_eventloop_, number_of_thread_));
    up_eventloop_threadpool_->start();

    accept_channel_->set_event(EPOLLIN | EPOLLET); // Set as accept data event
    accept_channel_->set_read_handler(std::bind(&net::handle_new_connection, this));
    accept_channel_->set_reset_handler(std::bind(&net::handle_reset, this));
    base_eventloop_->add_to_epoll(accept_channel_);
    started_ = true;
    base_eventloop_->loop(); //loop
}

int gsky::net::net::listen() {
    if(port_ < 0 || port_ > 65535) {
        d_cout << "listen port is not right\n";
        logger() << "listen port is not right\n";
        return -1;
    }
    int listen_fd = -1;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        return -1;
    }
    // cancel bind show "Address already in use" err
    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        close(listen_fd);
        perror("setsockopt: ");
        return -1;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(static_cast<uint16_t>(port_));

    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof (server_addr)) == -1) {
        close(listen_fd);
        perror("bind: ");
        return -1;
    }

    if(::listen(listen_fd, SOMAXCONN) == -1) {
        close(listen_fd);
        perror("listen: ");
        return -1;
    }
    return listen_fd;
}

void gsky::net::net::handle_new_connection() {
    struct sockaddr_in client_sockaddr;
    bzero(&client_sockaddr, sizeof (client_sockaddr));
    socklen_t client_sockaddr_len = sizeof (client_sockaddr);
    int accept_fd = 0;
    while((accept_fd = accept(listen_fd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_len)) > 0) {
        //d_cout << "new connection: " << inet_ntoa(client_sockaddr.sin_addr) << " : " << ntohs(client_sockaddr.sin_port) << "\n";
        // If the number of accept fd is greater than MAX_CONNECTED_FDS_NUM wiil be closed
        std::string client_ip = inet_ntoa(client_sockaddr.sin_addr);
        std::string client_port = std::to_string(ntohs(client_sockaddr.sin_port));
        if(gsky::data::firewall->wall(accept_fd, client_ip)) {
            //std::cout << "forbiden: " << ntohs(client_sockaddr.sin_port) << '\n';
            logger() << "FORBID IP: " + client_ip + ":" +  client_port;
            continue;
        }
        logger() << "-----------------------NEW CONNECTED CLIENT: " + client_ip + ":" + client_port + "------------------------";
        if(!util::set_fd_nonblocking(accept_fd)) {
            d_cout << "set fd nonblocking error\n";
        }
        //set as no delay
        util::set_fd_nodelay(accept_fd);
        // add event to deal with
        //d_cout << "handle new connection\n";
        eventloop *next_eventloop = up_eventloop_threadpool_->get_next_eventloop();
        sp_psp sph(new psp(accept_fd, next_eventloop));
        sph->set_client_info(client_ip, client_port);
        sph->get_sp_channel()->set_holder(sph);
        next_eventloop->push_back(std::bind(&psp::new_evnet, sph));
    }
    accept_channel_->set_event(EPOLLIN | EPOLLET);
}

void gsky::net::net::stop() {
    base_eventloop_->quit();
    up_eventloop_threadpool_->stop();
	std::cout << "stop net module\n";
}

void gsky::net::net::handle_reset() {
    //d_cout << "HandleConnected\n";
    base_eventloop_->update_epoll(accept_channel_);
}
