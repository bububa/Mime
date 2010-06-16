# Required Liberaries
# Boost 1.43.0
# openclas http://code.google.com/p/openclas/
# ThreadPool p.rt
# jsoncpp 0.5.0 http://jsoncpp.sourceforge.net/
# Scons Build System http://www.scons.org/
# Botan 1.8.8 http://botan.randombit.net/
# MongoDB

import re
import sys
import os
import shutil
# .Configure
def read_config_file(filename):
    try:
        fp = file(filename, 'r')
        parameters = dict([line.strip().split(" ", 1) for line in fp.readlines() if line.strip() and not line.startswith(';')])
        fp.close()
    except IOError:
        print "Can't read %s"%filename
        sys.exit(0)
    return parameters

def read_config_header():
    try:
        fp = file('src/libmime/Config.h', 'r')
        config_header = fp.read();
        fp.close()
    except IOError:
        print "Can't read Config.h"
        return false;
    return config_header

def configure():
    print 'Read configure.info'
    parameters = read_config_file('configure.info')
    if not parameters: return;
    print 'Writing configure header'
    header = read_config_header()
    if 'DATA_PATH' in parameters:
        header = re.sub('static const std::string DATA_PATH[^^]*?;', 'static const std::string DATA_PATH("%s");'%parameters['DATA_PATH'], header)
    if 'MONGODB_HOST' in parameters:
        header = re.sub('static const std::string MONGODB_HOST[^^]*?;', 'static const std::string MONGODB_HOST("%s");'%parameters['MONGODB_HOST'], header)
    if 'LOG_HOST' in parameters:
        header = re.sub('static const std::string LOG_HOST[^^]*?;', 'static const std::string LOG_HOST("%s");'%parameters['LOG_HOST'], header)
    if 'DICT_PATH' in parameters:
        header = re.sub('static const std::string DICT_PATH[^^]*?;', 'static const std::string DICT_PATH("%s");'%parameters['DICT_PATH'], header)
    if 'DEBUG_LEVEL' in parameters:
        debug_level = {'off':'DEBUG_OFF', 'normal':'DEBUG_NORMAL', 'warning':'DEBUG_WARNING', 'error':'DEBUG_ERROR', 'critical':'DEBUG_CRITICAL'}
        header = re.sub('static const int DEBUG_LEVEL[^^]*?;', 'static const int DEBUG_LEVEL = mime::%s;'%debug_level[parameters['DEBUG_LEVEL']], header)
    try:
        fp = file('src/libmime/Config.h', 'w')
        fp.write(header)
        fp.close()
    except IOError:
        print "Can't write Config.h"
        sys.exit(0)
    #cookie_path = os.path.join(parameters['DATA_PATH'], 'cookies')
    #rule_path = os.path.join(parameters['DATA_PATH'], 'rules')
    #if not os.path.exists(cookie_path):
    #    os.makedirs(cookie_path)
    #if not os.path.exists(rule_path):
    #    os.makedirs(rule_path)

def copy_headers():
    print "Copying header files"
    src = 'src/libmime'
    dst = 'include/mime'
    names = [n for n in os.listdir(src) if n.endswith('.h')]
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            shutil.copyfile(srcname, dstname)
            print "Copy %s to %s"%(srcname, dstname)
        except IOError:
            print "Can't copy %s to %s"%(srcname, dstname)
            sys.exit(0)

# Check Headers
env = Environment(LIBPATH=["#build/libmime", "#build/libticppd"])
conf = Configure(env)
libs = ('boost_regex', 'boost_filesystem', 'boost_system', 'boost_program_options', 'boost_date_time', 'boost_thread', 'json_linux-gcc-4.2.1_libmt', 'botan', 'mongoclient', 'libmmseg', 'tidy', 'ticppd');
for lib in libs:
    if not conf.CheckLib(lib):
        print 'Did not find lib%s.a or %s.lib, exiting!'%(lib, lib)
        sys.exit(0)
env = conf.Finish()
# Write config.h
configure()
# Copy headers to include path
copy_headers()
# Build
VariantDir('build', 'src', duplicate=0)
SConscript('build/SConscript')

#env = Environment()
#Import("airport eva inserter")
#env.Install('/usr/local/bin', [airport, eva, inserter])
#env.Alias('install', '/usr/local/bin')