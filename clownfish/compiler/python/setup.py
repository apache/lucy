from distutils.core import setup, Extension
from distutils.command.build import build as _build
from distutils.command.clean import clean as _clean
from distutils.cmd import Command as _Command
import distutils.ccompiler
import os
import subprocess
import platform
import sysconfig

PARENT_DIR      = os.path.abspath(os.pardir)
CFC_SOURCE_DIR  = os.path.join(PARENT_DIR, 'src')
CFC_INCLUDE_DIR = os.path.join(PARENT_DIR, 'include')
BASE_DIR = os.path.abspath(os.path.join(os.pardir, os.pardir, os.pardir))
LEMON_DIR = os.path.join(BASE_DIR, 'lemon')
LEMON_EXE_NAME = 'lemon' + sysconfig.get_config_var("EXE")
LEMON_EXE_PATH = os.path.join(LEMON_DIR, LEMON_EXE_NAME)

# There's no good way to get a string representing the compiler executable out
# of distutils, so for now we'll kludge it and assume it's the same as the
# compiler used to build python.
python_compiler = sysconfig.get_config_var('CC')
compiler_type   = distutils.ccompiler.get_default_compiler()

def _run_make(command=[], directory=None):
    current_directory = os.getcwd();
    if (directory != None):
        os.chdir(directory)
    if (compiler_type == 'msvc'):
        command.insert(0, 'Makefile.MSVC')
        command.insert(0, '-f')
    elif (platform.system() == 'Windows'):
        command.insert(0, 'Makefile.MinGW')
        command.insert(0, '-f')
    command.insert(0, "make")
    subprocess.check_call(command)
    if (directory != None):
        os.chdir(current_directory)

class lemon(_Command):
    description = "Compile the Lemon parser generator"
    user_options = []
    def initialize_options(self):
        pass
    def finalize_options(self):
        pass
    def run(self):
        if not os.path.exists(LEMON_EXE_PATH):
            _run_make(['CC=' + python_compiler], directory=LEMON_DIR)

class my_clean(_clean):
    def run(self):
        _clean.run(self)
        _run_make(command=['clean'], directory=LEMON_DIR)

class my_build(_build):
    def run(self):
        self.run_command('lemon')
        _build.run(self)

c_filepaths = []
for (dirpath, dirnames, files) in os.walk(CFC_SOURCE_DIR):
    for filename in files:
        if (filename.endswith('.c')):
            c_filepaths.append(os.path.join(dirpath, filename))

cfc_extension = Extension('clownfish.cfc',
                          define_macros = [('CFCPYTHON', None)],
                          include_dirs = [CFC_INCLUDE_DIR, CFC_SOURCE_DIR],
                          sources = c_filepaths)

setup(name = 'clownfish-cfc',
      version = '0.3.0',
      description = 'Clownfish compiler',
      author = 'Apache Lucy Project',
      author_email = 'dev at lucy dot apache dot org',
      url = 'http://lucy.apache.org',
      cmdclass = {
          'build': my_build,
          'clean': my_clean,
          'lemon': lemon,
      },
      ext_modules = [cfc_extension])

