import subprocess
import os.path

def has_grpcio_protoc():

    ""

    try:
        import grpc_tools.protoc
    except ImportError:
        return False
    return True

def invoke_protoc(argv):

    ""

    if not [x for x in argv if x.startswith('-I')]:
        argv.append("-I.")

    nanopb_include = os.path.dirname(os.path.abspath(__file__))
    argv.append('-I' + nanopb_include)

    if has_grpcio_protoc():
        import grpc_tools.protoc as protoc
        import pkg_resources
        proto_include = pkg_resources.resource_filename('grpc_tools', '_proto')
        argv.append('-I' + proto_include)

        return protoc.main(argv)
    else:
        return subprocess.call(argv)
