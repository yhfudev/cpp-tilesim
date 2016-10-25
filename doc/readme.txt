Supertile Simulation System

This software is a simulator for self-assembly of DNA, where researchers can inspect the actions of different tile sets running in three types of models: abstract Tile Assembly Model (aTAM), kinetic Tile Assembly Model (kTAM) and two handed Tile Assembly Model(2hTAM). This software supports the simulation both in 2D and 3D, rendering in OpenGL. There are also some tile set creators and convertors, such as the \emph{squarecreator} which create some types of square shape tile sets, \emph{convzzg2s}\cite{Yunhui09temp1} which convert arbitrary zig-zag tile sets at temperature $\tau=2$ to tile sets at temperature $\tau=1$.

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
   1) edit the supertiles & tiles & glues

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

4. some notes:
 1) if you use the flg_single_base_mode, please make sure that the if the xml data file contains some supertiles which contain many tiles in it may cause the supertiles will never be attached to the last one.

5. the libraries used in the project
 1) igraph
    http://igraph.sourceforge.net/
 2) pslib or GL2PS
    http://pslib.sourceforge.net/
    http://www.geuz.org/gl2ps/
 3) libxml2
 4) libdbi
    http://libdbi.sourceforge.net/

6. Compile in Debian systems.
  1) Install igraph:
     *) add two lines in your apt configure file: /etc/apt/source.list:
     deb http://cneurocvs.rmki.kfki.hu  /packages/binary/
     deb-src http://cneurocvs.rmki.kfki.hu /packages/source/
     *) sudo apt-get update
     *) sudo apt-get install libigraph libigraph-dev
  2) Install OpenGL glut:
     sudo apt-get install freeglut3-dev
  3) Install wxWidget:
     sudo apt-get install libwxgtk2.8-dev libwxsmithlib0-dev libwxsvg-dev libboost-dev
  4) Install libxml2:
     sudo apt-get install libxml2
  5) Compile the software
    ./configure && make

7. TODO
  1) add parameter to convazzg2s:  the times of duplicated '0' tileset;
  2) add parameter to tilesim_atam(): use rand() or sequence to get the next tile to be test.

8. Questions
You could contact me if you have any comments and questiones.

My email: yfu@broncs.utpa.edu

Thank you.

Yunhui Fu
