>speech_prog p about hoeur
if ispc($n)
  say The Hoeur guards one of several entrances into the underworld, and carries a nasty whip.
endif
~
>speech_prog p where hoeur
if ispc($n)
  say The Hoeur is a dangerous beast, and you can find him past the Haon-Dor Forest,
  say somewhere south of the Dark River.
endif
~
>speech_prog p about undercaves mountain mountains
if ispc($n)
  say The undercaves compromise a large labyrinth, deep within the Eastern Mountains.
  say This is a rarely travelled area by adventurers, so the pickings are good...
  say ...if you are up to the journey, $n.
endif
~
>speech_prog p where undercaves mountain mountains
if ispc($n)
  say The undercaves can be found deep with the Eastern Mountains.
  say If you follow the Dark River to the east, you will find a small ledge just north of the headwaters.
  say A ravine can be found there, providing entrance into the undercaves.
endif
~
>speech_prog p where gypsy gypsys aldur
if ispc($n)
  say To get to the gypsy village, $n, you must travel the
  say tracherous trail through the Eastern Mountains.
endif
~
>speech_prog p about gypsy gypsys aldur
if ispc($n)
  say The gypsies are an odd folk.
  say They have forsworn allegiance to any of the civilized kingdoms,
  say and instead have set up a small village of their own.
  say They sell a variety of wares to the traveller who passes through. 
endif
~
>speech_prog p about malvaria goblins goblin
if ispc($n)
  say Malvaria is home to the goblin king.
  say He has been quiet the past several years, but it is suspected that even now 
  say he is planning his next raid upon the hapless citizens of Sanctuary.
  say If you could thwart his plans, $n, before they can be realized, say you would surely be branded a hero.
endif
~
>speech_prog p where malvaria goblins goblin
if ispc($n)
  say Malvaria is up in the mountains, east of Sanctuary.
endif
~
>speech_prog p about gelfling gelflings
if ispc($n)
  say Beware the traps of the Gelflings.  They thrive on tricking the unwary traveller.
endif
~
>speech_prog p about skexie skexies
if ispc($n)
  say Beware the traps of the Skexies.  They thrive on tricking the unwary traveller.
endif
~
>speech_prog p where gelfling gelflings
if ispc($n)
  say The Gelflings can be found near the Haon-Dor Forest,
  say south of the Dark River along an ancient path.
endif
~
>speech_prog p where skexie skexies
if ispc($n)
  say The Skexies can be found near the Haon-Dor Forest,
  say south of the Dark River along an ancient path.
endif
~
>speech_prog p about mount olympus zeus hera
if ispc($n)
  say Mount Olympus is the home of Graecia's highest immortals.
  say Zeus and his entire entourage dwell in that mighty land.
  say Do not travel there, $n, unless you are well prepared for the journey.
endif
~
>speech_prog p where mount olympus zeus hera
if ispc($n)
  say Mount Olympus is near Greece, yet perhaps unreachable by mortals.
endif
~
>speech_prog p about hades pluto
if ispc($n)
  say Hades is the underworld home of some of the more powerful of the Graecian gods.
  say It is a deadly place, but holds many wonders.
endif
~
>speech_prog p where hades pluto
if ispc($n)
  say Hades is the vast underworld of the Graecian gods.
  say Truthfully, I know not its location, I have heard there is more than one way to reach it.
endif
~
>speech_prog p about greece graecia
if ispc($n)
  say Greece is home to many mythical warriors and beasts.
endif
~
>speech_prog p where greece graecia
if ispc($n)
  say Greece is far to the southeast, beyond the ocean that laps at the shores of Lycanthropia.
endif
~
>speech_prog p about python cheeser
if ispc($n)
  say The Lord Python is sometimes a silly character, and his kingdom is no exception.
  say Look for the guaranteed official baker, and his partner the cheeser.
  say And don't be surprised of a commoner stops you for a casual conversation about the ability of sheep to fly.
endif
~
>speech_prog p where python cheeser
if ispc($n)
  say The castle of Lord Python lies south of the crossroads east of Sanctuary
  say along an eastern, er, or was that southern path...
  shrug
