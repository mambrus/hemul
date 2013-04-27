Chapter 0.0: HEMUL
==================

Heuristic EMUlator of Logs (and others)

Chapter 1.0: PURPOSE & DEFINITION
=================================

Hemul is a transparent emulator for re-playing recorded contents of a file.
To be able to do that, file-contet must either contain time-stamps or flags
must be given to simulate period times.

Hemul behaviour is affected either by configuration file or by input
arguments.

Chapter 1.1: BEHAVIOUR DESCRIPTION:
===================================

1 Period time (uS)
------------------
  Base period time. In case file to re-play lacks timestams, this entity is
  used as the base metronome to determine the output of one line at a time.

2 Time distribution
-------------------
  To the time above dirstrubution can be added. This comes as a paired
  argument. The first is the lowest value added to "period time", the second
  the highest

3 Chunked output
----------------
  Output is normally line oriented, but that is not always the case from
  where file originated from, where it could had came a buffer-with at a
  time. As output is almost always buffered and flushed when buffer is
  full, line-feeds are not always present when content arrive and parsing
  such a file without read buffering could lead to missed entries. By using
  this field in conjunction with either period times one can emulate the
  real cases as they occured. A good value would be the pay-load of a TCP
  package, i.e. 1024 (or a few bytes less to emulate NAT translation).
 
4 Time-stamp regexp
-------------------
  File that contain timestamps need help finding it. This is achieved with
  the use of a regular expression.

5 Time-stamp format
-------------------
  To calculate delay between two lines, timestamps are always converted
  internally into a numerical format. This is Epoc representation to be
  precise, but that doesn't mean timestamps must be prepresented in Epoc,
  just the the difference between two entries must be in seconds and
  fractions of a second. Hemul understands any timeformat produced by the
  date-command (or strftime)

