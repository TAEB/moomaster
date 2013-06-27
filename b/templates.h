#ifndef templates_h
#define templates_h

static const char *minesend_template[] = {"\
&\
&\
&\
                           ---------     -------------       -------&\
                           |.......|     |.......|...|       |.....|*&\
       ---------        ----.......-------...........|       |--...-S-&\
       |.......|        |..........................-S-      --.......|&\
       |......-------   |--...........<............|*       |.......--&\
       |..--........-----..........................|.       |.-..----&\
       --..--.-----........|.....................--|        |-..--&\
        --..--.*| -----------..................---.----------..--&\
         |...--.|    |.*S...S..............---................--&\
        ----..-----  ------------........--- ------------...--|&\
        |.........--            ----------              ---...-- -----&\
       --.....---..--                           --------  --...---...--&\
    ----..|..-- --..---------------------      --......--  ---........|&\
  ---....-----   --..|..................---    |........|    |.......--&\
  |.......|       --......................S*.  --......-|    |--..----&\
  ---.--.--        ----.................---     ------..------...--&\
    |....S*.          |...............|..|         .*S...........|&\
    ------            --------------------           -------------\
",
"\
&\
&\
&\
  -----------------------------------------------------&\
  |                                                   |&\
  |^------------- ---   ----------------------------  |&\
  | |-----   | |--- |   |               ^          |  |&\
  | ||   | --- | |---   |             <    ^          |&\
  | ||   --|     |    ---                          |  |&\
  | ||  ^  |---  |  ..|                            |  |&\
  | |----  |  |  ---.@------------------------------  |-&\
  | |   |  |  |    |...                               |&\
  | |   |  |  |-----  ------------------------------  |&\
  | -----  ---|       |-----------------------     |  |&\
  |           |---- --|                      |     |  |&\
  ------------|   | | -------------------- | -------  |     |&\
  ------------- { | |--------------------- |          |     |&\
  |               | |                      |-------------  -----&\
  | --------------- |--------------------- |&\
  |          ^      |                    |                     |     ^&\
  -------------------                    --------------------------------&\
",
"\
&\
&\
 -----------------------   ---------------------------       -----------------&\
 |      ...            |   |    ...                  |       |               |&\
 |      ...            -----    ...                  ---     |               |&\
 |      ...             ...     ...                    |     |               |&\
 |                      ...    ---                     |   ---       ------- |&\
 |                  .......    | |                     |   |         |.....| |&\
 |         --- ---  .......    ---     -----------     --- |         |.....S |&\
 |         |.....|  ...........        |.........|       | |         |*....| |&\
 |         |.....|      .......---     |.........|       ---         ------- |&\
 |         |..{..|      .......| |     S....<....S                           |&\
 |         |.....|      ...    ---     |.........|   ---                     |&\
 |         |.....|      ...            |.........|   | |...                  |&\
 ----      --- ---      ...---         -----------   ---...    ---     ---   |&\
  --|..                 ...| |                          ...    | |     | |   |&\
  |*S..                ----- ---                        ...    | ---   --- ---&\
  |-|..                |       |...                 ... ...    |   |       |&\
  |*S..              ---       |...      ---        .{.        |   ---------&\
  --|..              |         |...      | |        ...        |&\
    ------------------         ----------- ---------------------&\
",
};

#endif
