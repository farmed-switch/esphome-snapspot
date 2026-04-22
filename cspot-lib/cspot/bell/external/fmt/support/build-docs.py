#!/usr/bin/env python

from __future__ import print_function
import errno, os, shutil, subprocess, sys, urllib
from subprocess import call, check_call, Popen, PIPE, STDOUT

def rmtree_if_exists(dir):
    try:
        shutil.rmtree(dir)
    except OSError as e:
        if e.errno == errno.ENOENT:
            pass

fmt_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.insert(0, os.path.join(fmt_dir, 'doc'))
import build
build.create_build_env()
html_dir = build.build_docs()

repo = 'fmtlib.github.io'
branch = os.environ['GITHUB_REF']
is_ci = 'CI' in os.environ
if is_ci and branch != 'refs/heads/master':
    print('Branch: ' + branch)
    exit(0)
if is_ci and 'KEY' not in os.environ:

    print('Skipping update of ' + repo)
    exit(0)

rmtree_if_exists(repo)
git_url = 'https://github.com/' if is_ci else 'git@github.com:'
check_call(['git', 'clone', git_url + 'fmtlib/{}.git'.format(repo)])

target_dir = os.path.join(repo, 'dev')
rmtree_if_exists(target_dir)
shutil.copytree(html_dir, target_dir, ignore=shutil.ignore_patterns('.*'))
if is_ci:
    check_call(['git', 'config', '--global', 'user.name', 'fmtbot'])
    check_call(['git', 'config', '--global', 'user.email', 'viz@fmt.dev'])

check_call(['git', 'add', '--all'], cwd=repo)
if call(['git', 'diff-index', '--quiet', 'HEAD'], cwd=repo):
    check_call(['git', 'commit', '-m', 'Update documentation'], cwd=repo)
    cmd = 'git push'
    if is_ci:
        cmd += ' https://$KEY@github.com/fmtlib/fmtlib.github.io.git master'
    p = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT, cwd=repo)

    print(p.communicate()[0].decode('utf-8').replace(os.environ['KEY'], '$KEY'))
    if p.returncode != 0:
        raise subprocess.CalledProcessError(p.returncode, cmd)
