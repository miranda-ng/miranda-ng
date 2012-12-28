<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- Edited with XML Spy v4.2 -->
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
  <html>
  <body>
  <h2>HTTP server configuration</h2>
    <table border="1">
      <tr bgcolor="#9acd32">
        <th align="left">Name</th>
        <th align="left">File</th>
        <th align="left">Max downloads</th>
        <th align="left">IP address</th>
        <th align="left">IP mask</th>
      </tr>
      <xsl:for-each select="config/share">
      <tr>
        <td>
			<xsl:element name="A">
				<xsl:attribute name="href"><xsl:value-of select="name"/></xsl:attribute>
				<xsl:value-of select="name"/>
			</xsl:element>
		  </td>
        <td><xsl:value-of select="file"/></td>
        <td><xsl:value-of select="max_downloads"/></td>
        <td><xsl:value-of select="ip_address"/></td>
        <td><xsl:value-of select="ip_mask"/></td>
      </tr>
      </xsl:for-each>
    </table>
  </body>
  </html>
</xsl:template>
</xsl:stylesheet>