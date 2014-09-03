#ifndef ACT_H
#define ACT_H

#define SHOW_EXP_TO_LEVEL(name, idx) if(GET_LEVEL(ch,idx)>0) { sprintf(buf2,#name "$CG:%Ld$CN ", exp_table[GET_LEVEL(ch, idx)] - GET_EXP(ch)); strcat(buf,buf2);   }


int shifter_form_test(int position);
int shifter_plate_test(int position);
int try_to_get_object(struct char_data *ch, struct obj_data *object);
void show_score(struct char_data *ch, struct char_data *to);
void show_attr(struct char_data *ch, struct char_data *to);
void show_affect(struct char_data *ch, struct char_data *to);
void shifter_normalize(struct char_data *ch);
void trans_affects(struct char_data *from, struct char_data *to);
void stop_all_followers(struct char_data *ch);
void hide_test(struct char_data *ch, int cmd);
void list_obj_in_heap(struct obj_data *list, struct char_data *ch);
void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode);

void do_absolve(struct char_data *ch, char *arg, int cmd);
void do_action(struct char_data *ch, char *arg, int cmd);
void do_adrenalize(struct char_data *ch, char *arg, int cmd);
void do_advance(struct char_data *ch, char *arg, int cmd);
void do_affect(struct char_data *ch, char *arg, int cmd);
void do_afk(struct char_data *ch, char *arg, int cmd);
void do_alias(struct char_data *ch, char *arg, int cmd);
void do_allow(struct char_data *ch, char *arg, int cmd);
void do_animal_friendship(struct char_data *ch, char *arg, int cmd);
void do_ansum(struct char_data* ch, char* argument, int cmd);
void do_appraise(struct char_data* ch, char* argument, int cmd);
void do_ask(struct char_data *ch, char *arg, int cmd);
void do_assist(struct char_data *ch, char *arg, int cmd);
void do_at(struct char_data *ch, char *arg, int cmd);
void do_attribute(struct char_data *ch, char *arg, int cmd); /* jdb 11-6 */
void do_auth(struct char_data *ch, char *arg, int cmd); /* jdb 3-1 */
void do_backstab(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data* ch, char* argument, int cmd);
void do_beep(struct char_data *ch, char *arg, int cmd);
void do_berserk(struct char_data *ch, char *arg, int cmd);
void do_bet(struct char_data *ch, char *arg, int cmd);
void do_bind(struct char_data *ch, char *arg, int cmd);
void do_blessing(struct char_data *ch, char *arg, int cmd);
void do_blur(struct char_data *ch, char *argument, int cmd);
void do_brainstorm(struct char_data *ch, char *arg, int cmd);
void do_breath(struct char_data *ch, char *arg, int cmd);
void do_brew(struct char_data *ch, char *arg, int cmd);
void do_bug(struct char_data *ch, char *arg, int cmd);
//void do_builder_stat(struct char_data *ch, char *argument, int cmd);
void do_canibalize(struct char_data *ch, char *arg, int cmd);
void do_cast(struct char_data *ch, char *arg, int cmd);
void do_chameleon(struct char_data *ch, char *argument, int cmd);
void do_charge_elements(struct char_data *ch, char *argument, int cmd);
void do_circle(struct char_data *ch, char *arg, int cmd);
void do_close(struct char_data *ch, char *arg, int cmd);
void do_cls(struct char_data *ch, char *arg, int cmd);
void do_cocoon(struct char_data *ch, char *argument, int cmd);
void do_color(struct char_data *ch, char *arg, int cmd);
void do_comm(struct char_data* ch, char* argument, int cmd);
void do_commands(struct char_data *ch, char *arg, int cmd);
void do_compare(struct char_data *ch, char *arg, int cmd);
void do_contract(struct char_data *ch, char *arg, int cmd);
void do_consider(struct char_data *ch, char *arg, int cmd);
void do_create(struct char_data *ch, char *arg, int cmd); /* smg 6-7 */
void do_credits(struct char_data *ch, char *argument, int cmd);
void do_creeping_doom(struct char_data* ch, char* argument, int cmd);
void do_csocket(struct char_data *ch, char *arg, int cmd);
void do_debug(struct char_data *ch, char *arg, int cmd);
void do_deny(struct char_data *ch, char *arg, int cmd);
/* void do_dir(struct char_data *ch, char *argument, int cmd); */
void do_disarm(struct char_data* ch, char* argument, int cmd);
void do_dispel(struct char_data* ch, char* arg, int cmd);
void do_donate(struct char_data *ch, char *arg, int cmd);
void do_doorbash(struct char_data *ch, char *arg, int cmd);
void do_dup(struct char_data *ch, char *arg, int cmd);
void do_gateway(struct char_data *ch, char *arg, int cmd);
void do_drain_mana(struct char_data *ch, char *arg, int cmd);
void do_drink(struct char_data *ch, char *arg, int cmd);
void do_drop(struct char_data* ch, char* argument, int cmd);
void do_eat(struct char_data* ch, char* argument, int cmd);
void do_echo(struct char_data *ch, char *argument, int cmd);
void do_edit(struct char_data *ch, char *arg, int cmd); /* jdb 9-29 */
void do_emote(struct char_data* ch, char* argument, int cmd);
void do_enter(struct char_data *ch, char *arg, int cmd);
void do_equipment(struct char_data *ch, char *argument, int cmd);
void do_events(struct char_data *ch, char *argument, int cmd);
void do_examine(struct char_data *ch, char *arg, int cmd);
void do_exit(struct char_data *ch, char *argument, int cmd);
void do_exits(struct char_data *ch, char *arg, int cmd);
void do_expset(struct char_data *ch, char *arg, int cmd);
void do_farlook(struct char_data *ch, char *arg, int cmd);
void do_fedit(struct char_data *ch, char *arg, int cmd);
void do_feint(struct char_data *ch, char *arg, int cmd);
void do_fury(struct char_data *ch, char *arg, int cmd);// 02/04/05 by mtr
void do_fiddle(struct char_data *ch, char *arg, int cmd); /* lestat */
void do_fill(struct char_data *ch, char *arg, int cmd); /* jdb 2-9 */
void do_find(struct char_data *ch, char *arg, int cmd);
void do_fire(struct char_data* ch, char* argument, int cmd);
void do_flags(struct char_data *ch, char *arg, int cmd);
void do_fsave(struct char_data *ch, char *arg, int cmd);
void do_illusionary_shroud(struct char_data* ch, char* argument, int cmd);
void do_inscribe_spellbook(struct char_data *ch, char *argument, int cmd);
void do_flee(struct char_data* ch, char* argument, int cmd);
void do_follow(struct char_data *ch, char *arg, int cmd);
void do_force(struct char_data* ch, char* argument, int cmd);
void do_form(struct char_data* ch, char* argument, int cmd);
void do_freeze(struct char_data *ch, char *arg, int cmd);
void do_friends(struct char_data *ch, char *arg, int cmd);
void do_gain(struct char_data *ch, char *arg, int cmd); /* jdb 1-19 */
void do_gateway(struct char_data *ch, char *arg, int cmd);
void do_generate(struct char_data *ch, char *arg, int cmd);
void do_get(struct char_data* ch, char* argument, int cmd);
void do_give(struct char_data *ch, char *arg, int cmd);
void do_gills(struct char_data *ch, char *arg, int cmd);
void do_gold(struct char_data *ch, char *arg, int cmd);
void do_goto(struct char_data *ch, char *arg, int cmd);
void do_grab(struct char_data *ch, char *argument, int cmd);
void do_great_sight(struct char_data *ch, char *arg, int cmd);
void do_group(struct char_data *ch, char *arg, int cmd);
void do_group_attack(struct char_data *ch, char *arg, int cmd);
void do_guard(struct char_data *ch, char *arg, int cmd);
void do_help(struct char_data *ch, char *argument, int cmd);
void do_helptopics(struct char_data *ch, char *arg, int cmd);
void do_heroic_rescue(struct char_data *ch, char *arg, int cmd);
void do_herolist(struct char_data *ch, char *arg, int cmd);
void do_hi(struct char_data *ch, char *arg, int cmd);
void do_hide(struct char_data* ch, char* argument, int cmd);
void do_highfive(struct char_data *ch, char *arg, int cmd); /* jdb 10-30 */
void do_hit(struct char_data *ch, char *argument, int cmd);
void do_holy_warcry(struct char_data* ch, char* argument, int cmd);
void do_home(struct char_data *ch, char *arg, int cmd);
void do_terror(struct char_data *ch, char *argument, int cmd);
void do_hunt(struct char_data *ch, char *arg, int cmd);
void do_hypnosis(struct char_data *ch, char *arg, int cmd);
void do_idea(struct char_data *ch, char *arg, int cmd);
void do_ident(struct char_data *ch, char *argument, int cmd);
void do_info(struct char_data *ch, char *arg, int cmd);
void do_instazone(struct char_data *ch, char *arg, int cmd); /* jdb 12-3 */
void do_insult(struct char_data *ch, char *argument, int cmd);
void do_inventory(struct char_data *ch, char *argument, int cmd);
void do_invis(struct char_data *ch, char *arg, int cmd);
void do_invisibility(struct char_data *ch, char *arg, int cmd);
void do_judge(struct char_data *ch, char *arg, int cmd);
void do_junk(struct char_data *ch, char *arg, int cmd); /* jdb 12-17 */
void do_kick(struct char_data* ch, char* argument, int cmd);
void do_kil(struct char_data *ch, char *arg, int cmd);
void do_kill(struct char_data *ch, char *argument, int cmd);
void do_laston(struct char_data *ch, char *argument, int cmd);
void do_lay_on_hands(struct char_data* ch, char* argument, int cmd);
void do_leave(struct char_data *ch, char *arg, int cmd);
void do_levels(struct char_data *ch, char *arg, int cmd);
void do_levitate(struct char_data *ch, char *arg, int cmd);
void do_limb(struct char_data *ch, char *arg, int cmd);
void do_load(struct char_data* ch, char* argument, int cmd);
void do_lock(struct char_data* ch, char* argument, int cmd);
void do_logall(struct char_data* ch, char* argument, int cmd);
void do_look(struct char_data* ch, char* argument, int cmd);
void do_lose(struct char_data* ch, char* argument, int cmd);
void do_mail_check(struct char_data *ch, char *arguments, int cmd);
void do_mail_receive(struct char_data* ch, char *arguments, int cmd);
void do_mail_send(struct char_data *ch, char *arguments, int cmd);
void do_make(struct char_data *ch, char *arg, int cmd);
void do_mask(struct char_data *ch, char *arg, int cmd);
void do_meditation(struct char_data *ch, char *arg, int cmd);
void do_mkick(struct char_data *ch, char *arg, int cmd);
void do_melt(struct char_data *ch, char *arg, int cmd);
void do_more(struct char_data* ch, char* arg, int cmd);
void do_mount(struct char_data *ch, char *arg, int cmd);
void do_move(struct char_data* ch, char* argument, int cmd);
void do_myspells(struct char_data *ch, char *arg, int cmd);
void do_mysongs(struct char_data *ch, char *arg, int cmd);
void do_name(struct char_data* ch, char* argument, int cmd);
void do_news(struct char_data *ch, char *argument, int cmd);
void do_nohassle(struct char_data *ch, char *argument, int cmd);  /* jdb 9-6 */
void do_noshout(struct char_data *ch, char *argument, int cmd);
void do_not_here(struct char_data *ch, char *argument, int cmd);
void do_offer(struct char_data *ch, char *arg, int cmd);
void do_ongoing_mana(struct char_data* ch, char* arg, int cmd);
void do_open(struct char_data *ch, char *arg, int cmd);
void do_opstat(struct char_data *ch, char *arg, int cmd);
void do_points(struct char_data *ch, char *arg, int cmd);
void do_order(struct char_data *ch, char *arg, int cmd);
void do_page(struct char_data *ch, char *arg, int cmd);
void do_palm(struct char_data *ch, char *arg, int cmd);
void do_passwd(struct char_data *ch, char *arg, int cmd); /* jdb 2-6 */
void do_peek(struct char_data *ch, char *arg, int cmd);
void do_phantasmal_killer(struct char_data *ch, char *arg, int cmd);
void do_pick(struct char_data *ch, char *arg, int cmd);
void do_pinch(struct char_data* ch, char* argument, int cmd);
void do_pkill(struct char_data *ch, char *arg, int cmd);
void do_plate(struct char_data *ch, char *arg, int cmd);
void do_player(struct char_data *ch, char *arg, int cmd);
void do_plrtog(struct char_data *ch, char *arg, int cmd);
void do_policy(struct char_data *ch, char *argument, int cmd);
void do_poofin(struct char_data *ch, char *arg, int cmd);
void do_poofout(struct char_data *ch, char *arg, int cmd);
void do_pose(struct char_data *ch, char *argument, int cmd);
void do_pour(struct char_data *ch, char *arg, int cmd);
void do_practice(struct char_data *ch, char *arg, int cmd);
void do_profile(struct char_data *ch, char *arg, int cmd);
void do_psi_attack(struct char_data* ch, char* argument, int cmd);
void do_pull(struct char_data *ch, char *argument, int cmd);  /* jdb 9-16 */
void do_purge(struct char_data *ch, char *arg, int cmd);
void do_push(struct char_data *ch, char *arg, int cmd);
void do_put(struct char_data *ch, char *argument, int cmd);
void do_quaff(struct char_data *ch, char *argument, int cmd);
void do_questup(struct char_data *ch, char *argument, int cmd);
void do_qui(struct char_data *ch, char *argument, int cmd);
void do_quick_draw(struct char_data *ch, char *argument, int cmd);
void do_quit(struct char_data *ch, char *argument, int cmd);
void do_ration(struct char_data *ch, char *arg, int cmd);
void do_read(struct char_data *ch, char *argument, int cmd);
void do_read_motd(struct char_data *ch, char *arg, int cmd);
void do_read_imotd(struct char_data *ch, char *arg, int cmd);
void do_recite(struct char_data *ch, char *argument, int cmd);
void do_reimburse(struct char_data *ch, char *argument, int cmd);
void do_remort(struct char_data *ch, char *arg, int cmd);
void do_remove(struct char_data* ch, char* argument, int cmd);
void do_rent(struct char_data *ch, char *arg, int cmd);
void do_reply(struct char_data *ch, char *arg, int cmd);
void do_report(struct char_data *ch, char *arg, int cmd);
void do_reroll(struct char_data *ch, char *arg, int cmd);
void do_rescue(struct char_data* ch, char* argument, int cmd);
void do_rest(struct char_data* ch, char* argument, int cmd);
void do_restore(struct char_data* ch, char* argument, int cmd);
void do_retreat(struct char_data *ch, char *argument, int cmd);
void do_return(struct char_data* ch, char* argument, int cmd);
void do_rload(struct char_data *ch, char *arg, int cmd); /* jdb 10-5 */
void do_rpstat(struct char_data *ch, char *arg, int cmd);
void do_rsave(struct char_data *ch, char *arg, int cmd); /* jdb 10-5 */
void do_run(struct char_data *ch, char *arg, int cmd);
void do_sample(struct char_data *ch, char *arg, int cmd);
void do_save(struct char_data *ch, char *argument, int cmd);
void do_say(struct char_data* ch, char* argument, int cmd);
void do_scan(struct char_data *ch, char *arg, int cmd);
void do_scatter(struct char_data *ch, char *arg, int cmd);
void do_score(struct char_data *ch, char *argument, int cmd);
void do_scry(struct char_data *ch, char *arg, int cmd);
void do_search(struct char_data *ch, char *arg, int cmd);
void do_set(struct char_data *ch, char *arg, int cmd); /* jdb 9-29 */
void do_setlog(struct char_data *ch, char *arg, int cmd);
void do_shift(struct char_data *ch, char *argument, int cmd);
void do_shout(struct char_data* ch, char* argument, int cmd);
void do_show(struct char_data *ch, char *arg, int cmd);
void do_shutdow(struct char_data *ch, char *arg, int cmd);
void do_shutdown(struct char_data *ch, char *argument, int cmd);
void do_silence(struct char_data *ch, char *argument, int cmd);
void do_sing(struct char_data *ch, char *arg, int cmd);
void do_sip(struct char_data *ch, char *arg, int cmd);
void do_sit(struct char_data *ch, char *argument, int cmd);
void do_sleep(struct char_data *ch, char *argument, int cmd);
void do_slownames(struct char_data *ch, char *argument, int cmd);
void do_sneak(struct char_data *ch, char *arg, int cmd);
void do_snoop(struct char_data *ch, char *argument, int cmd);
void do_spell_shield(struct char_data *ch, char *arg, int cmd);
void do_spells(struct char_data *ch, char *arg, int cmd); /* jdb 11-6 */
void do_split(struct char_data *ch, char *arg, int cmd);
void do_stand(struct char_data* ch, char* argument, int cmd);
void do_stat(struct char_data *ch, char *arg, int cmd);
void do_stats(struct char_data* ch, char* arg, int cmd);
void do_stay(struct char_data *ch, char *arg, int cmd);
void do_steal(struct char_data *ch, char *arg, int cmd);
void do_stealth(struct char_data *ch, char *argument, int cmd); /* jdb 9-17 */
void do_string(struct char_data *ch, char *arg, int cmd);
void do_stun(struct char_data* ch, char* argument, int cmd);
void do_sunray(struct char_data* ch, char* argument, int cmd);
void do_suggestion(struct char_data* ch, char* argument, int cmd);
void do_swapzone(struct char_data *ch, char *argument, int cmd);
void do_switch(struct char_data *ch, char *argument, int cmd);
void do_system(struct char_data *ch, char *argument, int cmd);
void do_tag(struct char_data *ch, char *arg, int cmd);
void do_tan(struct char_data *ch, char *arg, int cmd);
void do_taste(struct char_data *ch, char *arg, int cmd);
void do_taunt(struct char_data* ch, char* argument, int cmd);
void do_tell(struct char_data* ch, char* argument, int cmd);
void do_tell_group(struct char_data* ch, char* argument, int cmd);
void do_terrain(struct char_data *ch, char *arg, int cmd);
void do_thorn(struct char_data* ch, char* argument, int cmd);
void do_throw(struct char_data* ch, char* argument, int cmd);
void do_thrust(struct char_data *ch, char *arg, int cmd);
void do_time(struct char_data *ch, char *arg, int cmd);
void do_title(struct char_data *ch, char *arg, int cmd);
/* void do_trace(struct char_data *ch, char *arg, int cmd); */
void do_track(struct char_data *ch, char *arg, int cmd);
void do_trans(struct char_data *ch, char *argument, int cmd);
void do_teleport(struct char_data *ch, char *argument, int cmd);
void do_trip(struct char_data *ch, char *argument, int cmd);
void do_trivia(struct char_data *ch, char *argument, int cmd);
void do_twstory(struct char_data *ch, char *argument, int cmd);
void do_typo(struct char_data *ch, char *arg, int cmd);
void do_unalias(struct char_data *ch, char *arg, int cmd);
void do_unlock(struct char_data* ch, char* argument, int cmd);
//void do_uptime(struct char_data *ch, char *arg, int cmd); /* jdb 12-3 */
void do_use(struct char_data *ch, char *argument, int cmd);
void do_users(struct char_data *ch, char *arg, int cmd);
void do_viewfile(struct char_data *ch, char *arg, int cmd);
void do_wake(struct char_data* ch, char* argument, int cmd);
void do_wear(struct char_data* ch, char* argument, int cmd);
void do_weather(struct char_data *ch, char *arg, int cmd);
void do_where(struct char_data *ch, char *arg, int cmd);
void do_whisper(struct char_data *ch, char *arg, int cmd);
void do_who(struct char_data *ch, char *argument, int cmd);
void do_wield(struct char_data* ch, char* argument, int cmd);
void do_wimp(struct char_data *ch, char *argument, int cmd);
void do_wings(struct char_data *ch, char *argument, int cmd);
void do_wizhelp(struct char_data *ch, char *argument, int cmd);
void do_wizlist(struct char_data *ch, char *arg, int cmd);
void do_wizlistnew(struct char_data *ch, char *arg, int cmd);
void do_wizlock(struct char_data *ch, char *arg, int cmd);
void do_wizrep(struct char_data *ch, char *arg, int cmd);
void do_world(struct char_data *ch, char *arg, int cmd);
void do_write(struct char_data *ch, char *arg, int cmd);
void do_zap(struct char_data *ch, char *arg, int cmd); /* alex: */
void do_zexit(struct char_data *ch, char *argument, int cmd);
void do_zones(struct char_data *ch, char *arg, int cmd);
void do_zreset(struct char_data *ch, char *arg, int cmd);

