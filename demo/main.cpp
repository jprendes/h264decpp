#include "readers/h264.hpp"

#include <h264decpp/decoder.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

using namespace h264decpp;

namespace {

void h264_video_decode(std::string const & input_file, std::string const & output_file) {
	std::cout << "Decode file '" << input_file << "' to '" << output_file << "'" << std::endl;

	auto input = readers::h264(input_file);
	std::ofstream output(output_file);

	decoder dec;
	
	size_t total_frames = 0;
	auto start = std::chrono::steady_clock::now();

	while (auto nal = input.read()) {
		if (auto frame = dec.decode(*nal)) {
			total_frames++;
			output.write(reinterpret_cast<char const *>(frame->buffer.data()), frame->buffer.size());
		}
	}
	while (auto frame = dec.flush()) {
		total_frames++;
		output.write(reinterpret_cast<char const *>(frame->buffer.data()), frame->buffer.size());
	}

	auto end = std::chrono::steady_clock::now();

	printf("Done\n");

	auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	float elapsed = dt.count() / 1000.0;
	float speed = (total_frames + 1) / elapsed;
	std::cout << "Decoding time: " << elapsed << "s, speed: " << speed << " fps" << std::endl;
}

}

int main(int argc, char* argv[]) {
	if (argc == 3) {
		h264_video_decode(argv[1], argv[2]);
	} else {
		std::cout << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
	}
	return 0;		
}
