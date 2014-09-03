#	$Id: Makefile,v 1.6 2005/03/26 15:12:09 twdev Exp $	
#

C = c
CC=g++
#CC=gcc
LD=g++
#LD=gcc
#CC=i486-linuxlibc1-g++
#LD=i486-linuxlibc1-g++
#MAKEDEP=i486-linuxlibc1-g++
MAKEDEP=gcc
## use below for LINUX
#SYS_CFLAGS = -nostdinc -I/usr/i486-linuxlibc1/include/ -I/usr/i486-linuxlibc1/include/linux/ -I/usr/i486-linuxlibc1/include/g++/ -m486 -O -pipe
# SYS_CFLAGS = -m486 -O -pipe
SYS_CFLAGS = -O -pipe
#SYS_HEADER = s-linux.h
## use below for A/UX 
#SYS_CFLAGS =
#SYS_HEADER = s-aux.h
#RANLIB = echo
## use below for Sun Sparc
#SYS_CFLAGS = -pipe
#SYS_HEADER = s-sparc.h
## use below for escape.com
#SYS_CFLAGS=-pipe
SYS_HEADER = s-escape.h
RANLIB = ranlib
## for Vax running some ultrix or other...
#SYS_CFLAGS =
#SYS_HEADER = s-ultrix.h
#RANLIB = ranlib
##
# dont forget the -p after adding a 1 to the profile
# using the -pg option will allow creation of gmon.out for gprof
#PROFILE = -DUSE_profile=1 -p 
PROFILE = -DUSE_profile=0
#OPTIMIZE = -O2
#OPTIMIZE =
SYMBOLS = -g -pg
#SYMBOLS = -Wall
LIBs = -lm -lcrypt
#LIBRARIES = -nostdlib -L/usr/i486-linuxlibc1/lib/ #-lcrypt
LIBRARIES = -lcrypt
#LIBRARIES = -lm -lefence
#LIBRARIES = -lm -lefence -static
DEBUG_FLAGS =
OTHER_CFLAGS = -Wall -Wno-unused
CFLAGS = $(OPTIMIZE) $(SYMBOLS) $(PROFILE) $(SYS_CFLAGS) $(OTHER_CFLAGS) $(DEBUG_FLAGS)
LDFLAGS = $(OPTIMIZE) $(SYMBOLS) $(SYS_CFLAGS) $(OTHER_CFLAGS) $(PROFILE)
CPP_FLAGS = $(CFLAGS)
##
LIB_DBMS = db.a
##
##
HDRS =	\
	act.h \
	area.h \
	array.h \
	bet.h \
	board.h \
	casino.h \
	channels.h \
	char_create.h \
	cmdtab.h \
	comm.h \
	config.h \
	constants.h \
	db.h \
	db.random.h \
	db.zonehelp.h \
	editor.h \
	eng_procs.h \
	engine.h \
	events.h \
	fight.h \
	find.h \
	guilds.h \
	handler.h \
	hash.h \
	hero.h \
	ident.h \
	inet.h \
	interpreter.h \
	limits.h \
	list.h \
	log_tcp.h \
	magicutils.h \
	master.h \
	mobact.h \
	mobprog2.h \
	modify.h \
	multiclass.h \
	newsaves.h \
	opinion.h \
	page.h \
	parse.h \
	periodic.h \
	poly.h \
	proto.h \
	race.h \
	recept.h \
	ringlog.h \
	sblock.h \
	shop.h \
	signals.h \
	skills.h \
	smart_mobs.h \
	sound.h \
	spec.h \
	spell_events.h \
	block.h \
	spell_procs.h \
	spell_util.h \
	spells.h \
	spelltab.h \
	scrap.h \
	sstring.h \
	state.h \
        statistic.h \
	stats.h \
	structs.h \
	track.h \
	trackchar.h \
	trap.h \
	util_num.h \
	util_str.h \
	utility.h \
	utils.h \
        vnum.h \
	vnum_mob.h\
	weather.h \
	whod.h \
	wizlist.h \
	mobile_ui.h \
	path.h \
	strext.h \
	varfunc.h

