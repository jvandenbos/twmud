//Ok, a few comments about the hunter progs:
//First off, every charcter in the quest has
//a number (team variable) which declares
//them as either being on BLUE, or RED team.
//BLUE = "blue"
//RED  = "red"

//The hunters have 2 major variables:
//hunt, and level. Level, quite simply means
//what level they are. This will carry
//over when they respawn, as they are then
//carried over by use of the mpquestup
//function. The hunt variable can have the
//following values to it:
//1 - Defend Team Flag
//2 - Assault Enemy Flag
//3 - Return to Home

//I have also written a few immortal commands
//that you can use with the hunters.
//Set Levels, and Generate.
//-- "Set Levels XXX":
//  Changes the levels of each and every
//  hunter in the zone of that hunter type.
//
//-- "Generate XXX":
//  Creates, or destroys a number of the
//  hunters, until there is only X left.


#include ../include/capture_flag/blueteam
#include capture_flag/hunter_general
