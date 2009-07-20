#!/usr/bin/env python

class MalformedNetstringError:
    pass

def netstring_reader(fd):
    while True:
        n = ""
        while True:
            c = fd.read(1)
            if c == '':
                return
            if c == ':':
                break
            if len(n) > 10:
                raise MalformedNetstringError
            if '0' > c > '9':
                # Must be made up of digits.
                raise MalformedNetstringError
            if c == '0' and n == '':
                # We can't allow leading zeros.
                if fd.read(1) != ':':
                    raise MalformedNetstringError
                n = c
                break
            n += c
        n = int(n, 10)
        payload = fd.read(n)
        if len(payload) < n:
            return
        if fd.read(1) != ',':
            raise MalformedNetstringError
        yield payload


if __name__ == '__main__':
    fd = open("sample.dat", "r")
    try:
        for i in netstring_reader(fd):
            print i
    finally:
        fd.close()