SYS_HDRS = s-aux.h s-sparc.h s-ultrix.h s-bsd.h s-escape.h s-linux.h
##
OBJS =	\
	Opinion.o \
	Sound.o \
	Trap.o \
	act.comm.o \
	act.info.o \
	act.move.o \
	act.obj1.o \
	act.obj2.o \
	act.offensive.o \
	act.other.o \
	act.social.o \
	act.wizard.o \
	array.o \
	ban.o \
	block.o \
	board.o \
	casino.o \
	char_create.o \
	channels.o \
	cmdtab.o \
	comm.o \
	constants.o \
	db.o \
	db.mobile.o \
	db.object.o \
	db.player.o \
	db.random.o \
	db.relate.o \
	db.utility.o \
	db.world.o \
	db.zonehelp.o \
	editor.o \
	engine.o \
	enginit.o \
	englogic.o \
	engmobs.o \
	engprecall.o \
	events.o \
	exp.o \
	fight.o \
	find.o \
	guilds.o \
	handler.o \
	hash.o \
	hero.o \
	ident.o \
	interpreter.o \
	limits.o \
	list.o \
	magic.o \
	magicutils.o \
	mail.o \
	memory.o \
	mobact.o \
	mobcmd.o \
	mobprog.o \
	mobprog2.o \
	modify.o \
	multiclass.o \
	newsaves.o \
	page.o \
	parse.o \
	periodic.o \
	recept.o \
	ringlog.o \
	sblock.o \
	shop.o \
	signals.o \
	skills.o \
	smart_mobs.o \
	spec_assign.o \
	spec_astral.o \
	spec_breath.o \
	spec_dm.o \
	spec_mobs.o \
	spec_obj.o \
	spec_rooms.o \
	spec_utils.o \
	spell_parser.o \
	spell_events.o \
	spells.o \
	spelltab.o \
	scrap.o \
	sstring.o \
        statistic.o \
	stats.o \
	strstr.o \
	title.o \
	track.o \
	trackchar.o \
	utility.o \
	util_num.o \
	util_str.o \
	weather.o \
	whod.o \
	xrand.o

DB_O = \
	db.o \
	db.mobile.o \
	db.object.o \
	db.player.o \
	db.relate.o \
	db.utility.o \
	db.world.o \
	db.random.o \
	db.zonehelp.o \
	db.stubs.o \
	Opinion.o \
	array.o \
	constants.o \
	events.o \
	exp.o \
	handler.o \
	hash.o \
	limits.o \
	list.o \
	memory.o \
	multiclass.o \
	sstring.o \
#	util_num.o \
	util_str.o \
        statistic.o

SRCS =  \
	main.$C \
	Opinion.$C \
	Sound.$C \
	Trap.$C \
	act.comm.$C \
	act.info.$C \
	act.move.$C \
	act.obj1.$C \
	act.obj2.$C \
	act.offensive.$C \
	act.other.$C \
	act.social.$C \
	act.wizard.$C \
	array.$C \
	ban.$C \
	block.$C \
	board.$C \
	casino.$C \
	char_create.$C \
	channels.$C \
	cmdtab.$C \
	comm.$C \
	constants.$C \
	db.$C \
	db.mobile.$C \
	db.object.$C \
	db.player.$C \
	db.random.$C \
	db.relate.$C \
	db.utility.$C \
	db.world.$C \
	db.zonehelp.$C \
	editor.$C \
	engine.$C \
	enginit.$C \
	englogic.$C \
	engmobs.$C \
	engprecall.$C \
	events.$C \
	exp.$C \
	fight.$C \
	find.$C \
	guilds.$C \
	handler.$C \
	hash.$C \
	hero.$C \
	ident.$C \
	interpreter.$C \
	limits.$C \
	list.$C \
	magic.$C \
	magicutils.$C \
	mail.$C \
	memory.$C \
	mobact.$C \
	mobcmd.$C \
	mobprog.$C \
	mobprog2.$C \
	modify.$C \
	multiclass.$C \
	newsaves.$C \
	page.$C \
	parse.$C \
	periodic.$C \
	recept.$C \
	ringlog.$C \
	sblock.$C \
	shop.$C \
	signals.$C \
	skills.$C \
	smart_mobs.$C \
	spec_assign.$C \
	spec_astral.$C \
	spec_breath.$C \
	spec_dm.$C \
	spec_mobs.$C \
	spec_obj.$C \
	spec_rooms.$C \
	spec_utils.$C \
	spell_parser.$C \
	spell_events.$C \
	spells.$C \
	spelltab.$C \
	scrap.$C \
	sstring.$C \
        statistic.$C \
	stats.$C \
	strstr.$C \
	title.$C \
	track.$C \
	trackchar.$C \
	utility.$C \
	util_num.$C \
	util_str.$C \
	weather.$C \
	whod.$C \
	mkpass.$C \
	db.stubs.$C \
	xrand.$C \
	act.zone.$C \
	mobile_ui.$C \
	path.$C \
	strext.$C


