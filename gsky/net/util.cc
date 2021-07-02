#include <gsky/net/util.hh>

#include <string.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define MAX_BUF_SIZE          0x1000

ssize_t gsky::net::util::read(int fd, void *buffer, size_t length) {
    ssize_t read_left = length;
    ssize_t read_len = 0;
    ssize_t read_sum = 0;
    char *read_ptr = static_cast<char *>(buffer);
    while(read_left > 0) {
        if((read_len = ::read(fd, read_ptr, read_left)) < 0) {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN)  {
                if(read_sum > 0) return read_sum;
                return -1;
            }
            else 
                return -1;
        } else if (read_len == 0) {
            return 0;
        }
        read_sum += read_len;
        read_left -= read_len;
        read_ptr += read_len;
    }
    return read_sum;
}

ssize_t gsky::net::util::read(int fd, gsky::util::vessel &in_buffer) {
    ssize_t read_len = 0;
    ssize_t read_sum = 0;
    char buffer[MAX_BUF_SIZE];

    while(true) {
        if((read_len = ::read(fd, buffer, MAX_BUF_SIZE)) < 0) {
            if(errno == EINTR)
                continue;
            else if (errno == EAGAIN) {
                if(read_sum > 0) return read_sum;
                return -1;
            }
            else
                return -1;
        } else if (read_len == 0) {
            return 0;
        }
        read_sum += read_len;
        in_buffer.append(buffer, read_len);
    }
    return read_sum;
}

ssize_t gsky::net::util::read(int fd, gsky::util::vessel &in_buffer, int length) {
    ssize_t read_len = 0;
    ssize_t read_sum = 0;
    ssize_t read_left = length;
    char buffer[MAX_BUF_SIZE];
    while(true) {
        if((read_len = ::read(fd, buffer, read_left % MAX_BUF_SIZE)) < 0) {
            if(errno == EINTR)
                continue;
            else if (errno == EAGAIN) {
                if(read_sum > 0) return read_sum;
                return -1;
            }
            else
                return -1;
        } else if (read_len == 0) {
            return 0;
        }
        read_sum += read_len;
        read_left -= read_len;
        in_buffer.append(buffer, read_len);
        if(read_left <= 0) {
            break;
        }
    }
    return read_sum;
}

ssize_t gsky::net::util::write(int fd, void *buffer, size_t length) {
    ssize_t write_left = length;
    ssize_t write_len = 0;
    ssize_t write_sum = 0;
    char *write_ptr = static_cast<char *>(buffer);
    while(write_left > 0) {
        if((write_len = ::write(fd, write_ptr, write_left)) < 0) {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN) {
                return write_sum;
            }else {
                return -1;
            }
        }
        write_sum += write_len;
        write_left -= write_len;
        write_ptr += write_len;
    }
    return write_sum;
}

ssize_t gsky::net::util::write(int fd, gsky::util::vessel &out_buffer) {
    ssize_t write_len = 0;
    ssize_t write_sum = 0;
    while(out_buffer.size() > 0) {
        write_len = ::write(fd, out_buffer.data(), out_buffer.size());
        if(write_len < 0) {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN) {
                //std::cout << "EAGAIN\n";
                return write_sum;
            }else {
                return -1;
            }
        }
        write_sum += write_len;
        out_buffer.sub(write_len);
    }
    return write_sum;
}

ssize_t gsky::net::util::write(int fd, std::shared_ptr<gsky::util::vessel> out_buffer) {
    ssize_t write_len = 0;
    ssize_t write_sum = 0;
    while(out_buffer->size() > 0) {
        write_len = ::write(fd, out_buffer->data(), out_buffer->size());
        if(write_len < 0) {
            if(errno == EINTR)
                continue;
            else if(errno == EAGAIN) {
                //std::cout << "EAGAIN\n";
                return write_sum;
            }else {
                return -1;
            }
        }
        write_sum += write_len;
        out_buffer->sub(write_len);
    }
    return write_sum;
}

void gsky::net::util::ignore_sigpipe() {
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
}

bool gsky::net::util::set_fd_nonblocking(int listen_fd) {
    do {
        int flag = fcntl(listen_fd, F_GETFL, 0);
        if(flag == - 1)
            break;
        if(fcntl(listen_fd, F_SETFL, flag | O_NONBLOCK) == -1) {
            break;
        }
        return true;
    } while(false);
    return false;
}

void gsky::net::util::set_fd_nodelay(int fd) {
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof (enable));
}
void gsky::net::util::set_fd_nolinger(int fd) {
    struct linger linger_s;
    linger_s.l_onoff = 1;
    linger_s.l_linger = 30;
    setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger_s, sizeof (linger_s));

}
void gsky::net::util::shutdown_write_fd(int fd) {
    shutdown(fd, SHUT_WR);
}

void gsky::net::util::shutdown_read_fd(int fd) {
    shutdown(fd, SHUT_RD);
}

void gsky::net::util::shutdown_fd(int fd) {
    shutdown(fd, SHUT_RDWR);
}
