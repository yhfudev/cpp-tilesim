/******************************************************************************
 * Name:        convktam.c
 * Purpose:     convert parity system with snake error correction tile sets.
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

// convert to ktam error correction model
// such as snake, proof-reading etc.

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#include "pfutils.h"
#include "zzinfo.h"
#include "grouptiles.h"

// see H. Chen, A. Goel, "Error Free Self-Assembly using Error Prone Tiles"
// the size of the square: 2k * 2k
int
snake_tile_2k_2east (zigzag_form_t *pzzf, size_t k, tile_t *ptile, glue_t *pglue, size_t num_glue)
{
    tile_t tile;
    char glue_n[50];
    char glue_e[50];
    char glue_s[50];
    char glue_w[50];
    char tile_name[100];
    size_t strength_n;
    size_t strength_e;
    size_t strength_s;
    size_t strength_w;
    char *pg_n;
    char *pg_e;
    char *pg_s;
    char *pg_w;
    size_t maxsize;
    size_t i;
    size_t j;

    maxsize = 2 * k;
    // i is the line, j is the column
    for (i = 0; i < maxsize; i ++) {
        for (j = 0; j < maxsize; j ++) {
            strength_n = 0;
            strength_e = 0;
            strength_s = 0;
            strength_w = 0;
            // get the uniform glue name
            // normal
            snprintf (glue_n, sizeof (glue_n) - 1, "%s_%d,%d_gn", ptile->name, i, j);
            strength_n = 1;
            snprintf (glue_e, sizeof (glue_e) - 1, "%s_%d,%d_ge", ptile->name, i, j);
            strength_e = 1;
            if (0 < j) {
                snprintf (glue_w, sizeof (glue_w) - 1, "%s_%d,%d_ge", ptile->name, i, j - 1);
                strength_w = 1;
            }
            if (0 < i) {
                snprintf (glue_s, sizeof (glue_s) - 1, "%s_%d,%d_gn", ptile->name, i - 1, j);
                strength_s = 1;
            }
            // internal, rules
            if (i == 0) {
                // 1. T_{1,2i-1}: E=0; T_{1,2i}: W=0
                if (j + 2 < maxsize) {
                    if (j % 2 == 0) {
                        // T_{1,2i-1}
                    strength_e = 0;
                    } else {
                        // T_{1,2i}
                    strength_w = 0;
                    }
                }
            } else if (i % 2 == 1) {
                // 4. T_{2i,2i-1}: E=2; T_{2i,2i}: W=2
                if (j + 1 == i) {
                    // T_{2i,2i-1}
                    strength_e = 2;
                } else if (j == i) {
                    // T_{2i,2i}
                    strength_w = 2;
                }
            }
            if (j == 0) {
                // 2. T_{2i,1}: N=0; T_{2i+1,1}: S=0
                if ((i > 0) && (i + 1 < maxsize)) {
                    if (i % 2 == 0) {
                        // T_{2i+1,1}
                        strength_s = 0;
                    } else {
                        // T_{2i,1}
                        strength_n = 0;
                    }
                }
            } else if (j % 2 == 0) {
                assert (j > 0);
                // 3. T_{2i,2i+1}: N=2; T_{2i+1,2i+1}: S=2
                if (i + 1 == j) {
                    // T_{2i,2i+1}
                    strength_n = 2;
                } else if (i == j) {
                    // T_{2i+1,2i+1}
                    strength_s = 2;
                }
            }
            if (i + 3 == maxsize) {
                // 5. T_{2k-2,2k-1}: E=0; T_{2k-2,2k}: W=0
                if (j == i + 1) {
                    // T_{2k-2,2k-1}
                    strength_e = 0;
                } else if (j == i + 2) {
                    // T_{2k-2,2k}
                    strength_w = 0;
                }
            }
            if (j + 1 == maxsize) {
                // 6. T_{2k-2,2k}: N=2; T_{2k-1,2k}: S=2
                if (i + 2 == j) {
                    // T_{2k-2,2k}
                    strength_n = 2;
                } else if (i + 1 == j) {
                    // T_{2k-1,2k}
                    strength_s = 2;
                }
            }
            // side
            if (j < 1) {
                snprintf (glue_w, sizeof (glue_w) - 1, "%s_%d", ptile->gW, i);
                strength_w = gluearr_name2val (pglue, num_glue, ptile->gW);;
            } else if (j + 1 >= maxsize) {
                snprintf (glue_e, sizeof (glue_e) - 1, "%s_%d", ptile->gE, i);
                strength_e = gluearr_name2val (pglue, num_glue, ptile->gE);;
            }
            if (i < 1) {
                snprintf (glue_s, sizeof (glue_s) - 1, "%s_%d", ptile->gS, j);
                strength_s = gluearr_name2val (pglue, num_glue, ptile->gS);;
            } else if (i + 1 >= maxsize) {
                snprintf (glue_n, sizeof (glue_n) - 1, "%s_%d", ptile->gN, j);
                strength_n = gluearr_name2val (pglue, num_glue, ptile->gN);;
            }
            pg_n = ((strength_n == 0)?NULL:glue_n);
            pg_e = ((strength_e == 0)?NULL:glue_e);
            pg_s = ((strength_s == 0)?NULL:glue_s);
            pg_w = ((strength_w == 0)?NULL:glue_w);
            // add glues
            if (NULL != pg_n) {
                zzf_feed_glue (pzzf, glue_n, strength_n);
            }
            if (NULL != pg_e) {
                zzf_feed_glue (pzzf, glue_e, strength_e);
            }
            if (NULL != pg_s) {
                zzf_feed_glue (pzzf, glue_s, strength_s);
            }
            if (NULL != pg_w) {
                zzf_feed_glue (pzzf, glue_w, strength_w);
            }
            // add tile
            snprintf (tile_name, sizeof (tile_name) - 1, "%s_%d,%d", ptile->name, i, j);
            tile.gN = pg_n;
            tile.gE = pg_e;
            tile.gS = pg_s;
            tile.gW = pg_w;
            tile.name = tile_name;
            tile.label = NULL;
            tile.type = TILETYPE_NONE;
            zzf_feed_tile (pzzf, &tile);
        }
    }
}

int
expand_snake_side_we (zigzag_form_t *pzzf, size_t k, tile_t *ptile, glue_t *pglue, size_t num_glue)
{
    tile_t tile;
    char *pg_n;
    char *pg_e;
    char *pg_s;
    char *pg_w;
    char glue_n[50];
    char glue_e[50];
    char glue_s[50];
    char glue_w[50];
    char tile_name[100];
    size_t strength_n;
    size_t strength_e;
    size_t strength_s;
    size_t strength_w;
    size_t maxsize = 2 * k;
    size_t i;

    memset (tile_name, 0, sizeof (tile_name));

    for (i = 0; i < maxsize; i ++) {
        // normal
        snprintf (glue_n, sizeof (glue_n) - 1, "%s_%d", ptile->gN, i);
        strength_n = gluearr_name2val (pglue, num_glue, ptile->gN);
        snprintf (glue_s, sizeof (glue_s) - 1, "%s_%d", ptile->gS, i);
        strength_s = gluearr_name2val (pglue, num_glue, ptile->gS);
        snprintf (glue_w, sizeof (glue_w) - 1, "%s_%d_inter", ptile->name, i);
        strength_w = 2;
        snprintf (glue_e, sizeof (glue_e) - 1, "%s_%d_inter", ptile->name, i + 1);
        strength_e = 2;
        // sides
        if (i == 0) {
            snprintf (glue_w, sizeof (glue_w) - 1, "%s", ptile->gW);
            strength_w = gluearr_name2val (pglue, num_glue, ptile->gW);
        } else if (i + 1 >= maxsize) {
            snprintf (glue_e, sizeof (glue_e) - 1, "%s", ptile->gE);
            strength_e = gluearr_name2val (pglue, num_glue, ptile->gE);
        }
        pg_n = ((strength_n == 0)?NULL:glue_n);
        pg_e = ((strength_e == 0)?NULL:glue_e);
        pg_s = ((strength_s == 0)?NULL:glue_s);
        pg_w = ((strength_w == 0)?NULL:glue_w);
        // add glues
        if (0 < strength_n) {
            zzf_feed_glue (pzzf, glue_n, strength_n);
        }
        if (0 < strength_e) {
            zzf_feed_glue (pzzf, glue_e, strength_e);
        }
        if (0 < strength_s) {
            zzf_feed_glue (pzzf, glue_s, strength_s);
        }
        if (0 < strength_w) {
            zzf_feed_glue (pzzf, glue_w, strength_w);
        }
        // add tile
        strcpy (tile_name, "");
        if (NULL != ptile->name) {
            snprintf (tile_name, sizeof (tile_name) - 1, "%s_%d", ptile->name, i);
            if (0 == strcmp ("SEED", ptile->name)) {
                if (((i == 0) && (gluearr_name2val (pglue, num_glue, ptile->gW) < 1))
                    || ((i + 1 >= maxsize) && (gluearr_name2val (pglue, num_glue, ptile->gE) < 1))
                    ) {
                    strcpy (tile_name, "SEED");
                }
            }
        }
        tile.gN = pg_n;
        tile.gE = pg_e;
        tile.gS = pg_s;
        tile.gW = pg_w;
        tile.name = tile_name;
        tile.label = NULL;
        tile.type = TILETYPE_NONE;
        zzf_feed_tile (pzzf, &tile);
    }
}

int
expand_snake_side_ns (zigzag_form_t *pzzf, size_t k, tile_t *ptile, glue_t *pglue, size_t num_glue)
{
    tile_t tile;
    char *pg_n;
    char *pg_e;
    char *pg_s;
    char *pg_w;
    char glue_n[50];
    char glue_e[50];
    char glue_s[50];
    char glue_w[50];
    char tile_name[100];
    size_t strength_n;
    size_t strength_e;
    size_t strength_s;
    size_t strength_w;
    size_t maxsize = 2 * k;
    size_t i;

    memset (tile_name, 0, sizeof (tile_name));

    for (i = 0; i < maxsize; i ++) {
        // normal
        snprintf (glue_w, sizeof (glue_w) - 1, "%s_%d", ptile->gW, i);
        strength_w = gluearr_name2val (pglue, num_glue, ptile->gW);
        snprintf (glue_e, sizeof (glue_e) - 1, "%s_%d", ptile->gE, i);
        strength_e = gluearr_name2val (pglue, num_glue, ptile->gE);
        snprintf (glue_s, sizeof (glue_s) - 1, "%s_%d_inter", ptile->name, i);
        strength_s = 2;
        snprintf (glue_n, sizeof (glue_n) - 1, "%s_%d_inter", ptile->name, i + 1);
        strength_n = 2;
        // sides
        if (i == 0) {
            snprintf (glue_s, sizeof (glue_s) - 1, "%s", ptile->gS);
            strength_s = gluearr_name2val (pglue, num_glue, ptile->gS);
        } else if (i + 1 >= maxsize) {
            snprintf (glue_n, sizeof (glue_n) - 1, "%s", ptile->gN);
            strength_n = gluearr_name2val (pglue, num_glue, ptile->gN);
        }
        pg_n = ((strength_n == 0)?NULL:glue_n);
        pg_e = ((strength_e == 0)?NULL:glue_e);
        pg_s = ((strength_s == 0)?NULL:glue_s);
        pg_w = ((strength_w == 0)?NULL:glue_w);
        // add glues
        if (0 < strength_n) {
            zzf_feed_glue (pzzf, glue_n, strength_n);
        }
        if (0 < strength_e) {
            zzf_feed_glue (pzzf, glue_e, strength_e);
        }
        if (0 < strength_s) {
            zzf_feed_glue (pzzf, glue_s, strength_s);
        }
        if (0 < strength_w) {
            zzf_feed_glue (pzzf, glue_w, strength_w);
        }
        // add tile
        strcpy (tile_name, "");
        if (NULL != ptile->name) {
            snprintf (tile_name, sizeof (tile_name) - 1, "%s_%d", ptile->name, i);
            if (0 == strcmp ("SEED", ptile->name)) {
                if (((i == 0) && (gluearr_name2val (pglue, num_glue, ptile->gS) < 1))
                    || ((i + 1 >= maxsize) && (gluearr_name2val (pglue, num_glue, ptile->gN) < 1))
                    ) {
                    strcpy (tile_name, "SEED");
                }
            }
        }
        tile.gN = pg_n;
        tile.gE = pg_e;
        tile.gS = pg_s;
        tile.gW = pg_w;
        tile.name = tile_name;
        tile.label = NULL;
        tile.type = TILETYPE_NONE;
        zzf_feed_tile (pzzf, &tile);
    }
}

// create snake 2k*2k
int
zzi_2d2t_2snake (zigzag_info_t *pzzi_snake, zigzag_info_t *pzzi_origin, size_t k)
{
    size_t i;
    tile_t tile;
    zigzag_form_t zzf;
    zzf_init (&zzf, pzzi_snake);
    zzf_feed_glue (&zzf, "", 0);
    for (i = 0; i < pzzi_origin->sz_tiles; i ++) {
        switch (pzzi_origin->tiles[i].type) {
        case TILETYPE_DEE1:
            snake_tile_2k_2east (&zzf, k, &(pzzi_origin->tiles[i]), pzzi_origin->glues, pzzi_origin->sz_glues);
            break;
        case TILETYPE_SWN2:
            // the same
            if (pzzi_origin->tiles[i].gN) {
                zzf_feed_glue (&zzf, pzzi_origin->tiles[i].gN, 2);
            }
            if (pzzi_origin->tiles[i].gE) {
                zzf_feed_glue (&zzf, pzzi_origin->tiles[i].gE, 2);
            }
            tile.gN = pzzi_origin->tiles[i].gN;
            tile.gE = pzzi_origin->tiles[i].gE;
            tile.gS = NULL;
            tile.gW = NULL;
            tile.name = pzzi_origin->tiles[i].name;
            tile.label = pzzi_origin->tiles[i].label;
            tile.type = TILETYPE_SWN2;
            zzf_feed_tile (&zzf, &tile);
            break;
        case TILETYPE_TWE1:
            expand_snake_side_ns (&zzf, k, &(pzzi_origin->tiles[i]), pzzi_origin->glues, pzzi_origin->sz_glues);
            break;
        case TILETYPE_FW:
            expand_snake_side_we (&zzf, k, &(pzzi_origin->tiles[i]), pzzi_origin->glues, pzzi_origin->sz_glues);
            break;
        }
    }
    zzf_feed_end (&zzf);
    zzf_clear (&zzf);
}

int
gen_parity_system (zigzag_info_t *pzzi, const char *input_string)
{
    size_t i;
    char flg_01 = 0;
    char buf[50];
    char buf2[50];
    char buf3[50];
    tile_t tile;
    zigzag_form_t zzf;
    zzf_init (&zzf, pzzi);
    zzf_feed_glue (&zzf, "", 0);
    zzf_feed_glue (&zzf, "stop", 1);
    zzf_feed_glue (&zzf, "0", 1);
    zzf_feed_glue (&zzf, "1", 1);

    memset (&tile, 0, sizeof (tile));

    // generate the stop tile 0
    tile.gN = "";
    tile.gE = "";
    tile.gS = "stop";
    tile.gW = "0";
    tile.name = "ST0";
    tile.label = "Stop 0";
    tile.type = TILETYPE_DEE1;
    zzf_feed_tile (&zzf, &tile);
    // generate the stop tile 1
    tile.gN = "";
    tile.gE = "";
    tile.gS = "stop";
    tile.gW = "1";
    tile.name = "ST1";
    tile.label = "Stop 1";
    tile.type = TILETYPE_DEE1;
    zzf_feed_tile (&zzf, &tile);

    // generate the seed tile
    snprintf (buf, sizeof (buf) - 1, "g%02x", strlen (input_string));
    zzf_feed_glue (&zzf, buf, 2);
    tile.gN = "stop";
    tile.gE = NULL;
    tile.gS = NULL;
    tile.gW = buf;
    tile.name = "SEED";
    tile.label = NULL;
    tile.type = TILETYPE_FW;
    zzf_feed_tile (&zzf, &tile);

    // generate seed bar
    for (i = 0; i < strlen (input_string); i ++) {
        if ('1' == input_string[i]) {
            flg_01 |= 0x2;
            // place '1'
            tile.gN = "1";
        } else if ('0' == input_string[i]) {
            flg_01 |= 0x1;
            tile.gN = "0";
        } else {
            break;
        }
        zzf_feed_glue (&zzf, tile.gN, 1);

        snprintf (buf, sizeof (buf) - 1, "g%02x", i);
        snprintf (buf2, sizeof (buf2) - 1, "g%02x", i + 1);
        snprintf (buf3, sizeof (buf3) - 1, "S%02x", i);
        zzf_feed_glue (&zzf, buf, 2);
        zzf_feed_glue (&zzf, buf2, 2);
        tile.gE = buf2;
        tile.gS = NULL;
        tile.gW = buf;
        tile.name = buf3;
        tile.label = NULL;
        tile.type = TILETYPE_FW;
        zzf_feed_tile (&zzf, &tile);
    }
    if (i + 1 < strlen (input_string)) {
        // error
        return -1;
    }

    // generate the tile on the corner
    //{"gc", "g4",   "",   "",   "Sc", "", TILETYPE_SWN2},
    tile.gN = "gc";
    tile.gE = "g00";
    tile.gS = NULL;
    tile.gW = NULL;
    tile.name = "Sc";
    tile.label = NULL;
    tile.type = TILETYPE_SWN2;
    zzf_feed_glue (&zzf, tile.gN, 2);
    zzf_feed_glue (&zzf, tile.gE, 2);
    zzf_feed_tile (&zzf, &tile);

    // generate the start tile
    //{  "",  "0", "gc",   "",   "SS", "", TILETYPE_TWE1},
    tile.gN = NULL;
    tile.gE = "0";
    tile.gS = "gc";
    tile.gW = NULL;
    tile.name = "SS";
    tile.label = NULL;
    tile.type = TILETYPE_TWE1;
    zzf_feed_glue (&zzf, tile.gE, 1);
    zzf_feed_glue (&zzf, tile.gS, 2);
    zzf_feed_tile (&zzf, &tile);

    // generate the '00', '01', '10', '11' tiles
    //{  "",  "1",  "0",  "1",   "10", "", TILETYPE_DEE1},
    //{  "",  "1",  "1",  "0",   "01", "", TILETYPE_DEE1},
    //{  "",  "0",  "0",  "0",   "00", "", TILETYPE_DEE1},
    //{  "",  "0",  "1",  "1",   "11", "", TILETYPE_DEE1},
    zzf_feed_glue (&zzf, "0", 1);
    zzf_feed_glue (&zzf, "1", 1);
    if (flg_01 & 0x01) {
        // '0'
        tile.gN = NULL;
        tile.gE = "0";
        tile.gS = "0";
        tile.gW = "0";
        tile.name = "00";
        tile.label = NULL;
        tile.type = TILETYPE_DEE1;
        zzf_feed_tile (&zzf, &tile);

        tile.gN = NULL;
        tile.gE = "1";
        tile.gS = "0";
        tile.gW = "1";
        tile.name = "10";
        tile.label = NULL;
        tile.type = TILETYPE_DEE1;
        zzf_feed_tile (&zzf, &tile);
    }
    if (flg_01 & 0x02) {
        // '1'
        tile.gN = NULL;
        tile.gE = "1";
        tile.gS = "1";
        tile.gW = "0";
        tile.name = "01";
        tile.label = NULL;
        tile.type = TILETYPE_DEE1;
        zzf_feed_tile (&zzf, &tile);

        tile.gN = NULL;
        tile.gE = "0";
        tile.gS = "1";
        tile.gW = "1";
        tile.name = "11";
        tile.label = NULL;
        tile.type = TILETYPE_DEE1;
        zzf_feed_tile (&zzf, &tile);
    }

    zzf_feed_end (&zzf);
    zzf_clear (&zzf);
    return 0;
}

static void
version (FILE *out_stream)
{
    fprintf( out_stream, "Snake tile set convertor version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    fprintf( out_stream, "Copyright 2009 Yunhui Fu (yhfudev@gmail.com)\n\n" );
}

static void
help (FILE *out_stream, const char *progname)
{
    static const char *help_msg[] = {
        "Command line version of Snake Tile Set Convertor, replace each tile with a snake tile set 2k*2k", 
        "",
        "-i --input <file name>     the input file for convertion",
        "-o --outprefix <prefix>    specify the prefix of the output file",
        "-k --k <value>             specify the k parameter, create 2k*2k snake tile sets",
        "-w --xgrow                 also output to winfree's xgrow data file",
        "-e --example               output parity system (-s can be used with this parameter)",
        "-s --string                the input string of the parity system (use with -e)",

        "-V --version               display version information",
        "   --help                  display this help",
        "-v --verbose               be verbose",
        0 };
   const char **p = help_msg;

   fprintf (out_stream, "Usage: %s [options]\n", progname);
   while (*p) fprintf (out_stream, "%s\n", *p++);
}

int
main (int argc, char *argv[])
{
    int ret = 0;
    FILE *fpout = stdout;
    const char *fn_in = "";
    const char *fn_out = NULL;
    const char *input_string = "1111";
    char flg_example = 0;
    char flg_xgrow = 0;
    size_t k = 5;
    size_t maxsteps = 5000;
    char namebuf[100];
    zigzag_info_t zzi;
    zigzag_info_t zzi_snake;

    int c;
    struct option longopts[]  = {
        { "example",    1, 0, 'e' },
        { "string",     1, 0, 's' },
        { "k",          1, 0, 'k' },
        { "xgrow",      1, 0, 'w' },
        { "input",      1, 0, 'i' },
        { "outprefix",  1, 0, 'o' },
        { "version",    0, 0, 'V' },
        { "help",       0, 0, 501 },
        { "verbose",    0, 0, 'v' },
        { 0,            0, 0,  0  },
    };

    while ((c = getopt_long( argc, argv, "i:o:k:s:ewVLv", longopts, NULL )) != EOF) {
        switch (c) {
        case 'i': fn_in = optarg; break;
        case 'o': fn_out = optarg; break;
        case 'k': k = atoi (optarg); break;
        case 's': input_string = optarg; break;
        case 'e': flg_example = 1; break;
        case 'w': flg_xgrow = 1; break;
        case 'V': version (stdout); exit(1); break;
        case 'v': break;
        default:
        case 501: help (stdout, argv[0]); exit(1); break;
        }
    }
    //extern void test_revert (void);test_revert ();
    printf ("file in: '%s'; file out: '%s'\n", fn_in, fn_out);
    printf ("k=%d; input='%s'\n", k, input_string);
    zzi_init (&zzi);
    zzi_init (&zzi_snake);

    if (flg_example) {
        gen_parity_system (&zzi, input_string);
        zzi_output_c (&zzi, "snaketest", stderr);
        fn_out = "parity_example";
        fpout = stdout;
        if (fn_out) {
            snprintf (namebuf, sizeof (namebuf) - 1, "%s.tdp", fn_out);
            fpout = fopen (namebuf, "w");
            if (NULL != fpout) {
                fprintf (fpout, "%s.tds\nTemperature=2\nSEED 0 0 0\n", fn_out);
                fclose (fpout);
            }
            snprintf (namebuf, sizeof (namebuf) - 1, "%s.tds", fn_out);
            fpout = fopen (namebuf, "w");
            if (NULL == fpout) {
                printf ("Error in open file: %s\n", fn_out);
                exit (1);
            }
        }
        zzi_output_tds_2d2t (&zzi, fpout);
        if (NULL != fn_out) {
            fclose (fpout);
        }

        if (flg_xgrow) {
            fpout = stdout;
            if (fn_out) {
                snprintf (namebuf, sizeof (namebuf) - 1, "%s.tiles", fn_out);
                fpout = fopen (namebuf, "w");
                if (NULL == fpout) {
                    printf ("Error in open file: %s\n", fn_out);
                    exit (1);
                }
            }
            zzi_output_xgrow_tiles (&zzi, fpout);
            if (NULL != fn_out) {
                fclose (fpout);
            }
        }

    } else {
        if (NULL == fn_in) {
            printf ("Please specify the input file.\n");
            ret = -1;
            goto prog_end;
        }
        zzi_load_tdp (fn_in, &zzi);
        zzi_group_tiles (&zzi, "test", maxsteps);
    }
    zzi_2d2t_2snake (&zzi_snake, &zzi, k);
    zzi_group_tiles (&zzi_snake, "testsnake", maxsteps);

    fpout = stdout;
    if (fn_out) {
        snprintf (namebuf, sizeof (namebuf) - 1, "%s_snake_%d.tdp", fn_out, k);
        fpout = fopen (namebuf, "w");
        if (NULL != fpout) {
            fprintf (fpout, "%s_snake_%d.tds\nTemperature=2\nSEED 0 0 0\n", fn_out, k);
            fclose (fpout);
        }
        snprintf (namebuf, sizeof (namebuf) - 1, "%s_snake_%d.tds", fn_out, k);
        fpout = fopen (namebuf, "w");
        if (NULL == fpout) {
            printf ("Error in open file: %s\n", fn_out);
            exit (1);
        }
    }
    zzi_output_tds_2d2t (&zzi_snake, fpout);
    if (NULL != fn_out) {
        fclose (fpout);
    }

    if (flg_xgrow) {
        fpout = stdout;
        if (fn_out) {
            snprintf (namebuf, sizeof (namebuf) - 1, "%s_snake_%d.tiles", fn_out, k);
            fpout = fopen (namebuf, "w");
            if (NULL == fpout) {
                printf ("Error in open file: %s\n", fn_out);
                exit (1);
            }
        }
        zzi_output_xgrow_tiles (&zzi_snake, fpout);
        if (NULL != fn_out) {
            fclose (fpout);
        }
    }

prog_end:
    zzi_clear (&zzi);
    zzi_clear (&zzi_snake);
    return ret;
}