OTHER_FILES = Makefile nightrun spec_court.c guilds.c \
	heap.h Heap.c

ALL_FILES = $(HDRS) $(SYS_HDRS) $(SRCS) $(OTHER_FILES)

TAR_FILE = ../src.tgz

## %.c : RCS/%.c,v
## 	co $@
## 
## %.h : RCS/%.h,v
## 	co $@

all : thievesworld $(LIB_DBMS)
.PHONY : all

Makefile :;

thievesworld : main.o $(OBJS) $(LIB_DBMS)
	test ! -f tw || mv tw dms.old
	$(LD) $(LDFLAGS) -o $@ main.o $(OBJS) $(LIBRARIES) $(SYMBOLS)
	# strip thievesworld

dms.new : main.o $(OBJS)
	$(LD) $(CPP_FLAGS) -o $@ main.o $(OBJS) $(LIBRARIES)


$(LIB_DBMS) : $(DB_O)
	rm -f $(LIB_DBMS)
	ar csq $(LIB_DBMS) $(DB_O)
	$(RANLIB) $(LIB_DBMS)

proto:
	protoize -kg $(SRCS)

TAGS: $(HDRS) $(SRCS) system.h
	etags $(SRCS) $(HDRS)

tar:
	tar czf $(TAR_FILE) $(ALL_FILES)

protect:
	chmod a-w $(ALL_FILES)

depend : $(HDRS) $(SRCS) system.h
	sed -e '/^# DO NOT DELETE THIS LINE/,$$d' < Makefile > makefile.temp
	echo "# DO NOT DELETE THIS LINE -- make depend depends on it." >> makefile.temp
	$(MAKEDEP) -MM $(SRCS) >> makefile.temp
	mv makefile.temp Makefile

archive:
	rm -rf ../src.dist
	mkdir ../src.dist
	ln $(ALL_FILES) ../src.dist

clean : 
	rm -f $(OBJS) dms.new dms.old tw $(LIB_DBMS)

checkouts :
	co  $(CO_FLAGS) $(ALL_FILES)

checkins :
	ci  $(CI_FLAGS) $(ALL_FILES)

mkpass : mkpass.c
	gcc mkpass.c -o mkpass
	strip mkpass

system.h : $(SYS_HEADER)
	rm -f system.h
	ln -s $(SYS_HEADER) system.h

# DO NOT DELETE THIS LINE -- make depend depends on it.
main.o: main.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h proto.h comm.h \
 interpreter.h db.h array.h state.h utility.h signals.h whod.h page.h \
 modify.h editor.h fight.h weather.h spell_util.h statistic.h ident.h \
 periodic.h mobprog2.h
Opinion.o: Opinion.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h race.h \
 opinion.h db.h array.h multiclass.h comm.h utility.h
Sound.o: Sound.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h handler.h \
 db.h array.h comm.h utility.h
Trap.o: Trap.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h spells.h trap.h comm.h \
 db.h array.h constants.h multiclass.h utility.h fight.h spell_util.h \
 opinion.h handler.h act.h recept.h magicutils.h scrap.h
act.comm.o: act.comm.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 interpreter.h handler.h db.h array.h utility.h act.h multiclass.h \
 util_str.h find.h proto.h ansi.h \
 channels.h mobprog2.h
act.info.o: act.info.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 handler.h interpreter.h db.h array.h spells.h race.h trap.h hash.h \
 utility.h multiclass.h act.h constants.h modify.h skills.h fight.h \
 cmdtab.h spelltab.h spell_util.h util_str.h find.h sblock.h hero.h \
 vnum.h recept.h db.zonehelp.h track.h db.random.h ansi.h proto.h \
 varfunc.h
