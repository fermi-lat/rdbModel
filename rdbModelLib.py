#$Header:  $
# rdbModelLib.py
def generate(env, **kw):
    if not kw.get('depsOnly',0):
        env.Tool('addLibrary', library = ['rdbModel'], package = 'rdbModel')
    env.Tool('facilitiesLib')
    env.Tool('xmlBaseLib')
    env.Tool('addLibrary', library = env['xercesLibs'])
    env.Tool('addLibrary', library = env['MYSQLLibs'])

def exists(env):
    return 1
