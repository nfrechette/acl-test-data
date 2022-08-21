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
#include "utils.h"

#include <acl-sjson/api_v20.h>
#include <acl-sjson/api_v21.h>
#include <acl-sjson/track_array.h>

#include <cstdio>

bool info(const command_line_options& options)
{
	acl_sjson::track_array tracks;
	if (!read_tracks(options.input_filename.c_str(), tracks))
		return false;

	if (tracks.get_version() == acl_sjson::acl_version::unknown)
	{
		printf("Unknown ACL version used in input file\n");
		return false;
	}

	const acl_sjson::metadata_t& metadata = tracks.get_metadata();

	printf("Filename: %s\n", options.input_filename.c_str());
	if (metadata.size > 1024 * 1024)
		printf("File size: %.3f MB\n", double(metadata.size) / (1024.0 * 1024.0));
	else if (metadata.size > 1024)
		printf("File size: %.2f KB\n", double(metadata.size) / 1024.0);
	else
		printf("File size: %u Bytes\n", uint32_t(metadata.size));
	printf("Version: %s\n", acl_sjson::to_string(tracks.get_version()));
	printf("Num tracks: %u\n", static_cast<uint32_t>(tracks.get_num_tracks()));
	printf("Num samples per track: %u\n", static_cast<uint32_t>(tracks.get_num_samples_per_track()));
	printf("Sample rate: %.2f FPS\n", tracks.get_sample_rate());
	printf("Duration: %.2f seconds\n", tracks.get_duration());
	printf("Sample type: %s\n", acl_sjson::to_string(tracks.get_type()));

	if (tracks.get_type() == acl_sjson::sample_type::qvv)
	{
		// QVV
		printf("Rotation format: %s\n", to_string(metadata.variant.transform.rotation_format));
		printf("Translation format: %s\n", to_string(metadata.variant.transform.translation_format));
		if (metadata.variant.transform.scale_format != acl_sjson::vector_format_t::unknown)
			printf("Scale format: %s\n", to_string(metadata.variant.transform.scale_format));
	}
	else
	{
		// Scalar
	}

	return true;
}
