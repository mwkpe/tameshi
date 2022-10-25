/* A small test program sending UDP packets
 */


#include <cstdint>
#include <csignal>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <iostream>
#include "cxxopts.hpp"
#include "udpsocket.h"


struct Options
{
  std::string remote_ip;
  std::uint16_t data_port;
  std::uint16_t delay_time;
};


volatile std::sig_atomic_t signal_status;


void signal_handler(int signal)
{
  signal_status = signal;
  
  if (signal_status == SIGTERM) {
    std::cout << "Termination signal received" << std::endl;
  }
}


void send_udp(udp::Socket& udp_socket, std::atomic<bool>& stop, std::uint16_t delay_time)
{
  std::vector<std::uint8_t> data{0, 1, 2, 3, 4, 5, 6, 7};
  while (!stop.load()) {
    udp_socket.transmit(data);
    std::this_thread::sleep_for(std::chrono::milliseconds{delay_time});
  }
}


Options parse_args(int argc, char** argv)
{
  Options options{};

  try {
    cxxopts::Options cli_options{"tameshi", "UDP test sender"};
    cli_options.add_options()
      ("i,ip", "Remote device IP", cxxopts::value<std::string>(options.remote_ip))
      ("p,port", "UDP data port", cxxopts::value<std::uint16_t>(options.data_port))
      ("d,delay", "Delay time between packets", cxxopts::value<std::uint16_t>(options.delay_time)
          ->default_value("1000"))
    ;
    cli_options.parse(argc, argv);

    if (cli_options.count("ip") == 0) {
      throw std::runtime_error{"Remote IP must be specified, use the -i or --ip option"};
    }
    if (cli_options.count("port") == 0) {
      throw std::runtime_error{"UDP port must be specified, use the -p or --port option"};
    }

    return options;
  }
  catch (const cxxopts::OptionException& e) {
    throw std::runtime_error{e.what()};
  }
}


int main(int argc, char** argv)
{
  std::signal(SIGTERM, signal_handler);

  Options options;
  udp::Socket udp_socket;

  try {
    options = parse_args(argc, argv);
    udp_socket.open(options.remote_ip, options.data_port);  // Transmit frames to remote device
  }
  catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "Sending UDP packets to " << options.remote_ip << ":" << options.data_port << std::endl;

  std::atomic<bool> stop{false};
  auto sender = std::thread{&send_udp, std::ref(udp_socket), std::ref(stop), options.delay_time};

  while (signal_status != SIGTERM) {
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
  }

  std::cout << "Stopping UDP test sender..." << std::endl;
  stop.store(true);
  if (sender.joinable())
    sender.join();

  std::cout << "Program finished" << std::endl;
  return 0;
}
