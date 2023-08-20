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

#include "acl-sjson/api_v21.h"

#include <acl-sjson/io.h>
#include <acl-sjson/sample.h>
#include <acl-sjson/track.h>
#include <acl-sjson/track_array.h>

#include <sjson/parser.h>

#include <acl/compression/convert.h>
#include <acl/core/ansi_allocator.h>
#include <acl/core/compressed_tracks.h>
#include <acl/io/clip_reader.h>

#include <cstdio>

namespace
{
    static bool read_acl_bin_file(const char* input_filename, acl::compressed_tracks*& out_tracks)
    {
        char* tracks_data = nullptr;
        size_t file_size = 0;

        if (!acl_sjson::read_file(input_filename, tracks_data, file_size))
            return false;

        out_tracks = reinterpret_cast<acl::compressed_tracks*>(tracks_data);
        const acl::error_result is_valid = out_tracks->is_valid(true);
        if (is_valid.any())
        {
            printf("Invalid binary ACL file provided: %s\n", is_valid.c_str());
            acl_sjson::free_file_memory(tracks_data);
            return false;
        }

        return true;
    }

    static bool read_acl_sjson_file(acl::iallocator& allocator, const char* input_filename,
        acl::sjson_file_type& out_file_type,
        acl::sjson_raw_clip& out_raw_clip,
        acl::sjson_raw_track_list& out_raw_track_list,
		size_t& out_bytes_read)
    {
        char* sjson_file_buffer = nullptr;
        size_t file_size = 0;

        if (!acl_sjson::read_file(input_filename, sjson_file_buffer, file_size))
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

        acl_sjson::free_file_memory(sjson_file_buffer);
		out_bytes_read = file_size;
        return success;
    }

    static acl_sjson::sample_type get_sample_type(acl::track_type8 type)
    {
        switch (type)
        {
		default:
			return acl_sjson::sample_type::unknown;
        case acl::track_type8::float1f:
            return acl_sjson::sample_type::float1;
		case acl::track_type8::float2f:
            return acl_sjson::sample_type::float2;
		case acl::track_type8::float3f:
            return acl_sjson::sample_type::float3;
		case acl::track_type8::float4f:
            return acl_sjson::sample_type::float4;
		case acl::track_type8::vector4f:
            return acl_sjson::sample_type::vector4;
        case acl::track_type8::qvvf:
            return acl_sjson::sample_type::qvv;
        }
    }

    static acl_sjson::scalar_track_description get_description(const acl::track_desc_scalarf& desc)
    {
        acl_sjson::scalar_track_description out_desc;
        out_desc.output_index = desc.output_index;
        out_desc.precision = desc.precision;
        return out_desc;
    }

    static acl_sjson::transform_track_description get_description(const acl::track_desc_transformf& desc)
    {
        acl_sjson::transform_track_description out_desc;
        std::memcpy(&out_desc.default_value, &desc.default_value, sizeof(rtm::qvvf));
        out_desc.output_index = desc.output_index;
        out_desc.parent_index = desc.parent_index;
        out_desc.precision = desc.precision;
        out_desc.shell_distance = desc.shell_distance;
        out_desc.constant_rotation_threshold_angle = 0.00284714461F;
        out_desc.constant_translation_threshold = 0.001F;
        out_desc.constant_scale_threshold = 0.00001F;
        return out_desc;
    }

    static acl_sjson::track_description get_description(const acl::track& track)
    {
        acl_sjson::track_description desc;

        switch (track.get_type())
        {
        case acl::track_type8::float1f:
        case acl::track_type8::float2f:
        case acl::track_type8::float3f:
        case acl::track_type8::float4f:
        case acl::track_type8::vector4f:
        default:
            desc.scalar = get_description(track.get_description<acl::track_desc_scalarf>());
            break;
        case acl::track_type8::qvvf:
            desc.transform = get_description(track.get_description<acl::track_desc_transformf>());
            break;
        }

        return desc;
    }

    static acl_sjson::track_array convert_tracks(const acl::track_array& input_tracks, const acl_sjson::metadata_t& metadata)
    {
        acl_sjson::track_array out_tracks(input_tracks.get_name().c_str(), metadata);

        for (const acl::track& track_ : input_tracks)
        {
            const acl_sjson::sample_type type = get_sample_type(track_.get_type());
            const float sample_rate = track_.get_sample_rate();
			const char* name = track_.get_name().c_str();

            acl_sjson::track out_track(type, sample_rate, name);
            out_track.get_description() = get_description(track_);

            const uint32_t num_samples = track_.get_num_samples();
            const uint32_t sample_size = track_.get_sample_size();

            for (uint32_t sample_index = 0; sample_index < num_samples; ++sample_index)
            {
                const void* sample_ptr = track_[sample_index];

                acl_sjson::sample smpl;
                std::memcpy(&smpl, sample_ptr, sample_size);

                out_track.emplace_back(std::move(smpl));
            }

            out_tracks.emplace_back(std::move(out_track));
        }

        return out_tracks;
    }

    static acl_sjson::acl_version get_version(acl::compressed_tracks_version16 version)
    {
        switch (version)
        {
        case acl::compressed_tracks_version16::v02_00_00:
            return acl_sjson::acl_version::v02_00_00;
        case acl::compressed_tracks_version16::v02_01_99:
		case acl::compressed_tracks_version16::v02_01_99_1:
		case acl::compressed_tracks_version16::v02_01_99_2:
            return acl_sjson::acl_version::v02_01_00;
        default:
            return acl_sjson::acl_version::unknown;
        }
    }

