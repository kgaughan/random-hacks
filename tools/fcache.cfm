<cfsilent>
<!---#DOC

== CF\_FCACHE

*CF\_FCACHE* is a simple persistent fragment caching tag. Cached data is
stored in the `\_cache` directory in the same directory as the tag. The tag
accepts four different _action_s:

 * _clear_: Clear the whole cache.
 * _flush_: Remove a comma-seperated list of cached objects.
 * _cache_: Cache the data between the start end end tags.
 * _check_: Executes the code contained between the start and end tags if the
            given object isn't cached.

If the _action_ is _flush_, _cache_, or _check_ there is an extra attribute
_name_ that, in the case of _flush_, takes a comma-separated list of cached
object ids to invalidate, and in the case of _cache_ and _check_, takes the
id of the object being cached. The value of cache defaults to
`path:#CGI.REQUEST\_URI#`.

The files in the `\_cache` directory are named after the MD5 hash of the
object id.

If you want to use a different cache directory than the default, pass in the
_absolute path_ of the cache directory in with the _cache_ attribute.

=== Notes

It's recommended that you only use caching if the given piece of code can
benefit from caching, i.e., if the fragment is expensive to generate but
doesn't change too often.

It is also recommended that you attempt to understand and use HTTP's built-in
caching mechanisms before using this tag as, despite the efforts made to make
this tag as unobtrusive as possible, it will complicate the application's
logic. If the performance boost from HTTP caching is inadequate (possibly
because the content is on a page that uses cookies or sessions), identify the
places where *CF\_CACHE* can be most effective and use it there.

=== Credits

*CF\_FCACHE* was written by {http://talideon.com/ Keith Gaughan}. It is 
Copyright (c) Keith Gaughan, 2006-07. All Rights Reserved.

If you have any queries about the tag, you can contact the author using the
contact form on the website.

--->

<cfset exitTag = true>

<cfif THISTAG.ExecutionMode eq "START">
	<cfparam name="ATTRIBUTES.action" type="string" default="cache">
	<cfparam name="ATTRIBUTES.name"   type="string" default="path:#CGI.REQUEST_URI#">
	<cfparam name="ATTRIBUTES.cache"  type="string" default="#GetDirectoryFromPath(GetCurrentTemplatePath())#_cache/">

	<cfswitch expression="#ATTRIBUTES.action#">

		<cfcase value="clear">
			<!--- Clear the whole cache --->
			<cfdirectory directory="#ATTRIBUTES.cache#" name="dir" filter="*.cache">
			<cfloop query="dir">
				<cfset name = Left(name, Len(name) - 6)>
				<cflock timeout="1" name="FCACHE:#name#" type="exclusive">
					<cffile action="delete" file="#ATTRIBUTES.cache##name#.cache">
				</cflock>
			</cfloop>
		</cfcase>

		<cfcase value="flush">
			<!--- Clear selected items from the cache --->
			<cfloop index="name" list="#ATTRIBUTES.name#">
				<cfset name = Hash(name)>
				<cflock timeout="1" name="FCACHE:#name#" type="exclusive">
					<cffile action="delete" file="#ATTRIBUTES.cache##name#.cache">
				</cflock>
			</cfloop>
		</cfcase>

		<cfcase value="check">
			<!--- Executes the code contained the given object isn't cached --->
			<cfset name = Hash(name)>
			<cflock timeout="1" name="FCACHE:#name#" type="exclusive">
				<cfset exitTag = FileExists(ATTRIBUTES.cache & name & ".cache")>
			</cflock>
		</cfcase>

		<cfcase value="cache">
			<!--- Cache the data contained within the opening and closing tags. --->
			<cfset name = Hash(ATTRIBUTES.name)>
			<cflock timeout="1" name="FCACHE:#name#" type="readonly">
				<cfif FileExists(ATTRIBUTES.cache & name & ".cache")>
					<cffile action="read"
						file="#ATTRIBUTES.cache##name#.cache"
						variable="content">
				<cfelse>
					<cfset exitTag = false>
				</cfif>
			</cflock>
		</cfcase>

		<cfdefaultcase><!--- Ignore ---></cfdefaultcase>

	</cfswitch>
<cfelse>
	<cfswitch expression="#ATTRIBUTES.action#">

		<cfcase value="cache">
			<!--- Cache the data contained within the opening and closing tags. --->
			<cfset name = Hash(ATTRIBUTES.name)>
			<cflock timeout="1" name="FCACHE:#name#" type="exclusive">
				<cffile action="write"
					file="#ATTRIBUTES.cache##name#.cache"
					output="#THISTAG.GeneratedContent#"
					addnewline="No">
			</cflock>
		</cfcase>

	</cfswitch>
</cfif>

</cfsilent><cfif IsDefined("content")><cfoutput>#content#</cfoutput></cfif><cfif exitTag><cfexit method="ExitTag"></cfif>
