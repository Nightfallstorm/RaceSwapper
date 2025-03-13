#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

class HandleFormDelete : public RE::BSTEventSink<RE::TESFormDeleteEvent>
{
	RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>* a_eventSource) override
	{
		NPCAppearance::EraseNPCAppearance(a_event->formID);
		return RE::BSEventNotifyControl::kContinue;
	}
};
