from distutils.core import setup, Extension
from distutils.command.build import build as _build
from distutils.command.clean import clean as _clean
from distutils.cmd import Command as _Command
from distutils.dep_util import newer_group
import distutils.ccompiler
import os
import glob
import platform
import shutil
import subprocess
import sysconfig

# Get a compiler object and and strings representing the compiler type and
# CFLAGS.
compiler = distutils.ccompiler.new_compiler()
cflags = sysconfig.get_config_var('CFLAGS')
compiler_type = distutils.ccompiler.get_default_compiler()

# There's no public way to get a string representing the compiler executable
# out of distutils, but the member variable has been in the same place for a
# long time, so violating encapsulation may be ok.
compiler_name = " ".join(compiler.compiler)

BASE_DIR = os.path.abspath(os.path.join(os.pardir, os.pardir, os.pardir))
PARENT_DIR      = os.path.abspath(os.pardir)
CFC_SOURCE_DIR  = os.path.join(PARENT_DIR, 'src')
CFC_INCLUDE_DIR = os.path.join(PARENT_DIR, 'include')
COMMON_SOURCE_DIR    = os.path.join(PARENT_DIR, 'common')
CHARMONIZER_C        = os.path.join(COMMON_SOURCE_DIR, 'charmonizer.c')
CHARMONIZER_EXE_NAME = compiler.executable_filename('charmonizer')
CHARMONIZER_EXE_PATH = os.path.join(os.curdir, CHARMONIZER_EXE_NAME)
CHARMONY_H_PATH      = 'charmony.h'
LEMON_DIR = os.path.join(BASE_DIR, 'lemon')
LEMON_EXE_NAME = compiler.executable_filename('lemon')
LEMON_EXE_PATH = os.path.join(LEMON_DIR, LEMON_EXE_NAME)

def _quotify(text):
    text = text.replace('\\', '\\\\')
    text = text.replace('"', '\\"')
    return '"' + text + '"'

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

class charmony(_Command):
    description = "Build and run charmonizer"
    user_options = []
    def initialize_options(self):
        pass
    def finalize_options(self):
        pass
    def run(self):
        # Compile charmonizer.
        if newer_group([CHARMONIZER_C], CHARMONIZER_EXE_PATH):
            command = [compiler_name]
            if compiler_type == 'msvc':
                command.append('/Fe' + CHARMONIZER_EXE_PATH)
            else:
                command.extend(['-o', CHARMONIZER_EXE_PATH])
            command.append(CHARMONIZER_C)
            print(" ".join(command))
            subprocess.check_call(command)

        # Run charmonizer.
        if newer_group([CHARMONIZER_EXE_PATH], CHARMONY_H_PATH):
            command = [
                CHARMONIZER_EXE_PATH,
                '--cc=' + _quotify(compiler_name),
                '--enable-c',
                '--',
                cflags
            ]
            if 'CHARM_VALGRIND' in os.environ:
                command[0:0] = "valgrind", "--leak-check=yes";
            print(" ".join(command))
            subprocess.check_call(command)

class lemon(_Command):
    description = "Compile the Lemon parser generator"
    user_options = []
    def initialize_options(self):
        pass
    def finalize_options(self):
        pass
    def run(self):
        if not os.path.exists(LEMON_EXE_PATH):
            _run_make(['CC=' + _quotify(compiler_name)], directory=LEMON_DIR)

class my_clean(_clean):
    paths_to_clean = [
        CHARMONIZER_EXE_PATH,
        CHARMONY_H_PATH,
        '_charm*',
    ]
    def run(self):
        _clean.run(self)
        _run_make(command=['clean'], directory=LEMON_DIR)
        for elem in self.paths_to_clean:
            for path in glob.glob(elem):
                print("removing " + path)
                if os.path.isdir(path):
                    shutil.rmtree(path)
                else:
                    os.unlink(path)

class my_build(_build):
    def run(self):
        self.run_command('charmony')
        self.run_command('lemon')
        _build.run(self)

c_filepaths = []
for (dirpath, dirnames, files) in os.walk(CFC_SOURCE_DIR):
    for filename in files:
        if (filename.endswith('.c')):
            c_filepaths.append(os.path.join(dirpath, filename))

cfc_extension = Extension('clownfish.cfc',
                          define_macros = [('CFCPYTHON', None)],
                          include_dirs = [
                              CFC_INCLUDE_DIR,
                              CFC_SOURCE_DIR,
                              os.curdir,
                          ],
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
          'charmony': charmony,
      },
      ext_modules = [cfc_extension])

