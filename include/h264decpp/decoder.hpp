#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace h264decpp {

class decoder {
  public:
    decoder();
    ~decoder();

    struct frame {
      public:
        std::span<uint8_t const> buffer;
        size_t width, height;
    };

    std::optional<frame const> decode(std::span<uint8_t> input);
    std::optional<frame const> flush();

  private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

}