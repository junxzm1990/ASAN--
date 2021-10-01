#! /usr/bin/env/python 3.0
#
# Running this script will update the file "compile_all.bat", which can be 
# run to compile all the test cases into a single .exe file.  This script
# also edits source code and header files needed for a successful compilation
# with this batch file.
#

import sys,os

# add parent directory to search path so we can use py_common
sys.path.append("..")

import py_common

import update_main_cpp_and_testcases_h
import create_per_cwe_files

def create_batch_file(dirs, cflags, lflags):
	contents = ""
	contents += "\nrem NOTE: this batch file is to be run in a Visual Studio command prompt\n"

	contents += "\nrem Delete old files\n"
	contents += "del " + "*.obj\n"
	contents += "del " + "*.ilk\n"
	contents += "del " + "*.exe\n"

	contents += "\nrem Delete, compile and link files into .obj files in current directory\n"
	contents += 'cl ' + cflags + \
				' "testcasesupport\main.cpp"' + \
				' "testcasesupport\io.c"' + \
				' "testcasesupport\std_thread.c"\n'
				
	for dir in sorted(dirs):
		# testcasesupport is a special directory and needs to be handled separately
		if 'testcasesupport' not in dir:
			contents += 'cl ' + cflags + " "
		
			# Only add *.c if there is a .c file in the dir - there will always be at least 1 .cpp file (main.cpp)
			if py_common.find_files_in_dir(dir, "CWE.*\.c$"):
				contents += '"' + os.path.relpath(dir) + '\\CWE*.c" '
		
			if py_common.find_files_in_dir(dir, "CWE.*\.cpp$"):
				contents += '"' + os.path.relpath(dir) + '\\CWE*.cpp"'
		
			contents += '\n'
	
	contents += 'cl ' + "/Fe" + "Testcases" + " *.obj " + lflags + "\n"

	return contents

def get_directory_names_to_compile(directory):
	files = py_common.find_files_in_dir(directory, "(\.c|\.cpp)$")

	dirs = set()
	for file in files:
		base_dir = os.path.dirname(file)
		dirs.add(base_dir)
	
	return dirs

if __name__ == "__main__":

	# check if ./testcases directory exists, if not, we are running
	# from wrong working directory
	if not os.path.exists("testcases"):
		print_with_timestamp("Wrong working directory; could not find testcases directory")
		exit()
	
	# update main.cpp/testcases.h to call only this cwe's testcases
	testcase_files = update_main_cpp_and_testcases_h.build_list_of_primary_c_cpp_testcase_files("testcases", None)
	fcl = update_main_cpp_and_testcases_h.generate_calls_to_fxs(testcase_files)
	update_main_cpp_and_testcases_h.update_main_cpp("testcasesupport", "main.cpp", fcl)
	update_main_cpp_and_testcases_h.update_testcases_h("testcasesupport", "testcases.h", fcl)

	dirs = get_directory_names_to_compile("testcases")
	dirs = dirs.union(get_directory_names_to_compile("testcasesupport"))

	linker_flags = create_per_cwe_files.linker_flags.replace("..\\..\\", "")
	compile_flags = linker_flags + " /c"
	bat_contents = create_batch_file(dirs, compile_flags, linker_flags)

	py_common.write_file("compile_all.bat", bat_contents)
