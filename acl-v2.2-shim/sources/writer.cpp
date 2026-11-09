////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2025 Nicholas Frechette
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

#include "acl-sjson/api_v22.h"

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
		default:
			return 0;
		case acl_sjson::sample_type::float1:
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
		case acl_sjson::sample_type::vqv:
			return sizeof(rtm::vqvf);
		case acl_sjson::sample_type::vq:
			return sizeof(rtm::vqf);
		}
	}

	static acl::sample_interpolator_t get_sample_interpolator(acl_sjson::sample_interpolator interpolator)
	{
		switch (interpolator)
		{
		case acl_sjson::sample_interpolator::constant:
			return acl::sample_interpolator_t::constant;
		case acl_sjson::sample_interpolator::linear:
		default:
			return acl::sample_interpolator_t::linear;
		}
	}

	// An extended sample stores its interpolator and its value, copy each of them
	template<class track_type>
	static void copy_extended_samples(const acl_sjson::track& track_, acl::track& out_track)
	{
		track_type& typed_track = acl::track_cast<track_type>(out_track);

		const uint32_t num_samples = static_cast<uint32_t>(track_.get_num_samples());
		for (uint32_t sample_index = 0; sample_index < num_samples; ++sample_index)
		{
			typename track_type::sample_type& extended_sample = typed_track[sample_index];

			std::memcpy(&extended_sample.value, &track_[sample_index], sizeof(extended_sample.value));
			extended_sample.interpolator = get_sample_interpolator(track_.get_sample_interpolator(sample_index));
		}
	}

	static void copy_extended_samples(const acl_sjson::track& track_, acl::track& out_track)
	{
		switch (track_.get_type())
		{
		case acl_sjson::sample_type::float1:
			copy_extended_samples<acl::track_float1f_ex>(track_, out_track);
			break;
		case acl_sjson::sample_type::float2:
			copy_extended_samples<acl::track_float2f_ex>(track_, out_track);
			break;
		case acl_sjson::sample_type::float3:
			copy_extended_samples<acl::track_float3f_ex>(track_, out_track);
			break;
		case acl_sjson::sample_type::float4:
			copy_extended_samples<acl::track_float4f_ex>(track_, out_track);
			break;
		case acl_sjson::sample_type::vector4:
			copy_extended_samples<acl::track_vector4f_ex>(track_, out_track);
			break;
		default:
			ACL_ASSERT(false, "Unsupported type");
			break;
		}
	}

	static acl::track_desc_scalarf get_description(const acl_sjson::scalar_track_description& desc)
	{
		acl::track_desc_scalarf out_desc;
		out_desc.output_index = desc.output_index;
		out_desc.precision = desc.precision;
		return out_desc;
	}

	static acl::track_desc_transformf get_description(const acl_sjson::transform_track_description& desc)
	{
		acl::track_desc_transformf out_desc;

		std::memcpy(&out_desc.default_value, &desc.default_value, sizeof(rtm::vqvf));
		out_desc.output_index = desc.output_index;
		out_desc.parent_index = desc.parent_index;
		out_desc.precision = desc.precision;
		out_desc.shell_distance = desc.shell_distance;

		return out_desc;
	}

	static acl::track make_track(acl::iallocator& allocator, const acl_sjson::track& track)
	{
		const acl_sjson::track_description& desc = track.get_description();
		const acl_sjson::sample_type type = track.get_type();
		const uint32_t num_samples = static_cast<uint32_t>(track.get_num_samples());
		const float sample_rate = track.get_sample_rate();

		if (track.get_interpolator() == acl_sjson::track_interpolator::extended)
		{
			switch (type)
			{
			case acl_sjson::sample_type::float1:
				return acl::track_float1f_ex::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
			case acl_sjson::sample_type::float2:
				return acl::track_float2f_ex::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
			case acl_sjson::sample_type::float3:
				return acl::track_float3f_ex::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
			case acl_sjson::sample_type::float4:
				return acl::track_float4f_ex::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
			case acl_sjson::sample_type::vector4:
				return acl::track_vector4f_ex::make_reserve(get_description(desc.scalar), allocator, num_samples, sample_rate);
			default:
				ACL_ASSERT(false, "Unsupported type");
				return acl::track();
			}
		}

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
		case acl_sjson::sample_type::vqv:
			return acl::track_vqvf::make_reserve(get_description(desc.transform), allocator, num_samples, sample_rate);
		case acl_sjson::sample_type::vq:
			return acl::track_vqf::make_reserve(get_description(desc.transform), allocator, num_samples, sample_rate);
		default:
			ACL_ASSERT(false, "Unsupported type");
			return acl::track();
		}
	}

	static acl::track_array convert_tracks(acl::iallocator& allocator, const acl_sjson::track_array& input_tracks)
	{
		const uint32_t num_tracks = static_cast<uint32_t>(input_tracks.get_num_tracks());

		acl::track_array out_tracks(allocator, num_tracks);
		out_tracks.set_name(acl::string(allocator, input_tracks.get_name()));

		for (uint32_t track_index = 0; track_index < num_tracks; ++track_index)
		{
			const acl_sjson::track& track_ = input_tracks[track_index];

			acl::track out_track = make_track(allocator, track_);
			out_track.set_name(acl::string(allocator, track_.get_name()));

			if (track_.get_interpolator() == acl_sjson::track_interpolator::extended)
				copy_extended_samples(track_, out_track);
			else
			{
				const uint32_t num_samples = static_cast<uint32_t>(track_.get_num_samples());
				const uint32_t sample_size = get_sample_size(track_.get_type());

				for (uint32_t sample_index = 0; sample_index < num_samples; ++sample_index)
				{
					const acl_sjson::sample& smpl = track_[sample_index];
					void* sample_ptr = out_track[sample_index];

					std::memcpy(sample_ptr, &smpl, sample_size);
				}
			}

			out_tracks[track_index] = std::move(out_track);
		}

		return out_tracks;
	}
}

namespace acl_sjson_v22
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
				allocator.deallocate(output_tracks, output_tracks->get_size());
				return false;
			}

			output_file_stream.write(reinterpret_cast<const char*>(output_tracks), output_tracks->get_size());

			// Release the compressed data, no longer needed
			allocator.deallocate(output_tracks, output_tracks->get_size());

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
