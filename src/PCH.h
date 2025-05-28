#pragma once

#include "RE/Starfield.h"
#include "SFSE/SFSE.h"

#include <ClibUtil/simpleINI.hpp>

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif

#include <xbyak/xbyak.h>

namespace logger = SFSE::log;
namespace ini = clib_util::ini;

using namespace std::literals;

namespace stl
{
	template <class T, class U>
	constexpr void write_vfunc(const U a_id) noexcept
		requires(std::is_same_v<U, REL::ID> || std::is_same_v<U, REL::Offset>)
	{
		static REL::Relocation vtbl{ a_id };
		T::func = vtbl.write_vfunc(T::idx, T::thunk);
	}

	template <class T, class U>
	constexpr void write_vfunc(const REL::Relocation<U> a_id) noexcept
		requires(std::is_same_v<typename REL::Relocation<U>::value_type, std::uintptr_t>)
	{
		T::func = a_id.write_vfunc(T::idx, T::thunk);
	}

	template <class To, class From>
	constexpr void write_vfunc(const std::size_t a_vtableIdx = 0) noexcept
	{
		write_vfunc<From>(To::VTABLE[a_vtableIdx]);
	}
	
	void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);

	template <class T>
	void asm_replace(std::uintptr_t a_from)
	{
		asm_replace(a_from, T::size, reinterpret_cast<std::uintptr_t>(T::func));
	}
}

#include "Version.h"
