//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include "Server/server.hpp"

#include <filesystem>

int main([[maybe_unused]] int argc,[[maybe_unused]]  char* argv[]){
    std::cout << std::filesystem::current_path() << '\n' <<std::flush;
    try
    {
        http::server::server s("192.168.10.41", "8080", "html/");

        s.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}