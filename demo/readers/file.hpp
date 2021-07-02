#pragma once

#include "reader.hpp"
#include <fstream>
#include <optional>
#include <vector>
#include <string>

namespace h264decpp::readers {

class file : public reader<uint8_t> {
  public:
    file(std::string const & file, size_t buffer_size = 1024);

    std::optional<uint8_t> read() override;

  private:
    std::ifstream input;
    std::vector<uint8_t> buffer;
    size_t buffer_index;
};

}