endif
~
>speech_prog p about cavernous caves death balrog
if ispc($n)
  say Commonly referred to as the 'Caves of Death', these caverns hold some
  say of the most powerful subterranean beings known to man.
  say Correspondingly, these creatures also hold some of the largest treasures to be found.
endif
~
>speech_prog p where cavernous caves death balrog
if ispc($n)
  say These caverns can be reached down a chasm, high in the mountains in the northeast corner of Ranke.
endif
~
>speech_prog p about constellation constellations galaxy
if ispc($n)
  say This is a terrifying area, indeed.
  say The 'Galaxy', as it's so popularly referred to, is so devastating that it is impossible
  say to even utter a single word of speech while within its bounds.
endif
~
>speech_prog p where constellation constellations galaxy
if ispc($n)
  say The constellations are far in the sky above Graecia.
  stare
endif
~
>speech_prog p about darksociety darksocieties dark society thief thieves den
if ispc($n)
  say The Dark Society Guild is a dangerous lot.
  say I would give them a wide berth, unless you are certain of your abilities, $n.
  say They are said to hold a variety of items that help them further their dark profession.
endif
~
>speech_prog p where darksociety darksocieties dark society thief thieves den
if ispc($n)
  say The members of that guild have striven to keep their whereabouts hidden from others.
  say I have heard from my sources, however, that they can be found somewhere in Mordilnia.
endif
~
>speech_prog p about hill giant hillgiant hillgiants compound
if ispc($n)
  say The Hill Giants have built a mighty compound to serve as a base for their raids of terror.
  say It is rumored that somewhere within the walls is a secret entrance into vast underground chambers,
  say where they hide their most valuable treasures.
endif
~
>speech_prog p where hill giant hillgiant hillgiants compound
if ispc($n)
  say The hill giant can be found along the waters of the Silver River, north of the shire.
endif
~
>speech_prog p about paramor paramour druid druids skull bearer skullbearer skullbearers
if ispc($n)
  say The druids are a reclusive breed.
  say The hold many secrets, however, and rumor has it that Allanon has created a device of amazing power!
  say Beware the skull-bearers, though.
  say They reside nearby, and relish in creating terror.
endif
~
>speech_prog p where paramor paramour druid druids skull bearer skullbearer skullbearers
if ispc($n)
  say The Druid's keep can be found west of Mordilnia, in a forest.
endif
~
>speech_prog p about roogna evil-one evil
if ispc($n)
  look $n
  ponder
  say Fear should be your guide through this terrifying castle of doom.
endif
~
>speech_prog p where roogna evil-one evil
if ispc($n)
  say Castle Roogna is somewhere near the fabled land of Xanth.
  say To get there, though, you must descend a swirling hypo-gourde,
  say and pass through lands filled with dangers beyond reason.
endif
~
>speech_prog p about xanth centaur centaurs wiggle wiggles
if ispc($n)
  say Xanth is a mystical land, not quite on our own plane of existence, yet somehow accessible from it.
  say Magical creatures of all varieties inhabit it, and they hold treasures beyond your wildest dreams.
endif
~
>speech_prog p where xanth centaur centaurs wiggle wiggles
if ispc($n)
  say Xanth can not be reached by any means known to I or other more knowledgeable sages.
  say The captain of one of the boats docked at Sanctuary's pier, however,
  say once told a story of a mighty storm that came up suddenly,
  say sending his boat into a swirling whirlpool.
  say When the surviving crew came to their senses, they found themselves on the beach of a far off land.
  say Perhaps this was Xanth.
  shrug
endif
~
>speech_prog p about vortland lizardman lizardmen lizard
if ispc($n)
  say Ahh, the ancient city of Vortland.
  say Long ago, this place played a pivotal role in the development of mortalkind.
  say It was here that the lesser Aeon Oreurk gave the Weda Krizhtawnto the Yaddici,
  say who answered his call for knowledge.
  say Now, the peace-loving natives who still reside there are being threatened by fortune hunters,
  say who jumped at the mention of ancient treasure.
  say A conclave of lizard men is also rumored to dwell nearby in the jungle. 
