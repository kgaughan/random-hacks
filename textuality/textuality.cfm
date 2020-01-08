<cfscript>
/*
 * Textuality
 * by Keith Gaughan <kmgaughan@eircom.net>
 *
 * A ColdFusion 5 compatible text markup processor.
 *
 * Copyright (c) Keith Gaughan, 2006.
 * All Rights Reserved.
 */

/**
 * 
 */
function Textuality(s, imagePath, allowHtml, followLinks) {
	// The individual lines of the string.
	var lines       = Textuality_BreakIntoLines(s);
	// The string, as HTML.
	var output      = "";
	// The contents of the current block pending processing of inline markup.
	var inline      = "";
	// General loop counters.
	var i           = 0;
	var j           = 0;
	// The current state the processor's state machine is in.
	var state       = "/";
	// The line currently being processed.
	var line        = "";
	// A string, each character of which identifies which kind of list is
	// active currently at each level ('*' for unordered, '#' for ordered), and
	// how deep it's nested.
	var listCodes   = ArrayNew(1);
	// The depth/alignment of each open list.
	var listDepths  = ArrayNew(1);
	// The depth of the current list item being processed.
	var depthChange = 0;
	// Regex matches.
	var matches     = "";
	// Are we in a quoted block? How deep?
	var quoteDepth  = 0;

	for (i = 1; i lte ArrayLen(lines); i = i + 1) {
		line = lines[i];
		switch (state) {
			// Waiting for a block.
			case "/":
				// Starting a preformatted block.
				if (REFind("^=pre($| )", line)) {
					if (Len(line) eq 4) {
						output = output & "<pre>" & Chr(13) & Chr(10);
					} else {
						output = output &
							"<pre class=""" &
							Replace(Trim(Right(line, Len(line) - 4)), """", "&quot;", "ALL") &
							""">" & Chr(13) & Chr(10);
					}
					state  = "pre";
					break;
				}

				if (line eq "=quote") {
					output = output & "<blockquote>" & Chr(13) & Chr(10);
					quoteDepth = quoteDepth + 1;
					break;
				}

				if (line eq "=/quote" and quoteDepth gt 0) {
					output = output & "</blockquote>" & Chr(13) & Chr(10);
					quoteDepth = quoteDepth - 1;
					break;
				}

				// Let raw HTML through. Good for when there's something
				// the parser can't handle.
				if (allowHtml and line eq "=html") {
					state = "html";
					break;
				}

				// Header.
				if (Left(line, 2) eq "==") {
					output = output &
						Textuality_Heading(line, imagePath, followLinks) &
						Chr(13) & Chr(10);
					break;
				}

				// Lists (ordered and unordered, but not definition yet)
				if (REFind("^ +[*##] ", line) neq 0) {
					state = "list";

					matches       = REFind("^( +)([*##]) (.*)", line, 1, true);
					listDepths[1] = matches.len[2];
					listCodes[1]  = Mid(line, matches.pos[3], 1);
					inline        = Right(line, matches.len[4]);

					if (listCodes[1] eq "*") {
						output = output & "<ul><li>";
					} else {
						output = output & "<ol><li>";
					}
					break;
				}

				// Horizontal rule.
				if (line eq "----") {
					output = output & "<hr/>" & Chr(13) & Chr(10);
					break;
				}

				// Start of a paragraph.
				if (line neq "") {
					output = output & "<p>";
					inline = line;
					state  = "p";
					break;
				}

				// Blank line to pass through.
				output = output & Chr(13) & Chr(10);
				break;

			// A raw HTML block.
			case "html":
				if (line eq "=/html") {
					state = "/";
				} else {
					output = output & line & Chr(13) & Chr(10);
				}
				break;

			case "list":
				if (line eq "") {
					// End list
					output = output &
						Textuality_Inline(inline, imagePath, followLinks) &
						Textuality_CleanupLists(listCodes, 1) &
						Chr(13) & Chr(10) & Chr(13) & Chr(10);
					ArrayClear(listCodes);
					ArrayClear(listDepths);
					state = "/";
					break;
				}

				matches = REFind("^( +)([*##]) (.*)", line, 1, true);
				if (matches.len[1] eq 0) {
					// Build up the items's inline text.
					inline = inline & Chr(13) & Chr(10) & line;
					break;
				}

				// Start of a new list item.
				output      = output & Textuality_Inline(inline, imagePath, followLinks);
				inline      = Right(line, matches.len[4]);
				depthChange = matches.len[2] - listDepths[ArrayLen(listDepths)];

				// Introduce a deeper list.
				if (depthChange gt 0) {
					ArrayAppend(listDepths, matches.len[2]);
					ArrayAppend(listCodes,  Mid(line, matches.pos[3], 1));
					if (listCodes[ArrayLen(listCodes)] eq "*") {
						output = output & Chr(13) & Chr(10) & "<ul><li>";
					} else {
						output = output & Chr(13) & Chr(10) & "<ol><li>";
					}
					break;
				}

				// Close up any open lists within this one.
				if (depthChange lt 0) {
					j = 1;
					while (listDepths[j] lte matches.len[2]) {
						j = j + 1;
					}
					output = output & Textuality_CleanupLists(listCodes, j);
					// No, we can't use ArrayResize: that only expands.
					// How retarded is that!?
					while (ArrayLen(listCodes) gte j) {
						ArrayDeleteAt(listCodes,  j);
						ArrayDeleteAt(listDepths, j);
					}
				}

				// Continue same list (or close this one if it's a
				// different type.
				output = output & "</li>" & Chr(13) & Chr(10);
				if (listCodes[ArrayLen(listCodes)] neq Mid(line, matches.pos[3], 1)) {
					if (listCodes[ArrayLen(listCodes)] eq "*") {
						output = output & "</ul><ol>";
					} else {
						output = output & "</ol><ul>";
					}
					listCodes[ArrayLen(listCodes)] = Mid(line, matches.pos[3], 1);
				}
				output = output & "<li>";
				break;

			// A paragraph block.
			case "p":
				if (line eq "") {
					// End of paragraph.
					output = output &
						Textuality_Inline(inline, imagePath, followLinks) &
						"</p>" & Chr(13) & Chr(10) & Chr(13) & Chr(10);
					inline = "";
					state  = "/";
				} else {
					// Build up the paragraph's inline text.
					inline = inline & Chr(13) & Chr(10) & line;
				}
				break;

			// A preformatted text block.
			case "pre":
				if (lines[i] eq "=/pre") {
					// End the block.
					output = output & "</pre>" & Chr(13) & Chr(10);
					state  = "/";
				} else {
					// Copy over each preformatted line.
					output = output & HtmlEditFormat(line) & Chr(13) & Chr(10);
				}
				break;
		}
	}

	// Clean up at end.
	switch (state) {
		case "p":
			output = output &
				Textuality_Inline(inline, imagePath, followLinks) & "</p>";
			break;

		case "pre":
			output = output & "</pre>";
			break;

		case "list":
			output = output &
				Textuality_Inline(inline, imagePath, followLinks) &
				Textuality_CleanupLists(listCodes, 1);
			break;
	}

	return output & Chr(13) & Chr(10) &
		RepeatString("</blockquote>" & Chr(13) & Chr(10), quoteDepth);
}

/**
 * 
 */
function Textuality_BreakIntoLines(s) {
	var i = 0;
	var lines = "";

	// Hack the text into an array of lines.
	s  = Replace(s, Chr(13) & Chr(10), Chr(10),       "ALL");
	s  = Replace(s, Chr(10),           " " & Chr(10), "ALL");
	lines = ListToArray(s, Chr(10));

	// Trim the extra spaces added to the end so we could break it into lines.
	for (i = 1; i lte ArrayLen(lines); i = i + 1) {
		lines[i] = RTrim(lines[i]);
	}

	return lines;
}

/**
 * Formats the inline markup in a paragraph.
 */
function Textuality_Inline(s, imagePath, followLinks) {
	var result  = "";
	var iStart  = 1;
	var iEnd    = 0;
	var pending = ArrayNew(1);
	var i       = 0;
	var marker  = "";
	var tag     = "";

	s = Replace(HtmlEditFormat(s), "&quot;", """", "ALL");
	s = Replace(s, "\""", "&quot;", "ALL");
	s = Replace(s, "\'",  "&apos;", "ALL");
	s = Textuality_MagicQuotes(s);

	for (;;) {
		iEnd = FindOneOf("_*`{}\", s, iStart);
		if (iEnd eq 0) {
			// No more to process: exit.
			result = result & Mid(s, iStart, Len(s) - iStart + 1);
			break;
		}

		result = result & Mid(s, iStart, iEnd - iStart);
		iStart = iEnd;

		marker = Mid(s, iStart, 1);
		switch (marker) {
			case "\":
				if (iStart eq Len(s)) {
					result = result & "\";
					iStart = iStart + 1;
				} else {
					// Escape character: grab the next character and let it
					// through verbatim.
					result = result & Mid(s, iStart + 1, 1);
					iStart = iStart + 2;
				}
				break;

			case "{":
				iStart = iStart + 1;

				// Check for a smilie.
				matches = REFind(":([a-z]+):}", s, iStart, true);
				if (matches.pos[1] eq iStart) {
					// Legal ones:
					// angry, biggrin, blush, confused, cool, crazy, cry,
					// doze, hehe, laugh, plain, rolleyes, satisfied, sad,
					// shocked, smile, tongue, wink
					result = result & 
						"<img width=""20"" height=""20"" src=""" &
						imagePath & Mid(s, matches.pos[2], matches.len[2]) &
						".gif"" alt=""[" &
						Mid(s, matches.pos[2], matches.len[2]) & "]""/>";
					iStart = iStart + matches.len[1];
				} else {
					// Links: {http://example.com/ like so}.
					iEnd = FindOneOf("} ", s, iStart);
					result = result & "<a href=""" &
						Trim(Mid(s, iStart, iEnd - iStart)) & """";
					if (not followLinks) {
						result = result & " rel=""nofollow""";
					}
					result = result & ">";
					if (Mid(s, iEnd, 1) eq "}") {
						result = result &
							Trim(Mid(s, iStart, iEnd - iStart)) & "</a>";
					} else {
						ArrayAppend(pending, "{");
					}
					iStart = iEnd + 1;
				}
				break;

			case "}":
				// Close link.
				iStart = iStart + 1;
				if (ArrayLen(pending) gt 0 and pending[ArrayLen(pending)] eq "{") {
					ArrayDeleteAt(pending, ArrayLen(pending));
					result = result & "</a>";
				} else {
					result = result & "}";
				}
				// End link.
				break;

			case "`":
			case "*":
			case "_":
				iStart = iStart + 1;

				if (marker eq "`") {
					tag = "code";
				} else if (marker eq "*") {
					tag = "strong";
				} else if (marker eq "_") {
					tag = "em";
				}

				if (ArrayLen(pending) gt 0 and pending[ArrayLen(pending)] eq marker) {
					// Close segment.
					ArrayDeleteAt(pending, ArrayLen(pending));
					result = result & "</" & tag & ">";
				} else {
					// Open segment.
					ArrayAppend(pending, marker);
					result = result & "<" & tag & ">";
				}
				break;
		}
	}

	// Close any remaining tags.
	for (i = ArrayLen(pending); i gt 0; i = i - 1) {
		switch (pending[i]) {
			case "`":
				result = result & "</code>";
				break;

			case "*":
				result = result & "</strong>";
				break;

			case "_":
				result = result & "</em>";
				break;

			case "{":
				result = result & "</a>";
				break;
		}
	}

	// Cleanup regexes.
	result = REReplace(result, "</([^>]+)><\1>", "", "ALL");
	result = REReplace(result, "<([^>]+)></\1>", "", "ALL");

	// Convert linebreaks (not sure if I want this yet).
	result = Replace(result, Chr(10), /*"<br/>" &*/ Chr(13) & Chr(10), "ALL");

	return result;
}

