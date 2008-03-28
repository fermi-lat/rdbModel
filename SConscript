#$Id:  $
# rdbModel SConscript file
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('rdbModelLib', depsOnly = 1)
rdbModelLib = libEnv.SharedLibrary('rdbModel', [ 'src/Rdb.cxx', listFiles(['src/Tables/*.cxx']), listFiles(['src/Management/*.cxx']),listFiles(['src/Db/*.cxx'])])

progEnv.Tool('rdbModelLib')
test_build = progEnv.Program('test_build', ['src/test/test_build.cxx'])
test_errors = progEnv.Program('test_errors', ['src/test/test_errors.cxx'])
initRdb = progEnv.Program('initRdb', ['src/test/initRdbMain.cxx','src/test/InitRdb.cxx'])

progEnv.Tool('registerObjects', package = 'rdbModel', libraries = [rdbModelLib], testApps = [test_build, test_errors, initRdb],
            includes = listFiles(['rdbModel/*.h', 'rdbModel/Tables/*.h', 'rdbModel/Management/*.h', 'rdbModel/Db/*.h']))
