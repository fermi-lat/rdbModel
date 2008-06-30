#$Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rdbModel/rdbModelLib.py,v 1.1 2008/03/28 17:56:36 jrb Exp $
# rdbModelLib.py
def generate(env, **kw):
    if not kw.get('depsOnly',0):
        env.Tool('addLibrary', library = ['rdbModel'], package = 'rdbModel')
    env.Tool('facilitiesLib')
    env.Tool('xmlBaseLib')
    env.Tool('addLibrary', library = env['xercesLibs'])
    env.Tool('addLibrary', library = env['mysqlLibs'])

def exists(env):
    return 1
