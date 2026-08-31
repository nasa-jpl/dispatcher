#!/usr/bin/env python3
"""Insert a released section into CHANGELOG.rst.

Usage: update_changelog.py <new-version>

Collects the commits made since the most recent tag and writes them as a new
``<version> (<date>)`` section directly below the ``Forthcoming`` heading, in
the same layout catkin_generate_changelog produces:

    0.4.2 (2026-08-30 21:15)
    ------------------------
    * some commit subject
      an indented commit body
    * Contributors: Ada Lovelace

Run from the package root, in a checkout with full history and tags.
"""

import os
import re
import subprocess
import sys
from datetime import datetime, timezone

CHANGELOG = 'CHANGELOG.rst'

# Field/record separators; git emits them via %x1f / %x1e.
FIELD_SEP = '\x1f'
RECORD_SEP = '\x1e'

# "Forthcoming" plus its underline, at the start of a line.
FORTHCOMING_RE = re.compile(r'^Forthcoming[ \t]*\n-+[ \t]*\n', re.MULTILINE)
# The "^^^^" title block at the top of the file.
TITLE_RE = re.compile(r'^\^+[ \t]*\n.*\n\^+[ \t]*\n', re.MULTILINE)
# Bare issue references, e.g. #12 -- but not ones already inside a link.
ISSUE_RE = re.compile(r'(?<![\w`/])#(\d+)\b')
# Version bumps pushed by this workflow; they say nothing a reader needs.
BUMP_RE = re.compile(r'^chore: bump version to ')


def git(*args):
    """Run a git command and return stdout, or None if it failed."""
    result = subprocess.run(
        ('git',) + args, text=True, capture_output=True)
    if result.returncode != 0:
        return None
    return result.stdout


def last_tag():
    described = git('describe', '--tags', '--abbrev=0')
    return described.strip() if described else None


def commits_since(tag):
    """Yield (author, message) newest first for commits after `tag`."""
    revision = f'{tag}..HEAD' if tag else 'HEAD'
    log = git('log', f'--format=%an{FIELD_SEP}%B{RECORD_SEP}', revision)
    if log is None:
        sys.exit(f'error: could not read git log for {revision}')
    for record in log.split(RECORD_SEP):
        author, _, message = record.strip('\n').partition(FIELD_SEP)
        message = message.strip()
        if not author or not message or BUMP_RE.match(message):
            continue
        yield author, message


def issue_links(text):
    """Turn `#12` into an RST link to the issue, as catkin does."""
    repository = os.environ.get('GITHUB_REPOSITORY')
    if not repository:
        return text
    url = f'https://github.com/{repository}/issues'
    return ISSUE_RE.sub(lambda m: f'`#{m.group(1)} <{url}/{m.group(1)}>`_', text)


def bullet(message):
    """Format one commit message as an RST list item."""
    subject, *body = issue_links(message).split('\n')
    lines = [f'* {subject}']
    lines += [f'  {line}'.rstrip() for line in body]
    return '\n'.join(lines)


def section(version, entries, contributors):
    heading = f'{version} ({datetime.now(timezone.utc):%Y-%m-%d %H:%M})'
    lines = [heading, '-' * len(heading)]
    lines += entries
    if contributors:
        lines.append(f'* Contributors: {", ".join(contributors)}')
    return '\n'.join(lines) + '\n'


def main(argv):
    if len(argv) != 2:
        sys.exit(f'usage: {os.path.basename(argv[0])} <new-version>')
    version = argv[1]

    tag = last_tag()
    entries, authors = [], set()
    for author, message in commits_since(tag):
        entries.append(bullet(message))
        authors.add(author)
    if not entries:
        print(f'no commits since {tag or "the start of history"}, '
              f'adding an empty {version} section')

    with open(CHANGELOG, encoding='utf-8') as handle:
        changelog = handle.read()

    new_section = section(
        version, entries, sorted(authors, key=str.casefold))

    # Keep the empty "Forthcoming" heading so catkin_generate_changelog still
    # has somewhere to add to; the release goes directly below it.
    anchor = FORTHCOMING_RE.search(changelog) or TITLE_RE.search(changelog)
    if not anchor:
        sys.exit(f'error: found neither a "Forthcoming" heading nor a title '
                 f'block in {CHANGELOG}')
    cut = anchor.end()
    changelog = f'{changelog[:cut]}\n{new_section}\n{changelog[cut:].lstrip()}'

    with open(CHANGELOG, 'w', encoding='utf-8') as handle:
        handle.write(changelog)
    print(f'added {version} section with {len(entries)} entr'
          f'{"y" if len(entries) == 1 else "ies"} since {tag or "the start"}')


if __name__ == '__main__':
    main(sys.argv)
