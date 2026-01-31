#pragma once
#include "ints.hpp"
#include "bstype.hpp"
constexpr u32 four_cc(u8 a, u8 b, u8 c, u8 d) { return a << 24 | b << 16 | c << 8 | d; }

struct master_riff_chunk_t {
    u32be type_bloc_id;
    u32le size;
    u32be format;
};

static_assert(sizeof(master_riff_chunk_t) == 0x0C);

enum class audio_format_t : u16 {
    pcm = 0x0001,
    ieee_float = 0x0003,
    // According to ffmpeg output
    adpcm_yamaha = 0x0020,
    extensible = 0xfffe
};

enum class bloc_id_t : u32 { format = four_cc('f', 'm', 't', ' '), data = four_cc('d', 'a', 't', 'a') };

#pragma pack(push, 1)
struct wav_chunk_header_t {
    betype<bloc_id_t> block_id;
    u32le chunk_size;
};

static_assert(sizeof(wav_chunk_header_t) == 0x08);

struct wav_format_chunk_t {
    letype<audio_format_t> audio_format;
    u16le n_channels;
    u32le sample_rate;
    u32le bytes_per_sec;
    u16le bytes_per_block;
    u16le bits_per_sample;
};
static_assert(sizeof(wav_format_chunk_t) == 0x10);
#pragma pack(pop)

