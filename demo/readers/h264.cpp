#include "h264.hpp"

#include "file.hpp"

namespace h264decpp::readers {

h264::h264(std::string const & input_filename)
  : h264(std::make_shared<file>(input_filename))
{}

h264::h264(std::shared_ptr<reader<uint8_t>> input)
  : m_input(std::move(input))
  , m_frame_index(0)
{}

std::optional<std::span<uint8_t>> h264::read() {
    m_frame.erase(m_frame.begin(), m_frame.begin() + m_frame_index);
    m_frame_index = 3;
    while (consume()) {
        if (m_frame.size() < 7) continue;
        auto back = *reinterpret_cast<uint32_t *>(&m_frame[m_frame_index]);
        if (0x00010000 == (back & 0x00FFFFFF)) return std::span{m_frame.data(), m_frame_index};
        if (0x01000000 == back) return std::span{m_frame.data(), m_frame_index};
        m_frame_index++;
    }
    m_frame_index = m_frame.size();
    if (0 == m_frame_index) {
        return std::nullopt;
    }
    return std::span{m_frame.data(), m_frame_index};
}

bool h264::consume() {
    auto c = m_input->read();
    if (!c) return false;
    m_frame.push_back(*c);
    return true;
}

}