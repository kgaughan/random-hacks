#
# Copyright (c) Keith Gaughan, 2009.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of the author not be used in advertising or
# publicity pertaining to distribution of the software without specific,
# written prior permission.
#
# THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
# VINAY SAJIP BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
# ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

"""
A simple module for reading netstrings from a file object.

Copyright (c) Keith Gaughan, 2009. See the source code for the full licence.
"""


class MalformedNetstringError(Exception):
    """Raised if netstring_reader() hits something it can't consume."""
    pass

def netstring_reader(fdesc):
    """
    Wraps a file object in a generator that yields netstrings parsed from
    its contents.

    >>> import StringIO
    >>> fd = StringIO.StringIO('13:hello, world!,0:,12:how are you?,')
    >>> list(netstring_reader(fd))
    ['hello, world!', '', 'how are you?']
    >>> fd.close()
    """
    while True:
        nch = ""
        while True:
            char = fdesc.read(1)
            if char == '':
                return
            if char == ':':
                break
            if len(nch) > 10:
                raise MalformedNetstringError, "Excessive length"
            if '0' > char > '9':
                # Must be made up of digits.
                raise MalformedNetstringError, "Bad length digit"
            if char == '0' and nch == '':
                # We can't allow leading zeros.
                if fdesc.read(1) != ':':
                    raise MalformedNetstringError, "No leading zeros"
            nch += char
        nch = int(nch, 10)
        payload = fdesc.read(nch)
        if len(payload) < nch:
            return
        terminator = fdesc.read(1)
        if terminator != ',':
            raise MalformedNetstringError, "Bad terminator: '%s'" % terminator
        yield payload


def _test():
    """Run doctests on this module."""
    import doctest
    doctest.testmod()


if __name__ == '__main__':
    _test()
