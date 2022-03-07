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

#include <cstring>

command_line_options::command_line_options()
	: action(command_line_action::none)
	, input_filename()
	, output_filename()
{}

static void print_usage()
{
	printf("Usage: acl-sjson --convert <input_file> <output_file>\n");
	printf("This utility convers between two ACL file formats.\n");
	printf("Human readable files end with the *.acl.sjson extension.\n");
	printf("Binary files end with the *.acl extension.\n");
}

static bool is_str_equal(const char* argument0, const char* argument1)
{
	const size_t length1 = std::strlen(argument1);
	return std::strncmp(argument0, argument1, length1) == 0;
}

bool parse_command_line_arguments(int argc, char* argv[], command_line_options& out_options)
{
	command_line_options options;

	for (int arg_index = 1; arg_index < argc; ++arg_index)
	{
		const char* argument = argv[arg_index];

		if (is_str_equal(argument, "--convert"))
		{
			if (options.action != command_line_action::none)
			{
				printf("Only one action can be provided\n");
				print_usage();
				return false;
			}

			if (arg_index + 2 >= argc)
			{
				printf("--convert requires input and output files\n");
				print_usage();
				return false;
			}

			options.action = command_line_action::convert;
			options.input_filename = argv[arg_index + 1];
			options.output_filename = argv[arg_index + 2];

			arg_index += 2;
		}

		// Unknown arguments are ignored silently
	}

	if (options.action == command_line_action::none)
	{
		// No options provided, print usage
		print_usage();
		return false;
	}

	out_options = options;
	return true;
}