endif
~
>speech_prog p where vortland lizardman lizardmen lizard
if ispc($n)
  say Ships depart regularly from Sanctuary's pier for the ancient continent of Sherranpip.
  say Vortland is the dropoff point for a variety of adventures in those far off lands.
endif
~
>speech_prog p about frost giant frostgiant frostgiants
if ispc($n)
  say The frost giant rifts are a very dangerous place for exploration.
  say Not only are the inhabitants unfriendly, but the environment itself is harsh and untamed.
  say The giants carry a store of wealth, however, and all who have
  say returned from their icy realm have relished their newly got wealth.
endif
~
>speech_prog p where frost giant frostgiant frostgiants
if ispc($n)
  say The frost giant rifts can be found on the continent of The Cold Land.
  say Beyond Ice Haven, across the snowy plains to the southeast.
endif
~
>speech_prog p where great desert
if ispc($n)
  say The great desert is vast beyond reckoning.
  say It compromises an extensive peninsula in southeast Ranke, south of New Thalos and the Ishtar River. 
endif
~
>speech_prog p about great desert
if ispc($n)
  sigh
  say Woe be to thee who travels to that sea of sand.
  say Many dangers mar its unbroken surface, although many wonders there exist, too.
  say Be wary of the ravine of Kaus Cavell, for its rocky depths have taken the life of many unwary adventurers.
endif
~
>speech_prog p about mongolia khan mongol mongols
if ispc($n)
  say The Mongols are fearsome warriors, and fear no mortal threat.
  say Their chief, Genghis Kahn, is one of the most powerful soldiers alive.
endif
~
>speech_prog p where mongolia khan mongol mongols
if ispc($n)
  say The fearsome Mongol horde has set up its camp north of New Thalos, across the plains.
endif
~
>speech_prog p about dojo oriental ninja avatar sohei shukenja bushi
if ispc($n)
  ponder
  say The Dojo quest zone is the oriental abode of some of the world's nastiest and vindictive soldiers.
  say I would suggest extreme caution.
  say Actually, I would suggest you stay home and cuddle up to a good book.
  say I would not recommend the Dojo to any sane adventurer.
endif
~
>speech_prog p where dojo oriental ninja avatar sohei shukenja bushi
if ispc($n)
  shiver
  say No one knows the location of the Dojo, not even I.
endif
~
>speech_prog p about shire hobbit hobbits
if ispc($n)
  smile
  say Shire folk are good, honest folk.
  say The hobbits who live their have always shown me welcome when I pass through.
  say You will find no trouble there.
endif
~
>speech_prog p where shire hobbit hobbits
if ispc($n)
  say Head north from the crossroads east of Sanctuary.
endif
~
>speech_prog p about troy trojan achilles
if ispc($n)
  say I have no information on that land yet, $n.
  say Perhaps you could be the first to explore it!
endif
~
>speech_prog p where troy trojan achilles
if ispc($n)
  say I have no information on that land yet, $n.
  say Perhaps you could be the first to explore it!
endif
~
>speech_prog p where aucan kong
if ispc($n)
  say The Aucan village is rumored to exist somewhere on the southern
  say continent of Sherranpip, along the Kong River.
endif
~
>speech_prog p about aucan kong
if ispc($n)
  say This small village is home to the Aucans.
  say They are are normally a peaceful tribe, but of late have become warlike.
  say They have brought out their fabled Aucan armor to strengthen their defense against enemies.
endif
~
>speech_prog p about carnival
if ispc($n)
  say Well, $n, the carnival is a perfect starting place for budding adventurers.
  say There you will find a casual carnival scene with acrobats, magicians, fat ladies, and more!
endif
~
>speech_prog p where carnival
if ispc($n)
  say To reach the carnival, head north out of the west gate of Sanctuary. 
endif
~
>speech_prog p where temple
if ispc($n)
  say Head north from inside the east gate of Midagaard, $n, and there
  say you will find an entrance into the old temple.
endif
~
>speech_prog p about temple
if ispc($n)
  say Newbies may explore the ancient temple of Sanctuary.
  say It was there that Ryadel unleashed his evil horde upon the populace of Sanctuary,
  say over three hundred years ago.
  say The temple was sealed shut, but has now been reopened, and many of the lesser creatures still roam free.
