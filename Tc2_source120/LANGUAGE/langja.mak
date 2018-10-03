# -------------------------------------------
# langja.mak                 Kazubon 2001
#-------------------------------------------

DLLFILE=..\out\langen.tclock2
LANGID=0x411
RCFILE=langja.rc
RESFILE=langja.res

CC=cl
LINK=link
RC=rc

OBJS=main.obj
LIBS=

COPT=/c /W3 /O2
LOPT=/SUBSYSTEM:WINDOWS /DLL /OUT:$(DLLFILE) /NODEFAULTLIB

$(DLLFILE): $(OBJS) $(RESFILE)
    $(LINK) $(LOPT) $(OBJS) $(LIBS) $(RESFILE)

main.obj: main.c
    $(CC) $(COPT) main.c

$(RESFILE): $(RCFILE) resource.h
    $(RC) /l $(LANGID) /fo $(RESFILE) $(RCFILE)
