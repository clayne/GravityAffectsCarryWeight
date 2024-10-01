#include "Settings.h"

namespace Handler
{
	struct GetMaxCarryWeight
	{
		static float func(RE::Actor* a_actor)
		{
			auto carryWeight = a_actor->GetActorValue(*RE::ActorValue::GetSingleton()->carryWeight);
			
			auto localGravity = a_actor->GetGravityScale();
			if (localGravity < 0.1f) {
				localGravity = 0.1f;
			}

			// higher grav = less carry weight, lower grav = more carry weight
			carryWeight *= Settings::GetSingleton()->baseGravity / localGravity;
			
			RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kGetMaxCarryWeight, a_actor, std::addressof(carryWeight));
			
			return carryWeight;
		}
		static inline constexpr std::size_t size{ 0x5A };
	};

	struct AdjustItemWeight
	{
		static float thunk(RE::Actor* a_this, const RE::BGSInventoryItem& a_invItem, float a_weight, bool* a_shouldModifyWholeStackOut)
		{
			if (a_weight > 0.0f) {
				auto localGravity = a_this->GetGravityScale();
				if (localGravity < 0.1f) {
					localGravity = 0.1f;
				}

				// higher grav = weigh more, lower grav = weight less
				float itemWeight = a_weight * localGravity / Settings::GetSingleton()->baseGravity;

				return func(a_this, a_invItem, itemWeight, a_shouldModifyWholeStackOut);
			}

			return func(a_this, a_invItem, a_weight, a_shouldModifyWholeStackOut);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx = 0x99;
	};

	void Install()
	{
		const auto settings = Settings::GetSingleton();
		settings->LoadSettings();

		if (settings->adjustItemWeights) {
			stl::write_vfunc<AdjustItemWeight>(RE::VTABLE::PlayerCharacter[38]);
		} else {
			REL::Relocation<std::uintptr_t> target{ REL::ID(1721804) };
			stl::asm_replace<GetMaxCarryWeight>(target.address());
		}
	}
}

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
{
	switch (a_msg->type) {
	case SFSE::MessagingInterface::kPostLoad:
		Handler::Install();
		break;
	default:
		break;
	}
}

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Version::MAJOR);
	data.PluginName(Version::PROJECT);
	data.AuthorName("powerofthree");
	data.UsesAddressLibrary(true);
	data.IsLayoutDependent(true);
	data.CompatibleVersions({ SFSE::RUNTIME_LATEST });

	return data;
}();

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	logger::info("Game version : {}", a_sfse->RuntimeVersion());
	logger::info("Plugin version : {}", Version::NAME);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
