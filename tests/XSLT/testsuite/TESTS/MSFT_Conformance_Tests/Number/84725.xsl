<?xml version="1.0"?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output encoding="UTF-16"/>

<!-- XSLT - Numbering - format= "&#x58f9;" (Chinese traditional) -  letter-value="alphabetic" | "traditional" -->
<!-- Created : Khalil Jabrane -->
<!-- Date: 04/12/2000 -->

<xsl:template match="/">  
	<TABLE>  
		<xsl:for-each select="//現的輸入法">
			<TR><TD>  
				<TD>
					<!-- Chinese traditional -->
					<xsl:number format="&#x58f9;" letter-value="traditional"/> *					
					<xsl:value-of select = "text()"/>
				</TD>
			</TD></TR>
		</xsl:for-each>  
	</TABLE>
</xsl:template>  

</xsl:stylesheet>  