int clearpath(struct char_data* ch, room_num room, int direc);
void Dismount(struct char_data* ch, struct char_data* h, int pos);

void DisplayOneMove(struct char_data* ch, int dir, int was_in);
void DisplayGroupMove(struct char_data* ch, int dir, int was_in, int total);
void DisplayMove(struct char_data* ch, int dir, int was_in, int total);

int MoveOne(struct char_data* ch, int dir);

int AddToCharHeap(struct char_data* heap[], int *top,
		  int total[], struct char_data* k);

struct char_data* get_char_dir(struct char_data* ch, const char* arg, int *rf, int dr);
struct char_data* get_char_linear(struct char_data* ch, const char* arg,
				  int* rf, int* df);
struct obj_data* get_object_in_equip_vis(struct char_data* ch, const char* arg,
					 struct obj_data *equipment[], int *j);
int can_see_linear(struct char_data* ch, struct char_data* targ,
		   int* rng, int* dir);
int can_pkill(struct char_data *ch, struct char_data *victim);

void do_start(struct char_data* ch);
void boot_social_messages(void);
void boot_pose_messages(void);

int find_door(struct char_data* ch, char* type, char* dir);
void open_door(struct char_data* ch, int dir);

void name_from_drinkcon(struct obj_data* obj);
void name_to_drinkcon(struct obj_data* obj, int type);
void weight_change_object(struct obj_data* obj, int weight);


