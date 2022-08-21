import argparse
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
import time

def parse_argv():
	parser = argparse.ArgumentParser(add_help=False)

	actions = parser.add_argument_group(title='Actions', description='If no action is specified, on Windows, OS X, and Linux the solution/make files are generated.  Multiple actions can be used simultaneously.')
	actions.add_argument('-build', action='store_true')
	actions.add_argument('-clean', action='store_true')
	actions.add_argument('-convert', action='store_true', help="Converts the '-input' directory/file into '-output' directory/file with optional -target version")
	actions.add_argument('-input')
	actions.add_argument('-output')
	actions.add_argument('-target')

	target = parser.add_argument_group(title='Target')
	target.add_argument('-compiler', choices=['vs2019', 'clang14', 'gcc11', 'osx'], help='Defaults to the host system\'s default compiler')
	target.add_argument('-config', choices=['Debug', 'Release'], type=str.capitalize)
	target.add_argument('-cpu', choices=['x64', 'arm64'], help='Defaults to the host system\'s architecture')

	misc = parser.add_argument_group(title='Miscellaneous')
	misc.add_argument('-num_threads', help='No. to use while compiling and regressing')
	misc.add_argument('-ci', action='store_true', help='Whether or not this is a Continuous Integration build')
	misc.add_argument('-help', action='help', help='Display this usage information')

	num_threads = multiprocessing.cpu_count()
	if platform.system() == 'Linux' and sys.version_info >= (3, 4):
		num_threads = len(os.sched_getaffinity(0))
	if not num_threads or num_threads == 0:
		num_threads = 4

	parser.set_defaults(build=False, clean=False, compiler=None, config='Release', cpu=None, num_threads=num_threads)

	args = parser.parse_args()

	is_arm64_cpu = False
	if platform.machine() == 'arm64' or platform.machine() == 'aarch64':
		is_arm64_cpu = True

	# Sanitize and validate our options
	if not args.cpu:
		if is_arm64_cpu:
			args.cpu = 'arm64'
		else:
			args.cpu = 'x64'

	if args.cpu == 'arm64':
		is_arm_supported = False

		# Native compilation
		if platform.system() == 'Darwin' and is_arm64_cpu:
			is_arm_supported = True
		elif platform.system() == 'Linux' and is_arm64_cpu:
			is_arm_supported = True

		if not is_arm_supported:
			print('arm64 is only supported with OS X (M1 processors), and Linux')
			sys.exit(1)

	return args

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
		if compiler == 'clang14':
			os.environ['CC'] = 'clang-14'
			os.environ['CXX'] = 'clang++-14'
		elif compiler == 'gcc11':
			os.environ['CC'] = 'gcc-11'
			os.environ['CXX'] = 'g++-11'
		else:
			print('Unknown compiler: {}'.format(compiler))
			print('See help with: python make.py -help')
			sys.exit(1)

def do_generate_solution(install_dir, cmake_script_dir, args):
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
	cmake_generator = get_generator(compiler, cpu)
	if not cmake_generator:
		print('Using default generator')
	else:
		print('Using generator: {}'.format(cmake_generator))
		extra_switches.append(' -G "{}"'.format(cmake_generator))

	cmake_arch = get_architecture(compiler, cpu)
	if cmake_arch:
		print('Using architecture: {}'.format(cmake_arch))
		extra_switches.append(' -A {}'.format(cmake_arch))

	cmake_cmd = 'cmake .. -DCMAKE_INSTALL_PREFIX="{}" {}'.format(install_dir, ' '.join(extra_switches))

	result = subprocess.call(cmake_cmd, shell=True)
	if result != 0:
		sys.exit(result)

def do_build(args):
	config = args.config

	print('Building ...')
	cmake_cmd = 'cmake --build .'
	if platform.system() == 'Windows':
		cmake_cmd += ' --config {} --target INSTALL'.format(config)
	elif platform.system() == 'Darwin':
		cmake_cmd += ' --config {} --target install'.format(config)
	else:
		cmake_cmd += ' --target install'

	result = subprocess.call(cmake_cmd, shell=True)
	if result != 0:
		sys.exit(result)

def format_elapsed_time(elapsed_time):
	hours, rem = divmod(elapsed_time, 3600)
	minutes, seconds = divmod(rem, 60)
	if hours > 0:
		return '{:0>2}h {:0>2}m {:05.2f}s'.format(int(hours), int(minutes), seconds)
	elif minutes > 0:
		return '{:0>2}m {:05.2f}s'.format(int(minutes), seconds)
	else:
		return '{:05.2f}s'.format(seconds)

def print_progress(iteration, total, prefix='', suffix='', decimals = 1, bar_length = 40):
	# Taken from https://stackoverflow.com/questions/3173320/text-progress-bar-in-the-console
	# With minor tweaks
	"""
	Call in a loop to create terminal progress bar
	@params:
		iteration   - Required  : current iteration (Int)
		total       - Required  : total iterations (Int)
		prefix      - Optional  : prefix string (Str)
		suffix      - Optional  : suffix string (Str)
		decimals    - Optional  : positive number of decimals in percent complete (Int)
		bar_length  - Optional  : character length of bar (Int)
	"""
	str_format = "{0:." + str(decimals) + "f}"
	percents = str_format.format(100 * (iteration / float(total)))
	filled_length = int(round(bar_length * iteration / float(total)))
	bar = '█' * filled_length + '-' * (bar_length - filled_length)

	# We need to clear any previous line we might have to ensure we have no visual artifacts
	# Note that if this function is called too quickly, the text might flicker
	terminal_width = 80
	sys.stdout.write('{}\r'.format(' ' * terminal_width))
	sys.stdout.flush()

	sys.stdout.write('%s |%s| %s%s %s\r' % (prefix, bar, percents, '%', suffix)),
	sys.stdout.flush()

	if iteration == total:
		sys.stdout.write('\n')

