<?xml version="1.0" encoding="ISO-8859-1" ?>
		<!--
		 * template for HTTPServer Plugin for Miranda IM by Houdini
				http://addons.miranda-im.org/details.php?action=viewfile&id=2304

		 * the icons are taken from the 'Crystal Project' by Everaldo Coelho, licenced under LGPL
				http://www.everaldo.com/

		 * Template modded by Wishmaster
			[+] header bar
			[+] bottom bar with number of total files and total file size
			[+] tooltips on each field (with real byte-size)
			[+] moved design to seperate CSS file
			[*] proper support for "shortcut icon" aka "favicon" (with link tag)
			[*] lots of code changes
		-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<!-- site properties -->
	<xsl:output method="xml" version="1.1" doctype-public="-//W3C//DTD XHTML 1.1//EN"
		doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" encoding="ISO-8859-1" 
			omit-xml-declaration="no" indent="yes" standalone="no" media-type="application/xhtml+xml" />

	<xsl:template match="/child::config">
		<!-- title of the site -->
		<xsl:variable name="title">
			<xsl:choose>
				<xsl:when test="child::dirname!=''">Directory <xsl:value-of select="child::dirname" /></xsl:when>
				<xsl:otherwise>Miranda NG Webserver</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		
		<!-- total size (in bytes) of all files -->
		<xsl:variable name="size" select="sum(child::item/@size)" />
		
		<xsl:variable name="total-realsize">
			<xsl:call-template name="convert">
				<xsl:with-param name="bytes" select="$size" />
			</xsl:call-template>
		</xsl:variable>
		
		<!--begin of the site -->
		<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="de">
			<head>
				<!-- title of the page -->
				<title><xsl:value-of select="$title" /></title>
				
				<!-- path to the FavIcon -->
				<link type="image/vnd.microsoft.icon" rel="shortcut icon" href="/favicon.ico" />
				
				<!-- path to the CSS file -->
				<link type="text/css" rel="stylesheet" href="/theme/style/simpleTheme.css" />
			</head>
			<body>
				<table id="listing" class="listing">
					<caption id="tablecaption" class="tablecaption" title="{$title}"><xsl:value-of select="$title" /></caption>
					<tr id="header" class="header">
						<th id="fileheader" class="file">Name</th>
						<th id="sizeheader" class="size">Size</th>
						<th id="modifiedheader" class="modified">Modified</th>
						<th id="createdheader" class="created">Created</th>
					</tr>
					<xsl:for-each select="child::item">
						<!-- an item can have following attributes:
							@name		- name of the file /folder
							@isdir		- 'true' if the item represents a folder, missing  for files
							@ext		- extension of the file (only for files)
							@size 		- size of the file (number of bytes) (only for files)
							@created	- date the file was created (only for files)
							@modified	- date the file was last modified (only for files)
						-->
						<!--sorting order-->
						<xsl:sort select="@isdir='true'" order="descending" />
						<xsl:sort select="@ext" data-type="text" case-order="lower-first" order="ascending" /> <!-- sort by extension-->
						<xsl:sort select="@name" data-type="text" case-order="lower-first" order="ascending" /> <!-- sort by name-->
						<!--<xsl:sort select="@size" data-type="number" order="ascending" />--> <!-- sort by size-->
						
						<tr id="item{position()}">
							<xsl:attribute name="class"> 
								<!-- either even or odd-->
								<xsl:choose>
									<xsl:when test="(position() mod 2) = 0">even</xsl:when>
									<xsl:otherwise>odd</xsl:otherwise>
								</xsl:choose>
							</xsl:attribute>
							<xsl:choose>
								<xsl:when test="@isdir='true'">
									<!-- this is a folder -->
									<td class="folder" id="folder{position()}" colspan="4" title="Verzeichnis: {@name}">
										<a href="{@name}/" class="folder" id="link{position()}" title="{@name} (Verzeichnis)">
											<img src="/theme/icons/folder_gray.png" alt="folder" />
											<xsl:value-of select="@name" />
										</a>
									</td>
								</xsl:when>
								<xsl:otherwise>
									<!-- this is a file -->
									
									<!-- complete name of the file (with extension, if there is one)-->
									<xsl:variable name="filename">
										<xsl:value-of select="@name" /><xsl:if test="@ext!=''">.<xsl:value-of select="@ext" /></xsl:if>
									</xsl:variable>
									
									<!-- filesize with appropriate units-->
									<xsl:variable name="realsize">
										<xsl:call-template name="convert">
											<xsl:with-param name="bytes" select="@size" />
										</xsl:call-template>
									</xsl:variable>
									
									<!-- extension in lowercase -->
									<xsl:variable name="l_ext" select="translate(@ext,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')" />
									
									<td class="file" id="file{position()}" title="Datei: {$filename}">
										<a href="{$filename}" class="{$l_ext}" id="link{position()}" title="{$filename} ({$realsize})">
											<xsl:element name="img">
												<xsl:attribute name="alt"><xsl:value-of select="@ext" /></xsl:attribute>
												<xsl:attribute name="src">
													<xsl:choose>
														<!-- try to find appropriate icon (case-insensitive compare, as extensions are always case-insensitive in Windows) -->
														<xsl:when test="$l_ext='jpg' or $l_ext='jpeg' or $l_ext='gif' or $l_ext='png' or $l_ext='bmp'">/theme/icons/image2.png</xsl:when> 
														<xsl:when test="$l_ext='zip' or $l_ext='rar' or $l_ext='ace' or $l_ext='arj' or $l_ext='lha' or $l_ext='lhz' or $l_ext='tar' or $l_ext='gz' or $l_ext='7z'">/theme/icons/tar.png</xsl:when> 
														<xsl:when test="$l_ext='rtf' or $l_ext='doc'">/theme/icons/document2.png</xsl:when> 
														<xsl:when test="$l_ext='txt' or $l_ext='diz'">/theme/icons/txt.png</xsl:when> 
														<xsl:when test="$l_ext='pdf'">/theme/icons/pdf.png</xsl:when>
														<xsl:when test="$l_ext='tex'">/theme/icons/tex.png</xsl:when>
														<xsl:when test="$l_ext='java' or $l_ext='jar'">theme/icons/source_java.png</xsl:when> 
														<xsl:when test="$l_ext='exe' or $l_ext='bat' or $l_ext='dll'">/theme/icons/win_apps.png</xsl:when> 
														<xsl:when test="$l_ext='html' or $l_ext='htm' or $l_ext='xml' or $l_ext='php' or $l_ext='css'">/theme/icons/www.png</xsl:when> 
														<xsl:when test="$l_ext='avi' or $l_ext='mpg' or $l_ext='mpeg'">/theme/icons/video.png</xsl:when> 
														<xsl:when test="$l_ext='c' or $l_ext='cpp' or $l_ext='h'">/theme/icons/source.png</xsl:when> 
														<xsl:when test="$l_ext='mp3' or $l_ext='ogg' or $l_ext='wav' or $l_ext='mid' or $l_ext='mod' or $l_ext='xm'">/theme/icons/sound.png</xsl:when> 
														<xsl:otherwise>/theme/icons/empty.png</xsl:otherwise> 
													</xsl:choose>
												</xsl:attribute><!--src-->
											</xsl:element><!--img-->
											<xsl:value-of select="$filename" />
										</a>
									</td>
									
									<!-- size -->
									<td class="size" id="size{position()}" title="Size: {$realsize} ({format-number(@size,'###,###,###,###,### Byte')})" >
										<xsl:value-of select="$realsize" />
									</td>
									
									<!-- modified -->
									<td class="modified" id="modified{position()}" title="Modified: {@modified}">
										<xsl:value-of select="@modified" />
									</td>
									
									<!-- created -->
									<td class="created" id="created{position()}" title="Created: {@created}">
										<xsl:value-of select="@created" />
									</td>
								</xsl:otherwise>
							</xsl:choose>
						</tr>
					</xsl:for-each><!--item-->
					
					<tr id="stats" class="stats">
						<th id="filestats" class="file">
							<xsl:value-of select="count(child::item[@isdir='true' and @name!='..'])" /> Directories, <xsl:value-of select="count(child::item[not(@isdir='true')])" />  Files
						</th>
						<th id="sizestats" class="size" title="Total Size: {$total-realsize} ({format-number($size,'###,###,###,###,### Byte')})">
							<xsl:value-of select="$total-realsize" />
						</th>
						<th id="modifiedstats" class="modified">total</th>
						<th id="createdstats" class="created"><a href="#header" id="uplink" title="to the top">scroll up</a></th>
					</tr>
				</table>
			</body>
		</html>
	</xsl:template> <!--config-->
	
	<!-- converts byte sizes into human-readable text -->
	<xsl:template name="convert">
		<xsl:param name="bytes" />
		<xsl:choose>
			<xsl:when test="$bytes &lt;= 0">
				...
			</xsl:when>
			<xsl:when test="$bytes &lt; 1024">
				<xsl:value-of select="format-number($bytes,'#,### Byte')" />
			</xsl:when>
			<xsl:when test="$bytes &lt; 1048576">
				<xsl:value-of select="format-number($bytes div 1024,'#,###.# KB')" />
			</xsl:when>
			<xsl:when test="$bytes &lt; 1073741824">
				<xsl:value-of select="format-number($bytes div 1048576,'#,###.# MB')" />
			</xsl:when>
			<xsl:when test="$bytes &lt; 1099511627776">
				<xsl:value-of select="format-number($bytes div 1073741824,'#,###.# GB')" />
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="format-number($bytes div 1099511627776,'#,###.# TB')" />
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>