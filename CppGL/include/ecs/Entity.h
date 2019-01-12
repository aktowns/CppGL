#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>

typedef unsigned int EntityId;

struct Entity {
	static std::set<Entity*> &all() { 
		static std::set<Entity*> statics;
		return statics;
	}

	static EntityId nextId()
	{
		static EntityId idCount = 0;
		return idCount++;
	}

	EntityId id;

	explicit Entity(const EntityId &id = nextId()) : id(id) {
		all().insert(this);
	}

	~Entity() {
		all().erase(this);
	}

	explicit operator EntityId const () const {
		return id;
	}
};
