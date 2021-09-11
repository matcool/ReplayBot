#include "recorder.hpp"
#include <sstream>
#include <CCGL.h>

Recorder::Recorder() : m_width(1280), m_height(720), m_fps(60) {}

void Recorder::start(const std::string& path) {
    m_recording = true;
    m_finished_level = false;
    m_last_frame_t = m_extra_t = 0;
    m_after_end_extra_time = 0.f;
    m_renderer.m_width = m_width;
    m_renderer.m_height = m_height;
    m_renderer.begin();
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    auto song_file = play_layer->m_level->getAudioFileName();
    // these are always false???
    auto fade_in = play_layer->m_levelSettings->m_fadeIn;
    auto fade_out = play_layer->m_levelSettings->m_fadeOut;
    std::thread([&, path, song_file]() {
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
        while (m_recording) {
            m_lock.lock();
            if (!m_frames.empty()) {
                const auto& frame = m_frames.back();
                process.m_stdin.write(frame.data(), frame.size());
                m_frames.pop();
            }
            m_lock.unlock();
        }
        process.close();
        std::cout << "should be done now!" << std::endl;
        std::cout << "sike" << std::endl;
        auto total_time = m_last_frame_t; // 1 frame too short?
        {
            std::stringstream stream;
            stream << "ffmpeg -y -ss " << m_song_start_offset << " -i \"" << song_file
            << "\" -i \"" << path << "\" -t " << total_time << " -c:v copy "
            << "-filter:a \"";
            std::cout << "in " << fade_in << " out " << fade_out << std::endl;
            stream << "afade=t=in:d=0.5[out];[out]afade=t=out:d=0.5:st=" << (total_time - 0.5f - 3.f - 1.f);
            stream << "\" " << path << ".sound.mp4";
            std::cout << "executing: " << stream.str() << std::endl;
            auto process = subprocess::Popen(stream.str());
            process.close();
        }
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

std::vector<u8> MyRenderTexture::capture() {
    glViewport(0, 0, m_width, m_height);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    auto director = CCDirector::sharedDirector();
    auto scene = director->getRunningScene();
    scene->visit();
    
    // dynamic_array<u8> data(m_width * m_height * 3);
    std::vector<u8> data;
    data.resize(m_width * m_height * 3, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
    director->setViewport();
    return data;
}

void MyRenderTexture::end() {
    m_texture->release();
}

void Recorder::capture_frame() {
    const auto frame = m_renderer.capture();
    m_lock.lock();
    m_frames.push(frame);
    m_lock.unlock();
}

void Recorder::handle_recording(gd::PlayLayer* play_layer, float dt) {
    if (!play_layer->m_hasLevelCompleteMenu || m_after_end_extra_time < 3.f) {
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