require 'mkmf'
require 'rbconfig'

CLOWNFISH_INCLUDE_DIR   = File.join('..','..','..','include')
CLOWNFISH_SRC_DIR       = File.join('..','..','..','src')
$CFLAGS = "-I#{CLOWNFISH_INCLUDE_DIR} -I#{CLOWNFISH_SRC_DIR}"
$objs = ['CFC.' + RbConfig::CONFIG['OBJEXT']]
obj_glob = File.join(CLOWNFISH_SRC_DIR, '*.' + RbConfig::CONFIG['OBJEXT'])
Dir.glob(obj_glob).each do|o|
    $objs.push o
end

create_makefile 'CFC'