endif
~
>speech_prog p where chessboard
if ispc($n)
  chuckle
  say So you're interested in an old fashioned game of chess, eh $n?
  say Well, keep heading west from Tower Square.
  say It lies just outside the city wall.
endif
~
>speech_prog p about chessboard
if ispc($n)
  say This is an excellent area for new adventurers to test their mettle against both good and evil.
endif
~
>speech_prog p where sorceror's sorcerors sorceror
if ispc($n)
  say It is a long journey to the fabled sorceror's tower, $n.
  say To get there, you must travel through a dark forest, north of the Great Road.
endif
~
>speech_prog p about sorceror's sorcerors sorceror
if ispc($n)
  say Not many secrets leave those hallowed walls, $n.
  say You will have to go explore it for yourself, but beware the magics within!
endif
~
>speech_prog p about monk monks
if ispc($n)
  say The monks are very reclusive, although they have occasionally shared a bit of wisdom to the needy traveller.
  say Treat them with respect, and you may be rewarded.
endif
~
>speech_prog p where monk monks
if ispc($n)
  say Somewhere in the northern area of the Haon-Dor forest is the wild grove.
  say Continue north past those enchanted woods and you may yet find the monastery.
endif
~
>speech_prog p where draconian draconians dragon dragons tiamat draconia
if ispc($n)
  say The far off isle of Draconia lies far to the east, beyond New Thalos in the Dragon Sea.
endif
~
>speech_prog p about draconian draconians dragon dragons tiamat draconia
if ispc($n)
  say This bastion of evil is host to a variety of dragonkind.
  say Be wary, $n, for this place is not for the faint of heart.
endif
~
>speech_prog p where frost mountain cold-land cold
if ispc($n)
  shiver
  say Frost Mountain is at the frozen fringes of the world,
  say upon the antarctic continent of The Cold Land.
  say To get there, you will have to board a ship in Sanctuary, then journey through
  say the town of Ice Haven, and thence southwest across a frozen plain.
endif
~
>speech_prog p about frost mountain cold-land cold
if ispc($n)
  say You ask much of me, $n.
  say I have not travelled that way in many years, but I do recall great treasures
  say which have never seen warmer climes such as these we live in.
  say Perhaps you will be the first to carry these back to Ranke!
endif
~
>speech_prog p about mahn-tor mahntor minotaur minotaurs ogre ogres
if ispc($n)
  raise
  say Ahh, the fortress of Mahn-Tor?
  say This is the grand home of the minotaurs, a very challenging area that is filled with many interesting things!
endif
~
>speech_prog p where mahn-tor mahntor minotaur minotaurs ogre ogres
if ispc($n)
  ponder
  say This fortress lies somewhere in the icy mist of figid wasteland.
  say However...I have heard it can also be reached through a mystical portal.
  say The portal can be reached by passing through a forest, south of the Ishtar River
endif
~
>speech_prog p where loring lorings
if ispc($n)
  say The woods of Loring are rumored to lie somewhere in the wilderness,
  say between Sanctuary and Mordilnia.
endif
~
>speech_prog p about loring lorings
if ispc($n)
  say This is the home of Loring, the Arch-Druid.
  say Be careful, $n.
  say He has a loyal following and utilizes many tricks!
endif
~
>speech_prog p where tyrsis federation feds
if ispc($n)
  say Tyrsis is located northeast of the Silver River, past the Shire.
endif
~
>speech_prog p about tyrsis federation feds
if ispc($n)
  say Tyrsis is a Federation-held city, so you better watch your back.
  say I hear they are very suspicious of adventurers.
  say Many have been arrested as spies, so make sure you know a way out
  say of the city besides the main gate, or you might not be let out!
endif
~
>speech_prog p where viking vikings hamlet
if ispc($n)
   say The Viking Hamlet can be found to the northeast of Camelot.
endif
~
>speech_prog p about viking vikings hamlet
if ispc($n)
  say Well, those vikings are not too keen about allowing outsiders roam free in their lands,
  say although it is said that their shops hold many interesting items from across the seas,
  say so it may be worthwhile to visit their hamlet.
