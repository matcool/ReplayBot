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
		return action.x > x;
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
	std::cout << "n actions " << actions.size() << std::endl;
	for (auto& action : actions) {
		binWrite(&outfile, action.x);
		binWrite(&outfile, action.hold);
		binWrite(&outfile, action.player2);
	}
	outfile.close();
}

Replay::Replay(const char* path) {
	std::ifstream infile;
	infile.open(path, std::ios::binary | std::ios::in);
	this->fps = binRead<float>(&infile);
	while (!infile.eof()) {
		Action action = { binRead<float>(&infile), binRead<bool>(&infile), binRead<bool>(&infile) };
		// std::cout << "read action x=" << action.x << " hold=" << action.hold << " player2=" << action.player2 << std::endl;
		this->actions.push_back(action);
	}
	// FIX: for some reason it always reads garbage data at the end
	if (this->actions.back().hold > 1) {
		std::cout << "Last action had hold=" << this->actions.back().hold << ", removing it" << std::endl;
		this->actions.pop_back();
	}
	infile.close();
}