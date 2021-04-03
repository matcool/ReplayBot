#include "replay.hpp"
#include <fstream>
#include "utils.hpp"

void Replay::remove_actions_after(float x) {
	auto check = [&](const Action& action) -> bool {
		return action.x >= x;
	};
	actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
}

// these two functions are so stupid
// why cant c++ do it for me :(

template <typename T>
inline void bin_write(std::ofstream& stream, T value) {
	stream.write(cast<char*>(&value), sizeof(T));
}

template <typename T>
inline T bin_read(std::ifstream& stream) {
	T value;
	stream.read(cast<char*>(&value), sizeof(T));
	return value;
}

constexpr uint8_t format_ver = 1;
constexpr const char format_magic[5] = "RPLY";

void Replay::save(const std::string& path) {
	std::ofstream file;
	file.open(path, std::ios::binary | std::ios::out);
	file << format_magic << format_ver;
	bin_write(file, fps);
	for (const auto& action : actions) {
		uint8_t state = action.hold | action.player2 << 1;
		bin_write(file, action.x);
		file << state;
	}
	file.close();
}

Replay Replay::load(const std::string& path)  {
	Replay replay(0);
	std::ifstream file;
	file.open(path, std::ios::binary | std::ios::in);

	file.seekg(0, std::ios::end);
	size_t file_size = static_cast<size_t>(file.tellg());
	file.seekg(0);

	char magic[4];
	file.read(magic, 4);
	if (memcmp(magic, format_magic, 4) == 0) {
		auto ver = bin_read<uint8_t>(file);
		if (ver == 1) {
			replay.fps = bin_read<float>(file);
			size_t left = file_size - static_cast<size_t>(file.tellg());
			for (size_t _ = 0; _ < left / 5; ++_) {
				float x = bin_read<float>(file);
				auto state = bin_read<uint8_t>(file);
				bool hold = state & 1;
				bool player2 = state & 2;
				replay.add_action({ x, hold, player2 });
			}
		}
	} else {
		replay.fps = *reinterpret_cast<float*>(&magic);
		size_t left = file_size - static_cast<size_t>(file.tellg());
		for (size_t _ = 0; _ < left / 6; ++_) {
			float x = bin_read<float>(file);
			bool hold = bin_read<bool>(file);
			bool player2 = bin_read<bool>(file);
			replay.add_action({ x, hold, player2 });
		}
	}
	file.close();

	return std::move(replay);
}