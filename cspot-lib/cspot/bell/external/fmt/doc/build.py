#!/usr/bin/env python3

import errno, os, re, sys
from subprocess import check_call, CalledProcessError, Popen, PIPE, STDOUT

versions = ['1.0.0', '1.1.0', '2.0.0', '3.0.2', '4.0.0', '4.1.0', '5.0.0', '5.1.0', '5.2.0', '5.2.1', '5.3.0', '6.0.0', '6.1.0', '6.1.1', '6.1.2', '6.2.0', '6.2.1', '7.0.0', '7.0.1', '7.0.2', '7.0.3', '7.1.0', '7.1.1', '7.1.2', '7.1.3', '8.0.0', '8.0.1', '8.1.0', '8.1.1', '9.0.0', '9.1.0']

class Pip:
  def __init__(self, venv_dir):
    self.path = os.path.join(venv_dir, 'bin', 'pip')

  def install(self, package, commit=None):
    "Install package using pip."
    if commit:
      package = 'git+https://github.com/{0}.git@{1}'.format(package, commit)
    print('Installing {0}'.format(package))
    check_call([self.path, 'install', package])

def create_build_env(venv_dir='virtualenv'):

  if not os.path.exists(venv_dir):
    check_call(['python3', '-m', 'venv', venv_dir])

  pip = Pip(venv_dir)
  pip.install('wheel')
  pip.install('six')

  pip.install('docutils==0.17.1')

  pip.install('Jinja2<3.1')
  pip.install('sphinx-doc/sphinx', 'v3.3.0')
  pip.install('michaeljones/breathe', 'v4.25.0')

def build_docs(version='dev', **kwargs):
  doc_dir = kwargs.get('doc_dir', os.path.dirname(os.path.realpath(__file__)))
  work_dir = kwargs.get('work_dir', '.')
  include_dir = kwargs.get(
      'include_dir', os.path.join(os.path.dirname(doc_dir), 'include', 'fmt'))

  cmd = ['doxygen', '-']
  p = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=STDOUT)
  doxyxml_dir = os.path.join(work_dir, 'doxyxml')
  out, _ = p.communicate(input=r"".format(include_dir, doxyxml_dir).encode('UTF-8'))
  out = out.decode('utf-8')
  internal_symbols = [
    'fmt::detail::.*',
    'basic_data<>',
    'fmt::type_identity',
    'fmt::dynamic_formatter'
  ]
  noisy_warnings = [
    'warning: (Compound|Member .* of class) (' + '|'.join(internal_symbols) + \
      ') is not documented.',
    'warning: Internal inconsistency: .* does not belong to any container!'
  ]
  for w in noisy_warnings:
      out = re.sub('.*' + w + '\n', '', out)
  print(out)
  if p.returncode != 0:
    raise CalledProcessError(p.returncode, cmd)

  html_dir = os.path.join(work_dir, 'html')
  main_versions = reversed(versions[-3:])
  check_call([os.path.join(work_dir, 'virtualenv', 'bin', 'sphinx-build'),
              '-Dbreathe_projects.format=' + os.path.abspath(doxyxml_dir),
              '-Dversion=' + version, '-Drelease=' + version,
              '-Aversion=' + version, '-Aversions=' + ','.join(main_versions),
              '-b', 'html', doc_dir, html_dir])
  try:
    check_call(['lessc', '--verbose', '--clean-css',
                '--include-path=' + os.path.join(doc_dir, 'bootstrap'),
                os.path.join(doc_dir, 'fmt.less'),
                os.path.join(html_dir, '_static', 'fmt.css')])
  except OSError as e:
    if e.errno != errno.ENOENT:
      raise
    print('lessc not found; make sure that Less (http://lesscss.org/) ' +
          'is installed')
    sys.exit(1)
  return html_dir

if __name__ == '__main__':
  create_build_env()
  build_docs(sys.argv[1])