/**
 * 
 */
function Textuality_MagicQuotes(s) {
	var prefix = "(^|[ " & Chr(9) & Chr(10) & Chr(13);

	// Left quotes
	s = REReplace(s, prefix & "'])""",        "\1&##8220;", "ALL");
	s = REReplace(s, prefix & "]|&##8220;)'", "\1&##8216;", "ALL");

	// Right quotes
	s = Replace(s, """",        "&##8221;",  "ALL");
	s = Replace(s, "'&##8220;", "'&##8221;", "ALL");
	s = Replace(s, "'",         "&##8217;",  "ALL");

	return s;
}

/**
 * 
 */
function Textuality_Heading(s, imagePath, followLinks) {
	var matches = REFind("^(={2,7})(.*)$", s, 1, true);
	var level   = matches.len[2] - 1;
	if (matches.len[3] eq 0) {
		// Empty, so we ignore it.
		return "";
	}
	return "<h" & level & ">" &
		Textuality_Inline(Right(s, matches.len[3]), imagePath, followLinks) &
		"</h" & level & ">";
}

/**
 *
 */
function Textuality_CleanupLists(listCodes, toLevel) {
	var result = "";
	var i      = 0;

	for (i = ArrayLen(listCodes); i ge toLevel; i = i - 1) {
		if (listCodes[i] eq "*") {
			result = result & "</li></ul>";
		} else {
			result = result & "</li></ol>";
		}
	}

	return result;
}
</cfscript>