/* For BAN code added by Min 1996 */

void do_ban(struct char_data *ch, char *arg, int cmd);
void do_unban(struct char_data *ch, char *arg, int cmd);

/* for Min's mighty mobprog hack 1996 */

void do_mpstat(struct char_data *ch, char *arg, int cmd);
void do_mpjunk(struct char_data *ch, char *arg, int cmd);
void do_mpkill(struct char_data *ch, char *arg, int cmd);
void do_mpload(struct char_data *ch, char *arg, int cmd);
void do_mppurge(struct char_data *ch, char *arg, int cmd);
void do_mpgoto(struct char_data *ch, char *arg, int cmd);
void do_mpat(struct char_data *ch, char *arg, int cmd);
void do_mptransfer(struct char_data *ch, char *arg, int cmd);
void do_mpforce(struct char_data *ch, char *arg, int cmd);
void do_mpsend(struct char_data *ch, char *arg, int cmd);
void do_mpset(struct char_data *ch, char *arg, int cmd);
void do_mptrack(struct char_data *ch, char *arg, int cmd);
void do_mptrackgo(struct char_data *ch, char *arg, int cmd);
void do_mpquestup(struct char_data *ch, char *arg, int cmd);

void do_auction(struct char_data *ch, char *arg, int cmd);
void do_vote(struct char_data *ch, char *arg, int cmd);
void clear_voting();
void do_nominate(struct char_data *ch, char *arg, int cmd);
void do_temple(struct char_data *ch, char *arg, int cmd);