	static acl_sjson::rotation_format_t get_rotation_format(acl::rotation_format8 format)
	{
		switch (format)
		{
		case acl::rotation_format8::quatf_full:
			return acl_sjson::rotation_format_t::quatf_full;
		case acl::rotation_format8::quatf_drop_w_full:
			return acl_sjson::rotation_format_t::quatf_drop_w_full;
		case acl::rotation_format8::quatf_drop_w_variable:
			return acl_sjson::rotation_format_t::quatf_drop_w_variable;
		default:
			return acl_sjson::rotation_format_t::unknown;
		}
	}

	static acl_sjson::vector_format_t get_vector_format(acl::vector_format8 format)
	{
		switch (format)
		{
		case acl::vector_format8::vector3f_full:
			return acl_sjson::vector_format_t::vector3f_full;
		case acl::vector_format8::vector3f_variable:
			return acl_sjson::vector_format_t::vector3f_variable;
		default:
			return acl_sjson::vector_format_t::unknown;
		}
	}
}

namespace acl_sjson_v21
{
    bool read_tracks(const char* filename, acl_sjson::track_array& out_tracks)
    {
        acl::ansi_allocator allocator;

        acl::track_array input_tracks;

		acl_sjson::metadata_t metadata;
		std::memset(&metadata.variant, 0, sizeof(metadata.variant));

        if (acl_sjson::is_acl_bin_file(filename))
        {
            acl::compressed_tracks* tracks = nullptr;

            // Read the compressed data file
            if (!read_acl_bin_file(filename, tracks))
                return false;

            // Convert the compressed data into a raw track array
            const acl::error_result result = acl::convert_track_list(allocator, *tracks, input_tracks);
            if (result.any())
            {
                printf("Failed to convert input binary track list: %s\n", result.c_str());
                acl_sjson::free_file_memory(reinterpret_cast<char*>(tracks));
                return false;
            }

            metadata.version = get_version(tracks->get_version());
			metadata.size = tracks->get_size();
			metadata.name = tracks->get_name();
			metadata.track_variant = tracks->get_track_type() == acl::track_type8::qvvf ?
				acl_sjson::track_variant_t::transform : acl_sjson::track_variant_t::scalar;

			if (metadata.track_variant == acl_sjson::track_variant_t::transform)
			{
				{
					const acl::acl_impl::tracks_header& header = acl::acl_impl::get_tracks_header(*tracks);
					metadata.variant.transform.rotation_format = get_rotation_format(header.get_rotation_format());
					metadata.variant.transform.translation_format = get_vector_format(header.get_translation_format());

					if (header.get_has_scale())
						metadata.variant.transform.scale_format = get_vector_format(header.get_scale_format());
					else
						metadata.variant.transform.scale_format = acl_sjson::vector_format_t::unknown;
				}

				{
					const acl::acl_impl::transform_tracks_header& header = acl::acl_impl::get_transform_tracks_header(*tracks);
					metadata.variant.transform.num_segments = header.num_segments;
					metadata.variant.transform.num_animated_rotation_sub_tracks = header.num_animated_rotation_sub_tracks;
					metadata.variant.transform.num_animated_translation_sub_tracks = header.num_animated_translation_sub_tracks;
					metadata.variant.transform.num_animated_scale_sub_tracks = header.num_animated_scale_sub_tracks;
					metadata.variant.transform.num_constant_rotation_samples = header.num_constant_rotation_samples;
					metadata.variant.transform.num_constant_translation_samples = header.num_constant_translation_samples;
					metadata.variant.transform.num_constant_scale_samples = header.num_constant_scale_samples;
				}
			}

            // Release the compressed data, no longer needed
            acl_sjson::free_file_memory(reinterpret_cast<char*>(tracks));
        }
        else if (acl_sjson::is_acl_sjson_file(filename))
        {
            acl::sjson_file_type sjson_type = acl::sjson_file_type::unknown;
            acl::sjson_raw_clip sjson_clip;
            acl::sjson_raw_track_list sjson_track_list;
			size_t file_size = 0;

            if (!read_acl_sjson_file(allocator, filename, sjson_type, sjson_clip, sjson_track_list, file_size))
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
                printf("Unsupported SJSON type: %d\n", static_cast<int>(sjson_type));
                return false;
            }

            metadata.version = acl_sjson::acl_version::v02_01_00;
			metadata.size = file_size;
			metadata.name = input_tracks.get_name().c_str();
			metadata.track_variant = input_tracks.get_track_type() == acl::track_type8::qvvf ?
				acl_sjson::track_variant_t::transform : acl_sjson::track_variant_t::scalar;

			if (metadata.track_variant == acl_sjson::track_variant_t::transform)
			{
				metadata.variant.transform.rotation_format = acl_sjson::rotation_format_t::quatf_full;
				metadata.variant.transform.translation_format = acl_sjson::vector_format_t::vector3f_full;
				metadata.variant.transform.scale_format = acl_sjson::vector_format_t::vector3f_full;
			}
        }
		else
		{
			printf("Unknown ACL file format\n");
			return false;
		}

        out_tracks = convert_tracks(input_tracks, metadata);

        return true;
    }
}
