#include "Replay.h"
#include <iostream>
#include <fstream>

Replay::Replay(float fps) : fps(fps) {}

void Replay::addAction(Action action) {
	// std::cout << "Action: hold=" << action.hold << " player2=" << action.player2 << " x=" << action.x << std::endl;
	actions.push_back(action);
}

void Replay::removeActionsAfterX(float x) {
	auto check = [&](Action action) -> bool {
		return action.x >= x;
	};
	actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
}

template <typename T>
void binWrite(std::ofstream* stream, T val) {
	stream->write(reinterpret_cast<char*>(&val), sizeof(T));
}

template <typename T>
T binRead(std::ifstream* stream) {
	T val;
	stream->read(reinterpret_cast<char*>(&val), sizeof(T));
	return val;
}

void Replay::save(const char* path) {
	std::ofstream outfile;
	outfile.open(path, std::ios::binary | std::ios::out);
	binWrite(&outfile, fps);
	// std::cout << "n actions " << actions.size() << std::endl;
	for (auto& action : actions) {
		binWrite(&outfile, action.x);
		binWrite(&outfile, action.hold);
		binWrite(&outfile, action.player2);
	}
	outfile.close();
}

std::shared_ptr<Replay> Replay::load(const char* path) {
	std::ifstream infile;
	infile.open(path, std::ios::binary | std::ios::in);
	infile.seekg(0, std::ios::end);
	// this apparently can be inaccurate sometimes but i haven't seen it happen
	auto size = static_cast<long>(infile.tellg());
	infile.seekg(0);
	auto fps = binRead<float>(&infile);
	auto replay = std::make_shared<Replay>(fps);
	for (long _i = 0; _i < (size - 4) / 6; ++_i) {
		Action action = { binRead<float>(&infile), binRead<bool>(&infile), binRead<bool>(&infile) };
		replay->addAction(action);
	}
	infile.close();
	return replay;
}