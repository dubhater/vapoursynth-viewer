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

VapourSynth revision https://github.com/vapoursynth/vapoursynth/commit/314012b91982fa4342b9d4912c34aa950e47bd0c or newer needed.


Notes
=====

Something leaks, maybe 1 megabyte every time a script is (re)loaded.
