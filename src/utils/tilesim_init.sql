
/*
  Use this SQL source file to initialize the database.
*/

DROP TABLE IF EXISTS tilesimbucket;
CREATE TABLE tilesimbucket (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    birth       DATETIME, /* when inport form a xml file, if the birth element is not exist in the xml, then use the create time of the file. */
    temperature INTEGER,
    rotatable   CHAR, /* 1-true, 0-false */
    name        VARCHAR(255) UNIQUE NOT NULL
);

DROP TABLE IF EXISTS glue;
CREATE TABLE glue (
    id          INTEGER,
    idb         INTEGER, /* the bucket id */
    strength    INTEGER,
    name        VARCHAR(255) /*UNIQUE NOT NULL*/
);

DROP TABLE IF EXISTS tile;
CREATE TABLE tile (
    id          INTEGER,
    idb         INTEGER, /* the bucket id */
    idgnorth    INTEGER, /* the glue id at the north */
    idgeast     INTEGER, /* the glue id at the east */
    idgsouth    INTEGER, /* the glue id at the south */
    idgwest     INTEGER, /* the glue id at the west */
    idgfront    INTEGER, /* the glue id at the front */
    idgback     INTEGER, /* the glue id at the back */
    name        VARCHAR(255) /*UNIQUE NOT NULL*/
);

DROP TABLE IF EXISTS supertile;
CREATE TABLE supertile (
    id          INTEGER,
    idb         INTEGER, /* the bucket id */
    quantity    INTEGER, /* the number of this supertile */
    name        VARCHAR(255) /*UNIQUE NOT NULL*/
);

DROP TABLE IF EXISTS tileitem;
CREATE TABLE tileitem (
    id          INTEGER,
    idb         INTEGER, /* the bucket id */
    idstile     INTEGER, /* the supertile id */
    idtile      INTEGER, /* the tile id */
    rotnum      INTEGER, /* the rotate number of the tile */
    x           INTEGER, /* position of the tile */
    y           INTEGER, /* position of the tile */
    z           INTEGER, /* position of the tile */
    name        VARCHAR(255) /*UNIQUE NOT NULL*/
);

/* the history of one supertile */
/* record the parents of a new supertile, for analysis */
/* make SURE to let the idpstbase > idpsttest, 
   because the pair of the two parent supertile is the same.
 */
DROP TABLE IF EXISTS stilehistory;
CREATE TABLE stilehistory (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    idb         INTEGER, /* the bucket id */

    idstile     INTEGER, /* the supertile id */
    times       INTEGER, /* the number of supertile that created from this history */

    idpstbase   INTEGER, /* the parent supertile id: base */
    idpsttest   INTEGER, /* the parent supertile id: test */
    rotnum      INTEGER, /* the rotate number of the test supertile */
    x           INTEGER, /* position of the test supertile */
    y           INTEGER, /* position of the test supertile */
    z           INTEGER /* position of the test supertile */
);

/* the all of the posible combine position of two supertiles */
/* The mesh test of two supertiles may consume a lot of time, so let's save all of the positions of test tile and it'll save time when read from database */
/* idpsttest <= idpstbase */
/* if the count of the item == 1 and x < 0, then the two supertiles could not combined. */
/* if the count of the item < 1, then there's no record about the combination of the two supertiles */
DROP TABLE IF EXISTS stilecombineposition;
CREATE TABLE stilecombineposition (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    idb         INTEGER, /* the bucket id */

    temperature INTEGER, /* the temperature of the state */

    idpstbase   INTEGER, /* the supertile id: base */
    idpsttest   INTEGER, /* the supertile id: test */
    rotnum      INTEGER, /* the rotate number of the test supertile */
    x           INTEGER, /* position of the test supertile */
    y           INTEGER, /* position of the test supertile */
    z           INTEGER /* position of the test supertile */
);

/* the all of the steps of supertiles simulation */
/* This table will help to playback the simulation */
/* idpsttest <= idpstbase */
DROP TABLE IF EXISTS stilestephistory;
CREATE TABLE stilestephistory (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    idb         INTEGER, /* the bucket id */

    idpstbase   INTEGER, /* the supertile id: base */
    idpsttest   INTEGER, /* the supertile id: test */
    rotnum      INTEGER, /* the rotate number of the test supertile */
    x           INTEGER, /* position of the test supertile */
    y           INTEGER, /* position of the test supertile */
    z           INTEGER /* position of the test supertile */
);

-- SELECT * from sqlite_master;
-- INSERT INTO tilesimbucket (birth, temperature, rotatable, name) VALUES (DATETIME("1977-02-24 12:00:00"), 2, 0, "6-tile-to-3x3square");
