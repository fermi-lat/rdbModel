# -*- python -*-
# $Id: SConscript,v 1.14 2012/07/06 19:18:59 jrb Exp $
# rdbModel SConscript file
# Authors: Joanne Bogart <jrb@slac.stanford.edu>
# Version: rdbModel-02-15-00
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='rdbModel', toBuild='shared')
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





