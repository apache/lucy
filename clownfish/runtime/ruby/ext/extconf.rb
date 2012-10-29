require 'mkmf'
require 'rbconfig'

CORE    = File.join('..','..','..','..','..','..','runtime','core')
RUBY    = File.join('..','..','..','..','..','..','runtime','ruby')
AUTOGEN = File.join('..','..','..','..','..','..','runtime','ruby','autogen','include')
$CFLAGS = "-I#{CORE} -I#{RUBY} -I#{AUTOGEN}"

create_makefile 'Bind'
