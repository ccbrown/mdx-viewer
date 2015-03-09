SHELL = /bin/bash

EXENAME = Viewer

CFLAGS = -x objective-c++ -DTARGET_OS=MAC_OS_X -DVERSION=0 -I/usr/local/include/freetype2
LDFLAGS = -framework Cocoa -framework OpenGL -lfreetype -lboost_thread -lpng -lz

BLDDIR = build

SRCDIR = src
OBJDIR = obj

XIBDIR = xib
NIBDIR = nib

FILDIR = files
RESDIR = resources/macosx

SRCS := $(shell find $(SRCDIR) -name '*.c') $(shell find $(SRCDIR) -name '*.cpp') $(shell find $(SRCDIR) -name '*.m')
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(patsubst $(SRCDIR)/%.m,$(OBJDIR)/%.o,$(SRCS))))

XIBS := $(shell find $(XIBDIR) -name '*.xib')
NIBS := $(patsubst $(XIBDIR)/%.xib,$(NIBDIR)/%.nib,$(XIBS))

FILES := $(shell find $(FILDIR) -not \( -name .svn -a -prune \))
DIRS := $(shell find $(FILDIR) -not \( -name .svn -a -prune \))

RESOURCES := $(shell find $(RESDIR) -not \( -name .svn -a -prune \))

FILES_OS_X := $(patsubst $(FILDIR)/%,$(BLDDIR)/macosx/%,$(FILES))
RESOURCES_OS_X := $(patsubst $(RESDIR)/%,$(BLDDIR)/macosx/$(EXENAME).app/%,$(RESOURCES))

all: macosx

macosx: $(FILES_OS_X) $(BLDDIR)/macosx/$(EXENAME).app/Contents/MacOS/$(EXENAME) $(RESOURCES_OS_X)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.m
	$(CXX) $(CFLAGS) -c $< -o $@

$(NIBDIR)/%.nib: $(XIBDIR)/%.xib
	mkdir -p $(dir $@)
	ibtool --compile $@ $<

$(BLDDIR)/macosx/$(EXENAME).app/%: $(RESDIR)/%
	mkdir -p $(dir $@)
	cp -R $< $(dir $@)
	touch $@
	
$(BLDDIR)/macosx/%: $(FILDIR)/%
	mkdir -p $(dir $@)
	cp -R $< $(dir $@)
	touch $@

$(BLDDIR)/macosx/$(EXENAME).app/Contents/MacOS/$(EXENAME): $(OBJDIR) $(OBJS) $(NIBS)
	mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	cp -R $(NIBDIR)/* $(BLDDIR)/macosx/$(EXENAME).app/Contents/Resources

clean:
	if test -d $(BLDDIR); then rm -r $(BLDDIR); fi
	if test -d $(OBJDIR); then rm -r $(OBJDIR); fi
	if test -d $(NIBDIR); then rm -r $(NIBDIR); fi