endif
~
>speech_prog p where asgard thor friga odin
if ispc($n)
  say Asgard is rumored to be found over the rainbow.
  stare
endif
~
>speech_prog p about asgard thor friga odin
if ispc($n)
  say Only the experienced should dare enter the realm of the viking gods.
  say If you do, and are succesful, you can surely expect to carry back
  say magical arms and armor beyond what you have ever dreamed!
endif
~
>speech_prog p where cloud cloudgiant cloudgiants gorn
if ispc($n)
  say The fortress and city of the cloud giants is located north of
  say the Great Road, in the far northeast corner of Ranke high in
  say the sky above the mountains.
endif
~
>speech_prog p about cloud cloudgiant cloudgiants gorn
if ispc($n)
  say Those giants are fearsome warriors, not to be trifled with!
endif
~
>speech_prog p where new thalos newthalos
if ispc($n)
  smile
  say The prosperous city of New Thalos lies beyond the Eastern Mountains, along the Ishtar River.
endif
~
>speech_prog p about new thalos newthalos
if ispc($n)
  say New Thalos!  $n, you do not know of this prosperous city?
  say It is the largest city in Eastern Ranke!
  say It has a thriving commercial area, and sells wonders beyond belief.
  say It also serves as a starting point for the most daring adventures.
endif
~
>speech_prog p where moria
if ispc($n)
  shiver
  say The fabled mines of Moria are northeast of the Shire.
  say But be wary, $n.
  say Many evil beings guard this gateway through the Eastern Mountains.
endif
~
>speech_prog p about moria
if ispc($n)
  ponder
  say Moria is an evil place, and has a long history, most of which has been forgotten through time.
  say Now, however, the occasional brave adventurer passes through its depths either
  say to slay the evil within, or to find passage through the formidable Eastern Mountains.
endif
~
>speech_prog p where desert nomad nomads camp
if ispc($n)
  say It is said that a band of nomads roam the great desert,
  say and make camp at one of the many oases in the region.
endif
~
>speech_prog p about desert nomad nomads camp
if ispc($n)
  say The desert nomads are not a friendly lot.
  say Be wary should you find yourself crossing their path.
  say They hold a store of exotic items, however.
endif
~
>speech_prog p where desert caverns
if ispc($n)
  say I have only passed that way but once,
  say and only by accident after the ravine of Kaus Cavell almost took my life.
  say A small ledge halfway down the ravine provides entrance to the lanbyrinth.
endif
~
>speech_prog p about desert caverns
if ispc($n)
  say I spent not long in those depths, however I encountered several nasty beasts,
  say who had more than their share of gold and wealth.
endif
~
>speech_prog p where drow drows
if ispc($n)
  say shiver
  say The city of the drows and the perverted shrine to their spider goddess
  say can be found deep under the Eastern Mountains.
  say At the bottom of a deep canyon within those peaks is where you can
  say find the entrance to their hidden city.
endif
~
>speech_prog p about drow drows
if ispc($n)
  say The drow elves are of the purest evil.
  say They dwell in their dark city, plotting only overthrow of goodness.
  say They are also continually waging war against their sworn enemies, the gnomes.
endif
~
>speech_prog p where old thalos oldthalos
if ispc($n)
  say This once great city now lies abandoned, deep within the great desert,
  say south of the now thriving city of New Thalos.
endif
~
>speech_prog p about old thalos oldthalos
if ispc($n)
  sigh
  say Alas, the fabled walls of Thalos the Old were breached and turned to
  say rubble three hundred years ago by the Beholder and his army of Lamias
  say after a seventeen year seige.  The defenders were butchered, and the
  say few survivors journeyed north in the Great Exodus to eventually found
  say New Thalos.  The Beholder still rules in this once mighty city, now in ruins.
endif
~
>speech_prog p where pyramid
if ispc($n)
  say The pyramid lies somewhere deep within the Great Desert.
  say Many brave adventurers have died of hunger and thirst before ever finding this
  say mysterious artifact, and mirages abound.
