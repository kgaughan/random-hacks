#!/usr/bin/env python
#
# pipedarg
# by Keith Gaughan <http://talideon.com/>
# 
# The software is hereby placed in the public domain, and the author disclaims
# ownership over it and all responsibility for any damage caused directly or
# indirectly through its use. It may be freely copied and/or mangled, provided
# that altered versions have a different name and are not attributed to the
# original author.
#

"""
Copies standard input to a temporary file and executes the command specified in
the arguments with the name of the temporary file as the last argument.
"""

from __future__ import with_statement
import sys
import os
import tempfile

with tempfile.NamedTemporaryFile() as tf:
	tf.write(sys.stdin.read())
	tf.flush()
	args = sys.argv[1:]
	os.spawnvp(os.P_WAIT, args[0], args + [tf.name])