void do_guild( struct char_data *ch, char *arg, int cmd);

void do_flail( struct char_data *ch, char *arg, int cmd);
void do_divert( struct char_data *ch, char *arg, int cmd);
void do_gouge( struct char_data *ch, char *arg, int cmd);
void do_tolerance(struct char_data *ch, char *arg, int cmd);
void do_ego_whip(struct char_data *ch, char *arg, int cmd);
void do_awe(struct char_data *ch, char *arg, int cmd);
void do_shield_punch(struct char_data *ch, char *arg, int cmd);
void do_wizset(struct char_data *ch, char *arg, int cmd);
void do_wizcomm(struct char_data *ch, char *arg, int cmd);
void do_qcontrol(struct char_data *ch, char *arg, int cmd);
void do_rpsay(struct char_data* ch, char* argument, int cmd);
void CreateOneRoom(room_num loc_nr);

void do_explode(struct char_data *ch, char *arg, int cmd);
void do_qfunction(struct char_data *ch, char *arg, int cmd);
void do_restring(struct char_data *ch, char *arg, int cmd);
void do_vprint(struct char_data *ch, char *arg, int cmd);
void do_pulse_skill(struct char_data *ch, char *arg, int cmd);
void do_aura(struct char_data *ch, char *arg, int cmd);
void do_set_trap(struct char_data *ch, char *arg, int cmd);
void do_calculate(struct char_data *ch, char *arg, int cmd);
void do_rewiz(struct char_data *ch, char *arg, int cmd);
void do_probe(struct char_data *ch, char *arg, int cmd);

void do_forge(struct char_data *ch, char *arg, int cmd);
#endif
