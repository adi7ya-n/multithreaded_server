#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <bitset>
#include <thread>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <functional>
#include <utility>
#include <bits/stdc++.h>
#include <algorithm>
#include <array>
#include <queue>
#include <sstream>
#include <list>

namespace asio = boost::asio;

using tcp = boost::asio::ip::tcp;
using err = boost::system::error_code;

using std::unique_ptr, std::make_unique, std::vector, std::thread, std::mutex,
    std::lock_guard, std::pair, std::make_pair, std::remove_if, std::array,
    std::atomic, std::queue, std::bitset, std::string, std::cout, boost::weak_ptr;

#endif