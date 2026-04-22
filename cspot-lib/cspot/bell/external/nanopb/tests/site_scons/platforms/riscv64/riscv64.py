

def set_riscv64_platform(env):
    env.Replace(EMBEDDED = "RISCV64")
    env.Replace(CC  = "riscv64-linux-gnu-gcc",
                CXX = "riscv64-linux-gnu-g++")
    env.Replace(TEST_RUNNER = "/usr/bin/qemu-riscv64")
    env.Append(LINKFLAGS = "-static")

