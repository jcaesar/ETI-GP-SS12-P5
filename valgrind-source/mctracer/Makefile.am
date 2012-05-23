include $(top_srcdir)/Makefile.tool.am

EXTRA_DIST = docs/tr-manual.xml

bin_PROGRAMS = tr-consumer

tr_consumer_SOURCES = tr_consumer.c shm_consumer.c
tr_consumer_CFLAGS  = $(AM_CFLAGS_PRI)
tr_consumer_LDFLAGS = $(AM_CFLAGS_PRI)

#----------------------------------------------------------------------------
# mctracer-<platform>
#----------------------------------------------------------------------------

noinst_PROGRAMS  = mctracer-@VGCONF_ARCH_PRI@-@VGCONF_OS@
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += mctracer-@VGCONF_ARCH_SEC@-@VGCONF_OS@
endif

MCTRACER_SOURCES_COMMON = tr_main.c shm_vgprod.c

mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_SOURCES      = \
	$(MCTRACER_SOURCES_COMMON)
mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_PRI_CAPS@)
mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_PRI_CAPS@)
mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDFLAGS      = \
	$(TOOL_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LINK = \
	$(top_builddir)/coregrind/link_tool_exe_@VGCONF_OS@ \
	@VALT_LOAD_ADDRESS_PRI@ \
	$(LINK) \
	$(mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS) \
	$(mctracer_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDFLAGS)

if VGCONF_HAVE_PLATFORM_SEC
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_SOURCES      = \
	$(MCTRACER_SOURCES_COMMON)
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_SEC_CAPS@)
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_SEC_CAPS@)
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDFLAGS      = \
	$(TOOL_LDFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LINK = \
	$(top_builddir)/coregrind/link_tool_exe_@VGCONF_OS@ \
	@VALT_LOAD_ADDRESS_SEC@ \
	$(LINK) \
	$(mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CFLAGS) \
	$(mctracer_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDFLAGS)
endif


