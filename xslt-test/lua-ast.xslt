<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output indent="yes"/>
    <xsl:strip-space elements="*" />

    <xsl:template match="/">
        <function-defs>
            <xsl:apply-templates select="*" />
        </function-defs>
    </xsl:template>

    <xsl:template match="Set[group[position()=1]/Id][group[position()=2]/Function]">
        <function-def>
            <xsl:attribute name="name">
                <xsl:value-of select="group[position()=1]/Id/@value" />
                <xsl:text>(</xsl:text>
                <xsl:apply-templates select="group[position()=2]/Function" />
                <xsl:text>)</xsl:text>
            </xsl:attribute>
            <xsl:copy-of select="group/Function/location" />
            <xsl:copy-of select="group/Function/end-location" />
        </function-def>
    </xsl:template>

    <xsl:template match="Function/group[position()=1]/Id">
        <xsl:value-of select="@value" />
        <xsl:if test="position() != last()"><xsl:text>,</xsl:text></xsl:if>
    </xsl:template>
</xsl:stylesheet>
