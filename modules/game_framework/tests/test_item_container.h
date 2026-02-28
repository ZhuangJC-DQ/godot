/**************************************************************************/
/*  test_item_container.h                                                 */
/**************************************************************************/

#pragma once

#include "../item/item.h"
#include "../item/item_manager.h"

#include "tests/test_macros.h"

namespace TestItemContainer {

TEST_CASE("[ItemManager] Create and destroy items") {
	ItemManager manager;

	uint64_t id = manager.create_item(1);
	CHECK(id > 0);
	CHECK(manager.is_valid_item(id));
	CHECK(manager.get_item_count() == 1);

	manager.destroy_item(id);
	CHECK(!manager.is_valid_item(id));
	CHECK(manager.get_item_count() == 0);
}

TEST_CASE("[ItemManager] Container operations") {
	ItemManager manager;

	uint64_t container_id = manager.create_item(100);
	manager.set_as_container(container_id, 10);
	CHECK(manager.is_container(container_id));
	CHECK(manager.get_max_slots(container_id) == 10);

	uint64_t item_id = manager.create_item(1);
	CHECK(manager.add_to_container(item_id, container_id));
	CHECK(manager.get_container_id(item_id) == container_id);

	CHECK(manager.remove_from_container(item_id));
	CHECK(manager.get_container_id(item_id) == 0);
}

} // namespace TestItemContainer
