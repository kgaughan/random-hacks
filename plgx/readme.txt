
PLGX v1.0.0 -- A simple DHTML-based presentation engine
by Keith Gaughan


Introduction
============

I dropped off to sleep last night rather early (9pm) last night and ended up
waking rather early too (3.30am). Here's something I knocked together to try
and put myself to sleep again.

PLGX, which stands for "PLGX is a Meaningless Acronym", lets you build simple
presentations using HTML. 


Usage
=====

Each page is made up of a "part", a page element to be hidden initially and
revealed later. You mark these elements in the page with the "part" class.

The presentation consists of a set of slides. These are <div> elements
directly inside the <body> element annoted with the class "part". Within each
slide, you can mark each element to be revealed within a slide with the "part"
class.

The presentation itself is controlled using the left and right cursor keys,
which move backwards and forwards in the presentation respectively.

A basic three-slide sample presentation is supplied.


Limitations and Assumptions
===========================

It works fine in Firefox 1.5 and Opera 8, and may work in other browsers too.
It doesn't work in IE 6, but that doesn't bother me too much yet.

It currently works under the assumption that document.getElementsByTagName()
does a depth-first search to find each of the document elements. If your
browser doesn't, it will break. If anybody finds a relatively common browser
out there that supports document.getElementsByTagName() that doesn't do this,
I'll happily patch the code to do the search by hand. Until then, I'm happy
with the current situation.

The reveal order is fixed to the order element appear in the file.

The code assumes that the later an element appears in the presentation, the
higher its z-order is.


Credits and License
===================

(This license applies to the files "plgx.js" and "plgx.css". All other files
in this distribution are Public Domain.)

Copyright (c) Keith Gaughan, 2006.

You can do whatever you like with this just as long as you (i) don't claim
PLGX as your own work, (ii) leave any and all copyright notices in place, and
(iii) keep in mind that this comes with NO WARRANTIES and that the author is
not liable if this software causes something horrible to happen.


Contacting the Author
=====================

The project homepage is at http://talideon.com/projects/plgx/.

If you want to contact me, you can do so with the contact form on my site at
http://talideon.com/about/contact/.


History
=======

1.0.0 (2006-04-19)
	Initial Release.
