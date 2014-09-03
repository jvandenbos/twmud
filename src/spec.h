#ifndef SPEC_H
#define SPEC_H

/* Types of special calls */
#define SPEC_INIT	0
#define SPEC_CMD	1
#define SPEC_IDLE	2
#define SPEC_FIGHT	3

/*  structures */
struct memory {
  short pointer;
  char **names;
  int *status;
  short index;
  short c;
};

void breath_weapon(struct char_data* ch, int mana_cost, breath_func func);
char* how_good(int percent);
struct char_data* FindMobInRoomWithFunction(int room, spec_proc_func func);
int affect_status(struct memory *mem, struct char_data* ch,
		  struct char_data* t, int aff_status);
int GuildMaster(struct char_data* ch, int cmd, char* arg,
		int clss, struct char_data* mast);

/* mob special procs */
SPECIAL(generic_warrior);

SPECIAL(blocker);
SPECIAL(Blocker);
SPECIAL(liquid_proc);
SPECIAL(cocoon_proc);
SPECIAL(erok);
SPECIAL(ClassMaster);
SPECIAL(practice_master);
SPECIAL(bounty_hunter);
SPECIAL(fire_elemental);
SPECIAL(water_elemental);
SPECIAL(earth_elemental);
SPECIAL(air_elemental);
SPECIAL(archer);
SPECIAL(cityguard);
SPECIAL(ThalosGuildGuard);
SPECIAL(SultanGuard);
SPECIAL(NewThalosCitzen);
SPECIAL(NewThalosMayor);
SPECIAL(MordGuard);
SPECIAL(warrior_blocker);
SPECIAL(StatTeller);
SPECIAL(ThrowerMob);
SPECIAL(Demon);
SPECIAL(Devil);
SPECIAL(Inquisitor);
SPECIAL(temple_labrynth_liar);
SPECIAL(AbyssGateKeeper);
SPECIAL(temple_labrynth_sentry);
SPECIAL(NudgeNudge);
SPECIAL(RustMonster);
SPECIAL(PaladinGuildGuard);
SPECIAL(GameGuard);
SPECIAL(GreyParamedic);
SPECIAL(AmberParamedic);
SPECIAL(tormentor);
SPECIAL(receptionist);
SPECIAL(MageGuildMaster);
SPECIAL(ThiefGuildMaster);
SPECIAL(ClericGuildMaster);
SPECIAL(WarriorGuildMaster);
SPECIAL(PaladinGuildMaster);
SPECIAL(DruidGuildMaster);
SPECIAL(PsiGuildMaster);
SPECIAL(RangerGuildMaster);
SPECIAL(guild_guard);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(mayor);
SPECIAL(eric_johnson);
SPECIAL(andy_wilcox);
SPECIAL(zombie_master);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(magic_user2);
SPECIAL(generic_cleric);
SPECIAL(old_generic_cleric);
SPECIAL(ghoul);
SPECIAL(vampire);
SPECIAL(wraith);
SPECIAL(shadow);
SPECIAL(geyser);

SPECIAL(green_slime);
SPECIAL(BreathWeapon);
SPECIAL(DracoLich);
SPECIAL(Drow);
SPECIAL(Leader);
SPECIAL(MidgaardCitizen);
SPECIAL(NewThalosMayor);
SPECIAL(NewThalosCitizen);
SPECIAL(citizen);
SPECIAL(SultanGuard);
SPECIAL(ninja_master);
SPECIAL(WizardGuard);
SPECIAL(Tytan);
SPECIAL(replicant);
SPECIAL(regenerator);
SPECIAL(blink);
SPECIAL(RepairGuy);
SPECIAL(WeaponGuy);
SPECIAL(Ringwraith);
SPECIAL(sisyphus);
SPECIAL(jabberwocky);
SPECIAL(flame);
SPECIAL(banana);
SPECIAL(paramedics);
SPECIAL(jugglernaut);
SPECIAL(delivery_elf);
SPECIAL(delivery_beast);
SPECIAL(Keftab);
SPECIAL(StormGiant);
SPECIAL(Kraken);
SPECIAL(Manticore);
SPECIAL(fighter);
SPECIAL(AGGRESSIVE);
SPECIAL(CarrionCrawler);

SPECIAL(guardian);
SPECIAL(lattimore);
SPECIAL(coldcaster);
SPECIAL(trapper);
SPECIAL(keystone);
SPECIAL(ghostsoldier);
SPECIAL(troguard);
SPECIAL(shaman);
SPECIAL(golgar);
SPECIAL(trogcook);
SPECIAL(web_slinger);
SPECIAL(lone_troll);
SPECIAL(attack_evil);
SPECIAL(astral_pool);
SPECIAL(tyrsis_magician);
SPECIAL(mob_hero_ring);
SPECIAL(herohunter);

SPECIAL(possession_mob);
SPECIAL(dungeon_master);
SPECIAL(vampire_ss);
SPECIAL(avatar_mallune);

/* objects */
SPECIAL(nodrop);
SPECIAL(soap);
SPECIAL(astral_berry);
SPECIAL(gate_proc);
SPECIAL(mana_regen);
SPECIAL(mirror);
SPECIAL(stones);
SPECIAL(obj_hero_ring);
SPECIAL(genie_lamp);
SPECIAL(portal);

/* rooms */
SPECIAL(dump);
SPECIAL(to_donation);
SPECIAL(chalice);
SPECIAL(kings_hall);
SPECIAL(pet_shops);
SPECIAL(morgue_shops);
SPECIAL(pray_for_items);
SPECIAL(bank);
SPECIAL(House);
SPECIAL(Fountain);
SPECIAL(Donation);
SPECIAL(metahospital);
SPECIAL(pawnshop);
SPECIAL(storeroom);
SPECIAL(guild_bank);

SPECIAL(mindflayer);
SPECIAL(firecaster);
SPECIAL(eleccaster);

SPECIAL(BirdKing);
SPECIAL(N_Sonic_Wall);

/*Volcano */
SPECIAL(small_fire_caster);

/* Alkian burial zone specs */
SPECIAL(acidcaster);
SPECIAL(poisoncaster);
SPECIAL(energycaster);
SPECIAL(generic_alkian);
SPECIAL(alkian_warrior);
SPECIAL(alkian_fetch);
SPECIAL(alkian_holly);
SPECIAL(alkian_demonspawn);
SPECIAL(alkian_undertaker);
SPECIAL(alkian_master);

/* the deep */
SPECIAL(deep_warrior);

/* clan keep specs (quilan project)*/
SPECIAL(clan_altar);
SPECIAL(clan_restring_shop);
SPECIAL(clan_donation_shop);

/* More clan coolness */
SPECIAL(clan_forger);

/* initialization routines */
void assign_mobiles(void);
void assign_rooms(void);
void assign_objects(void);

/* utilities */
void give_reimb_items(struct char_data* ch, int level);
void npc_steal(struct char_data *ch,struct char_data *victim);


#endif
