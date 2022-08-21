#pragma once

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

#include "acl-sjson/acl_version.h"

#include <string>

namespace acl_sjson
{
	enum class rotation_format_t
	{
		unknown,
		quatf_full,
		quatf_drop_w_full,
		quatf_drop_w_variable,
	};

	const char* to_string(rotation_format_t format);

	enum class vector_format_t
	{
		unknown,
		vector3f_full,
		vector3f_variable,
	};

	const char* to_string(vector_format_t format);

	enum class track_variant_t
	{
		unknown,
		transform,
		scalar,
	};

	const char* to_string(track_variant_t variant);

	struct transform_metadata_t
	{
		rotation_format_t rotation_format;
		vector_format_t translation_format;
		vector_format_t scale_format;

		uint32_t num_segments;

		uint32_t num_animated_rotation_sub_tracks;
		uint32_t num_animated_translation_sub_tracks;
		uint32_t num_animated_scale_sub_tracks;

		uint32_t num_constant_rotation_samples;
		uint32_t num_constant_translation_samples;
		uint32_t num_constant_scale_samples;
	};

	struct scalar_metadata_t
	{
	};

	struct metadata_t
	{
		acl_version		version = acl_version::unknown;
		size_t			size = 0;

		std::string		name;

		track_variant_t	track_variant = track_variant_t::unknown;

		union
		{
			transform_metadata_t transform;
			scalar_metadata_t scalar;
		} variant;
	};
}
