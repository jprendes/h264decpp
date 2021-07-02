#pragma once

#include "reader.hpp"

#include <memory>
#include <span>
#include <vector>

namespace h264decpp::readers {

class h264 : public reader<std::span<uint8_t>> {
  public:
    h264(std::string const & input_filename);
    h264(std::shared_ptr<reader<uint8_t>> input);

    std::optional<std::span<uint8_t>> read() override;

  private:
    std::shared_ptr<reader<uint8_t>> m_input;
    std::vector<uint8_t> m_frame;
    size_t m_frame_index;

    bool consume();
};

}