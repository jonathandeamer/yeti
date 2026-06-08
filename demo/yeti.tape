# VHS tape for the yeti gameplay GIF; see https://github.com/charmbracelet/vhs
Output demo/yeti.gif

Require bash

Set Shell "bash"
Set FontFamily "Menlo"
Set FontSize 20
Set Width 1200
Set Height 760
Set Theme "Snazzy"
Set Framerate 24
Set Padding 16
Set Margin 20
Set MarginFill "#101319"
Set BorderRadius 8
Set WindowBar Colorful
Set TypingSpeed 0 ms
Set WaitTimeout 20 s

Hide
Type "clear && ./yeti"
Enter
Show
Wait+Screen /DIST/
Sleep 700 ms

Right@90 ms 8
Sleep 250 ms
Left@90 ms 13
Sleep 250 ms
Right@80 ms 17
Sleep 350 ms
Left@80 ms 14
Sleep 500 ms
Right@70 ms 10
Sleep 900 ms

# Return to the centre and let the mountain win once.
Left@80 ms 9
Sleep 4 s
Wait+Screen /PRESS R TO RESTART/
Sleep 900 ms
Type "r"
Sleep 500 ms

Right@80 ms 9
Sleep 350 ms
Left@80 ms 12
Sleep 350 ms
Right@70 ms 15
Sleep 1.5 s

Hide
Type "q"
