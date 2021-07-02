#pragma once

#include <memory>
#include <optional>

namespace h264decpp::readers {

template <typename T>
class reader {
  public:
    virtual ~reader() = default;
    virtual std::optional<T> read() = 0;
};

}