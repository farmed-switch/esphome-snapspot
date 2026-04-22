#!/usr/bin/env python

import glob
import os.path
import re
import sys

warnings = 0

def report(rule, location, description):
    global warnings
    warnings += 1
    print(f'{warnings:3}. {location}:  {description} [{rule}]')

def check_structure():
    expected_sections = [
        'Template parameters',
        'Specializations',
        'Iterator invalidation',
        'Requirements',
        'Member types',
        'Member functions',
        'Member variables',
        'Static functions',
        'Non-member functions',
        'Literals',
        'Helper classes',
        'Parameters',
        'Return value',
        'Exception safety',
        'Exceptions',
        'Complexity',
        'Possible implementation',
        'Default definition',
        'Notes',
        'Examples',
        'See also',
        'Version history'
    ]

    required_sections = [
        'Examples',
        'Version history'
    ]

    files = sorted(glob.glob('api/**/*.md', recursive=True))
    for file in files:
        with open(file) as file_content:
            section_idx = -1
            existing_sections = []
            in_initial_code_example = False
            previous_line = None
            h1sections = 0
            last_overload = 0
            documented_overloads = {}
            current_section = None

            for lineno, original_line in enumerate(file_content.readlines()):
                line = original_line.strip()

                if line.startswith('# '):
                    h1sections += 1

                if h1sections > 1:
                    report('structure/unexpected_section', f'{file}:{lineno+1}', f'unexpected top-level title "{line}"')
                    h1sections = 1

                if line == '# Overview':
                    report('style/title', f'{file}:{lineno+1}', 'overview pages should have a better title than "Overview"')

                if len(line) > 160 and '|' not in line:
                    report('whitespace/line_length', f'{file}:{lineno+1} ({current_section})', f'line is too long ({len(line)} vs. 160 chars)')

                if line.startswith('<!-- NOLINT'):
                    current_section = line.strip('<!-- NOLINT')
                    current_section = current_section.strip(' -->')
                    existing_sections.append(current_section)

                if line.startswith('## '):

                    if current_section in documented_overloads and last_overload != 0:
                        if len(documented_overloads[current_section]) > 0 and len(documented_overloads[current_section]) != last_overload:
                            expected = list(range(1, last_overload+1))
                            undocumented = [x for x in expected if x not in documented_overloads[current_section]]
                            unexpected = [x for x in documented_overloads[current_section] if x not in expected]
                            if len(undocumented):
                                report('style/numbering', f'{file}:{lineno} ({current_section})', f'undocumented overloads: {", ".join([f"({x})" for x in undocumented])}')
                            if len(unexpected):
                                report('style/numbering', f'{file}:{lineno} ({current_section})', f'unexpected overloads: {", ".join([f"({x})" for x in unexpected])}')

                    current_section = line.strip('## ')
                    existing_sections.append(current_section)

                    if current_section in expected_sections:
                        idx = expected_sections.index(current_section)
                        if idx <= section_idx:
                            report('structure/section_order', f'{file}:{lineno+1}', f'section "{current_section}" is in an unexpected order (should be before "{expected_sections[section_idx]}")')
                        section_idx = idx
                    else:
                        if 'index.md' not in file:
                            report('structure/unknown_section', f'{file}:{lineno+1}', f'section "{current_section}" is not part of the expected sections')

                if last_overload != 0 and not in_initial_code_example:
                    if len(original_line) and original_line[0].isdigit():
                        number = int(re.findall(r"^(\d+).", original_line)[0])
                        if current_section not in documented_overloads:
                            documented_overloads[current_section] = []
                        documented_overloads[current_section].append(number)

                if line == '```cpp' and section_idx == -1:
                    in_initial_code_example = True

                if in_initial_code_example and line.startswith('//') and line not in ['// since C++20', '// until C++20']:

                    if any(map(str.isdigit, line)):
                        number = int(re.findall(r'\d+', line)[0])
                        if number != last_overload + 1:
                            report('style/numbering', f'{file}:{lineno+1}', f'expected number ({number}) to be ({last_overload +1 })')
                        last_overload = number

                    if any(map(str.isdigit, line)) and '(' not in line:
                        report('style/numbering', f'{file}:{lineno+1}', f'number should be in parentheses: {line}')

                if line == '```' and in_initial_code_example:
                    in_initial_code_example = False

                if line == '' and previous_line == '':
                    report('whitespace/blank_lines', f'{file}:{lineno}-{lineno+1} ({current_section})', 'consecutive blank lines')

                untitled_admonition = re.match(r'^(\?\?\?|!!!) ([^ ]+)$', line)
                if untitled_admonition and untitled_admonition.group(2) != 'example':
                    report('style/admonition_title', f'{file}:{lineno} ({current_section})', f'"{untitled_admonition.group(2)}" admonitions should have a title')

                previous_line = line

            if 'index.md' not in file:
                for required_section in required_sections:
                    if required_section not in existing_sections:
                        report('structure/missing_section', f'{file}:{lineno+1}', f'required section "{required_section}" was not found')

def check_examples():
    example_files = sorted(glob.glob('../../examples/*.cpp'))
    markdown_files = sorted(glob.glob('**/*.md', recursive=True))

    for example_file in example_files:
        example_file = os.path.join('examples', os.path.basename(example_file))

        found = False
        for markdown_file in markdown_files:
            content = ' '.join(open(markdown_file).readlines())
            if example_file in content:
                found = True
                break

        if not found:
            report('examples/missing', f'{example_file}', 'example file is not used in any documentation file')

if __name__ == '__main__':
    print(120 * '-')
    check_structure()
    check_examples()
    print(120 * '-')

    if warnings > 0:
        sys.exit(1)
