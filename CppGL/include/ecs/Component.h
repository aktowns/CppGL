#pragma once

#include "Entity.h"
#include <set>

/*
enum GroupByMode {
	Join = 0, Merge = 1, Exclude = 2
};

template<typename T>
inline std::set<Entity> &any() {
	static std::set<Entity> entities;
	return entities;
}

template<int Mode>
inline std::set<Entity> groupBy(const std::set<Entity> &a, const std::set<Entity> &b) {
	const std::set<Entity>  *tiny, *large;

	if (a.size() < b.size())
	{
		tiny = &a, large = &b;
	}
	else
	{
		tiny = &b, large = &a;
	}

	std::set<Entity> newset;

	if (Mode == Merge)
	{
		newset = *large; for (auto &id : *tiny) newset.insert(id);
	}
	else if (Mode == Exclude)
	{
		newset = *large; for (auto &id : *tiny) newset.erase(id);
	}
	else
	{
		for (auto &id : *tiny) {
			if (large->find(id) != large->end()) {
				newset.insert(id);
			}
		}
	}
	return newset;
}

// sugars {
template<class T>                            std::set<Entity> join() { return any<T>();                                  }
template<class T, class U>                   std::set<Entity> join() { return groupBy<Join>( any<T>(), any<U>() );      }
template<class T, class U, class V>          std::set<Entity> join() { return groupBy<Join>( any<T>(), join<U,V>() );   }
template<class T, class U, class V, class W> std::set<Entity> join() { return groupBy<Join>( any<T>(), join<U,V,W>() ); }
template<class T>                            std::set<Entity> join(const T &t)                                     { return any<T>(); }
template<class T, class U>                   std::set<Entity> join(const T &t, const U &u)                         { return join<T,U>(); }
template<class T, class U, class V>          std::set<Entity> join(const T &t, const U &u, const V &v)             { return join<T,U,V>(); }
template<class T, class U, class V, class W> std::set<Entity> join(const T &t, const U &u, const V &v, const W &w) { return join<T,U,V,W>(); }
template<class T> std::set<Entity> exclude(const std::set<Entity> &B) { return groupBy<Exclude>( any<T>(), B ); }
template<class T> std::set<Entity> exclude(const std::set<Entity> &A, const T &t) { return groupBy<Exclude>( A, any<T>() ); }
// }

struct Component {
	static std::vector<const Component*> &registered() {
		static std::vector<const Component*> vector;
		return vector;
	}

	template<typename T>
	static std::map<EntityId, T> &components() {
		static std::map<EntityId, T> objects;
		return objects;
	}

	template<typename T>
	static inline bool has(const EntityId &id) {
		return components<T>().find(id) != components<T>().end();
	}

	template<typename T>
	static inline decltype(T::value_type) &get(const EntityId &id) {
		static decltype(T::value_type) invalid, reset;
		return has<T>(id) ? components<T>()[id].value_type : invalid = reset;
	}

	template<typename T>
	static inline decltype(T::value_type) &add(const EntityId &id) {
		any<T>().insert(id);
		return components<T>()[id].value_type;
	}

	template<typename T>
	static inline bool del(const EntityId &id) {
		Component::add<T>(id);
		components<T>().erase(id);
		any<T>().erase(id);
		return !has<T>(id);
	}

	explicit Component(Entity &ent)
	{
		add<Component>(ent.id);
		Component::registered().push_back(this);
	}

	~Component() {
		auto &list = Component::registered();
		for (auto &it : list) {
			if (it == this) {
				std::swap(it, list.back());
				list.pop_back();
			}
		}
	}

	void purge(const EntityId &id) const 
	{
		del<Component>(id);
	}

	void swap(const EntityId &dst, const EntityId &src) const 
	{
		if (has<Component>(dst) && has<Component>(src)) {
			std::swap(get<Component>(dst), get<Component>(src));
		}
	}

	void merge(const EntityId &dst, const EntityId &src) const 
	{
		add<Component>(dst) = get<Component>(src);
	}

	void copy(const EntityId &dst, const EntityId &src) const 
	{
		if (has<Component>(src)) {
			merge(dst, src);
		}
		else {
			purge(dst);
		}
	}
	inline Component &operator()(const EntityId &id) {
		return get<Component>(id);
	}

	inline const Component &operator()(const EntityId &id) const {
		return get<Component>(id);
	}
};

inline EntityId purge(const EntityId &id) { // clear
	for (auto &it : Component::registered()) {
		it->purge(id);
	}
	return id;
}

inline EntityId swap(const EntityId &dst, const EntityId &src) {
	for (auto &it : Component::registered()) {
		it->swap(dst, src);
	}
	return dst;
}

inline EntityId merge(const EntityId &dst, const EntityId &src) {
	for (auto &it : Component::registered()) {
		it->merge(dst, src);
	}
	return dst;
}

inline EntityId copy(const EntityId &dst, const EntityId &src) {
	for (auto &it : Component::registered()) {
		it->copy(dst, src);
	}
	return dst;
}

inline EntityId spawn(const EntityId &src) {
	return copy(Entity::nextId(), src);
}

inline EntityId reset(const EntityId &id) {
	return copy(id, 0);
}
*/

