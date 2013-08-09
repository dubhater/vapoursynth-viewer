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

VapourSynth revision https://github.com/vapoursynth/vapoursynth/commit/314012b91982fa4342b9d4912c34aa950e47bd0c or newer needed.


Notes
=====

Something leaks, maybe 1 megabyte every time a script is (re)loaded.
