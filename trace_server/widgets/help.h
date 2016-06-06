#pragma once

char const html_help[] = "\
	<center><h1>Quick help</h1></center>\
	<h2>General shortcuts</h2>\
	<table>\
		<tr>\
			<td> Shortcut </td> <td> Description </td>\
		</tr>\
		<tr>\
			<td> Scroll Lock </td>\
			<td> show / hide the logging server window </td>\
		</tr>\
		<tr>\
			<td> F1 </td>\
			<td> this screen. </td>\
		</tr>\
		<tr>\
			<td> Ctrl + O </td>\
			<td> Open trace file </td>\
		</tr>\
		<tr>\
			<td> Ctrl + S </td>\
			<td> Save trace file </td>\
		</tr>\
		<tr>\
			<td> Ctrl + Shift + S </td>\
			<td> Export to CSV formatted file</td>\
		</tr>\
		<tr>\
			<td> Ctrl + W </td>\
			<td> Close current tab </td>\
		</tr>\
	</table>\
	<h2>Text search shortcuts</h2>\
	<table>\
		<tr>\
			<td> Shortcut </td> <td> Description </td>\
		</tr>\
		<tr>\
			<td> Ctrl + F </td>\
			<td> Find text in column. Specific column can be selected in the combobox on the right.</td>\
		</tr>\
		<tr>\
			<td> / </td>\
			<td> Find text in column. Specific column can be selected in the combobox on the right.</td>\
		</tr>\
		<tr>\
			<td> some windows key </td>\
			<td> Find next occurence </td>\
		</tr>\
		<tr>\
			<td> some windows key2 </td>\
			<td> Find prev occurence </td>\
		</tr>\
		<tr>\
			<td> Ctrl + C </td>\
			<td> Copy selection to clipboard </td>\
		</tr>\
		<tr>\
			<td> Ctrl + Ins </td>\
			<td> Copy selection to clipboard </td>\
		</tr>\
	</table>\
	<h2>Filtering shortcuts</h2>\
	<table>\
		<tr>\
			<td> Shortcut </td> <td> Description </td>\
		</tr>\
		<tr>\
			<td> c </td>\
			<td> clear current view (same as clicking on last row and pressing X) </td>\
		</tr>\
		<tr>\
			<td> ctrl + L </td>\
			<td> clear current view </td>\
		</tr>\
		<tr>\
			<td> space </td>\
			<td> toggle reference row </td>\
		</tr>\
		<tr>\
			<td> x </td>\
			<td> exclude currently selected row from view </td>\
		</tr>\
		<tr>\
			<td> Del </td>\
			<td> Hide previous rows </td>\
		</tr>\
		<tr>\
			<td> Ctrl + Del </td>\
			<td> Shows again hidden rows by Del</td>\
		</tr>\
		<tr>\
			<td> </td>\
			<td> </td>\
		</tr>\
	</table>\
	<h2>Mouse operations:</h2>\
	<table>\
		<tr>\
			<td> Shortcut </td> <td> Description </td>\
		</tr>\
		<tr>\
			<td> click on table </td>\
			<td> sets current cell for search and for operations using current cell, like pressing Del or X</td>\
		</tr>\
		<tr>\
			<td> double click on table </td>\
			<td> if double click occurs within { } scope, the scope will be collapsed (and grayed) </td>\
		</tr>\
		<tr>\
			<td> double click on coloring regexp </td>\
			<td> color selection </td>\
		</tr>\
	</table>";

