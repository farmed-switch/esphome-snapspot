""

import SCons.Action
import SCons.Builder
import SCons.Util
from SCons.Script import Dir, File
import os.path
import platform

try:
    warningbase = SCons.Warnings.SConsWarning
except AttributeError:
    warningbase = SCons.Warnings.Warning

class NanopbWarning(warningbase):
    pass
SCons.Warnings.enableWarningClass(NanopbWarning)

def _detect_nanopb(env):
    ""
    if 'NANOPB' in env:

        return env['NANOPB']

    p = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
    if os.path.isdir(p) and os.path.isfile(os.path.join(p, 'pb.h')):

        return p

    raise SCons.Errors.StopError(NanopbWarning,
        "Could not find the nanopb root directory")

def _detect_python(env):
    ""
    if 'PYTHON' in env:
        return env['PYTHON']

    p = env.WhereIs('python3')
    if p:
        return env['ESCAPE'](p)

    p = env.WhereIs('py.exe')
    if p:
        return env['ESCAPE'](p) + " -3"

    return env['ESCAPE'](sys.executable)

def _detect_nanopb_generator(env):
    ""
    generator_cmd = os.path.join(env['NANOPB'], 'generator-bin', 'nanopb_generator' + env['PROGSUFFIX'])
    if os.path.exists(generator_cmd):

        return env['ESCAPE'](generator_cmd)
    else:

        return env['PYTHON'] + " " + env['ESCAPE'](os.path.join(env['NANOPB'], 'generator', 'nanopb_generator.py'))

def _detect_protoc(env):
    ""
    if 'PROTOC' in env:

        return env['PROTOC']

    n = _detect_nanopb(env)
    p1 = os.path.join(n, 'generator-bin', 'protoc' + env['PROGSUFFIX'])
    if os.path.exists(p1):

        return env['ESCAPE'](p1)

    p = os.path.join(n, 'generator', 'protoc')
    if os.path.exists(p):

        if env['PLATFORM'] == 'win32':
            return env['ESCAPE'](p + '.bat')
        else:
            return env['ESCAPE'](p)

    p = env.WhereIs('protoc')
    if p:

        return env['ESCAPE'](p)

    raise SCons.Errors.StopError(NanopbWarning,
        "Could not find the protoc compiler")

def _detect_protocflags(env):
    ""
    if 'PROTOCFLAGS' in env:
        return env['PROTOCFLAGS']

    p = _detect_protoc(env)
    n = _detect_nanopb(env)
    p1 = os.path.join(n, 'generator', 'protoc' + env['PROGSUFFIX'])
    p2 = os.path.join(n, 'generator-bin', 'protoc' + env['PROGSUFFIX'])
    if p in [env['ESCAPE'](p1), env['ESCAPE'](p2)]:

        return ''

    e = env['ESCAPE']
    if env['PLATFORM'] == 'win32':
        return e('--plugin=protoc-gen-nanopb=' + os.path.join(n, 'generator', 'protoc-gen-nanopb.bat'))
    else:
        return e('--plugin=protoc-gen-nanopb=' + os.path.join(n, 'generator', 'protoc-gen-nanopb'))

def _nanopb_proto_actions(source, target, env, for_signature):
    esc = env['ESCAPE']

    prefix = os.path.dirname(str(source[-1]))
    srcfile = esc(os.path.relpath(str(source[0]), prefix))
    include_dirs = '-I.'
    for d in env['PROTOCPATH']:
        d = env.GetBuildPath(d)
        if not os.path.isabs(d): d = os.path.relpath(d, prefix)
        include_dirs += ' -I' + esc(d)

    source_extension = os.path.splitext(str(target[0]))[1]
    header_extension = '.h' + source_extension[2:]
    nanopb_flags = env['NANOPBFLAGS']
    if nanopb_flags:
      nanopb_flags = '--source-extension=%s,--header-extension=%s,%s:.' % (source_extension, header_extension, nanopb_flags)
    else:
      nanopb_flags = '--source-extension=%s,--header-extension=%s:.' % (source_extension, header_extension)

    return SCons.Action.CommandAction('$PROTOC $PROTOCFLAGS %s --nanopb_out=%s %s' % (include_dirs, nanopb_flags, srcfile),
                                      chdir = prefix)

def _nanopb_proto_emitter(target, source, env):
    basename = os.path.splitext(str(source[0]))[0]
    source_extension = os.path.splitext(str(target[0]))[1]
    header_extension = '.h' + source_extension[2:]
    target.append(basename + '.pb' + header_extension)

    source.append(File("SConscript"))

    if os.path.exists(basename + '.options'):
        source.append(basename + '.options')

    return target, source

_nanopb_proto_builder = SCons.Builder.Builder(
    generator = _nanopb_proto_actions,
    suffix = '.pb.c',
    src_suffix = '.proto',
    emitter = _nanopb_proto_emitter)

_nanopb_proto_cpp_builder = SCons.Builder.Builder(
    generator = _nanopb_proto_actions,
    suffix = '.pb.cpp',
    src_suffix = '.proto',
    emitter = _nanopb_proto_emitter)

def generate(env):
    ""

    env['NANOPB'] = _detect_nanopb(env)
    env['PROTOC'] = _detect_protoc(env)
    env['PROTOCFLAGS'] = _detect_protocflags(env)
    env['PYTHON'] = _detect_python(env)
    env['NANOPB_GENERATOR'] = _detect_nanopb_generator(env)
    env.SetDefault(NANOPBFLAGS = '')

    env.SetDefault(PROTOCPATH = [".", os.path.join(env['NANOPB'], 'generator', 'proto')])

    env.SetDefault(NANOPB_PROTO_CMD = '$PROTOC $PROTOCFLAGS --nanopb_out=$NANOPBFLAGS:. $SOURCES')
    env['BUILDERS']['NanopbProto'] = _nanopb_proto_builder
    env['BUILDERS']['NanopbProtoCpp'] = _nanopb_proto_cpp_builder

def exists(env):
    return _detect_protoc(env) and _detect_protoc_opts(env)

