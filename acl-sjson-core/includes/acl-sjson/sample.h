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

namespace acl_sjson
{
	struct quat
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct float1
	{
		float x;
	};

	struct float2
	{
		float x;
		float y;
	};

	struct float3
	{
		float x;
		float y;
		float z;
	};

	struct float4
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct vector4
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct qvv
	{
		quat rotation;
		vector4 translation;
		vector4 scale;
	};

	enum class sample_type
	{
		unknown,
		float1,
		float2,
		float3,
		float4,
		vector4,
		quat,
		qvv,
	};

	const char* to_string(sample_type type);

	union sample
	{
		float1 f1;
		float2 f2;
		float3 f3;
		float4 f4;
		vector4 v4;
		quat q;
		qvv transform;
	};
}
