#pragma once

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <ranges>
#include <type_traits>
#include <utility>

template <typename T>
concept writeable = true;
enum class SeekFrom : int { Beg = SEEK_SET, End = SEEK_END, Cur = SEEK_CUR };

class wbfile {
    FILE* m_file;
    constexpr static auto FILE_BUFFER_SIZE = 512 * 1024u;
public:
    explicit wbfile(const std::filesystem::path& path) : m_file(std::fopen(path.c_str(), "wb")) {
        if (m_file == nullptr)
            throw std::runtime_error(std::format("Failed to open '{}': {}", path.string(), std::strerror(errno)));
        std::setvbuf(m_file, nullptr, _IOFBF, FILE_BUFFER_SIZE);
    }
    ~wbfile() {
        if (m_file)
            std::fclose(m_file);
    }
    wbfile(const wbfile&) = delete;
    wbfile& operator=(const wbfile&) = delete;

    wbfile(wbfile&& old) noexcept : m_file(std::exchange(old.m_file, nullptr)) {}

    template<writeable T>
    void write(T const& val) {
        const auto bytesWritten = std::fwrite(&val, 1, sizeof(T), m_file);
        if (bytesWritten != sizeof(T))
            throw std::runtime_error(
                    std::format("Wrote only {:#x} bytes when writing {:#x}-byte object", bytesWritten, sizeof(T)));

    }

    template<writeable T>
    std::size_t write_range(std::span<const T> s) {
        return std::fwrite(s.data(), sizeof(T), s.size(), m_file);
    }

    void seek(long offset, SeekFrom from = SeekFrom::Beg) {
        if (std::fseek(m_file, offset, static_cast<int>(from)) != 0)
            throw std::runtime_error(std::format("Failed to seek to {:#x}", offset));
    }
    void move(long offset) {
        if (std::fseek(m_file, offset, SEEK_CUR) != 0)
            throw std::runtime_error(std::format("Failed to seek by {:+#x}", offset));
    }
    [[nodiscard]] unsigned tell() const {
        auto pos = std::ftell(m_file);
        if (pos < 0)
            throw std::runtime_error("Failed to get file position");

        return pos;
    }
    [[nodiscard]] FILE* handle() { return m_file; }
};

