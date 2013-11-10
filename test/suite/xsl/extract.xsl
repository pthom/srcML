<xsl:stylesheet
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:src="http://www.sdml.info/srcML/src"
	xmlns="http://www.sdml.info/srcML/src"
	xmlns:cpp="http://www.sdml.info/srcML/cpp"
	xmlns:str="http://exslt.org/strings"
	xmlns:func="http://exslt.org/functions"
	xmlns:exsl="http://exslt.org/common"
	extension-element-prefixes="str exsl func"
	exclude-result-prefixes="src"
	version="1.0">

<!--
	extractunit.xsl

	Extracts a specific imbedded unit

	Michael L. Collard
	collard@mcs.kent.edu
-->

<xsl:param name="number"/>

<xsl:output method="xml" omit-xml-declaration="yes"/>

<xsl:template match="/src:unit">
	<xsl:apply-templates select="src:unit"/>
</xsl:template>

<!-- filter out second-level units of the wrong position -->
<xsl:template match="/src:unit/src:unit[position() != $number]"/>

<!-- default identity copy -->
<xsl:template match="@*|node()">
	<xsl:copy>
	  <xsl:apply-templates select="@*|node()"/>
	</xsl:copy>
</xsl:template>

</xsl:stylesheet>