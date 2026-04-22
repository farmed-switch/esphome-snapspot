

def set_mips_platform(env):
    env.Replace(EMBEDDED = "MIPS")
    env.Replace(CC  = "mips-linux-gnu-gcc",
                CXX = "mips-linux-gnu-g++")
    env.Replace(TEST_RUNNER = "/usr/bin/qemu-mips")
    env.Append(LINKFLAGS = "-static")

