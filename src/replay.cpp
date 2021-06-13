#include "replay.hpp"
#include <fstream>
#include "utils.hpp"

void Replay::remove_actions_after(float x) {
	const auto check = [&](const Action& action) -> bool {
		return action.x >= x;
	};
	actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
}

void Replay::remove_actions_after(unsigned frame) {
	const auto check = [&](const Action& action) -> bool {
		return action.frame >= frame;
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

/*
 * Format changelog
 *
 * - version 1
 * header | action[]
 * header: "RPLY" | version as u8 | fps as f32
 * action: x as f32 | state as u8
 * 
 * - version 2
 * add `type as u8` between version and fps
 * change action x to be either float or an unsigned int
 */

constexpr uint8_t format_ver = 2;
constexpr const char* format_magic = "RPLY";

void Replay::save(const std::string& path) {
	std::ofstream file;
	file.open(path, std::ios::binary | std::ios::out);
	file << format_magic << format_ver << type;
	bin_write(file, fps);
	for (const auto& action : actions) {
		uint8_t state = static_cast<uint8_t>(action.hold) | static_cast<uint8_t>(action.player2) << 1;
		if (type == ReplayType::XPOS)
			bin_write(file, action.x);
		else if (type == ReplayType::FRAME)
			bin_write(file, action.frame);
		file << state;
	}
	file.close();
}

Replay Replay::load(const std::string& path)  {
	Replay replay(0, ReplayType::XPOS);
	std::ifstream file;
	file.open(path, std::ios::binary | std::ios::in);

	file.seekg(0, std::ios::end);
	size_t file_size = static_cast<size_t>(file.tellg());
	file.seekg(0);

	char magic[4];
	file.read(magic, 4);
	if (memcmp(magic, format_magic, 4) == 0) {
		auto ver = bin_read<uint8_t>(file);
		if (ver == 1 || ver == 2) {
			if (ver == 2) replay.type = ReplayType(bin_read<uint8_t>(file));
			replay.fps = bin_read<float>(file);
			size_t left = file_size - static_cast<size_t>(file.tellg());
			float x;
			unsigned frame;
			for (size_t _ = 0; _ < left / 5; ++_) {
				if (replay.type == ReplayType::XPOS)
					x = bin_read<float>(file);
				else if (replay.type == ReplayType::FRAME)
					frame = bin_read<unsigned>(file);
				auto state = bin_read<uint8_t>(file);
				bool hold = state & 1;
				bool player2 = state & 2;
				Action action = { 0, hold, player2 };
				if (replay.type == ReplayType::XPOS)
					action.x = x;
				else if (replay.type == ReplayType::FRAME)
					action.frame = frame;
				replay.add_action(action);
			}
		}
	} else {
		replay.fps = *cast<float*>(&magic);
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