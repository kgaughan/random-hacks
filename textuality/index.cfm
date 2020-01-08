<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" lang="en-ie" xml:lang="en-ie"><head>

	<title>Textuality Test Page</title>
	<meta name="Author" content="Keith Gaughan"/>
	<meta name="Copyright" content="Copyright (c) Keith Gaughan, 2006"/>

	<style type="text/css" media="screen">
	textarea {
		width:          100%;
	}
	.display {
		border:         1px solid black;
		padding:        0 1em;
		overflow:       scroll;
		height:         12em;
		margin:         1ex 0;
	}
	blockquote {
		border-left:    2px solid blue;
		margin-left:    1ex;
		padding-left:   1ex;
	}
	pre {
		background:     #FFE;
		border:         1px solid #EED;
		padding:        1ex;
	}
	code {
		background:     #FFE;
		border:         1px solid #EED;
		padding:        0.05ex 0.25ex;
	}
	img {
		vertical-align: middle;
	}
	</style>

</head><body>

<h1><a href="http://talideon.com/projects/textuality/">Textuality Test
Page</a></h1>

<p>This page allows you to test drive <em>Textuality</em>. By default, the
textarea below displays the Textuality manual in Textuality. If you want to
reload the manual into the textarea, <a href="<cfoutput>#CGI.SCRIPT_NAME#</cfoutput>"
title="Test page with manual loaded">click here</a>. Take a read through it
first and compare it to what you see in the textarea.
<a href="./textuality.zip">Download Textuality</a>.</p>

<cffile action="read" file="#ExpandPath('manual.txt')#" variable="manual">
<cfparam type="string" name="FORM.body" default="#manual#">

<form method="post" action="<cfoutput>#CGI.SCRIPT_NAME#</cfoutput>">
<div>
<textarea name="body" cols="80" rows="10">
<cfoutput>#HTMLEditFormat(FORM.body)#</cfoutput>
</textarea>
</div>
<div>
<input type="submit" value="Submit"/>
</div>

<cfinclude template="textuality.cfm">

<div class="display">
<cfoutput>#Textuality(FORM.body, "", true, true)#</cfoutput>
</div>

<address>
Copyright &copy; Keith Gaughan, 2006. All Rights Reserved.
</address>

</body></html>
