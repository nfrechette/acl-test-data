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
	acl_sjson::track_array tracks(acl_sjson::acl_version::unknown);
	if (!read_tracks(options.input_filename.c_str(), tracks))
		return false;

	if (tracks.get_version() == acl_sjson::acl_version::unknown)
	{
		printf("Unknown ACL version used in input file\n");
		return false;
	}

	printf("Filename: %s\n", options.input_filename.c_str());
	printf("Version: %s\n", acl_sjson::to_string(tracks.get_version()));
	printf("Num tracks: %u\n", static_cast<uint32_t>(tracks.get_num_tracks()));
	printf("Num samples per track: %u\n", static_cast<uint32_t>(tracks.get_num_samples_per_track()));
	printf("Sample rate: %.2f\n", tracks.get_sample_rate());

	return true;
}