endif
~
>speech_prog p about pyramid
if ispc($n)
  say I myself have not dared tread foot upon that sacred edifice.
  say Although I hear the rewards are great for those who succeed in their quest.
  say Many hidden treasures still lie buried in that tomb.
endif
~
>speech_prog p where haon-dor haondor forest
if ispc($n)
  smile
  say $n, that great forest lies just outside the west gate of Sanctuary.
  say It's twisting forest trails lead paths to many exciting places.
endif
~
>speech_prog p about haon-dor haondor forest
if ispc($n)
  say The Haon-Dor is home to a variety of animal life, and even a few magical beings.
  say Wolves prowl in the dark fringes of the forest.
endif
~
>speech_prog p where wild grove dryad dryads
if ispc($n)
  say The wild grove can be reached via a freshly cut path,
  say north of the forest trail through the Haon-Dor.
endif
~
>speech_prog p about wild grove dryad dryads
if ispc($n)
  say Many woodland creatures live in these secluded woods.
  say Several forest dryads have also been occasionally spotted.
endif
~
>speech_prog p where quickling quicklings
if ispc($n)
  say The quicklings have secluded themselves deep within a gnarled forest.
  say There is a way to reach them via the Dark River, southwest of Sanctuary,
  say or you can try the Haon-Dor itself.
endif
~
>speech_prog p about quickling quicklings
if ispc($n)
  say Cyrathen says:
  say The quicklings are a dangerous folk.
  say They despise any race but their own, and keep a sharp watch on their borders.
  say They carry several magical items of interest, however, but ask no questions before attacking.
endif
~
>speech_prog p where souk malacca merchant merchants
if ispc($n)
  say Ahh, so you wish to travel to this rich, exotic seaport, $n?
  say Well, it lies not on Ranke, but on another continent far to the east.
  say You may take passage on a ship on New Thalos which makes frequent journeys there to trade.
endif
~
>speech_prog p about souk malacca merchant merchants
if ispc($n)
  say Souk is a wonderful city, one of my very favorites.
  say It is a city of merchants, of buying and selling.
  say Some here would sell their own mothers for a brick of gold.
  say Mind your own business, and you won't get hassled.
  say Be on the lookout, though, for the occasional thief.
endif
~
>speech_prog p where dwarven dwarf dwarves bolfin
if ispc($n)
  say The dwarven kingdom is deep within a tall mountain, north of Sanctuary.
  say Follow the dusty trail around the city wall, then head north.
endif
~
>speech_prog p about dwarven dwarf dwarves bolfin
if ispc($n)
  say It has been reported that the mighty dwarven kingdom is home to the
  say best armor and weaponsmiths in the world.
  say Unfortunately, with the attack of the enfans upon the city of Orshingal to the north,
  say King Bolfin has sealed the doors to his realm.
  say Perhaps once the menace in Orshingal has lessened, the gates will again be reopened.
endif
~
>speech_prog p where camelot arthur launcelot mordred gawain
if ispc($n)
  say To get to Camelot requires a winding journey through the Haon-Dor Forest.
  say If you stay to the path as westward as possible, though,
  say I am certain you will eventually come upon the King's Road, northwest of the forest.
endif
~
>speech_prog p about camelot arthur launcelot mordred gawain
if ispc($n)
  say The legend of Arthur and Launcelot lives on here.
  say Merlin and Morganna, say Mordred and Gawain, can all be found within the kingdom.
  say The knights of the round table are valiant warriors, and honorable.
endif
~
>speech_prog p where prydain merlin morganna
if ispc($n)
  say Prydain is north of Camelot.
endif
~
>speech_prog p about prydain merlin morganna
if ispc($n)
  say Prydain has seen some interesting developments if late, but watch your purse!
endif
~
>speech_prog p where antipaladin anti-paladin antipaladins anti-paladins
if ispc($n)
  emote scratches his beard thoughtfully.
  say I have chosen to stay far away from this dark realm, for personal reasons,
  say although I hear they are not too far away at all from their antipodes, the Paladins.
endif
~
>speech_prog p about antipaladin anti-paladin antipaladins anti-paladins
if ispc($n)
  say I have heard that the antipaladins are building up fortifications for their home.
  say Also, horrid creations of their Master roam the halls of their guild, looking for would be invaders.
