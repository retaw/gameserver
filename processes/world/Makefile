objects = \
		  shabake.o\
		  daily_task.o\
		  daily_task_manager.o\
		  daily_task_config.o\
		  trade_manager.o\
		  role_stall.o\
		  stall_msg.o\
		  bonfire_manager.o\
		  bonfire_config.o\
		  first_manager.o\
		  first_config.o\
		  function_icon.o\
		  world_boss.o\
		  trigger_cfg.o\
		  trigger.o\
		  trigger_manager.o\
		  action_manager.o\
		  bubble_point_manager.o\
		  bubble_point_config.o\
		  dragon_heart.o\
		  dragon_heart_msg.o\
		  dragon_heart_cfg.o\
		  exp_area.o\
		  exp_area_manager.o\
		  exp_area_config.o\
		  task.o\
		  task_base.o\
		  task_msg.o\
		  fenjie.o\
		  fenjie_manager.o\
		  fenjie_config.o\
		  zhuansheng.o\
		  zhuansheng_manager.o\
		  zhuansheng_config.o\
		  reward_manager.o\
		  reward_random_config.o\
		  reward_fixed_config.o\
		  use_object.o\
		  use_object_manager.o\
		  dragon_ball.o\
		  dragon_ball_manager.o\
		  dragon_ball_config.o\
		  fire.o\
		  pet_msg.o\
		  pet.o\
		  pet_base.o\
		  pet_manager.o\
		  wing.o\
		  wing_manager.o\
		  wing_config.o\
		  equip_quality_manager.o\
		  equip_quality_config.o\
		  merge_manager.o\
		  merge_config.o\
		  ronghe.o\
		  ronghe_config.o\
		  wash.o\
		  wash_manager.o\
		  wash_config.o\
		  weapon_lucky.o\
		  lucky_config.o\
		  title.o\
		  title_manager.o\
		  title_config.o\
		  horse.o\
		  horse_base.o\
		  horse_msg.o\
		  guanzhi.o\
		  guanzhi_manager.o\
		  guanzhi_config.o\
		  strong_equip.o\
		  strong_config.o\
		  mail.o\
		  mail_manager.o\
		  watch_manager.o\
		  massive_config.o\
		  attack_mode.o\
		  map_base.o\
		  role_counter.o\
		  store.o\
		  store_manager.o\
		  hero_msg_manager.o\
		  hero_manager.o\
		  hero.o\
		  hero_config.o\
		  hero_ids.o\
		  channel.o\
		  gm.o\
		  suit_config.o\
		  nonsuch_config.o\
		  scene_object_manager.o\
		  scene_object.o\
		  stone_config.o\
		  stone_package.o\
		  role_equip_package.o\
		  hero_equip_package.o\
		  equip_package.o\
		  role_package.o\
		  object_config.o\
		  relive_config.o\
		  exp_config.o\
		  object.o\
		  package.o\
		  package_set.o\
		  package_config.o\
		  main.o\
		  world.o\
		  load_cfg.o\
		  register_tcp_msg_handler.o\
		  register_timer_handler.o\
		  pk.o\
		  role.o\
		  role_manager.o\
		  grid.o\
		  screen.o\
		  scene.o\
		  scene_manager.o\
		  roles_and_scenes.o\
		  role_action.o\
		  npc.o\
		  npc_manager.o\
		  skill_manager.o\
		  skill_effect_manager.o\
		  skill_effect_element.o\
		  role_pk.o\
		  level_props.o\
		  config_table.o\
		  config_table_manager.o\
		  ai/ai_condition.o\
		  ai/ai_action.o\
		  ai/ai_factory.o\
		  ai/ai.o\
		  ai/ai_manager.o\
		  buff_manager.o\
		  buff_scene.o\
		  pk_state.o\
		  pk_cfg.o\
		  attribute.o\
		  relations_manager.o\
		  expbead_manager.o\
		  role_sundry.o\
		  factiontask.o\
		  factionactive_manager.o\
		  field_boss_manager.o\
		  private_boss_base.o\
		  private_boss.o\
		  private_boss_msg.o\
		  boss_home_manager.o\



CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror\
		   $(optimization_flag)\
		   -I$(PROCESSES_DIR)/world\
		   $(google_protobuf_cc_flag)\
		   $(base_cc_flag)\

ln_flags = -Wl,-dn\
		   -L$(PROTOCOL_DIR)/protobuf -lprotobuf_message\
		   -L$(PROTOCOL_DIR)/rawmsg -lrawmsg\
		   $(base_ln_flag)\
		   $(google_protobuf_ln_flag)\
		   -Wl,-dy\
		   -lpthread\

target_ln_dependents = $(dependent_water_files)\
					   $(PROTOCOL_DIR)/protobuf/libprotobuf_message.a\
					   $(PROTOCOL_DIR)/rawmsg/librawmsg.a

target = world_exec

.PHONY:
all: $(target)

$(target) : $(objects) $(target_ln_dependents)
	@#此行用于调试   @set -e -x; \ #此行用于调试
	g++ $(objects) $(ln_flags) -o $(target) 

%.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

%.d: %.cpp
#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
	rm -f $@.$$$$

-include $(objects:.o=.d)

.PHONY: clean distclean
clean:
	rm -f $(objects) $(objects:.o=.d) *.d *.d.* 
	rm -f $(target)

distclean: clean
