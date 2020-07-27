
#pragma once

#include "ui_system.h"
class UiWorldmapImpl;

class UiWorldmap : public UiSystem, public SaveGameAwareUiSystem {
public:
	static constexpr auto Name = "Worldmap-UI";
	UiWorldmap(int width, int height);
	~UiWorldmap();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(const UiSaveFile &saveGame) override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	void Show(int mode);
	void Hide();

	// Called by the dialog scripts to travel to an area based on a dialog choice
	void TravelToArea(int area);
	bool NeedToClearEncounterMap();
	
private:
	std::unique_ptr<UiWorldmapImpl> mImpl;
};

UiWorldmap &ui_worldmap();
