# Regression test data

In order to test the compression and decompression code path for our algorithms, we use a reduced subset of the [Carnegie-Mellon University](https://github.com/nfrechette/acl/blob/develop/docs/cmu_performance.md) database as well as some custom clips. This subset aims to cover a broad spectrum of clips.

## Best compression ratio

*  13_07
*  27_02
*  30_01
*  30_07
*  31_14
*  31_20
*  49_09
*  49_15
*  137_32
*  137_41

## Worst compression ratio

*  76_07
*  81_16
*  82_01
*  82_18
*  90_10
*  93_01
*  102_09
*  103_01
*  121_10
*  140_05

## Shortest duration

*  76_07
*  81_16
*  82_01
*  82_18
*  90_10
*  93_01
*  102_09
*  103_01
*  121_10
*  140_05

## Longest duration

*  12_04
*  15_02
*  15_04
*  15_05
*  29_12
*  29_13
*  30_11
*  30_12
*  32_11
*  32_12

## Most accurate

*  09_02
*  81_16
*  82_01
*  82_18
*  90_10
*  93_01
*  103_01
*  131_10
*  127_14
*  140_05

## Least accurate

*  26_06
*  27_06
*  27_08
*  28_01
*  28_04
*  40_08
*  81_18
*  104_30
*  142_17
*  144_34

## Miscellaneous

*  09_02_multi_root: 09_02 where the two hand bones have been converted to root bones
*  09_02_with_scale: 09_02 where one track has positive, and negative scale
*  09_02_with_zero_scale: 09_02 where one track has positive, zero, and negative scale
*  81_18_looping: 81_18 where the first sample repeats at the end to force a loop
*  track_list_float1f: a mix of float1f tracks
*  track_list_float2f: a mix of float2f tracks
*  track_list_float3f: a mix of float3f tracks
*  track_list_float3f_looping: same as above where the first sample repeats at the end to force a loop
*  track_list_float4f: a mix of float4f tracks
*  track_list_vector4f: a mix of vector4f tracks
*  track_list_remap_output: a mix of float4f tracks where the output is remapped
