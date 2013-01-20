<?xasml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" 
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- Icons by Everaldo Coelho (www.everaldo.com) -->

<xsl:template match="/">

	<xsl:variable name="dirname">
		<xsl:choose>
			<xsl:when test="config/dirname=''">my Miranda Webserver</xsl:when>
			<xsl:otherwise><xsl:value-of select="config/dirname"/></xsl:otherwise>
		</xsl:choose>
	</xsl:variable>

<html>
	<head>
		<title>Directory Listing of <xsl:value-of select="$dirname"/></title>
		<style type="text/css">
			td { font-family:tahoma, arial; font-size:14px; color:#000033;}
			a:link { text-decoration: none; color:#003366; }
			a:visited { text-decoration: none; color:#003366; }
			a:active { text-decoration: none; color:#FFAA33; }
			a:hover { text-decoration: underline; color:#FFAA33; }
		</style>
	</head>

	<body bgcolor='#B0B0CC'>
		<font face='tahoma, arial'>
			<table border='0' cellspacing='1' cellpadding='2' width='700'>
				<tr height='40' bgcolor='#EEEEFF'>
					<td colspan='3' align='center'>
						<h3 style='margin:0px'><font color="#666688">Directory Listing of <xsl:value-of select="$dirname"/></font></h3>
					</td>
				</tr>
				<xsl:for-each select="config/item">
					<xsl:sort select="@isdir" order="descending"/>
					<xsl:sort select="@name"/>
					<xsl:element name='tr'>
						<xsl:attribute name="bgcolor">
							<xsl:choose>
								<xsl:when test="position() mod 2 = 0">#EEEEFF</xsl:when>
								<xsl:otherwise>#DDDDEE</xsl:otherwise>
							</xsl:choose>
						</xsl:attribute>
						<xsl:element name='td'>
							<xsl:attribute name="colspan">
								<xsl:choose>
									<xsl:when test="@isdir='true'">3</xsl:when>
									<xsl:otherwise>1</xsl:otherwise>
								</xsl:choose>
							</xsl:attribute>
							<xsl:variable name="ext">
								<xsl:value-of select="translate(@ext, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
							</xsl:variable>
							<xsl:variable name="icon">
								<xsl:choose>
									<xsl:when test="@isdir='true'">folder_gray.png</xsl:when>
									<xsl:when test="$ext='jpg' or $ext='jpeg' or $ext='gif' or $ext='png' or $ext='bmp'">image2.png</xsl:when>
									<xsl:when test="$ext='zip' or $ext='rar' or $ext='ace' or $ext='arj' or $ext='lha' or ext='lhz' or ext='tar' or ext='gz'">tar.png</xsl:when>
									<xsl:when test="$ext='rtf' or $ext='doc'">document2.png</xsl:when>
									<xsl:when test="$ext='txt' or $ext='diz'">txt.png</xsl:when>
									<xsl:when test="$ext='pdf'">pdf.png</xsl:when>
									<xsl:when test="$ext='exe' or $ext='bat'">win_apps.png</xsl:when>
									<xsl:when test="$ext='html' or $ext='htm' or $ext='xml'">www.png</xsl:when>
									<xsl:when test="$ext='avi' or $ext='mpg'">video.png</xsl:when>
									<xsl:when test="$ext='c' or $ext='cpp' or $ext='h'">source.png</xsl:when>
									<xsl:when test="$ext='mp3' or $ext='ogg' or $ext='wav' or $ext='mid' or $ext='mod' or $ext='xm'">sound.png</xsl:when>
									<xsl:otherwise>empty.png</xsl:otherwise>
								</xsl:choose>
							</xsl:variable>
							<xsl:element name='img'>
								<xsl:attribute name="src">/icons/<xsl:value-of select="$icon"/></xsl:attribute>
							</xsl:element>
							<xsl:element name='img'>
								<xsl:attribute name="src">/placeholder.gif</xsl:attribute>
								<xsl:attribute name="width">5</xsl:attribute>
							</xsl:element>
							<xsl:element name="a">
								<xsl:attribute name="href">
									<xsl:value-of select="@name"/>
									<xsl:if test="@ext!=''">.<xsl:value-of select="@ext"/></xsl:if>
									<xsl:if test="@isdir='true'">/</xsl:if>
								</xsl:attribute>
								<xsl:value-of select="@name"/>
								<xsl:if test="@ext!=''">.<xsl:value-of select="@ext"/></xsl:if>
							</xsl:element>
						</xsl:element>
						<xsl:choose>
							<xsl:when test="@isdir='true'"></xsl:when>
							<xsl:otherwise>
								<td><xsl:value-of select="@modified"/></td>
								<!-- <td><xsl:value-of select="@created"/></td> -->
								<td width='100' align='right'>
									<xsl:choose>
										<xsl:when test="@size >= 1073741824">
											<xsl:value-of select="round(@size div 107374182.4) div 10"/><font size="-1"> GB</font>
										</xsl:when>
										<xsl:when test="@size >= 1048576">
											<xsl:value-of select="round(@size div 104857.6) div 10"/><font size="-1"> MB</font>
										</xsl:when>
										<xsl:when test="@size >= 1024">
											<xsl:value-of select="round(@size div 102.4) div 10"/><font size="-1"> KB</font>
										</xsl:when>
										<xsl:otherwise>
											<xsl:value-of select="@size"/><font size="-1"> Byte</font>
										</xsl:otherwise>
									</xsl:choose>
								</td>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:element>
				</xsl:for-each>
			</table>
		</font>
	</body>
</html>

</xsl:template>

</xsl:stylesheet>
