#include <chrono>
#include <cstdlib>

#include <memory>
#include <vector>
#include <span>
#include <thread>

#include "file.hpp"
#include "wav.hpp"

#include <coreinit/time.h>
#include <mic/mic.h>
#include <sndcore2/core.h>
#include <sysapp/launch.h>
#include <vpad/input.h>

#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/log_cafe.h>
#include <whb/proc.h>
#include <wut.h>

constexpr auto MIC_WORK_SAMPLE_COUNT = 1 << 18;

[[noreturn]] void ReturnToMenu() {
    SYSLaunchMenu();
    while (WHBProcIsRunning()) {
    }
    std::exit(0);
}

void SaveWAV(const std::filesystem::path& path, std::vector<int16_t> buffer, unsigned sampleRate);


int main() {
    WHBProcInit();
    WHBLogUdpInit();
    WHBLogCafeInit();

    //Cemu 2.6 relies on AX being initialized to generate mic samples
    AXInit();
    WHBLogPrint("Started");
    auto micSamples = std::unique_ptr<int16_t[]>(new (std::align_val_t{0x40}) int16_t[MIC_WORK_SAMPLE_COUNT]);
    MICWorkMemory workmem{MIC_WORK_SAMPLE_COUNT, micSamples.get()};
    MICError micError;
    auto micHandle = MICInit(MIC_INSTANCE_0, 0, &workmem, &micError);
    if (micError != MIC_ERROR_OK) {
        WHBLogPrintf("MICInit failed: %d", micError);
        ReturnToMenu();
    }
    bool isMicOpen = false;
    VPADStatus vpadStatus;
    VPADReadError vpadReadError;
    std::vector<int16_t> readSamples;

    auto localTimeZone = std::chrono::current_zone();

    OSTick lastTime = OSGetTick();
    while (WHBProcIsRunning()) {
        if (VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadReadError) == 1) {
            if (vpadStatus.trigger & VPAD_BUTTON_A) {
                isMicOpen = !isMicOpen;
                if (isMicOpen) {
                    readSamples.clear();
                    micError = MICOpen(micHandle);
                    WHBLogPrintf("MICOpen: %d", micError);
                } else {
                    micError = MICClose(micHandle);
                    WHBLogPrintf("MICClose: %d", micError);
                    const auto localNow = localTimeZone->to_local(std::chrono::system_clock::now());
                    SaveWAV(std::format("MIC-{0:%F}--{0:%H}-{0:%M}-{0:%S}.wav",
                                        std::chrono::floor<std::chrono::milliseconds>(localNow)), readSamples, 32000);
                }
            }
        }
        auto currentTime = OSGetTick();
        if (isMicOpen && (currentTime - lastTime) > OSMillisecondsToTicks(200)) {
            MICStatus micStatus{};
            micError = MICGetStatus(micHandle, &micStatus);
            if (micError != MIC_ERROR_OK) {
                WHBLogPrintf("MICStatus: %d", micError);
            }
            else if (micStatus.availableData != 0) {
                WHBLogPrintf("Available: %d", micStatus.availableData);
                auto const offset = micStatus.availableData + micStatus.bufferPos;
                if (offset > MIC_WORK_SAMPLE_COUNT) {
                    readSamples.append_range(std::span(micSamples.get() + micStatus.bufferPos, MIC_WORK_SAMPLE_COUNT - micStatus.bufferPos));
                    readSamples.append_range(std::span(micSamples.get(), offset - MIC_WORK_SAMPLE_COUNT));
                }
                else {
                    readSamples.append_range(std::span(micSamples.get() + micStatus.bufferPos, micStatus.availableData));
                }
                MICSetDataConsumed(micHandle, micStatus.availableData);
            }
            lastTime = currentTime;
        }
    }

    MICUninit(micHandle);
    WHBLogUdpDeinit();
    return 0;
}

void SaveWAV(const std::filesystem::path& path, std::vector<int16_t> buffer, unsigned sampleRate) {

    master_riff_chunk_t riffChunk {
        .type_bloc_id = 'RIFF',
        .size = 4 + sizeof(wav_chunk_header_t) + sizeof(wav_format_chunk_t) + sizeof(wav_chunk_header_t) + buffer.size() * sizeof(int16_t),
        .format = 'WAVE'
    };

    wav_chunk_header_t formatHeader {.block_id = bloc_id_t::format, .chunk_size = sizeof(wav_format_chunk_t)};

    wav_format_chunk_t formatChunk;
    formatChunk.audio_format = audio_format_t::pcm;
    formatChunk.n_channels = 1;
    formatChunk.sample_rate = sampleRate;
    formatChunk.bits_per_sample = 8 * sizeof(int16_t);
    formatChunk.bytes_per_block = (formatChunk.n_channels * formatChunk.bits_per_sample) / 8;
    formatChunk.bytes_per_sec = formatChunk.sample_rate * formatChunk.bytes_per_block;
;
    wav_chunk_header_t dataHeader {.block_id = bloc_id_t::data, .chunk_size = buffer.size() * sizeof(int16_t)};

    auto file = wbfile(path);
    file.write(riffChunk);
    file.write(formatHeader);
    file.write(formatChunk);
    file.write(dataHeader);

    // Byteswap samples because Wii U samples are big-endian
    for (auto &x: buffer) {
        x = std::byteswap(x);
    }
    file.write_range<int16_t>(buffer);
}
