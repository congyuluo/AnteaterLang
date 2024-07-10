import subprocess
import os

from common import *

def anteater_compile_c(source_file):
    if not source_file.endswith(".c"):
        raise ValueError("Usage: anteater_compile_c <source_file.c>")

    include_dir = "/usr/local/include/anteaterlang"
    lib_dir = "/usr/local/lib"

    try:
        # Compile the user function
        compile_command = [
            "gcc",
            "-fPIC",
            f"-I{include_dir}",
            "-c",
            source_file,
            "-o",
            "userFunctions.o"
        ]
        result = subprocess.run(compile_command, check=True)

        if result.returncode != 0:
            return result.stderr

        # Create the shared library
        shared_lib_command = [
            "gcc",
            "-shared",
            "-o",
            C_ACC_LIBRARY_SO_PATH,
            "userFunctions.o",
            f"-L{lib_dir}",
            "-llang"
        ]
        result = subprocess.run(shared_lib_command, check=True)

        if result.returncode != 0:
            return result.stderr

        # Clean up
        if os.path.exists("userFunctions.o"):
            os.remove("userFunctions.o")

    except subprocess.CalledProcessError as e:
        return False, e.stderr
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
