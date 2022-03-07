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

#include "command_line_options.h"

#include <sjson/parser.h>
#include <sjson/writer.h>

#include <acl/compression/convert.h>
#include <acl/core/ansi_allocator.h>
#include <acl/core/compressed_tracks.h>
#include <acl/io/clip_reader.h>
#include <acl/io/clip_writer.h>

#include <fstream>
#include <cstdio>

static bool read_file(acl::iallocator& allocator, const char* input_filename, char*& out_buffer, size_t& out_file_size)
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
	out_file_size = static_cast<size_t>(_ftelli64(file));
#else
	out_file_size = static_cast<size_t>(ftello(file));
#endif

	if (out_file_size == static_cast<size_t>(-1L))
	{
		printf("Failed to read input file size\n");
		fclose(file);
		return false;
	}

	rewind(file);

	out_buffer = acl::allocate_type_array_aligned<char>(allocator, out_file_size, 64);
	const size_t result = fread(out_buffer, 1, out_file_size, file);
	fclose(file);

	if (result != out_file_size)
	{
		printf("Failed to read input file\n");
		acl::deallocate_type_array(allocator, out_buffer, out_file_size);
		return false;
	}

	return true;
}

static bool read_acl_bin_file(acl::iallocator& allocator, const char* input_filename, acl::compressed_tracks*& out_tracks)
{
	char* tracks_data = nullptr;
	size_t file_size = 0;

	if (!read_file(allocator, input_filename, tracks_data, file_size))
		return false;

	out_tracks = reinterpret_cast<acl::compressed_tracks*>(tracks_data);
    const acl::error_result is_valid = out_tracks->is_valid(true);
	if (is_valid.any())
	{
		printf("Invalid binary ACL file provided: %s\n", is_valid.c_str());
		acl::deallocate_type_array(allocator, tracks_data, file_size);
		return false;
	}

	return true;
}

static bool read_acl_sjson_file(acl::iallocator& allocator, const char* input_filename,
	acl::sjson_file_type& out_file_type,
	acl::sjson_raw_clip& out_raw_clip,
	acl::sjson_raw_track_list& out_raw_track_list)
{
	char* sjson_file_buffer = nullptr;
	size_t file_size = 0;

	if (!read_file(allocator, input_filename, sjson_file_buffer, file_size))
		return false;

	acl::clip_reader reader(allocator, sjson_file_buffer, file_size - 1);

	const acl::sjson_file_type ftype = reader.get_file_type();
	out_file_type = ftype;

	bool success = false;
	switch (ftype)
	{
	case acl::sjson_file_type::unknown:
	default:
		printf("\nUnknown file type\n");
		break;
	case acl::sjson_file_type::raw_clip:
		success = reader.read_raw_clip(out_raw_clip);
		break;
	case acl::sjson_file_type::raw_track_list:
		success = reader.read_raw_track_list(out_raw_track_list);
		break;
	}

	if (!success)
	{
		const acl::clip_reader_error err = reader.get_error();
		if (err.error != acl::clip_reader_error::None)
			printf("\nError on line %d column %d: %s\n", err.line, err.column, err.get_description());
	}

	acl::deallocate_type_array(allocator, sjson_file_buffer, file_size);
	return success;
}

static bool is_acl_bin_file(const char* filename)
{
	const size_t filename_len = filename != nullptr ? std::strlen(filename) : 0;
	return filename_len >= 4 && strncmp(filename + filename_len - 4, ".acl", 4) == 0;
}

