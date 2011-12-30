require 'mkmf'
CLOWNFISH_INCLUDE_DIR   = File.join('..','..','..','include')
CLOWNFISH_SRC_DIR       = File.join('..','..','..','src')
$CFLAGS = "-I#{CLOWNFISH_INCLUDE_DIR} -I#{CLOWNFISH_SRC_DIR}"
$objs = ['CFC.o']
Dir.glob("#{CLOWNFISH_SRC_DIR}/*.o").each do|o|
    $objs.push o
end
create_makefile 'CFC'