act.move.o: act.move.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 interpreter.h handler.h db.h array.h spells.h trap.h act.h \
 spell_util.h utility.h constants.h multiclass.h casino.h util_str.h \
 find.h statistic.h proto.h mobprog2.h
act.obj1.o: act.obj1.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 interpreter.h handler.h db.h array.h trap.h multiclass.h utility.h \
 constants.h act.h fight.h find.h util_str.h recept.h spells.h \
 statistic.h channels.h proto.h bet.h mobprog2.h
act.obj2.o: act.obj2.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 interpreter.h handler.h db.h array.h spells.h act.h utility.h \
 constants.h multiclass.h spelltab.h find.h periodic.h util_str.h \
 proto.h mobprog2.h
act.offensive.o: act.offensive.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 interpreter.h handler.h db.h array.h spells.h opinion.h race.h \
 multiclass.h casino.h fight.h utility.h act.h spell_util.h \
 constants.h spec.h spell_procs.h find.h util_str.h statistic.h \
 proto.h varfunc.h
act.other.o: act.other.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 handler.h db.h array.h spells.h act.h utility.h recept.h constants.h \
 multiclass.h fight.h modify.h spec.h spelltab.h cmdtab.h util_str.h \
 find.h interpreter.h state.h ringlog.h ansi.h spell_util.h proto.h \
 signals.h varfunc.h trackchar.h
act.social.o: act.social.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 handler.h db.h array.h utility.h multiclass.h find.h util_str.h
act.wizard.o: act.wizard.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 interpreter.h handler.h db.h array.h spells.h hash.h race.h \
 multiclass.h constants.h utility.h act.h modify.h fight.h spelltab.h \
 board.h magicutils.h spec.h recept.h util_str.h find.h sblock.h \
 periodic.h statistic.h spell_util.h proto.h hero.h vnum.h cmdtab.h \
 strext.c strext.h mobprog2.h varfunc.h trackchar.h db.random.h
array.o: array.c config.h system.h s-linux.h array.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h
ban.o: ban.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h comm.h interpreter.h \
 handler.h db.h array.h utility.h util_str.h
block.o: block.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h db.h \
 array.h utility.h multiclass.h smart_mobs.h block.h spec.h
board.o: board.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h db.h \
 array.h board.h multiclass.h utility.h modify.h handler.h util_str.h \
 spec.h
casino.o: casino.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 utility.h interpreter.h db.h array.h util_str.h
char_create.o: char_create.c config.h system.h s-linux.h char_create.h \
 structs.h list.h sstring.h events.h guilds.h utils.h vnum_mob.h \
 limits.h language.h cmdtab.h race.h
channels.o: channels.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h channels.h \
 db.h array.h util_str.h comm.h utility.h multiclass.h proto.h \
 cmdtab.h
cmdtab.o: cmdtab.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h parse.h \
 cmdtab.h channels.h act.h track.h proto.h
comm.o: comm.c config.h system.h s-linux.h constants.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h inet.h \
 comm.h handler.h db.h array.h state.h utility.h whod.h editor.h \
 page.h weather.h fight.h spell_util.h modify.h multiclass.h signals.h \
 board.h util_str.h act.h recept.h interpreter.h statistic.h ident.h \
 trackchar.h ansi.h proto.h
constants.o: constants.c config.h system.h s-linux.h constants.h \
 structs.h list.h sstring.h events.h guilds.h utils.h vnum_mob.h \
 limits.h trap.h
db.o: db.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h comm.h \
 handler.h race.h opinion.h hash.h wizlist.h utility.h fight.h act.h \
 spec.h cmdtab.h spelltab.h modify.h recept.h weather.h spell_util.h \
 multiclass.h constants.h board.h util_str.h statistic.h db.zonehelp.h \
 proto.h interpreter.h mobprog2.h trackchar.h db.random.h
db.mobile.o: db.mobile.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 comm.h handler.h race.h opinion.h hash.h wizlist.h utility.h fight.h \
 act.h spec.h cmdtab.h spelltab.h modify.h recept.h weather.h \
 spell_util.h multiclass.h constants.h board.h sound.h
