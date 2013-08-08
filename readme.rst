Description
===========

VSViewer is a simple program for previewing VapourSynth scripts.


Usage
=====

Open a VapourSynth script and move around in it using left, right, page down, page up, home, end.


Compilation
===========

In Linux::

   qmake
   make

Make sure qmake is from Qt4.


Notes
=====

Something leaks. Maybe I don't free something that should be freed, or maybe the problem is in VapourSynth. Either way, loading a new script adds about 12 megabytes to the program's memory use.
