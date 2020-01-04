
                                    INIFILE
                                    =======

                  A library for reading and writing INI files
                                by Keith Gaughan


Background
==========

If there's one good thing about Windows (and as far as Windows goes, the fact
that there's *anything* good about it is a revelation!), it's the INI file
format. It's nice, simple, and clean. It's also deprecated in favour of storing
configuration details in the registry. Talk about over-engineering!
  
Regardless, I've been meaning to write a platform-agnostic library for reading
and writing INI files for a while now, and, though I've yet to think of a good
reason for doing it, last night I wrote one.

Ah, yes. I do have a good reason for writing this. It occurred to me that INI
files were well suited to storing address books, say having one section per
person, with the details about them being stored in the key/value pairs, and
seeing as I'm intending on writing mail program, it's useful.


File Format
===========

For those who don't know the file format, here's an overview.

INI files are broken into sections, and each section contains a set of
key/value pairs. Comment lines start with a semicolon (`;'). Sections sit on
lines by themselves, starting with a left square bracket (`['), and ending with
a right square bracket (`]'). Pairs sit on lines by themselves and the key is
seperated from the value by an equals (`=') sign. Whitespace around the key is
ignored, as is all leading whitespace on any line. Empty lines are ignored, as
are any the parser can't make sense of.

Here's a sample file:

    ;
    ; A Comment
    ;

    This pair=isn't in a section and so gets ignored.

    [Important Stuff]
    foo=bar
    fred=barney

    [Another Section]
    1=One
    Two=2
    foo=baz

This is the classic version of the file format. Accept no substitute.

I've heard rumours that comments can also start with (`#'), but the library
doesn't cope with them. If I get confirmation, I might add it.


Usage
=====

I'm presuming you know how to build and link the library using your compiler.
If you don't, check the manuals that came with it.

There's pretty thorough documentation in inifile.h, so go there if you want a
reference manual. Here's an overview of how to open, read, write, save, &c.:

INI files are represented by the INIFile type. INI_Load() loads an INI file
into memory and returns a handle to it as a pointer. If either can't allocate
the memory to hold the file, or can't find the file, it returns NULL. Mind you,
I'm thinking of changing this behaviour so that it only returns NULL if it runs
out of memory, creating a new, empty file otherwise. Be warned.

Reading is done with INI_Read(); Writing with INI_Write(). You can save with
INI_Save(). After you're finished, pass the handle to INI_Free() to free any
memory associated with the INI file. Here's a code snippet:
 
    INI_File* ini;

    ini = INI_File("my.ini");
    printf("foo (1): '%s'\n", INI_Read(ini, "Important Stuff", "foo"));
    printf("foo (2): '%s'\n", INI_Read(ini, "Another Section", "foo"));
    INI_Write(ini, "Another Section", "Two", "Too!");
    INI_Save(ini);
    INI_Free(ini);

The output of this is:

    foo (1): 'bar'
    foo (2): 'baz'


Technical Details
=================

 * The library stores everything in a bunch of linked lists. Though that might
   seem slow, it's just fine. I did some eyeball tests reading, fiddling, and
   writing a 1MB INI file, and it didn't take any more than .5 secs in total.
   The box I'm using right now contains a 1GHz Celeron inside. Most INI files
   don't get anywhere near that size, so performance is acceptable.

 * Don't put any newlines or any other control characters in the section names,
   key names, or values. They'll screw things up. Sorry, but they're for you to
   escape, not the library. It can't escape them without potentially becoming
   incompatible with other libraries.

 * There are files that masquarade as INI files. This library will accept any
   real ones, and ignore any lines that don't make sense.

 * The maximum line length is currently 4096 bytes, including the newline.
   That constraint only applies when reading in the file though.


Contacting
==========

I don't give out my email address anymore due to spam, but I do have a contact
form on my website. You can email me from:

    http://talideon.com/about/contact.cfm

Don't worry if I don't get back immediately. I'll probably be too busy, but be
assured that I'll be feeling guilty about not mailing back. I'm not kidding!


Licence and Warranty
====================

Copyright (c) Keith Gaughan, 2004. All Rights Reserved.

Permission is granted to anyone to use this software for any purpose on any
computer system, and to alter it and redistribute it, subject to the following
restrictions:

 1. The author is not responsible for the consequences of use of this software,
    no matter how awful, even if they arise from flaws in it.

 2. The origin of this software must not be misrepresented, either by explicit
    claim or by omission. Since few users ever read sources, credits must
    appear in the documentation.

 3. Altered versions must be plainly marked as such, and must not be
    misrepresented as being the original software. Since few users ever read
    sources, credits must appear in the documentation.

 4. The author reserves the right to change the licencing details on any future
    releases of this package.

 5. This notice may not be removed or altered.

(Licence derived from the one on Henry Spencer's regex package. Credit where
credit's due.)

