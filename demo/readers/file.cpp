#include "file.hpp"

namespace h264decpp::readers {

file::file(std::string const & file, size_t buffer_size)
    : input(file.c_str())
    , buffer_index(0)
{
    buffer.reserve(buffer_size);
}

std::optional<uint8_t> file::read() {
    if (buffer_index == buffer.size()) {
        auto n = input.readsome(reinterpret_cast<char*>(buffer.data()), buffer.size());
        if (n == 0) {
            if (input.eof() || !input.good()) {
                return std::nullopt;
            }
            input.read(reinterpret_cast<char*>(buffer.data()), 1);
            n = 1;
        }
        buffer.resize(n);
        buffer_index = 0;
    }
    return buffer[buffer_index++];
}

}