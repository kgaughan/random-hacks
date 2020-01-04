/*
 * PLGX - A simple DHTML-based presentation system.
 * by Keith Gaughan
 *
 * Homepage: http://talideon.com/projects/plgx/
 *
 * Copyright (c) Keith Gaughan, 2006.
 * You can do whatever you like with this just as long as you (i) don't
 * claim PLGX as your own work, (ii) leave any and all copyright notices
 * in place, and (iii) keep in mind that this comes with NO WARRANTIES and
 * that the author is not liable if this software causes something horrible
 * to happen.
 */

// Keycodes for the cursor keys.
var CURSOR_LEFT  = 37;
var CURSOR_RIGHT = 39;

var iPart = 0;
var parts = [];

var ePrevious, eNext;

function hideCurrentPart() {
	parts[iPart].style.visibility = 'hidden';
}

function showCurrentPart() {
	parts[iPart].style.visibility = 'visible';
}

function nextPart() {
	if (iPart < parts.length - 1) {
		++iPart;
		showCurrentPart();
	}
	return false;
}

function previousPart() {
	if (iPart > 0) {
		hideCurrentPart();
		--iPart;
	}
	return false;
}

window.onload = function() {
	// Assumption: that the returned array was arrived at depth- rather than
	// breadth-first.
	var elems = document.getElementsByTagName('*');
	for (var i = 0; i < elems.length; ++i) {
		if (/\bpart\b/.test(elems[i].className)) {
			parts[parts.length] = elems[i];
		}
	}

	ePrevious = document.getElementById('previous');
	eNext     = document.getElementById('next');
	if (ePrevious && eNext) {
		ePrevious.onclick = previousPart;
		eNext.onclick     = nextPart;
	}

	showCurrentPart();
}

window.onkeypress = function(e) {
	e = e || window.event;
	if (e.keyCode == CURSOR_LEFT) {
		previousPart();
	} else if (e.keyCode == CURSOR_RIGHT) {
		nextPart();
	}
}

window.onunload = function() {
	window.onload     = null;
	window.onkeypress = null;
	window.onunload   = null;

	if (ePrevious && eNext) {
		ePrevious.onclick = null;
		eNext.onclick     = null;
	}
}
