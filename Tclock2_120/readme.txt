Tclock2 v1.2.0
Aug 3, 2003
Two_toNe

Tclock2 is the ultimate Windows Taskbar and Desktop enhancement tool. Allowing you to customize the clock text, flatten various parts, skin various parts and customize the desktop as well. With so many different options, the tasbbar can now be 'Your Taskbar', not just another windows taskbar.

This started out as a simple bug fix and upgrade for TClock. Since then with all the additions and changes made to it, it has become more than an upgrade. It's now its own beast. It no longer is a TClock drop-in replacement. Tclock2 stores all it's settings in a different place in the registry and has a completely different settings setup. I won't go over the settings, since the settings dialog is pretty much self-explanatory.

Commandline Parameters:
-------------------------------
/exit = closes Tclock2
/prop = displays the settings dialog
/refresh = refreshes Tclock2 without saving anything to the registry or ini file

And speaking of ini files, you can now change Tclock2 schemes by simply replacing the Tclock2.ini file in the Tclock2 directory. To get a ini file with all your current settings, just simply create a empty file in the Tclock2 directory called Tclock2.ini. Now exit Tclock2 and restart it and the empty file will now be filled with all your current settings.


Startbutton Skinning:
------------------------
Using the already built-in power of positioning, Tclock2 can skin the startbutton up to a maximum size of 160x160 pixels. The bitmap used to store the startbutton skin contains three images. The images are stacked ontop of one another. The first image is the normal state, second image is the mouseover state and third image is the mousedown state. The bitmap must contain all three of the states and must be in BMP format. Transparency is supported, use RGB(255,0,255) or hex FF00FF for transparent areas. I've included an example of what the startbutton image should look like, its called startbutton_skin.bmp.


Clock & Tray Skinning:
------------------
Clock & Tray skinning is quite a bit different than the startbutton skin, this bitmap only contains one image. When the clock resizes itself, it uses edge scaling to resize the clock skin. Meaning, you select a top, bottom, left and right corners that don't get stretched when the clock resizes. The edges in between those corners and the middle then get stretched to fit the clock. At the moment only stretching is supported. Transparency is supported, use RGB(255,0,255) or hex FF00FF for transparent areas.


Startmenu Banner Skinning:
---------------------------------
The startmenu banner skinning has always been in TClock, but now it's alot more flexible. Now you can choose a top and bottom edge of the bitmap that won't get tiled. When painting, those edges that you selected will be painted at the top and bottom of the startmenu, painting the middle in between those. Also, the menu bitmap can now be stretched too.Transparency is supported, use RGB(255,0,255) or hex FF00FF for transparent areas.


Begin Changes: 4 August 2003:
-----------
(CLOCK/TRAY)
bugfix:		tray painting should work correctly now (ME/2K/XP)
bugfix:		clock should paint correctly when using Tclock2 while a visual style is running (XP)
bugfix:		gradient clock fill now works in 95 & NT4 (95/NT4)
addition:	transparency support added for clock/tray skins (all)
addition:	added tons more clock/tooltip display formatting, source taken from TClockch (all)
addition:	multiline clock tooltip is now possible, uses same formatting as clock (all)
addition:	added formatting text file, listing all available options (all)
addition:	clock text can now be rotated left or right, usefull with side positioned taskbar (all)
addition:	horizontal position of clock text can now be adjusted (all)

(START BUTTON)
addition:	transparency support added for start button bitmap & skins (all)
addition:	start button can now use large icon (all)

(START MENU)
bugfix:		rewrote the startmenu detection code, should work much better now (98/ME/2K/XP)
bugfix:		startmenu banner painting should now work correctly (98/ME/2K/XP)
addition:	transparency support added for startmenu banner skins (all)
addition:	startmenu banner can now be hidden when startmenu is veiwed in large icon mode (98/ME/2K/XP)
addition:	startmenu banner can now be visible when startmenu is veiwed in small icon mode (98/ME/2K/XP)

(TASKBAR)
addition:	taskbar can now be made completely clear, ADVANCED OPTION - USE WITH CAUTION (XP)

(DESKTOP)
bugfix:		fixed a few minor problems when using an alternate right click app (all)
addition:	desktop icon text can now be hidden (all)

