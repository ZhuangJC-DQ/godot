/**************************************************************************/
/*  game_character.cpp                                                    */
/**************************************************************************/

#include "game_character.h"

#include "core/string/print_string.h"

GameCharacter::GameCharacter() {
}

GameCharacter::GameCharacter(uint64_t p_id, CharacterType p_type) :
		id(p_id), type(p_type) {
}

GameCharacter::~GameCharacter() {
	custom_properties.clear();
}

void GameCharacter::set_hp(int p_hp) {
	hp = CLAMP(p_hp, 0, max_hp);
	if (hp <= 0) {
		alive = false;
	}
}

void GameCharacter::set_max_hp(int p_max_hp) {
	max_hp = MAX(1, p_max_hp);
	if (hp > max_hp) {
		hp = max_hp;
	}
}

void GameCharacter::set_mp(int p_mp) {
	mp = CLAMP(p_mp, 0, max_mp);
}

void GameCharacter::set_max_mp(int p_max_mp) {
	max_mp = MAX(0, p_max_mp);
	if (mp > max_mp) {
		mp = max_mp;
	}
}

int GameCharacter::take_damage(int p_damage) {
	int actual_damage = MAX(0, p_damage - defense);
	set_hp(hp - actual_damage);
	return actual_damage;
}

void GameCharacter::heal(int p_amount) {
	if (!alive) {
		return;
	}
	set_hp(hp + p_amount);
}

void GameCharacter::set_custom_property(const String &p_key, const Variant &p_value) {
	custom_properties[p_key] = p_value;
}

Variant GameCharacter::get_custom_property(const String &p_key) const {
	if (custom_properties.has(p_key)) {
		return custom_properties[p_key];
	}
	return Variant();
}

bool GameCharacter::has_custom_property(const String &p_key) const {
	return custom_properties.has(p_key);
}

void GameCharacter::remove_custom_property(const String &p_key) {
	custom_properties.erase(p_key);
}

String GameCharacter::to_string() const {
	static const char *type_names[CHARACTER_TYPE_MAX] = { "Player", "NPC", "Enemy" };
	const char *type_str = (type >= 0 && type < CHARACTER_TYPE_MAX) ? type_names[type] : "Unknown";

	return vformat("Character[%d] type=%s name='%s' level=%d hp=%d/%d mp=%d/%d atk=%d def=%d spd=%.1f alive=%s",
			id, type_str, name, level, hp, max_hp, mp, max_mp, attack, defense, speed,
			alive ? "true" : "false");
}
