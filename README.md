# FT8-GO (Great Outdoors)
<<<<<<< HEAD
1) See INSTALL for information on preparing a USB stick.
Summary: Create a Linux Lite 3.4 32-bit installer USB stick, ISO available at:
https://repo.linuxliteos.com/linuxlite/isos/3.4/linux-lite-3.4-32bit.iso

2) To create an FT8-GO ("Great Outdoors") installation USB stick:
Copy the files at the "Releases" page (except the source code files,
the last two) to the prepared USB stick.

3) Follow the user-installation instructions, below.

================================================================

Instructions for installing FT8-GO ("Great Outdoors") by WM8Q
=======
FT8-GO (Great Outdoors) description
===================================
>>>>>>> a73edd4d3224a9014401950364f3c5d22f86b0a2

FT8-GO creates a single-purpose laptop with optimized FT8
operation and helpful additions, recommended for
re-purposing laptops up to 12 years old.
(You cam use a laptop that's otherwise completely unusable
because of the processing demands of current operating
systems and FT8 versions).

<<<<<<< HEAD
When you're ready to go to the "Great Outdoors" for FT8,
leave your expensive new laptop at home, an old laptop
does the job.
=======
When you go to the "Great Outdoors" for FT8, leave your expensive
new laptop at home, an old laptop does the job.
>>>>>>> a73edd4d3224a9014401950364f3c5d22f86b0a2

Plus: One-click upload to Logbook of the World,
one-click downloading current QSOs before operating.

One-click POTA log generation, ready for upload
using the POTA web site.

One-click Field Day* log generation, ready for
upload to ARRL.

Manual, semi-automatic, or full automatic QSOs,
choose what suits you from minute to minute.

You can grab any FT8-GO laptop before you head out for
field operation, take the one with the best battery
charge, best USB ports, best screen, etc. and
get current "worked-before" status with one click.

Instructions for creating the FT8-GO ("Great Outdoors") installer
=================================================================

1) See INSTALL for information on preparing a USB stick.
Summary: Create a Linux Lite 3.4 32-bit installer USB stick, ISO available at:
https://repo.linuxliteos.com/linuxlite/isos/3.4/linux-lite-3.4-32bit.iso

2) To create an FT8-GO ("Great Outdoors") installation USB stick:
Copy the files (except the two source code files) at the "Releases" page to the prepared USB stick.

3) Follow the user-installation instructions, below.

FT8-GO User Installation instructions
=====================================

Before starting:
================

- Find an old slow laptop (a 'Netbook' from 2012 is perfect),
  do not use a laptop you can't do without, all of the
  information on this laptop will be erased.

- If using an old laptop, check the processor's Passmark
  benchmark score online... it must be 235 or higher.
  A solid-state disk (SSD) is highly desirable, install
  one first, if at all possible.
  
- Backup your station, call, and certificate info in TQSL
  to a USB stick (see TQSL help).
  
- Research how to make your laptop start up ("boot")
  from a USB stick.
  
- Have your Logbook of the World (LOTW) password ready.

To install FT8-GO on your laptop:
================================

<<<<<<< HEAD
- Start up your laptop with the FT8-Go USB stick.
=======
- Start up your laptop with the FT8-GO USB stick.
>>>>>>> a73edd4d3224a9014401950364f3c5d22f86b0a2

- Select "Install Linux" icon.

- Language: Select "English".

- Connect: Select  your wifi access point,
  enter your password.
  An internet connection is required for installation!

- Preparing: Continue.

- Type: "Erase disk".

- Write changes: Continue.

- Where: Select your time zone.

- Keyboard: Usually US.

- Who: Ente a short user name,
  "qrz" suggested to avoid a stuck num-lock key later.
  Password: Suggest "pass", go for simple.
  Select "Log in automatically".
  Continue.

- Wait... and ....wait...

- Complete: Pull the USB stick out,
  turn the laptop power off then on again

- Let the laptop start,
  wait for the"desktop" (icons) to appear.

- Insert the FT8-Go USB stick again,
  wait for a file viewer to appear.

- Double-click on "INSTALL_WSJTX.desktop".

- Click on "Mark as executable".

- Enter your password in the black box that appears.

- Wait for the black box to close,
  WSJT-X and TQSL are installed.

- Close the file viewer.

Setting up TQSL:
================

- Double-click on TQSL on the "desktop"

- At "Notice", click OK.

- At "Request callsign certificate, select "No".

- At "Update available" select "OK".

- At "Install new configuration", select "OK".

- At "Configuration updated", select "OK".

- Insert the USB with the TQSL backup information.

- Select "Restore station locations (etc.)".

- Navigate to the USB stick, double-click on "tqslbackup.tbk".

- Close TQSL

Setting up WSJT-X:
==================

- Double-click on "wsjtx" on the "desktop".

- Select File|Settings then the "General" tab.

- Enter your call sign, grid locator, and LOTW password.

- Select "OK", and the WSJT-X main screen is displayed.

- Select File|Download LOTW all QSOs"
  (you must still beconnected to the internet) 

- You will see "Waiting for logbook".

- Wait until the "Configuration" menu is shown at the WSJT-X main screen.

- Select File|Settings and the Radio tab, set up for your radio. 

- Select "OK" and you're ready to go!

If you're having problems with Num Lock being enabled,
run "Lite Tweaks" from the desktop menu.

Operating FT8-GO:
=================

CAUTION: When you select 'Listen' or 'Call CQ',
your transmitter will be enabled and transmit
will start and continue to operate AUTOMATICALLY!

You can operate either manually or automatically.
For manual operation, select "Tx" and work calls
as usual for WSJT-X.

You can break into automatic operation at any time
by double-clicking on a call sign, as usual.
When that QSO is done, automatic operation
will continue.

If you're activating a POTA park, select File|Settings
and the "Advanced" tab, and select "POTA".

If you're operating ARRL Field Day, select File|Settings
and the "Advanced" tab, and select "Field Day".

When you've accumulated any number of QSOs,
select "File|Upload new QSOs to LOTW".

To update your laptop with your latest QSOs
(*essential* for determining un-worked callers),
select "File|Download all QSOs from LOTW"
every time before you begin an FT8 session.

