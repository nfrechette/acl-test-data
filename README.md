# acl-test-data
Regression test data for the [Animation Compression Library](https://github.com/nfrechette/acl)

## How to generate regression test files

Every time regression tests need to be updated, whether a new test is added or a new version, we have to convert the original reference clips into our various target versions.

Steps:
*  Build everything through: `python make.py -build`
*  Run conversion: `python make.py -convert -input=./regression_tests -output=./output_clips`

Note that the build and conversion can be combined into a single step.

By default, this converts every file into the latest version supported. An optional target version can be provided with `-target 2.0`. Supported target versions are:

*  2.0
*  2.1

By default, clips will maintain their original compression format: raw files remain raw, compressed files remain compressed.

For convenience, a single command can generate the zip file used for a package release:
`python make.py -package`
It will output its results under `./output_regression_tests` and a zip file is created: `./acl_regression_tests_vXXX.zip` where `XXX` is the version specified at the top of `make.py`.