static bool read_input_tracks(const command_line_options& options, acl::iallocator& allocator, acl::track_array& input_tracks)
{
    if (options.input_filename == options.output_filename)
	{
		printf("Input and output cannot be the same file\n");
		return false;
	}

    if (is_acl_bin_file(options.input_filename.c_str()))
    {
        acl::compressed_tracks* tracks = nullptr;

        // Read the compressed data file
        if (!read_acl_bin_file(allocator, options.input_filename.c_str(), tracks))
            return false;
        
        // Convert the compressed data into a raw track array
        const acl::error_result result = acl::convert_track_list(allocator, *tracks, input_tracks);
        if (result.any())
        {
            printf("Failed to convert input binary track list: %s\n", result.c_str());
            deallocate_type_array(allocator, tracks, tracks->get_size());
            return false;
        }

        // Release the compressed data, no longer needed
		deallocate_type_array(allocator, tracks, tracks->get_size());
    }
    else
    {
        acl::sjson_file_type sjson_type = acl::sjson_file_type::unknown;
	    acl::sjson_raw_clip sjson_clip;
	    acl::sjson_raw_track_list sjson_track_list;

        if (!read_acl_sjson_file(allocator, options.input_filename.c_str(), sjson_type, sjson_clip, sjson_track_list))
            return false;
        
        switch (sjson_type)
        {
        case acl::sjson_file_type::raw_clip:
            if (!sjson_clip.additive_base_track_list.is_empty())
            {
                printf("Additive base not supported yet\n");
                return false;
            }
            else if (sjson_clip.has_settings)
            {
                printf("Settings not supported yet\n");
                return false;
            }

            input_tracks = std::move(sjson_clip.track_list);
            break;
        case acl::sjson_file_type::raw_track_list:
            if (sjson_track_list.has_settings)
            {
                printf("Settings not supported yet\n");
                return false;
            }

            input_tracks = std::move(sjson_track_list.track_list);
            break;
        default:
            printf("Unsupported SJSON type: %d\n", sjson_type);
            return false;
        }
    }

    return true;
}

static bool write_output_tracks(const command_line_options& options, acl::iallocator& allocator, const acl::track_array& input_tracks)
{
    if (is_acl_bin_file(options.output_filename.c_str()))
    {
        // Convert our input tracks into a compressed_tracks instance
        acl::compressed_tracks* output_tracks = nullptr;
        const acl::error_result result = acl::convert_track_list(allocator, input_tracks, output_tracks);
        if (result.any())
        {
            printf("Failed to convert tracks: %s\n", result.c_str());
            return false;
        }

#ifdef _WIN32
		char output_filename[64 * 1024] = { 0 };
		snprintf(output_filename, acl::get_array_size(output_filename), "\\\\?\\%s", options.output_filename.c_str());
#else
		const char* output_filename = options.output_filename.c_str();
#endif

        std::ofstream output_file_stream(output_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        if (!output_file_stream.is_open() || !output_file_stream.good())
        {
            printf("Failed to open output file for writing: %s\n", options.output_filename.c_str());
            deallocate_type_array(allocator, output_tracks, output_tracks->get_size());
            return false;
        }

        output_file_stream.write(reinterpret_cast<const char*>(output_tracks), output_tracks->get_size());

        // Release the compressed data, no longer needed
		deallocate_type_array(allocator, output_tracks, output_tracks->get_size());

        if (!output_file_stream.good())
        {
            printf("Failed to write output file: %s\n", options.output_filename.c_str());
            return false;
        }

        output_file_stream.close();
        if (!output_file_stream.good())
        {
            printf("Failed to close output file: %s\n", options.output_filename.c_str());
            return false;
        }
    }
    else
    {
        const acl::error_result result = acl::write_track_list(input_tracks, options.output_filename.c_str());
        if (result.any())
        {
            printf("Failed to write output file: %s\n", result.c_str());
            return false;
        }
    }

    return true;
}

bool convert(const command_line_options& options)
{
    acl::ansi_allocator allocator;

    // Read the input file into a tracks array
	acl::track_array input_tracks;
    if (!read_input_tracks(options, allocator, input_tracks))
        return false;
    
    // Write the tracks array out into our desired format
    if (!write_output_tracks(options, allocator, input_tracks))
        return false;

    // Done!
    return true;
}
