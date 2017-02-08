Build instructions for the Apache Lucy C library
================================================

Clownfish dependency
--------------------

**IMPORTANT:** The Apache Lucy C library depends on the Clownfish object system
which must be built first. Please read this section, or your build will
probably **FAIL**.

Lucy needs the following Clownfish components:

* The command-line Clownfish compiler 'cfc'.
* The Clownfish header files of the Clownfish runtime.
* The 'libcfish' shared library.

If you installed Clownfish to /usr/local or /usr on UNIX, no further action
should be required. If you installed Clownfish to an alternate location,
make sure to use the `--clownfish-prefix` configure option:

    ./configure --clownfish-prefix=[clownfish-install-dir]

It's also possible to build Lucy with an uninstalled Clownfish build from
the Clownfish source directory. To setup the required environment variables,
source the file `devel/bin/setup_env.sh` from the Clownfish source tree:

    source [path_to_clownfish]/devel/bin/setup_env.sh

Or, if you're on the Windows shell:

    [path_to_clownfish]/devel/bin/setup_env.bat

Building under UNIX and derivatives or Cygwin
---------------------------------------------

    ./configure --prefix [install-prefix]
    make
    make test
    make install

Building under Windows
----------------------

You need MSVC or gcc as C compiler and nmake, mingw32-make, or standard
GNU make as make utility. When building under cmd.exe, configure with:

    configure.bat --prefix [install-prefix]

Configuration
-------------

    [environment] ./configure [options] [-- [cflags]]

### Options

    --enable-coverage

Enable code coverage. Create HTML pages with coverage data using
lcov by running "make coverage".

    --clownfish-prefix=[path]

Specify an alternative location for Clownfish if it isn't installed
in a system directory.

### Environment variables

    CC

The C compiler.

    TARGET_CC

The target compiler when cross-compiling. `CC` can be set in addition
to specify the host compiler.