endif
~
>speech_prog p where ice haven icehaven
if ispc($n)
  say Ice Haven, and the far off continent of The Cold Land
  say can be reached by ship off of Sanctuary's southern pier.
endif
~
>speech_prog p about ice haven icehaven
if ispc($n)
  shiver
  say So you want to know about Ice Haven, $n?
  say It is the only outpost of human civilization upon that frigid continent of The Cold Land.
  say A powerful man runs the town, but the citizens are more worried about survival than politics.
endif
~
>speech_prog p where astral gith githezai
if ispc($n)
  chuckle
  say The astral plane?
  laugh
  say Well look around you, $n!  Can't you see it?  Of course you can't!
  say It is there nonetheless, coexisting with our own material plane but in a separate dimension.
  say The only means of getting there is through powerful magics.
  say Some are able to weave this spell, but for those who can't,
  say a shopkeeper in New Thalos sells a magical fruit!
endif
~
>speech_prog p about astral gith githezai
if ispc($n)
  raise
  say The astral plane is a dangerous place, $n.
  say Some of the most powerful creatures in existence roam its grey landscape.
  say They also carry some of the most powerful items ever created.
endif
~
>speech_prog p where valley ch'tar chtar
if ispc($n)
  say The Valley of Ch'Tar can be found east of Mirkwood, and west of the Great Road.
endif
~
>speech_prog p about valley ch'tar chtar
if ispc($n)
  say I am sorry, $n, but this valley has been shrouded in mystery for many years,
  say you will have to go discover its secrets for yourself.
endif
~
>speech_prog p where lycanthropia lycanthropes lycanthrope
if ispc($n)
  say Lycanthropia is southeast of Sanctuary.
  say To get there, head south from the crossroads east of town.
endif
~
>speech_prog p about lycanthropia lycanthropes lycanthrope
if ispc($n)
  say Lycanthropia is home to a variety of shapeshifters,
  say although most generally tend to take the form of werebeasts.
endif
~
>speech_prog p where mordilnia mid
if ispc($n)
  say Mordilnia is southwest of Sanctuary, south through the Haon-Dor Forest. 
endif
~
>speech_prog p about mordilnia mid
if ispc($n)
  say Ahh, Mordilnia.  Now that is a lively town.
  say There you can find the mid-level guildmasters, as well as several trade establishments.
endif
~
>speech_prog p where mistamere
if ispc($n)
  say Castle Mistamere is near Tyrsis, north of the Silver River.
endif
~
>speech_prog p about mistamere
if ispc($n)
  say I know little of that dark castle, since it was taken over by humanoids.
endif
~
>speech_prog p where arachnos spider spiders
if ispc($n)
  say Arachnos is somewhere in Mirkwood, east of Moria, and north of the Gypsy swamp.
endif
~
>speech_prog p about arachnos spider spiders
if ispc($n)
  say Arachnos is home to a variety of nasty creatures, although some of them are not as evil as others.
  say Be wary if you tread there.
endif
~
>speech_prog p where orshingal enfan enfans
if ispc($n)
  say Orshingal is north of Sanctuary, beyond the dwarven kingdom.
endif
~
>speech_prog p about orshingal enfan enfans
if ispc($n)
  frown
  say I wish I had good tidings to report of that town, $n.
  say Unfortunately, evil enfans have ransacked the once peaceful city, and now ravage the town.
  say Perhaps you will be the one to once and for all quench this wicked threat.
endif
~
>speech_prog p where troglodytes troglodyte caves
if ispc($n)
  say The caves of the troglodytes are far from Ranke,
  say upon the ancient continent of Sherranpip near an ancient road.
endif
~
>speech_prog p about troglodytes troglodyte caves
if ispc($n)
  say The caves of the troglodytes are rumored to carry many valuable weapons and armor.
  say They guard their treasures and their lands with an almost religious fervor, though.
  say Beware, $n.
endif
~
>speech_prog p where sauria dinosaur dinosaurs
if ispc($n)
  say This land out of time is rumored to actually be somewhere in Sherranpip,
  say beyond the crystal mist of a tall waterfall.
  say I have not yet confirmed this, however.
