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

bool convert(const command_line_options& options)
{
    if (options.input_filename == options.output_filename)
	{
		printf("Input and output cannot be the same file\n");
		return false;
	}

    acl_sjson::track_array tracks;
    if (!read_tracks(options.input_filename.c_str(), tracks))
        return false;

    if (tracks.get_version() == acl_sjson::acl_version::unknown)
    {
        printf("Unknown ACL version used in input file\n");
        return false;
    }

    switch (options.output_version)
    {
    case acl_sjson::acl_version::v02_00_00:
        if (!acl_sjson_v20::write_tracks(options.output_filename.c_str(), tracks))
            return false;
        break;
    case acl_sjson::acl_version::v02_01_00:
        if (!acl_sjson_v21::write_tracks(options.output_filename.c_str(), tracks))
            return false;
        break;
    default:
		// By default, if no target version is specified, we maintain the source version
		switch (tracks.get_version())
		{
		case acl_sjson::acl_version::v02_00_00:
			if (!acl_sjson_v20::write_tracks(options.output_filename.c_str(), tracks))
				return false;
			break;
		case acl_sjson::acl_version::v02_01_00:
			if (!acl_sjson_v21::write_tracks(options.output_filename.c_str(), tracks))
				return false;
			break;
		default:
			printf("Unsupported source version\n");
			return false;
		}
		break;
    }

    // Done!
    return true;
}
