<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <!-- FileName: str59 -->
  <!-- Document: http://www.w3.org/TR/xpath -->
  <!-- DocVersion: 19990922 -->
  <!-- Section: 4.2 String Functions -->
  <!-- Purpose: Test of 'contains()' function. -->

<xsl:template match="doc">
	<out>
		<xsl:value-of select="contains('abc', 'bc')"/>
	</out>
</xsl:template>

</xsl:stylesheet>