db.object.o: db.object.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 comm.h handler.h race.h opinion.h hash.h wizlist.h utility.h fight.h \
 act.h spec.h cmdtab.h spelltab.h modify.h recept.h weather.h \
 spell_util.h multiclass.h constants.h board.h util_str.h statistic.h \
 sound.h proto.h spells.h db.random.h
db.player.o: db.player.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 race.h constants.h handler.h spelltab.h channels.h multiclass.h \
 util_num.h util_str.h utility.h spells.h hero.h vnum.h statistic.h \
 ansi.h comm.h opinion.h track.h proto.h spell_events.h varfunc.h \
 char_create.h
db.random.o: db.random.c structs.h list.h sstring.h events.h guilds.h \
 utils.h vnum_mob.h limits.h utility.h spells.h db.h array.h \
 db.random.h
db.relate.o: db.relate.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 comm.h constants.h utility.h handler.h act.h multiclass.h trap.h \
 fight.h statistic.h
db.utility.o: db.utility.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h \
 array.h utility.h
db.world.o: db.world.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 comm.h hash.h handler.h find.h utility.h opinion.h spell_util.h act.h \
 proto.h statistic.h
db.zonehelp.o: db.zonehelp.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h \
 array.h utility.h constants.h util_str.h db.zonehelp.h sblock.h
editor.o: editor.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 modify.h utility.h act.h util_str.h interpreter.h
engine.o: engine.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h fight.h \
 spells.h engine.h spelltab.h utility.h act.h util_num.h skills.h
enginit.o: enginit.c config.h system.h s-linux.h spells.h engine.h \
 structs.h list.h sstring.h events.h guilds.h utils.h vnum_mob.h \
 limits.h spelltab.h multiclass.h db.h array.h
englogic.o: englogic.c config.h system.h s-linux.h constants.h \
 structs.h list.h sstring.h events.h guilds.h utils.h vnum_mob.h \
 limits.h fight.h engine.h spelltab.h spell_util.h spells.h db.h \
 array.h utility.h multiclass.h handler.h
engmobs.o: engmobs.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h spelltab.h \
 engine.h
engprecall.o: engprecall.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h \
 fight.h spells.h comm.h engine.h spelltab.h utility.h spell_procs.h \
 multiclass.h act.h handler.h db.h array.h
events.o: events.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 utility.h util_str.h state.h
exp.o: exp.c config.h system.h s-linux.h constants.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h cmdtab.h
fight.o: fight.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 handler.h db.h array.h spells.h fight.h utility.h recept.h \
 magicutils.h opinion.h multiclass.h act.h spec.h spell_util.h \
 constants.h mobact.h spelltab.h channels.h util_str.h interpreter.h \
 statistic.h find.h vnum.h ansi.h newsaves.h hero.h ringlog.h proto.h \
 cmdtab.h scrap.h state.h race.h mobprog2.h varfunc.h
find.o: find.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h find.h \
 utility.h util_str.h multiclass.h
guilds.o: guilds.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h proto.h \
 modify.h comm.h find.h recept.h db.h array.h
handler.o: handler.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h db.h \
 array.h handler.h spells.h utility.h modify.h multiclass.h \
 constants.h spell_util.h act.h fight.h opinion.h race.h track.h \
 interpreter.h newsaves.h
hash.o: hash.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h hash.h
hero.o: hero.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h race.h \
 constants.h multiclass.h util_str.h utility.h hero.h vnum.h
ident.o: ident.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 util_str.h comm.h db.h array.h spell_util.h ident.h proto.h
interpreter.o: interpreter.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 constants.h interpreter.h db.h array.h race.h board.h master.h act.h \
 utility.h cmdtab.h multiclass.h handler.h page.h recept.h util_str.h \
 fight.h spells.h ringlog.h periodic.h proto.h char_create.h \
 language.h spec.h mobprog2.h trackchar.h
limits.o: limits.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h spells.h \
 comm.h race.h constants.h multiclass.h utility.h db.h array.h fight.h \
 handler.h act.h recept.h spec.h
