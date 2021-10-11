#include "recorder.hpp"
#include <sstream>
#include <CCGL.h>
#include <filesystem>
#include <fstream>

Recorder::Recorder() : m_width(1280), m_height(720), m_fps(60) {}

void Recorder::start(const std::string& path) {
#ifndef SHOW_CONSOLE
    static bool has_console = false;
    if (!has_console) {
        has_console = true;
        AllocConsole();
        static std::ofstream conout("CONOUT$", std::ios::out);
        std::cout.rdbuf(conout.rdbuf());
    }
#endif
    m_recording = true;
    m_frame_has_data = false;
    m_current_frame.resize(m_width * m_height * 3, 0);
    m_finished_level = false;
    m_last_frame_t = m_extra_t = 0;
    m_after_end_extra_time = 0.f;
    m_renderer.m_width = m_width;
    m_renderer.m_height = m_height;
    m_renderer.begin();
    auto gm = gd::GameManager::sharedState();
    auto play_layer = gm->getPlayLayer();
    auto song_file = play_layer->m_level->getAudioFileName();
    auto fade_in = play_layer->m_levelSettings->m_fadeIn;
    auto fade_out = play_layer->m_levelSettings->m_fadeOut;
    auto bg_volume = gm->m_fBackgroundMusicVolume;
    auto sfx_volume = gm->m_fEffectsVolume;
    if (play_layer->m_level->songID == 0)
        song_file = CCFileUtils::sharedFileUtils()->fullPathForFilename(song_file.c_str(), false);
    auto is_testmode = play_layer->m_isTestMode;
    std::thread([&, path, song_file, fade_in, fade_out, bg_volume, sfx_volume, is_testmode]() {
        std::stringstream stream;
        stream << "ffmpeg -y -f rawvideo -pix_fmt rgb24 -s " << m_width << "x" << m_height << " -r " << m_fps
        << " -i - "; 
        if (!m_codec.empty())
            stream << "-c:v " << m_codec << " ";
        if (!m_bitrate.empty())
            stream << "-b:v " << m_bitrate << " ";
        if (!m_extra_args.empty())
            stream << m_extra_args << " ";
        else
            stream << "-pix_fmt yuv420p ";
        stream << "-vf \"vflip\" -an \"" << path << "\" "; // i hope just putting it in "" escapes it
        std::cout << "executing: " << stream.str() << std::endl;
        auto process = subprocess::Popen(stream.str());
        while (m_recording || m_frame_has_data) {
            m_lock.lock();
            if (m_frame_has_data) {
                const auto frame = m_current_frame; // copy it
                m_frame_has_data = false;
                m_lock.unlock();
                process.m_stdin.write(frame.data(), frame.size());
            }
            else m_lock.unlock();
        }
        if (process.close()) {
            std::cout << "ffmpeg errored :(" << std::endl;
            return;
        }
        std::cout << "video should be done now" << std::endl;
        if (!m_include_audio || !std::filesystem::exists(song_file)) return;
        wchar_t buffer[MAX_PATH];
        if (!GetTempFileNameW(widen(std::filesystem::temp_directory_path().string()).c_str(), L"rec", 0, buffer)) {
            std::cout << "error getting temp file" << std::endl;
            return;
        }
        auto temp_path = narrow(buffer) + "." + std::filesystem::path(path).filename().string();
        std::filesystem::rename(buffer, temp_path);
        auto total_time = m_last_frame_t; // 1 frame too short?
        {
            std::stringstream stream;
            stream << "ffmpeg -y -ss " << m_song_start_offset << " -i \"" << song_file
            << "\" -i \"" << path << "\" -t " << total_time << " -c:v copy "
            << "-filter:a \"volume=1[0:a]";
            if (fade_in && !is_testmode)
                stream << ";[0:a]afade=t=in:d=2[0:a]";
            if (fade_out && m_finished_level)
                stream << ";[0:a]afade=t=out:d=2:st=" << (total_time - m_after_end_duration - 3.5f) << "[0:a]";
            std::cout << "in " << fade_in << " out " << fade_out << std::endl;
            stream << "\" \"" << temp_path << "\"";
            std::cout << "executing: " << stream.str() << std::endl;
            auto process = subprocess::Popen(stream.str());
            if (process.close()) {
                std::cout << "oh god adding the song went wrong cmon" << std::endl;
                return;
            }
        }
        std::filesystem::remove(widen(path));
        std::filesystem::rename(temp_path, widen(path));
        std::cout << "video + audio should be done now!" << std::endl;
    }).detach();
}

void Recorder::stop() {
    m_renderer.end();
    m_recording = false;
}

void MyRenderTexture::begin() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);

    m_texture = new CCTexture2D;
    {
        auto data = malloc(m_width * m_height * 3);
        memset(data, 0, m_width * m_height * 3);
        m_texture->initWithData(data, kCCTexture2DPixelFormat_RGB888, m_width, m_height, CCSize(static_cast<float>(m_width), static_cast<float>(m_height)));
        free(data);
    }

    glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &m_old_rbo);

    glGenFramebuffersEXT(1, &m_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->getName(), 0);

    m_texture->setAliasTexParameters();

    m_texture->autorelease();

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_old_rbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
}

void MyRenderTexture::capture(std::mutex& lock, std::vector<u8>& data, volatile bool& lul) {
    glViewport(0, 0, m_width, m_height);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    auto director = CCDirector::sharedDirector();
    auto scene = director->getRunningScene();
    scene->visit();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    lock.lock();
    lul = true;
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    lock.unlock();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
    director->setViewport();
}

void MyRenderTexture::end() {
    m_texture->release();
}

void Recorder::capture_frame() {
    while (m_frame_has_data) {}
    m_renderer.capture(m_lock, m_current_frame, m_frame_has_data);
}

void Recorder::handle_recording(gd::PlayLayer* play_layer, float dt) {
    if (!play_layer->m_hasLevelCompleteMenu || m_after_end_extra_time < m_after_end_duration) {
        if (play_layer->m_hasLevelCompleteMenu) {
            m_after_end_extra_time += dt;
            m_finished_level = true;
        }
        auto frame_dt = 1. / static_cast<double>(m_fps);
        auto time = play_layer->m_time + m_extra_t - m_last_frame_t;
        if (time >= frame_dt) {
            gd::FMODAudioEngine::sharedEngine()->setBackgroundMusicTime(
                play_layer->m_time + m_song_start_offset);
            m_extra_t = time - frame_dt;
            m_last_frame_t = play_layer->m_time;
            capture_frame();
        }
    } else {
        stop();
    }
}