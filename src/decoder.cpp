#include <cstring>
#include <h264decpp/decoder.hpp>

#include <memory>
#include <optional>
#include <stdexcept>
#include <span>
#include <vector>

extern "C" {

#include <libavcodec/avcodec.h>

}

extern "C" {

extern AVCodec ff_h264_decoder;
extern AVCodecParser ff_h264_parser;

}

namespace {

template<typename Fcn>
class OnCleanup {
  private:
    Fcn m_fcn;
  public:
    OnCleanup(Fcn const & fcn) : m_fcn(fcn) {}
    OnCleanup(Fcn && fcn) : m_fcn(std::move(fcn)) {}
    ~OnCleanup() { try { m_fcn(); } catch (...) {} }
};

}

namespace h264decpp {

struct decoder::impl {
  public:
    impl() {
        avcodec_register(&ff_h264_decoder);
        av_register_codec_parser(&ff_h264_parser);
    }

    void init() {
        m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!m_codec) {
            throw std::runtime_error("Codec not found");
        }

        m_codec_ctx = avcodec_alloc_context3(m_codec);
        if (!m_codec_ctx) {
            throw std::runtime_error("Could not allocate video codec context");
        }
        
        if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
            throw std::runtime_error("Could not open codec");
        }
        
        m_parser = av_parser_init(AV_CODEC_ID_H264);
        if(!m_parser) {
            throw std::runtime_error("Could not create H264 parser");
        }

        m_frame = av_frame_alloc();
        if (!m_frame) {
            throw std::runtime_error("Could not allocate video frame");
        }
    }

    ~impl() {
        if (m_frame) {
            av_frame_free(&m_frame);
        }

        if (m_parser) {
            av_parser_close(m_parser);
        }

        if (m_codec_ctx) {
            avcodec_close(m_codec_ctx);
            av_free(m_codec_ctx);
        }
    }

    std::optional<frame> decode(std::span<uint8_t> input) {
        m_buffer.insert(m_buffer.end(), input.begin(), input.end());

        uint8_t* data = NULL;
        int size = 0;

        int bytes_used = av_parser_parse2(m_parser, m_codec_ctx, &data, &size, m_buffer.data(), m_buffer.size(), 0, 0, AV_NOPTS_VALUE);

        if (size == 0) {
            return std::nullopt;
        }

        if (bytes_used > 0) {
            av_init_packet(&m_packet);
            m_packet.data = data;
            m_packet.size = size;

            auto consume_buffer = OnCleanup([&] {
                m_buffer.erase(m_buffer.begin(), m_buffer.begin() + bytes_used);
            });

            return decode_frame();
        }

        return std::nullopt;
    }

    std::optional<frame> flush() {
        uint8_t* data = NULL;
        int size = 0;

        int bytes_used = av_parser_parse2(m_parser, m_codec_ctx, &data, &size, m_buffer.data(), m_buffer.size(), 0, 0, AV_NOPTS_VALUE);

        av_init_packet(&m_packet);
        m_packet.data = size ? data : NULL;
        m_packet.size = size;

        auto consume_buffer = OnCleanup([&] {
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + bytes_used);
        });

        return decode_frame();
    }

  private:
    AVCodec *m_codec = nullptr;
    AVCodecContext *m_codec_ctx = nullptr;
    AVCodecParserContext* m_parser = nullptr;
    AVFrame *m_frame = nullptr;
    AVPacket m_packet;

    std::vector<uint8_t> m_buffer;
    std::vector<uint8_t> m_frame_buffer;

    std::optional<frame> decode_frame() {
        int got_frame = 0;
        if (avcodec_decode_video2(m_codec_ctx, m_frame, &got_frame, &m_packet) < 0) {
            throw std::runtime_error("Error while decoding frame");
        }
        if (got_frame) {
            size_t width = m_frame->width;
            size_t height = m_frame->height;
            auto half_width = width / 2;
            auto half_height = height / 2;

            auto data = m_frame->data;
            auto stride = m_frame->linesize;

            m_frame_buffer.resize(width * height + 2 * half_width * half_height);

            auto ptr = m_frame_buffer.data();
            for (size_t i = 0; i < height; ++i) {
                memcpy(ptr, data[0] + i * stride[0], width);
                ptr += width;
            }
            for (size_t i = 0; i < half_height; ++i) {
                //memset(ptr, 0, half_width);
                memcpy(ptr, data[1] + i * stride[1], half_width);
                ptr += half_width;
            }
            for (size_t i = 0; i < half_height; ++i) {
                //memset(ptr, 0, half_width);
                memcpy(ptr, data[2] + i * stride[2], half_width);
                ptr += half_width;
            }

            return frame{ m_frame_buffer, width, height };
        }

        return std::nullopt;
    }
};

decoder::decoder() : m_impl(std::make_unique<impl>()) {
    m_impl->init();
}

decoder::~decoder() {}

std::optional<decoder::frame> decoder::decode(std::span<uint8_t> input) {
    return m_impl->decode(input);
}

std::optional<decoder::frame> decoder::flush() {
    return m_impl->flush();
}

}