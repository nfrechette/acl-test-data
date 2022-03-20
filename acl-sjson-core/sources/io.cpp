////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2022 Nicholas Frechette
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "acl-sjson/io.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace acl_sjson
{
	static char* align_to(char* value, std::size_t alignment)
	{
		return reinterpret_cast<char*>((reinterpret_cast<intptr_t>(value) + (alignment - 1)) & ~(alignment - 1));
	}

    bool read_file(const char* input_filename, char*& out_buffer, std::size_t& out_file_size)
    {
        // Use the raw C API with a large buffer to ensure this is as fast as possible
        std::FILE* file = nullptr;

#ifdef _WIN32
        char path[64 * 1024] = { 0 };
        snprintf(path, acl::get_array_size(path), "\\\\?\\%s", input_filename);
        fopen_s(&file, path, "rb");
#else
        file = fopen(input_filename, "rb");
#endif

        if (file == nullptr)
        {
            printf("Failed to open input file\n");
            return false;
        }

        // Make sure to enable buffering with a large buffer
        const int setvbuf_result = setvbuf(file, nullptr, _IOFBF, 1 * 1024 * 1024);
        if (setvbuf_result != 0)
        {
            printf("Failed to set input file buffering settings\n");
            fclose(file);
            return false;
        }

        const int fseek_result = fseek(file, 0, SEEK_END);
        if (fseek_result != 0)
        {
            printf("Failed to seek in input file\n");
            fclose(file);
            return false;
        }

#ifdef _WIN32
        out_file_size = static_cast<std::size_t>(_ftelli64(file));
#else
        out_file_size = static_cast<std::size_t>(ftello(file));
#endif

        if (out_file_size == static_cast<std::size_t>(-1L))
        {
            printf("Failed to read input file size\n");
            fclose(file);
            return false;
        }

        rewind(file);

        // Allocate an extra 128 bytes
        // We'll align to 64 bytes and store the padding at the start of the allocation in the preceding 4 bytes
        char* buffer = static_cast<char*>(malloc(out_file_size + 128));
        out_buffer = align_to(buffer + 64, 64);

        uint32_t* allocation_padding = reinterpret_cast<uint32_t*>(out_buffer - sizeof(uint32_t));
        *allocation_padding = static_cast<uint32_t>(out_buffer - buffer);

        const std::size_t result = fread(out_buffer, 1, out_file_size, file);
        fclose(file);

        if (result != out_file_size)
        {
            printf("Failed to read input file\n");
            free_file_memory(out_buffer);
            return false;
        }

        return true;
    }

    void free_file_memory(char* buffer)
    {
        if (buffer == nullptr)
            return; // Nothing to do

        const uint32_t* allocation_padding = reinterpret_cast<const uint32_t*>(buffer - sizeof(uint32_t));
        char* ptr = buffer - *allocation_padding;
        free(ptr);
    }

    bool is_acl_bin_file(const char* filename)
    {
        const size_t filename_len = filename != nullptr ? std::strlen(filename) : 0;
        return filename_len >= 4 && strncmp(filename + filename_len - 4, ".acl", 4) == 0;
    }
}
