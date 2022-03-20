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

#include <sjson/writer.h>

#include <acl/compression/convert.h>
#include <acl/core/ansi_allocator.h>
#include <acl/core/compressed_tracks.h>
#include <acl/io/clip_writer.h>

#include <rtm/types.h>

#include <fstream>

namespace
{
    static uint32_t get_sample_size(acl_sjson::sample_type type)
    {
        switch (type)
        {
        case acl_sjson::sample_type::float1:
        default:
            return sizeof(float);
        case acl_sjson::sample_type::float2:
            return sizeof(rtm::float2f);
        case acl_sjson::sample_type::float3:
            return sizeof(rtm::float3f);
        case acl_sjson::sample_type::float4:
            return sizeof(rtm::float4f);
        case acl_sjson::sample_type::vector4:
            return sizeof(rtm::vector4f);
        case acl_sjson::sample_type::quat:
            return sizeof(rtm::quatf);
        case acl_sjson::sample_type::qvv:
            return sizeof(rtm::qvvf);
        }
    }

    static acl::track_desc_scalarf get_description(const acl_sjson::scalar_track_description& desc)
    {
        acl::track_desc_scalarf out_desc;
        out_desc.output_index = desc.output_index;
        out_desc.precision = desc.precision;
        return out_desc;
    }

    static acl::track_desc_transformf get_description(const acl_sjson::transform_track_description& desc, acl_sjson::acl_version version)
    {
        acl::track_desc_transformf out_desc;

        if (version >= acl_sjson::acl_version::v02_01_00)
            std::memcpy(&out_desc.default_value, &desc.default_value, sizeof(rtm::qvvf));

        out_desc.output_index = desc.output_index;
        out_desc.parent_index = desc.parent_index;
        out_desc.precision = desc.precision;
        out_desc.shell_distance = desc.shell_distance;
        out_desc.constant_rotation_threshold_angle = desc.constant_rotation_threshold_angle;
        out_desc.constant_translation_threshold = desc.constant_translation_threshold;
        out_desc.constant_scale_threshold = desc.constant_scale_threshold;
        
        return out_desc;
    }

    static acl::track make_track(acl::iallocator& allocator, const acl_sjson::track& track, acl_sjson::acl_version version)
    {
        const acl_sjson::track_description& desc = track.get_description();
        const acl_sjson::sample_type type = track.get_type();
        const uint32_t num_samples = static_cast<uint32_t>(track.get_num_samples());
        const float sample_rate = track.get_sample_rate();

        switch (type)
        {
        case acl_sjson::sample_type::float1:
            return acl::track_float1f::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
        case acl_sjson::sample_type::float2:
            return acl::track_float2f::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
        case acl_sjson::sample_type::float3:
            return acl::track_float3f::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
        case acl_sjson::sample_type::float4:
            return acl::track_float4f::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
        case acl_sjson::sample_type::vector4:
            return acl::track_vector4f::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
        case acl_sjson::sample_type::quat:
            ACL_ASSERT(false, "Unsupported type");
            return acl::track();
        case acl_sjson::sample_type::qvv:
            return acl::track_qvvf::make_reserve(get_description(desc.transform, version), allocator, num_samples, sample_rate);
        default:
            ACL_ASSERT(false, "Unsupported type");
            return acl::track();
        }
    }

    static acl::track_array convert_tracks(acl::iallocator& allocator, const acl_sjson::track_array& input_tracks)
    {
        const uint32_t num_tracks = static_cast<uint32_t>(input_tracks.get_num_tracks());
        const acl_sjson::acl_version version = input_tracks.get_version();

        acl::track_array out_tracks(allocator, num_tracks);

        for (uint32_t track_index = 0; track_index < num_tracks; ++track_index)
        {
            const acl_sjson::track& track_ = input_tracks[track_index];

            acl::track out_track = make_track(allocator, track_, version);

            const uint32_t num_samples = static_cast<uint32_t>(track_.get_num_samples());
            const uint32_t sample_size = get_sample_size(track_.get_type());

            for (uint32_t sample_index = 0; sample_index < num_samples; ++sample_index)
            {
                const acl_sjson::sample& smpl = track_[sample_index];
                void* sample_ptr = out_track[sample_index];

                std::memcpy(sample_ptr, &smpl, sample_size);
            }

            out_tracks[track_index] = std::move(out_track);
        }

        return out_tracks;
    }
}

namespace acl_sjson_v21
{
    bool write_tracks(const char* filename, const acl_sjson::track_array& tracks)
    {
        acl::ansi_allocator allocator;

        const acl::track_array input_tracks = convert_tracks(allocator, tracks);

        if (acl_sjson::is_acl_bin_file(filename))
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
            snprintf(output_filename, acl::get_array_size(output_filename), "\\\\?\\%s", filename);
#else
            const char* output_filename = filename;
#endif

            std::ofstream output_file_stream(output_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            if (!output_file_stream.is_open() || !output_file_stream.good())
            {
                printf("Failed to open output file for writing: %s\n", filename);
                acl::deallocate_type_array(allocator, output_tracks, output_tracks->get_size());
                return false;
            }

            output_file_stream.write(reinterpret_cast<const char*>(output_tracks), output_tracks->get_size());

            // Release the compressed data, no longer needed
            acl::deallocate_type_array(allocator, output_tracks, output_tracks->get_size());

            if (!output_file_stream.good())
            {
                printf("Failed to write output file: %s\n", filename);
                return false;
            }

            output_file_stream.close();
            if (!output_file_stream.good())
            {
                printf("Failed to close output file: %s\n", filename);
                return false;
            }
        }
        else
        {
            const acl::error_result result = acl::write_track_list(input_tracks, filename);
            if (result.any())
            {
                printf("Failed to write output file: %s\n", result.c_str());
                return false;
            }
        }
        
        return true;
    }
}
