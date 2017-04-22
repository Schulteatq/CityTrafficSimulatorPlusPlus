#ifndef CTS_LUA_CTS_LUA_H
#define CTS_LUA_CTS_LUA_H

#include <cts-lua/config.h>
#include <cts-core/base/math.h>

#include <sol.hpp>

namespace sol {

	// First, the expected size
	// Specialization of a struct
	template <>
	struct lua_size<cts::vec2> : std::integral_constant<int, 2> {};

	// Then, specialize the type
	// this makes sure Sol can return it properly
	template <>
	struct lua_type_of<cts::vec2> : std::integral_constant<sol::type, sol::type::poly> {};

	// Now, specialize various stack structures
	namespace stack {

		template <>
		struct checker<cts::vec2> {
			template <typename Handler>
			CTS_LUA_API static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				// indices can be negative to count backwards from the top of the stack,
				// rather than the bottom up
				// to deal with this, we adjust the index to
				// its absolute position using the lua_absindex function 
				int absolute_index = lua_absindex(L, index);
				// Check first and second second index for being the proper types
				bool success = stack::check<double>(L, absolute_index, handler)
					&& stack::check<double>(L, absolute_index + 1, handler);
				tracking.use(2);
				return success;
			}
		};

		template <>
		struct getter<cts::vec2> {
			CTS_LUA_API static cts::vec2 get(lua_State* L, int index, record& tracking) {
				int absolute_index = lua_absindex(L, index);
				// Get the first element
				double x = stack::get<double>(L, absolute_index);
				// Get the second element, 
				// in the +1 position from the first
				double y = stack::get<double>(L, absolute_index + 1);
				// we use 2 slots, each of the previous takes 1
				tracking.use(2);
				return cts::vec2{ x, y };
			}
		};

		template <>
		struct pusher<cts::vec2> {
			CTS_LUA_API static int push(lua_State* L, const cts::vec2& things) {
				int amount = stack::push(L, things[0]);
				// amount will be 1: int pushes 1 item
				amount += stack::push(L, things[1]);
				// amount 2 now, since bool pushes a single item
				// Return 2 things
				return amount;
			}
		};

	}
}



namespace cts
{
	namespace lua
	{
		struct CTS_LUA_API Registration
		{
			static void registerWith(sol::state& luaState);
		};
	}
}

#endif // CTS_LUA_CTS_LUA_H
