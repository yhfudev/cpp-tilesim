TileSim

This software is designed for the simulation of supertiles described in the Schweller's paper "Staged Self-Assembly: Nanomanufacture of Arbitrary Shape with O(1) Glues" (http://www.cs.panam.edu/~shwellerr/papers/NaturalComputing.pdf).

Here is the simple description of the software:
1. Functions:
  1) simulate the conbination of tiles & supertiles
  2) Load the initial data of the tiles & supertiles from XML files
  3) show all of the supertiles. Use the button in the toolbox or use the keys: '<' or '>'
  4) Animate the processing of simulation
 2. Internal:
   1) use OpenGL as display library
   2) use XML format to store the data, and it should be portable for other software to parse.
 3. Functions to be implemented:
   1) show the supertiles in 3D.
   2) edit the supertiles & tiles & glues

Usage:

1. Load & Run the simulation
Click the right button of the mouse and select the item "Load & Run"->"testdata1a".
The software will read the data from file "test/testdata1a.xml"

2. Check the result

Use the keys in the keyboard, each press of the '<' or '>' will display the pre-item or next item in the supertile list.

The left-bottom of the window shows the information of current supertile. Such as:

TEMP=2; PAGE= 10/25; CNT=99

TEMP is the temperature; PAGE is the index of the supertile; CNT is the number of the supertile.

The right-bottom of the windw shows the list of gule stregth. Glue is mark in colores in the tiles.

the right of the window shows all of the tiles that used in the simulation.

3. Check the saved result
The result of the simulation will be saved automatically when you select the menu item "Load & Run"->"testdata1a". The saved file will be named by "test/testdata1a_output.xml".

When you select the menu item from the "Load Result"->"testdata1a", the data will be read to the memory from file "test/testdata1a_output.xml"

You can also us the key '<' and/or '>' to browser all of the supertiles which are created during the simulation.


4. Questions
You could contact me if you have any comments and questiones.

My email: yhfudev@gmail.com

Thank you.

Yunhui Fu