list.o: list.c config.h system.h s-linux.h list.h structs.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h
magic.o: magic.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 spells.h handler.h db.h array.h utility.h fight.h spell_util.h \
 multiclass.h act.h trap.h magicutils.h opinion.h constants.h \
 newsaves.h spelltab.h util_str.h recept.h race.h periodic.h \
 spell_procs.h poly.h vnum.h find.h statistic.h proto.h cmdtab.h \
 interpreter.h spell_events.h
magicutils.o: magicutils.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 spells.h handler.h multiclass.h utility.h fight.h db.h array.h act.h
mail.o: mail.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h comm.h interpreter.h \
 handler.h db.h array.h trap.h utility.h constants.h act.h find.h \
 util_str.h recept.h statistic.h multiclass.h proto.h
memory.o: memory.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h
mobact.o: mobact.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h handler.h \
 db.h array.h comm.h opinion.h trap.h spells.h state.h act.h \
 multiclass.h fight.h utility.h constants.h mobact.h hash.h skills.h \
 util_str.h track.h sound.h spec.h mobprog2.h proto.h
mobcmd.o: mobcmd.c structs.h list.h sstring.h events.h guilds.h \
 utils.h vnum_mob.h limits.h db.h array.h handler.h find.h \
 interpreter.h comm.h spells.h proto.h modify.h act.h util_str.h \
 track.h config.h system.h s-linux.h constants.h opinion.h varfunc.h
mobprog.o: mobprog.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 interpreter.h handler.h db.h array.h proto.h fight.h find.h varfunc.h \
 comm.h
mobprog2.o: mobprog2.c structs.h list.h sstring.h events.h guilds.h \
 utils.h vnum_mob.h limits.h db.h array.h varfunc.h find.h proto.h \
 interpreter.h utility.h comm.h fight.h opinion.h track.h util_str.h \
 config.h system.h s-linux.h
modify.o: modify.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h handler.h \
 db.h array.h comm.h editor.h multiclass.h recept.h act.h utility.h \
 modify.h find.h util_str.h constants.h interpreter.h proto.h \
 trackchar.h
multiclass.o: multiclass.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 handler.h opinion.h race.h multiclass.h utility.h db.h array.h
newsaves.o: newsaves.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h newsaves.h \
 utility.h multiclass.h
page.o: page.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h comm.h utility.h \
 multiclass.h db.h array.h recept.h util_str.h
parse.o: parse.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h parse.h \
 utility.h util_str.h
periodic.o: periodic.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 handler.h db.h array.h race.h spells.h spell_util.h multiclass.h \
 util_num.h fight.h comm.h periodic.h
recept.o: recept.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 recept.h utility.h comm.h spec.h race.h master.h util_str.h \
 magicutils.h multiclass.h handler.h act.h interpreter.h vnum.h hero.h \
 cmdtab.h proto.h trackchar.h
ringlog.o: ringlog.c config.h system.h s-linux.h
sblock.o: sblock.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h modify.h
shop.o: shop.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h comm.h handler.h db.h \
 array.h act.h utility.h multiclass.h constants.h spec.h shop.h find.h \
 util_str.h vnum.h
signals.o: signals.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 utility.h signals.h
skills.o: skills.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h race.h \
 spells.h comm.h handler.h hash.h db.h array.h interpreter.h fight.h \
 utility.h constants.h multiclass.h skills.h parse.h spell_util.h \
 act.h modify.h magicutils.h spelltab.h newsaves.h spell_procs.h \
 spec.h trap.h util_str.h find.h track.h statistic.h vnum.h opinion.h \
 cmdtab.h proto.h spell_events.h
smart_mobs.o: smart_mobs.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 spells.h engine.h spelltab.h smart_mobs.h spec.h multiclass.h
spec_assign.o: spec_assign.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h \
 array.h spec.h utility.h shop.h board.h smart_mobs.h mobile_ui.h \
 proto.h
spec_astral.o: spec_astral.c config.h system.h s-linux.h engine.h \
 structs.h list.h sstring.h events.h guilds.h utils.h vnum_mob.h \
 limits.h spelltab.h spells.h smart_mobs.h multiclass.h spec.h db.h \
 array.h comm.h find.h util_str.h utility.h act.h handler.h periodic.h \
 proto.h
spec_breath.o: spec_breath.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 spells.h db.h array.h spell_procs.h utility.h util_num.h multiclass.h \
 act.h spec.h
