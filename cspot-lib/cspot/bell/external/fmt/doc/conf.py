

import sys, os, re, subprocess

needs_sphinx = '1.2'

if os.environ.get('READTHEDOCS', None) == 'True':
  subprocess.call('doxygen')

extensions = ['sphinx.ext.ifconfig', 'breathe']

breathe_default_project = "format"
breathe_domain_by_extension = {"h" : "cpp"}

templates_path = ['_templates']

source_suffix = '.rst'

project = u'fmt'
copyright = u'2012-present, Victor Zverovich'

exclude_patterns = ['virtualenv']

default_role = 'cpp:any'

pygments_style = 'sphinx'

highlight_language = 'c++'

primary_domain = 'cpp'

html_theme = 'basic-bootstrap'

html_theme_path = ['.']

html_static_path = ['_static']

html_sidebars = {
  '**': ['localtoc.html', 'relations.html', 'sourcelink.html', 'searchbox.html']
}

htmlhelp_basename = 'formatdoc'

latex_elements = {

}

latex_documents = [
  ('index', 'format.tex', u'fmt documentation',
   u'Victor Zverovich', 'manual'),
]

man_pages = [
    ('index', 'fmt', u'fmt documentation', [u'Victor Zverovich'], 1)
]

texinfo_documents = [
  ('index', 'fmt', u'fmt documentation',
   u'Victor Zverovich', 'fmt', 'One line description of project.',
   'Miscellaneous'),
]

