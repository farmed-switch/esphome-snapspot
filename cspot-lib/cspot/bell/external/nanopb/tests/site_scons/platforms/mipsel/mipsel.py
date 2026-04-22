

def set_mipsel_platform(env):
    env.Replace(EMBEDDED = "MIPSEL")
    env.Replace(CC  = "mipsel-linux-gnu-gcc",
                CXX = "mipsel-linux-gnu-g++")
    env.Replace(TEST_RUNNER = "/usr/bin/qemu-mipsel")
    env.Append(LINKFLAGS = "-static")

