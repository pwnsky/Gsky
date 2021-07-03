#pragma once
#include <cstdlib>
#include <string>
#include <memory>

#include <gsky/gsky.hh>
#include <gsky/util/vessel.hh>

namespace gsky{
namespace net {
namespace util {
ssize_t read(int fd, void *buffer, size_t length);
ssize_t read(int fd, gsky::util::vessel &in_buffer);
ssize_t read(int fd, gsky::util::vessel &in_buffer, int length);

ssize_t write(int fd, void *buffer, size_t length);
ssize_t write(int fd, gsky::util::vessel &out_buffer);
ssize_t write(int fd, std::shared_ptr<gsky::util::vessel> out_buffer);
void ignore_sigpipe();                  //avoid server terminate with SIGPIPE signal
bool set_fd_nonblocking(int listen_fd); //set fd as non bloking
void set_fd_nodelay(int fd);            //set fd no delay
void set_fd_nolinger(int fd);           //set fd no linger
void shutdown_write_fd(int fd);         //shutdown fd of write
void shutdown_read_fd(int fd);
void shutdown_fd(int fd);

}}}

