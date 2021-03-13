applicationfiles = hex-a-hop.desktop
applicationfilesdir = $(APPDIR)
applicationfiles_DATA = $(applicationfiles)

dist_appdata_DATA = hex-a-hop.appdata.xml
appdatadir = $(datarootdir)/appdata

htmlfiles = hints.html
htmlfilesdir = $(htmldir)
htmlfiles_DATA = $(htmlfiles)

iconfiles = hex-a-hop.png hex-a-hop-16.png
iconfilesdir = $(DATADIR)
iconfiles_DATA = $(iconfiles)

dist_man_MANS = hex-a-hop.6

freedata = \
	emi.png \
	gradient.png \
	levels.dat \
	map.png \
	map_top.png \
	music-ending.ogg \
	music-game.ogg \
	music-game-1.ogg \
	tiles.png \
	tiles_reflect.png \
	title.png \
	sound-crack.ogg \
	sound-death.ogg \
	sound-explode-big.ogg \
	sound-explode-small.ogg \
	sound-floater-enter.ogg \
	sound-ice.ogg \
	sound-laser.ogg \
	sound-trampoline.ogg \
	sound-used-antiice.ogg \
	sound-used-jump.ogg \
	sound-win.ogg
freeinfo = \
	music-ending.ogg.xml \
	music-game-1.ogg.xml \
	music-game.ogg.xml \
	sound-crack.ogg.xml \
	sound-death.ogg.xml \
	sound-explode-big.ogg.xml \
	sound-explode-small.ogg.xml \
	sound-floater-enter.ogg.xml \
	sound-ice.ogg.xml \
	sound-laser.ogg.xml \
	sound-trampoline.ogg.xml \
	sound-used-antiice.ogg.xml \
	sound-used-jump.ogg.xml \
	sound-win.ogg.xml
nonfreedata = \
	sound-builder-nonfree.ogg \
	sound-collapse-nonfree.ogg \
	sound-found-antiice-nonfree.ogg \
	sound-found-jump-nonfree.ogg \
	sound-lift-down-nonfree.ogg \
	sound-lift-up-nonfree.ogg \
	sound-spinner-nonfree.ogg
nonfreeinfo = \
	sound-builder-nonfree.ogg.xml \
	sound-collapse-nonfree.ogg.xml \
	sound-found-antiice-nonfree.ogg.xml \
	sound-found-jump-nonfree.ogg.xml \
	sound-lift-down-nonfree.ogg.xml \
	sound-lift-up-nonfree.ogg.xml \
	sound-spinner-nonfree.ogg.xml

datadir = $(DATADIR)
data_DATA = $(freedata)
EXTRA_DIST = $(freedata) $(freeinfo) \
	$(applicationfiles) $(htmlfiles) $(iconfiles) \
	hex-a-hop.xpm \
	hex-a-hop.icns

MAINTAINERCLEANFILES = Makefile.in
