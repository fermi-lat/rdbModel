#$Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rdbModel/rdbModelLib.py,v 1.2 2008/06/30 20:18:34 glastrm Exp $
# rdbModelLib.py
def generate(env, **kw):
    if not kw.get('depsOnly',0):
        env.Tool('addLibrary', library = ['rdbModel'], package = 'rdbModel')
        if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
	    env.Tool('findPkgPath', package = 'rdbModel') 
    env.Tool('facilitiesLib')
    env.Tool('xmlBaseLib')
    env.Tool('addLibrary', library = env['xercesLibs'])
    env.Tool('addLibrary', library = env['mysqlLibs'])

def exists(env):
    return 1