def do_convert(args, root_dir):
	old_cwd = os.getcwd()
	os.chdir(root_dir)

	if not os.path.exists(args.input):
		print('Input conversion directory or file not found: {}'.format(args.input))
		sys.exit(1)

	if not args.output:
		print('Output conversion directory or file not found: {}'.format(args.output))
		sys.exit(1)

	# Validate that our tool is present
	if platform.system() == 'Windows':
		tool_path = './bin/acl-sjson.exe'
	else:
		tool_path = './bin/acl-sjson'

	tool_path = os.path.abspath(tool_path)
	if not os.path.exists(tool_path):
		print('acl-sjson executable not found: {}'.format(tool_path))
		sys.exit(1)

	# Grab all the clips to convert
	conversion_clips = []
	if os.path.isfile(args.input):
		if not args.input.endswith('.acl.sjson') and not args.input.endswith('.acl'):
			print('Expected an ACL file format as input: {}'.format(args.input))
			sys.exit(1)

		if not args.output.endswith('.acl.sjson') and not args.output.endswith('.acl'):
			print('Expected an ACL file format as output: {}'.format(args.output))
			sys.exit(1)

		input_filename = os.path.abspath(args.input)
		output_filename = os.path.abspath(args.output)

		if not os.path.exists(output_filename):
			os.makedirs(output_filename)

		conversion_clips.append((input_filename, output_filename))
	elif os.path.isdir(args.input):
		for (dirpath, dirnames, filenames) in os.walk(args.input):
			for filename in filenames:
				if not filename.endswith('.acl.sjson') and not filename.endswith('.acl'):
					continue

				# Ignore the reference file format, it isn't a valid file
				if filename == 'format_reference.acl.sjson':
					continue

				input_filename = os.path.join(dirpath, filename)
				# Always convert to binary
				output_filename = os.path.join(args.output, filename.replace('.acl.sjson', '.acl'))
				conversion_clips.append((input_filename, output_filename))

		if os.path.exists(args.output):
			shutil.rmtree(args.output)

		if not os.path.exists(args.output):
			os.makedirs(args.output)
	else:
		print('Unexpected input found: {}'.format(args.input))
		sys.exit(1)

	if len(conversion_clips) == 0:
		print('No clips found to convert')
		sys.exit(0)

	conversion_start_time = time.perf_counter()
	num_processed = 0
	print_progress(num_processed, len(conversion_clips), 'Converting clips:', '{} / {}'.format(num_processed, len(conversion_clips)))
	conversion_failed = False
	for (input_filename, output_filename) in conversion_clips:
		cmd = '"{}" --convert "{}" "{}"'.format(tool_path, input_filename, output_filename)

		if args.target:
			cmd = "{} --target {}".format(cmd, args.target)

		if platform.system() == 'Windows':
			cmd = cmd.replace('/', '\\')

		result = subprocess.call(cmd, shell=True)

		num_processed += 1
		print_progress(num_processed, len(conversion_clips), 'Converting clips:', '{} / {}'.format(num_processed, len(conversion_clips)))

		if result != 0:
			print('Failed to run conversion for clip: {}'.format(input_filename))
			print(cmd)
			conversion_failed = True

	conversion_end_time = time.perf_counter()
	print('Done in {}'.format(format_elapsed_time(conversion_end_time - conversion_start_time)))

	os.chdir(old_cwd)

	if conversion_failed:
		sys.exit(1)

if __name__ == "__main__":
	args = parse_argv()

	root_dir = os.getcwd()
	build_dir = os.path.join(root_dir, 'build')
	install_dir = os.path.join(root_dir, 'bin')
	cmake_script_dir = os.path.join(root_dir, 'cmake')

	if args.clean:
		print('Cleaning previous build ...')
		if os.path.exists(build_dir):
			shutil.rmtree(build_dir)
		if os.path.exists(install_dir):
			shutil.rmtree(install_dir)

	if not os.path.exists(build_dir):
		os.makedirs(build_dir)
	if not os.path.exists(install_dir):
		os.makedirs(install_dir)

	os.chdir(build_dir)

	print('Using config: {}'.format(args.config))
	print('Using cpu: {}'.format(args.cpu))
	if args.compiler:
		print('Using compiler: {}'.format(args.compiler))
	print('Using {} threads'.format(args.num_threads))

	# Make sure 'make' runs with all available cores
	os.environ['MAKEFLAGS'] = '-j{}'.format(args.num_threads)

	do_generate_solution(install_dir, cmake_script_dir, args)

	if args.build:
		do_build(args)

	if args.convert:
		do_convert(args, root_dir)

	sys.exit(0)