endif
~
>speech_prog p about sauria dinosaur dinosaurs
if ispc($n)
  say Sauria is the home to reptilian beasts from ages past.
  say Their minds might be small, but beware their bite!
endif
~
>speech_prog p where cthulhu dagon deepone deepones deep innsmouth
if ispc($n)
  say Cthulhu and the evil cult of Dagon have dug themselves in near and
  say around the town of Innsmouth, on the southwest shore of Ranke,
  say beyond the Haon-Dor Forest.
endif
~
>speech_prog p about cthulhu dagon deepone deepones deep innsmouth
if ispc($n)
  say It is rumored that Cthulhu has taken another coming into this world.
  say To stop him is to know great honor.
endif
~
>speech_prog p where deadhame zombie zombies
if ispc($n)
  say A twisted road to this perverted place can be found outside the northeast wall of Sanctuary.
endif
~
>speech_prog p about deadhame zombie zombies
if ispc($n)
  say The undead who inhabit deadhame are at times quite eccentric.
  say Tread lightly if you journey that way.
endif
~
>speech_prog p where abyss demi demilich demi-lich kalas
if ispc($n)
  say The abyss offers many entrances into its dark labyrinth, for those daring enough to enter, $n.
endif
~
>speech_prog p about abyss demi demilich demi-lich kalas
if ispc($n)
  say This area is the home of such feared beings as The demi-lich and Kalas.
  say To enter is to indeed flaunt ones bravery.
  say Unique and mystical items have been rumored to be found on the creatures of this plane.
endif
~
>speech_prog p where paladin paladins
if ispc($n)
  say Cyrathen says:
  say The paladin stonghold can be found on the way to Camelot, down a
  say golden path.
endif
~
>speech_prog p about paladin paladins
if ispc($n)
  say The mighty Paladins of Ranke ABHOR Evil, and will slay evil beings on sight!
  look $n
  if isgood($n)
    say You shouldn't have a problem, $n, for you seem pure of heart.
  else
    ponder
    say You may want to kill a few orcs before you try going there, $n.
  endif
endif
~
>speech_prog p where fire giant firegiant firegiants
if ispc($n)
  say The fire giant castle can be found north of Orshingal, through some treacherous mountains.
endif
~
>speech_prog p about fire giant firegiant firegiants
if ispc($n)
  say The fire giants defend their castle against intruders vehemently.
  say The surrounding areas are also said to harbor a variety of fiery creatures and powerful items.
endif
~
>speech_prog p where gnome gnomes
if ispc($n)
  say The gnome caves are rumored to lie down a deep mineshaft in the Eastern Mountains.
  say I recall being able to hear the odd noises of their mining long
  say before I found the entrance to their lands.
endif
~
>speech_prog p about gnome gnomes
if ispc($n)
  say The gnomes are a hardy breed, and spend most of their time mining precious metals from the earth.
  say Their leaders are reported to carry some intriguing items.
  say The gnomes are also under a constant state of war with their sworn enemies, the darkelves.
endif
~
>speech_prog p where sea elf seaelf seaelves silverbow saughin
if ispc($n)
  say The sea elf kingdom lies deep beneath the surface of the ocean, south of Ranke.
  say Reportedly, Mordilnia is the nearest surface city to it. 
endif
~
>speech_prog p about sea elf seaelf seaelves silverbow saughin
if ispc($n)
  say The sea elves are an ancient race, who now dwell underwater.
  say Their people are mighty warriors, who are constantly struggling with the say evil saughin warriors.
  say The fabled Silverbow, repairman of weaponry, is also known to reside here.
endif
~
>rand_prog 20
if ispc($r)
  if rand(50)
    if rand(50)
      say The world of Ranke is vast.
      say And yet, I have been lucky enough to have seen much of it.
    else
      emote stares at a large hand-drawn map on the wall.
      sigh
    endif
  else
    if rand(50)
      emote polishes a magnifying glass.
    else
      say The world lays before you, young adventurer.
      say What are YOU searching for?
    endif
  endif
endif
~
@