spec_dm.o: spec_dm.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 db.h array.h comm.h find.h spec.h spells.h util_str.h util_num.h \
 constants.h multiclass.h act.h
spec_mobs.o: spec_mobs.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h db.h \
 array.h find.h spells.h constants.h spec.h interpreter.h area.h \
 opinion.h fight.h race.h spelltab.h utility.h act.h spell_procs.h \
 multiclass.h util_str.h handler.h spell_util.h skills.h recept.h \
 trap.h hero.h vnum.h track.h statistic.h periodic.h proto.h state.h \
 cmdtab.h engine.h scrap.h
spec_obj.o: spec_obj.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h db.h array.h \
 find.h comm.h spells.h util_str.h utility.h handler.h multiclass.h \
 act.h hero.h vnum.h cmdtab.h spell_util.h fight.h
spec_rooms.o: spec_rooms.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h \
 cmdtab.h db.h array.h find.h comm.h interpreter.h util_str.h \
 multiclass.h act.h utility.h util_num.h ansi.h spell_util.h recept.h \
 statistic.h spec.h proto.h
spec_utils.o: spec_utils.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h \
 constants.h db.h array.h comm.h find.h multiclass.h act.h util_num.h
spell_parser.o: spell_parser.c config.h system.h s-linux.h structs.h \
 list.h sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 db.h array.h spells.h handler.h multiclass.h spelltab.h constants.h \
 utility.h fight.h act.h sound.h spec.h modify.h find.h periodic.h \
 util_str.h spell_util.h
spell_events.o: spell_events.c config.h system.h s-linux.h \
 spell_events.h structs.h list.h sstring.h events.h guilds.h utils.h \
 vnum_mob.h limits.h utility.h comm.h
spells.o: spells.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h db.h \
 array.h interpreter.h spells.h handler.h poly.h spell_procs.h \
 spell_util.h multiclass.h act.h constants.h find.h spelltab.h \
 utility.h util_str.h statistic.h
spelltab.o: spelltab.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h spells.h \
 parse.h spelltab.h spell_procs.h eng_procs.h
scrap.o: scrap.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 spells.h db.h array.h comm.h constants.h
sstring.o: sstring.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h
statistic.o: statistic.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h
stats.o: stats.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 fight.h stats.h util_str.h interpreter.h handler.h
strstr.o: strstr.c config.h system.h s-linux.h
title.o: title.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h constants.h
track.o: track.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h track.h db.h \
 array.h hash.h constants.h util_str.h comm.h mobprog2.h
trackchar.o: trackchar.c structs.h list.h sstring.h events.h guilds.h \
 utils.h vnum_mob.h limits.h db.h array.h utility.h comm.h trackchar.h \
 find.h util_str.h multiclass.h recept.h handler.h
utility.o: utility.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h spells.h \
 race.h db.h array.h opinion.h comm.h hash.h area.h state.h \
 multiclass.h utility.h handler.h fight.h constants.h act.h trap.h \
 mobact.h sound.h magicutils.h recept.h spell_procs.h spelltab.h \
 channels.h util_str.h track.h find.h spell_util.h varfunc.h hero.h \
 vnum.h proto.h char_create.h
util_num.o: util_num.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h
util_str.o: util_str.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 util_str.h
weather.o: weather.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h comm.h \
 handler.h db.h array.h weather.h utility.h multiclass.h fight.h \
 interpreter.h proto.h trap.h mobprog2.h
whod.o: whod.c config.h system.h s-linux.h structs.h list.h sstring.h \
 events.h guilds.h utils.h vnum_mob.h limits.h multiclass.h utility.h \
 comm.h db.h array.h util_str.h
mkpass.o: mkpass.c
db.stubs.o: db.stubs.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h utility.h \
 track.h
xrand.o: xrand.c
act.zone.o: act.zone.c config.h system.h s-linux.h
mobile_ui.o: mobile_ui.c config.h system.h s-linux.h structs.h list.h \
 sstring.h events.h guilds.h utils.h vnum_mob.h limits.h spec.h \
 spell_util.h act.h comm.h db.h array.h board.h multiclass.h utility.h \
 modify.h handler.h util_str.h
path.o: path.c config.h system.h s-linux.h path.h
strext.o: strext.c strext.h
