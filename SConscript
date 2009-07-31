# -*- python -*-
# $Id: SConscript,v 1.6 2009/07/31 00:35:27 jrb Exp $
# rdbModel SConscript file
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: rdbModel-02-14-01
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

progEnv.Tool('registerTargets', package = 'rdbModel',
             libraryCxts = [[rdbModelLib,libEnv]],
             testAppCxts = [[test_build, progEnv], [test_errors, progEnv],
                            [initRdb,progEnv]],
             includes = listFiles(['rdbModel/*'], recursive = True),
             xml = listFiles(['xml/*'], recursive=True))





