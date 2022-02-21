import argparse
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys

def parse_argv():
	parser = argparse.ArgumentParser(add_help=False)

	actions = parser.add_argument_group(title='Actions', description='If no action is specified, on Windows, OS X, and Linux the solution/make files are generated.  Multiple actions can be used simultaneously.')
	actions.add_argument('-build', action='store_true')
	actions.add_argument('-clean', action='store_true')

	target = parser.add_argument_group(title='Target')
	target.add_argument('-compiler', choices=['vs2019', 'clang9', 'gcc9', 'osx'], help='Defaults to the host system\'s default compiler')
	target.add_argument('-config', choices=['Debug', 'Release'], type=str.capitalize)
	target.add_argument('-cpu', choices=['x86', 'x64', 'arm64'], help='Only supported for Windows, OS X, and Linux; defaults to the host system\'s architecture')

	misc = parser.add_argument_group(title='Miscellaneous')
	misc.add_argument('-num_threads', help='No. to use while compiling and regressing')
	misc.add_argument('-help', action='help', help='Display this usage information')

	num_threads = multiprocessing.cpu_count()
	if platform.system() == 'Linux' and sys.version_info >= (3, 4):
		num_threads = len(os.sched_getaffinity(0))
	if not num_threads or num_threads == 0:
		num_threads = 4

	parser.set_defaults(build=False, clean=False, compiler=None, config='Release', cpu='x64', use_avx=False, use_simd=True, num_threads=num_threads)

	args = parser.parse_args()

	# Sanitize and validate our options

	return args

def get_cmake_exes():
	if platform.system() == 'Windows':
		return ('cmake.exe', 'ctest.exe')
	else:
		return ('cmake', 'ctest')

def get_generator(compiler, cpu):
	if not compiler:
		return None

	if platform.system() == 'Windows':
		if compiler == 'vs2019':
			return 'Visual Studio 16 2019'
	elif platform.system() == 'Darwin':
		if compiler == 'osx':
			return 'Xcode'
	else:
		return 'Unix Makefiles'

	print('Unknown compiler: {}'.format(compiler))
	print('See help with: python make.py -help')
	sys.exit(1)

def get_architecture(compiler, cpu):
	if not compiler:
		return None

	if platform.system() == 'Windows':
		if compiler == 'vs2019':
			if cpu == 'x86':
				return 'Win32'
			else:
				return cpu

	# This compiler/cpu pair does not need the architecture switch
	return None

def set_compiler_env(compiler, args):
	if platform.system() == 'Linux':
		os.environ['MAKEFLAGS'] = '-j{}'.format(args.num_threads)
		if compiler == 'clang9':
			os.environ['CC'] = 'clang-9'
			os.environ['CXX'] = 'clang++-9'
		elif compiler == 'gcc9':
			os.environ['CC'] = 'gcc-9'
			os.environ['CXX'] = 'g++-9'
		else:
			print('Unknown compiler: {}'.format(compiler))
			print('See help with: python make.py -help')
			sys.exit(1)

def do_generate_solution(cmake_exe, build_dir, cmake_script_dir, args):
	compiler = args.compiler
	cpu = args.cpu
	config = args.config

	if compiler:
		set_compiler_env(compiler, args)

	extra_switches = ['--no-warn-unused-cli']
	extra_switches.append('-DCPU_INSTRUCTION_SET:STRING={}'.format(cpu))

	if not platform.system() == 'Windows' and not platform.system() == 'Darwin':
		extra_switches.append('-DCMAKE_BUILD_TYPE={}'.format(config.upper()))

	# Generate IDE solution
	print('Generating build files ...')
	cmake_cmd = '"{}" .. -DCMAKE_INSTALL_PREFIX="{}" {}'.format(cmake_exe, build_dir, ' '.join(extra_switches))
	cmake_generator = get_generator(compiler, cpu)
	if not cmake_generator:
		print('Using default generator')
	else:
		print('Using generator: {}'.format(cmake_generator))
		cmake_cmd += ' -G "{}"'.format(cmake_generator)

	cmake_arch = get_architecture(compiler, cpu)
	if cmake_arch:
		print('Using architecture: {}'.format(cmake_arch))
		cmake_cmd += ' -A {}'.format(cmake_arch)

	result = subprocess.call(cmake_cmd, shell=True)
	if result != 0:
		sys.exit(result)

def do_build(cmake_exe, args):
	config = args.config

	print('Building ...')
	cmake_cmd = '"{}" --build .'.format(cmake_exe)
	if platform.system() == 'Windows':
		cmake_cmd += ' --config {} --target INSTALL'.format(config)
	elif platform.system() == 'Darwin':
		cmake_cmd += ' --config {} --target install'.format(config)
	else:
		cmake_cmd += ' --target install'

	result = subprocess.call(cmake_cmd, shell=True)
	if result != 0:
		sys.exit(result)

if __name__ == "__main__":
	args = parse_argv()

	cmake_exe, ctest_exe = get_cmake_exes()
	compiler = args.compiler
	cpu = args.cpu
	config = args.config

	build_dir = os.path.join(os.getcwd(), 'build')
	cmake_script_dir = os.path.join(os.getcwd(), 'cmake')

	if args.clean and os.path.exists(build_dir):
		print('Cleaning previous build ...')
		shutil.rmtree(build_dir)

	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	print('Using config: {}'.format(config))
	print('Using cpu: {}'.format(cpu))
	if compiler:
		print('Using compiler: {}'.format(compiler))
	print('Using {} threads'.format(args.num_threads))

	do_generate_solution(cmake_exe, build_dir, cmake_script_dir, args)

	if args.build:
		do_build(cmake_exe, args)

	sys.exit(0)