class World final
{
	template<typename T>
	static std::map<EntityId, T*> &components() {
		static std::map<EntityId, T*> components;
		return components;
	}

public:
	template<typename T>
	T* addComponent(T* component, Entity& e)
	{
		World::components<T>().insert(std::pair<EntityId, T*>(e.id, component));
		return component;
	}

	template<typename T>
	std::optional<T*> getComponent(EntityId e)
	{
		auto map = World::components<T>();
		auto it = map.find(e);
		if (it == map.end())
		{
			return std::nullopt;
		}
		return it->second;
	}

	template<typename T>
	std::optional<T*> getComponent(Entity& e)
	{
		return getComponent<T>(e.id);
	}

	template<typename T>
	void each(std::function<void(const EntityId&, T*)> f)
	{
		auto s1 = World::components<T>();
		for (auto it = s1.begin(); it != s1.end(); ++it)
		{
			f(it->first, it->second);
		}
	}

	template<typename T, typename U>
	void each(std::function<void(const EntityId&, T*, U*)> f)
	{
		auto s1 = World::components<T>();
		auto s2 = World::components<U>();

		// This is far from optimal.
		for (auto it = s1.begin(); it != s1.end(); ++it)
		{
			auto c2 = getComponent<U>(it->first);
			if (c2.has_value())
				f(it->first, it->second, c2.value());
		}
	}

	template<typename T, typename U, typename V>
	void each(std::function<void(const EntityId&, T*, U*, V*)> f)
	{
		auto s1 = World::components<T>();
		auto s2 = World::components<U>();
		auto s3 = World::components<T>();

		for (auto it = s1.begin(); it != s1.end(); ++it)
		{
			auto c2 = getComponent<U>(it->first);
			if (c2.has_value()) {
				auto c3 = getComponent<V>(it->first);
				if (c3.has_value())
					f(it->first, it->second, c2.value(), c3.value());
			}
        }
    }
};

struct PositionComponent final
{
    float x, y, z;
    PositionComponent(const float x, const float y, const float z) : x(x), y(y), z(z) {}
};

struct DebugComponent final
{
    std::string name;
    explicit DebugComponent(const std::string& name) : name(name) {}
};

struct RigidBodyComponent final
{
		
};


void testing()
{
	auto world = World();
	auto entity = Entity();

	world.addComponent(new PositionComponent(10.0, 10.0, 5.0), entity);
	world.addComponent(new DebugComponent("test player"), entity);

	auto entity2 = Entity();
	world.addComponent(new DebugComponent("test npc"), entity2);

	world.each<PositionComponent, DebugComponent>( 
        [](const EntityId& ent, auto* pos, DebugComponent* dbg)
        {
            fmt::print("processing entity {}: {}", ent, dbg->name);
        }
    );
    
}
