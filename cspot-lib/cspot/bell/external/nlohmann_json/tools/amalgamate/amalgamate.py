#!/usr/bin/env python3

from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import argparse
import datetime
import json
import os
import re

class Amalgamation(object):

    def actual_path(self, file_path):
        if not os.path.isabs(file_path):
            file_path = os.path.join(self.source_path, file_path)
        return file_path

    def find_included_file(self, file_path, source_dir):
        search_dirs = self.include_paths[:]
        if source_dir:
            search_dirs.insert(0, source_dir)

        for search_dir in search_dirs:
            search_path = os.path.join(search_dir, file_path)
            if os.path.isfile(self.actual_path(search_path)):
                return search_path
        return None

    def __init__(self, args):
        with open(args.config, 'r') as f:
            config = json.loads(f.read())
            for key in config:
                setattr(self, key, config[key])

            self.verbose = args.verbose == "yes"
            self.prologue = args.prologue
            self.source_path = args.source_path
            self.included_files = []

    def generate(self):
        amalgamation = ""

        if self.prologue:
            with open(self.prologue, 'r') as f:
                amalgamation += datetime.datetime.now().strftime(f.read())

        if self.verbose:
            print("Config:")
            print(" target        = {0}".format(self.target))
            print(" working_dir   = {0}".format(os.getcwd()))
            print(" include_paths = {0}".format(self.include_paths))
        print("Creating amalgamation:")
        for file_path in self.sources:

            print(" - processing \"{0}\"".format(file_path))
            t = TranslationUnit(file_path, self, True)
            amalgamation += t.content

        with open(self.target, 'w') as f:
            f.write(amalgamation)

        print("...done!\n")
        if self.verbose:
            print("Files processed: {0}".format(self.sources))
            print("Files included: {0}".format(self.included_files))
        print("")

def _is_within(match, matches):
    for m in matches:
        if match.start() > m.start() and \
                match.end() < m.end():
            return True
    return False

class TranslationUnit(object):

    cpp_comment_pattern = re.compile(r"//.*?\n")

    c_comment_pattern = re.compile(r"/\*.*?\*/", re.S)

    string_pattern = re.compile("[^']" r'".*?(?<=[^\\])"', re.S)

    include_pattern = re.compile(
        r'#\s*include\s+(<|")(?P<path>.*?)("|>)', re.S)

    pragma_once_pattern = re.compile(r'#\s*pragma\s+once', re.S)

    def _search_content(self, index, pattern, contexts):
        match = pattern.search(self.content, index)
        if match:
            contexts.append(match)
            return match.end()
        return index + 2

    def _find_skippable_contexts(self):

        skippable_contexts = []

        i = 1
        content_len = len(self.content)
        while i < content_len:
            j = i - 1
            current = self.content[i]
            previous = self.content[j]

            if current == '"':

                i = self._search_content(j, self.string_pattern,
                                         skippable_contexts)
            elif current == '*' and previous == '/':

                i = self._search_content(j, self.c_comment_pattern,
                                         skippable_contexts)
            elif current == '/' and previous == '/':

                i = self._search_content(j, self.cpp_comment_pattern,
                                         skippable_contexts)
            else:

                i += 1

        return skippable_contexts

    def _process_pragma_once(self):
        content_len = len(self.content)
        if content_len < len("#include <x>"):
            return 0

        skippable_contexts = self._find_skippable_contexts()

        pragmas = []
        pragma_once_match = self.pragma_once_pattern.search(self.content)
        while pragma_once_match:
            if not _is_within(pragma_once_match, skippable_contexts):
                pragmas.append(pragma_once_match)

            pragma_once_match = self.pragma_once_pattern.search(self.content,
                                                                pragma_once_match.end())

        prev_end = 0
        tmp_content = ''
        for pragma_match in pragmas:
            tmp_content += self.content[prev_end:pragma_match.start()]
            prev_end = pragma_match.end()
        tmp_content += self.content[prev_end:]
        self.content = tmp_content

    def _process_includes(self):
        content_len = len(self.content)
        if content_len < len("#include <x>"):
            return 0

        skippable_contexts = self._find_skippable_contexts()

        includes = []
        include_match = self.include_pattern.search(self.content)
        while include_match:
            if not _is_within(include_match, skippable_contexts):
                include_path = include_match.group("path")
                search_same_dir = include_match.group(1) == '"'
                found_included_path = self.amalgamation.find_included_file(
                    include_path, self.file_dir if search_same_dir else None)
                if found_included_path:
                    includes.append((include_match, found_included_path))

            include_match = self.include_pattern.search(self.content,
                                                        include_match.end())

        prev_end = 0
        tmp_content = ''
        for include in includes:
            include_match, found_included_path = include
            tmp_content += self.content[prev_end:include_match.start()]
            tmp_content += "// {0}\n".format(include_match.group(0))
            if found_included_path not in self.amalgamation.included_files:
                t = TranslationUnit(found_included_path, self.amalgamation, False)
                tmp_content += t.content
            prev_end = include_match.end()
        tmp_content += self.content[prev_end:]
        self.content = tmp_content

        return len(includes)

    def _process(self):
        if not self.is_root:
            self._process_pragma_once()
        self._process_includes()

    def __init__(self, file_path, amalgamation, is_root):
        self.file_path = file_path
        self.file_dir = os.path.dirname(file_path)
        self.amalgamation = amalgamation
        self.is_root = is_root

        self.amalgamation.included_files.append(self.file_path)

        actual_path = self.amalgamation.actual_path(file_path)
        if not os.path.isfile(actual_path):
            raise IOError("File not found: \"{0}\"".format(file_path))
        with open(actual_path, 'r') as f:
            self.content = f.read()
            self._process()

def main():
    description = "Amalgamate C source and header files."
    usage = " ".join([
        "amalgamate.py",
        "[-v]",
        "-c path/to/config.json",
        "-s path/to/source/dir",
        "[-p path/to/prologue.(c|h)]"
    ])
    argsparser = argparse.ArgumentParser(
        description=description, usage=usage)

    argsparser.add_argument("-v", "--verbose", dest="verbose",
                            choices=["yes", "no"], metavar="", help="be verbose")

    argsparser.add_argument("-c", "--config", dest="config",
                            required=True, metavar="", help="path to a JSON config file")

    argsparser.add_argument("-s", "--source", dest="source_path",
                            required=True, metavar="", help="source code path")

    argsparser.add_argument("-p", "--prologue", dest="prologue",
                            required=False, metavar="", help="path to a C prologue file")

    amalgamation = Amalgamation(argsparser.parse_args())
    amalgamation.generate()

if __name__ == "__main__":
    main()
