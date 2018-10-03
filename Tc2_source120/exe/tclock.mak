# ------------------------------------------
# tclock.mak            KAZUBON 1998-2001
#-------------------------------------------

EXEFILE=..\out\tclock2.exe
RCFILE=tclock.rc
RESFILE=tclock.res
RESOURCEH=..\language\resource.h

CC=cl
LINK=link
RC=rc

OBJS=main.obj propsheet.obj pagecolor.obj pageclock.obj pageformat.obj pagestartmenu.obj \
 pageformat2.obj pagealarm.obj alarmday.obj pagemouse.obj \
 pageskin.obj pagetaskbar.obj pagesntp.obj pagemisc.obj pageabout.obj \
 alarm.obj menu.obj mouse.obj deskcal.obj timer.obj pagedesktop.obj \
 sntp.obj soundselect.obj utl.obj tclock.res
LIBS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib\
 shell32.lib winmm.lib comctl32.lib ole32.lib uuid.lib version.lib\
 wsock32.lib Shlwapi.lib oleaut32.lib winspool.lib odbc32.lib odbccp32.lib 



COPT=/c /W3 /O2 /DWIN32
LOPT=/SUBSYSTEM:WINDOWS /NODEFAULTLIB /opt:NOWIN98

$(EXEFILE): $(OBJS) TC2DLL.lib
    $(LINK) $(LOPT) /OUT:$(EXEFILE) $(OBJS) TC2DLL.lib $(LIBS)

main.obj: main.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) main.c
propsheet.obj: propsheet.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) propsheet.c
pagecolor.obj: pagecolor.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagecolor.c
pageclock.obj: pageclock.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pageclock.c
pageformat.obj: pageformat.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pageformat.c
pageformat2.obj: pageformat2.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pageformat2.c
pagealarm.obj: pagealarm.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagealarm.c
alarmday.obj: alarmday.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) alarmday.c
pagemouse.obj: pagemouse.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagemouse.c
pageskin.obj: pageskin.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pageskin.c
pagestartmenu.obj: pagestartmenu.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagestartmenu.c
pagetaskbar.obj: pagetaskbar.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagetaskbar.c
pagedesktop.obj: pagedesktop.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagedesktop.c
pagesntp.obj: pagesntp.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagesntp.c
pagemisc.obj: pagemisc.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pagemisc.c
pageabout.obj: pageabout.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) pageabout.c
alarm.obj: alarm.c tclock.h
    $(CC) $(COPT) alarm.c
menu.obj: menu.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) menu.c
mouse.obj: mouse.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) mouse.c
deskcal.obj: deskcal.c tclock.h
    $(CC) $(COPT) deskcal.c
timer.obj: timer.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) timer.c
sntp.obj: sntp.c tclock.h
    $(CC) $(COPT) sntp.c
soundselect.obj: soundselect.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) soundselect.c
utl.obj: utl.c tclock.h $(RESOURCEH)
    $(CC) $(COPT) utl.c

$(RESFILE): $(RCFILE)
    $(RC) /l 0x409 /fo $(RESFILE) $(RCFILE)
