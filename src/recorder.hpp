#pragma once

#include "includes.h"
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include "../libraries/subprocess.hpp"
#include <string>

using u8 = uint8_t;

template <typename T>
class dynamic_array {
    size_t m_size;
    T* m_data;
public:
    dynamic_array(const size_t size) : m_size(size) {
        m_data = new T[m_size]();
    }
    // dynamic_array(const dynamic_array& other) {
    //     std::cout << "copying other " << other.size() << std::endl;
    //     m_size = other.size();
    //     m_data = new T[m_size]();
    //     for (size_t i = 0; i < size(); ++i)
    //         this[i] = other[i];
    // }
    ~dynamic_array() {
        delete[] m_data;
    }
    T& operator[](const size_t index) { return m_data[index]; }
    const T& operator[](const size_t index) const { return m_data[index]; }
    const auto size() const { return m_size; }
    auto data() { return m_data; }
    const auto data() const { return m_data; }
};

class MyRenderTexture {
public:
    unsigned m_width, m_height;
    int m_old_fbo, m_old_rbo;
    unsigned m_fbo;
    CCTexture2D* m_texture;
    void begin();
    void end();
    std::vector<u8> capture();
};

class Recorder {
public:
    Recorder();
    std::queue<std::vector<u8>> m_frames;
    std::mutex m_lock;
    MyRenderTexture m_renderer;
    unsigned m_width, m_height;
    unsigned m_fps;
    bool m_recording = false;
    double m_last_frame_t, m_extra_t;
    bool m_until_end = true;
    std::string m_codec = "", m_bitrate = "30M", m_extra_args = "";

    void start(const std::string& path);
    void stop();
    void capture_frame();
};