(MISC)
bugfix:		fixed versioning errors some people were getting (all)
bugfix:		included the manifest file so the properties dialog will use current visual style (XP)
addition:	arranged and orginized the property dialog a little better (all)
addition:	calendar can now be closed when it loses focus (all)
addition:	Tclock2 menu can now be made compact (all)
-----------
End Changes: 4 August 2003:


1 Feb 2003:
MAJOR BUGFIX:	Fixed clock updating (95/98/ME)
bugfix:		small icons on desktop no longer chrashes explorer when the system colors are changed, thanks to shinter for helping me find this one (2K/XP)
bugfix:		desktop icon text color now sets and saves itself correctly (all)
bugfix:		desktop icon size will no longer revert back to 32x32 when settings are applied (all)
bugfix:		desktop icon text is set back to correct color when Tclock2 is ended (all)
bugfix:		right clicking on a desktop icon brings up the default menu when alt right click app is set (all)
addition:	right clicking on a desktop icon can be optionally set to launch the alt right click app (all)
bugfix:		color combos in the property dialog should now work correctly (all)
bugfix:		task buttons are refreshed correctly when they are added, source provided by Marco (2K)
addition:	task buttons can now have separators between them, source and idea provided by Marco (95/98/ME/NT4/2K)
addition:	calender can now be opened with a mouse click action, requested by Sycore (all)
bugfix:		calender no longer shows up in the taskbar (all)
change:		property dialog now uses an 8 point font, requested by Steve Diabo (all)
bugfix:		Tclock2 can now be set to not start up when windows starts (all)
bugfix:		task switch icons only now works in winXP (XP)
bugfix:		about tab in property dialog now shows the correct version number (all)
addition:	added .lnk file extension to the alt desktop right click app open dialog (all)
addition:	added .icl file extension to the start button icon open dialog (all)
bugfix:		startmenu banner skinning should be fixed in win2K (2K)
addition:	when using Tclock2 with XP styles, property dialog uses the styles (XP)
change:		cleaned up property dialog a little bit (all)

20 Dec 2002:
bugfix:		startmenu banner skinning now works in WinXP
bugfix:		mouseover for flat and skinned start button is now faster
bugfix:		transparency for WinXP startpanel now works
addition:	taskbar can now be made flat (removes edge border)
bugfix:		grippers are set back to the correct color when Tclock2 exits
addition:	tray can now be skinned with edge scaling in WinME, Win2K & WinXP
addition:	desktop is now hooked
bugfix:		all flickering is gone for transparent desktop icon text background
addition:	desktop icon text color can now be changed
bugfix:	     	small desktop icons now work correctly in all Window versions
addition:	an alternate program can be executed with the desktop right click ( ctrl + right click brings up the default 		menu)
addition:	start button font can now be changed (size, bold & italic can be set also)
addition:	start button font color can now be changed
bugfix:		got rid of alot of timers, now uses more Windows API
bugfix:		Tclock2 will now terminate trayserver.exe if objectbar isn't running
addition:	the settings dialog has been enlarged
change:		Tclock2 now has a new icon

20 July 2002:
bugfix:		tasks are resized correctly when items are closed
bugfix:		flat task switch works in WinXP
bugfix:		start button changing works in WinXP
addition:	two new styles of start button changing (skinned & custom color)
bugfix:		fill tray works in WinXP
addition:	clock can now be skinned with edge scaling in all Win versions
addition:	rebar grippers can be hidden in all Win versions
addition:	start menu banner can now use edge scaling to preserve the top and bottom edges
addition:	start menu banner can now be stretched
addition:	desktop icons can now use small icon view
addition:	task switch can now just show the icons only (see alpha.txt for problems with this)
addition:	Tclock2 can be refreshed without saving its settings
addition:	Tclock2 has it's own built-in calender (no custom settings yet)
remove:		deskcal.exe support is no longer in Tclock2
bugfix:		can now set the tranparency of the startmenu from the settings dialog
bonus:		the build sizes for the exe and dll are smaller (thanks to Visual Studio 6)
addition:	the settings dialog has been completely redone, settings are organized better


Credits:
---------
Obviously the biggest credit goes to Kazubon for creating TClock and for releasing the source code to it.
Next to all the customizers at Virtual Plastic and the WinMod Forums, you guys rock :)
Lastly, to my wife for putting up with my computer addiction


Contact:
----------
Email:	tclock2@yahoo.com
Web Site:  http://home.inreach.com/2ton/tclock2/tclock2.htm


