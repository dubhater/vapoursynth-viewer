Description
===========

VSViewer is a simple program for editing and previewing VapourSynth scripts.

Screenshot: http://i.imgur.com/l7eyPZj.png


Usage
=====

Hit F5 to preview. Move around in the preview window using left, right, page down, page up, home, end. Press g to go to a specific frame.


Compilation
===========

In Linux::

   qmake
   make

Make sure qmake is from Qt4.

VapourSynth revision R20 or newer needed.


Notes
=====

Something leaks, maybe 1 megabyte every time a script is (re)loaded.
