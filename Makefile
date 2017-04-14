SUBDIRS = \
	PolyKit/Sources \
	APlayer/APlayerKit \
	APlayer/Server \
	APlayer/Agents/Decruncher \
	APlayer/Agents/DiskSaver \
	APlayer/Agents/MediaKit \
	APlayer/Agents/MikModConverter \
	APlayer/Agents/ModuleConverter \
	APlayer/Agents/ProWizard \
	APlayer/Agents/Reverb \
	APlayer/Agents/Scope \
	APlayer/Agents/SpinSquare \
	APlayer/Clients/MainWindowSystem \
	APlayer/Converters/AudioIFF \
	APlayer/Converters/IFF-8SVX \
	APlayer/Converters/IFF-16SV \
	APlayer/Converters/Raw \
	APlayer/Converters/RIFF-WAVE \
	APlayer/Players/AHX \
	APlayer/Players/Fred \
	APlayer/Players/FutureComposer \
	APlayer/Players/GameMusic \
	APlayer/Players/JamCracker \
	APlayer/Players/MediaFile \
	APlayer/Players/MikMod \
	APlayer/Players/ModTracker \
	APlayer/Players/OctaMED \
	APlayer/Players/Oktalyzer \
	APlayer/Players/Sample \
	APlayer/Players/Sawteeth \
	APlayer/Players/SidPlay \
	APlayer/Players/SoundFX \
	APlayer/Players/SoundMonitor \
	APlayer/Players/TFMX \
#	Bonus \
#	Compressor

.PHONY: subdirs $(SUBDIRS)

subdirs: dist/lib $(SUBDIRS)

dist/lib:
	mkdir -p dist/lib
	mkdir -p dist/add-ons/Agents
	mkdir -p dist/add-ons/Clients
	mkdir -p dist/add-ons/Converters
	mkdir -p dist/add-ons/Players

$(SUBDIRS):
	$(MAKE) -C $@

# Dependency chain
APlayer/APlayerKit: PolyKit/Sources
APlayer: APlayer/APlayerKit PolyKit/Sources
