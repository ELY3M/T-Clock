# -------------------------------------------
# tcdll.mak              Kazubon 1998-2001
#-------------------------------------------

DLLFILE=..\out\tc2dll.tclock
LIBFILE=..\exe\tc2dll.lib
RCFILE=tcdll.rc
RESFILE=tcdll.res
DEFFILE=tcdll.def

CC=cl
LINK=link
RC=rc

OBJS=main.obj tclock.obj format.obj desktop.obj startbtn.obj \
 startmenu.obj taskswitch.obj traynotify.obj\
 font.obj bmp.obj utl.obj newapi.obj sysres.obj cpu.obj mbm.obj permon.obj net.obj
LIBS=kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comctl32.lib  msimg32.lib

COPT=/c /W3 /O1 /DWIN32
LOPT=/SUBSYSTEM:WINDOWS /OPT:NOWIN98 /DLL /DEF:$(DEFFILE) /IMPLIB:$(LIBFILE)

$(DLLFILE): $(OBJS) $(RESFILE)
    $(LINK) $(LOPT) /OUT:$(DLLFILE) $(OBJS) $(LIBS) $(RESFILE)

main.obj: main.c tcdll.h
    $(CC) $(COPT) main.c
tclock.obj: tclock.c tcdll.h
    $(CC) $(COPT) tclock.c
format.obj: format.c tcdll.h
    $(CC) $(COPT) format.c
desktop.obj: desktop.c tcdll.h
    $(CC) $(COPT) desktop.c
startbtn.obj: startbtn.c tcdll.h
    $(CC) $(COPT) startbtn.c
startmenu.obj: startmenu.c tcdll.h
    $(CC) $(COPT) startmenu.c
taskswitch.obj: taskswitch.c tcdll.h
    $(CC) $(COPT) taskswitch.c
traynotify.obj: traynotify.c tcdll.h
    $(CC) $(COPT) traynotify.c
font.obj: font.c tcdll.h
    $(CC) $(COPT) font.c
bmp.obj: bmp.c tcdll.h
    $(CC) $(COPT) bmp.c
utl.obj: utl.c tcdll.h
    $(CC) $(COPT) utl.c
newapi.obj: newapi.c tcdll.h
    $(CC) $(COPT) newapi.c
sysres.obj: sysres.c tcdll.h
    $(CC) $(COPT) sysres.c
cpu.obj: cpu.c tcdll.h
    $(CC) $(COPT) cpu.c
mbm.obj: mbm.c tcdll.h
    $(CC) $(COPT) mbm.c
permon.obj: permon.c tcdll.h
    $(CC) $(COPT) permon.c
net.obj: net.c tcdll.h
    $(CC) $(COPT) net.c

$(RESFILE): $(RCFILE)
    $(RC) /l 0x409 /fo $(RESFILE) $(RCFILE)

