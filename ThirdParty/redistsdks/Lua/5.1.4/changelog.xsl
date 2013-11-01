<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet
    xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
    version='1.0'>

  <xsl:output method="html" indent="yes" encoding="US-ASCII"
              doctype-public="-//W3C//DTD HTML 4.01//EN"
              doctype-system="http://www.w3.org/TR/html401/strict.dtd"/>


  <xsl:template match="/changelog">
    <HTML><head></head><BODY>
    <h1>
      <xsl:value-of select="./@name"/>
      <xsl:text> change log file</xsl:text>
    </h1>
    <xsl:apply-templates />
    </BODY></HTML>
  </xsl:template>
  
  
  <xsl:template match="version">
    <h2>
    <xsl:text>Version </xsl:text>
    <xsl:value-of select="./@major"/>
    <xsl:text>.</xsl:text>
    <xsl:value-of select="./@minor"/>
    <xsl:if test="./@fix &gt; 0">
      <xsl:text>.</xsl:text>
      <xsl:value-of select="./@fix"/>
    </xsl:if>
    <xsl:if test="./@build &gt; 0">
      <xsl:text> Build </xsl:text>
      <xsl:value-of select="./@build"/>
    </xsl:if>
    <xsl:if test="./@desc != ''">
      <xsl:text> - </xsl:text>
      <xsl:value-of select="./@desc"/>
    </xsl:if>
    </h2>
    <ul>
      <xsl:apply-templates />
    </ul>
  </xsl:template>


  <xsl:template match="changed">
    <li>
      <span style="color:green">Changed: </span>
  	  <xsl:value-of select="." disable-output-escaping="yes"/>
  	</li>
  </xsl:template>
	    

  <xsl:template match="fixed">
    <li>
      <span style="color:red">Fixed: </span>
  	  <xsl:value-of select="." disable-output-escaping="yes"/>
  	</li>
  </xsl:template>
  

  <xsl:template match="added">
    <li>
      <span style="color:blue">New: </span>
  	  <xsl:value-of select="." disable-output-escaping="yes"/>
  	</li>
  </xsl:template>
  
  
  <xsl:template match="oldchangelog">
  	<xsl:value-of select="." disable-output-escaping="yes"/>
  </xsl:template>
  

</xsl:stylesheet>
