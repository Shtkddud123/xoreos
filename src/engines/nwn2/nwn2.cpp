/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file engines/nwn2/nwn2.cpp
 *  Engine class handling Neverwinter Nights 2
 */

#include "common/util.h"
#include "common/filelist.h"
#include "common/stream.h"
#include "common/configman.h"

#include "aurora/resman.h"
#include "aurora/talkman.h"
#include "aurora/error.h"

#include "graphics/aurora/cursorman.h"
#include "graphics/aurora/fontman.h"
#include "graphics/aurora/fps.h"

#include "events/events.h"

#include "engines/aurora/util.h"
#include "engines/aurora/loadprogress.h"
#include "engines/aurora/resources.h"
#include "engines/aurora/model.h"

#include "engines/nwn2/nwn2.h"
#include "engines/nwn2/modelloader.h"
#include "engines/nwn2/console.h"
#include "engines/nwn2/campaign.h"

namespace Engines {

namespace NWN2 {

const NWN2EngineProbe kNWN2EngineProbe;

const Common::UString NWN2EngineProbe::kGameName = "Neverwinter Nights 2";

NWN2EngineProbe::NWN2EngineProbe() {
}

NWN2EngineProbe::~NWN2EngineProbe() {
}

Aurora::GameID NWN2EngineProbe::getGameID() const {
	return Aurora::kGameIDNWN2;
}

const Common::UString &NWN2EngineProbe::getGameName() const {
	return kGameName;
}

bool NWN2EngineProbe::probe(const Common::UString &UNUSED(directory),
                            const Common::FileList &rootFiles) const {

	// If either the ini file or the binary is found, this should be a valid path
	if (rootFiles.contains("/nwn2.ini", true))
		return true;
	if (rootFiles.contains("/nwn2main.exe", true))
		return true;

	return false;
}

bool NWN2EngineProbe::probe(Common::SeekableReadStream &UNUSED(stream)) const {
	return false;
}

Engines::Engine *NWN2EngineProbe::createEngine() const {
	return new NWN2Engine;
}


NWN2Engine::NWN2Engine() {
}

NWN2Engine::~NWN2Engine() {
}

void NWN2Engine::run(const Common::UString &target) {
	_baseDirectory = target;

	init();
	if (EventMan.quitRequested())
		return;

	CursorMan.hideCursor();
	CursorMan.set();

	playVideo("atarilogo");
	playVideo("oeilogo");
	playVideo("wotclogo");
	playVideo("nvidialogo");
	playVideo("legal");
	playVideo("intro");
	if (EventMan.quitRequested())
		return;

	CursorMan.showCursor();

	Console console;
	Campaign campaign(console);

	const std::list<CampaignDescription> &campaigns = campaign.getCampaigns();
	if (campaigns.empty())
		error("No campaigns found");

	bool showFPS = ConfigMan.getBool("showfps", false);

	Graphics::Aurora::FPS *fps = 0;
	if (showFPS) {
		fps = new Graphics::Aurora::FPS(FontMan.get(Graphics::Aurora::kSystemFontMono, 13));
		fps->show();
	}

	campaign.loadCampaign(*campaigns.begin());

	campaign.run();

	delete fps;
}

void NWN2Engine::init() {
	LoadProgress progress(16);

	progress.step("Setting base directory");
	ResMan.registerDataBaseDir(_baseDirectory);
	indexMandatoryDirectory("", 0, 0, 1);

	// NWN2's new file types overlap with other engines in the 3000s block
	ResMan.addTypeAlias((Aurora::FileType) 3000, Aurora::kFileTypeOSC);
	ResMan.addTypeAlias((Aurora::FileType) 3001, Aurora::kFileTypeUSC);
	ResMan.addTypeAlias((Aurora::FileType) 3002, Aurora::kFileTypeTRN);
	ResMan.addTypeAlias((Aurora::FileType) 3003, Aurora::kFileTypeUTR);
	ResMan.addTypeAlias((Aurora::FileType) 3004, Aurora::kFileTypeUEN);
	ResMan.addTypeAlias((Aurora::FileType) 3005, Aurora::kFileTypeULT);
	ResMan.addTypeAlias((Aurora::FileType) 3006, Aurora::kFileTypeSEF);
	ResMan.addTypeAlias((Aurora::FileType) 3007, Aurora::kFileTypePFX);
	ResMan.addTypeAlias((Aurora::FileType) 3008, Aurora::kFileTypeCAM);
	ResMan.addTypeAlias((Aurora::FileType) 3009, Aurora::kFileTypeLFX);
	ResMan.addTypeAlias((Aurora::FileType) 3010, Aurora::kFileTypeBFX);
	ResMan.addTypeAlias((Aurora::FileType) 3011, Aurora::kFileTypeUPE);
	ResMan.addTypeAlias((Aurora::FileType) 3012, Aurora::kFileTypeROS);
	ResMan.addTypeAlias((Aurora::FileType) 3013, Aurora::kFileTypeRST);
	ResMan.addTypeAlias((Aurora::FileType) 3014, Aurora::kFileTypeIFX);
	ResMan.addTypeAlias((Aurora::FileType) 3015, Aurora::kFileTypePFB);
	ResMan.addTypeAlias((Aurora::FileType) 3016, Aurora::kFileTypeZIP);
	ResMan.addTypeAlias((Aurora::FileType) 3017, Aurora::kFileTypeWMP);
	ResMan.addTypeAlias((Aurora::FileType) 3018, Aurora::kFileTypeBBX);
	ResMan.addTypeAlias((Aurora::FileType) 3019, Aurora::kFileTypeTFX);
	ResMan.addTypeAlias((Aurora::FileType) 3020, Aurora::kFileTypeWLK);
	ResMan.addTypeAlias((Aurora::FileType) 3021, Aurora::kFileTypeXML);
	ResMan.addTypeAlias((Aurora::FileType) 3022, Aurora::kFileTypeSCC);
	ResMan.addTypeAlias((Aurora::FileType) 3033, Aurora::kFileTypePTX);
	ResMan.addTypeAlias((Aurora::FileType) 3034, Aurora::kFileTypeLTX);
	ResMan.addTypeAlias((Aurora::FileType) 3035, Aurora::kFileTypeTRX);

	indexMandatoryDirectory("", 0, 0, 1);

	progress.step("Adding extra archive directories");
	ResMan.addArchiveDir(Aurora::kArchiveZIP, "data");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "modules");
	ResMan.addArchiveDir(Aurora::kArchiveERF, "hak");

	progress.step("Loading main resource files");

	indexMandatoryArchive(Aurora::kArchiveZIP, "2da.zip"           ,  1);
	indexMandatoryArchive(Aurora::kArchiveZIP, "actors.zip"        ,  2);
	indexMandatoryArchive(Aurora::kArchiveZIP, "animtags.zip"      ,  3);
	indexMandatoryArchive(Aurora::kArchiveZIP, "convo.zip"         ,  4);
	indexMandatoryArchive(Aurora::kArchiveZIP, "ini.zip"           ,  5);
	indexMandatoryArchive(Aurora::kArchiveZIP, "lod-merged.zip"    ,  6);
	indexMandatoryArchive(Aurora::kArchiveZIP, "music.zip"         ,  7);
	indexMandatoryArchive(Aurora::kArchiveZIP, "nwn2_materials.zip",  8);
	indexMandatoryArchive(Aurora::kArchiveZIP, "nwn2_models.zip"   ,  9);
	indexMandatoryArchive(Aurora::kArchiveZIP, "nwn2_vfx.zip"      , 10);
	indexMandatoryArchive(Aurora::kArchiveZIP, "prefabs.zip"       , 11);
	indexMandatoryArchive(Aurora::kArchiveZIP, "scripts.zip"       , 12);
	indexMandatoryArchive(Aurora::kArchiveZIP, "sounds.zip"        , 13);
	indexMandatoryArchive(Aurora::kArchiveZIP, "soundsets.zip"     , 14);
	indexMandatoryArchive(Aurora::kArchiveZIP, "speedtree.zip"     , 15);
	indexMandatoryArchive(Aurora::kArchiveZIP, "templates.zip"     , 16);
	indexMandatoryArchive(Aurora::kArchiveZIP, "vo.zip"            , 17);
	indexMandatoryArchive(Aurora::kArchiveZIP, "walkmesh.zip"      , 18);

	progress.step("Loading expansions resource files");

	// Expansion 1: Mask of the Betrayer (MotB)
	indexOptionalArchive(Aurora::kArchiveZIP, "2da_x1.zip"           , 20);
	indexOptionalArchive(Aurora::kArchiveZIP, "actors_x1.zip"        , 21);
	indexOptionalArchive(Aurora::kArchiveZIP, "animtags_x1.zip"      , 22);
	indexOptionalArchive(Aurora::kArchiveZIP, "convo_x1.zip"         , 23);
	indexOptionalArchive(Aurora::kArchiveZIP, "ini_x1.zip"           , 24);
	indexOptionalArchive(Aurora::kArchiveZIP, "lod-merged_x1.zip"    , 25);
	indexOptionalArchive(Aurora::kArchiveZIP, "music_x1.zip"         , 26);
	indexOptionalArchive(Aurora::kArchiveZIP, "nwn2_materials_x1.zip", 27);
	indexOptionalArchive(Aurora::kArchiveZIP, "nwn2_models_x1.zip"   , 28);
	indexOptionalArchive(Aurora::kArchiveZIP, "nwn2_vfx_x1.zip"      , 29);
	indexOptionalArchive(Aurora::kArchiveZIP, "prefabs_x1.zip"       , 30);
	indexOptionalArchive(Aurora::kArchiveZIP, "scripts_x1.zip"       , 31);
	indexOptionalArchive(Aurora::kArchiveZIP, "soundsets_x1.zip"     , 32);
	indexOptionalArchive(Aurora::kArchiveZIP, "sounds_x1.zip"        , 33);
	indexOptionalArchive(Aurora::kArchiveZIP, "speedtree_x1.zip"     , 34);
	indexOptionalArchive(Aurora::kArchiveZIP, "templates_x1.zip"     , 35);
	indexOptionalArchive(Aurora::kArchiveZIP, "vo_x1.zip"            , 36);
	indexOptionalArchive(Aurora::kArchiveZIP, "walkmesh_x1.zip"      , 37);

	// Expansion 2: Storm of Zehir (SoZ)
	indexOptionalArchive(Aurora::kArchiveZIP, "2da_x2.zip"           , 40);
	indexOptionalArchive(Aurora::kArchiveZIP, "actors_x2.zip"        , 41);
	indexOptionalArchive(Aurora::kArchiveZIP, "animtags_x2.zip"      , 42);
	indexOptionalArchive(Aurora::kArchiveZIP, "lod-merged_x2.zip"    , 43);
	indexOptionalArchive(Aurora::kArchiveZIP, "music_x2.zip"         , 44);
	indexOptionalArchive(Aurora::kArchiveZIP, "nwn2_materials_x2.zip", 45);
	indexOptionalArchive(Aurora::kArchiveZIP, "nwn2_models_x2.zip"   , 46);
	indexOptionalArchive(Aurora::kArchiveZIP, "nwn2_vfx_x2.zip"      , 47);
	indexOptionalArchive(Aurora::kArchiveZIP, "prefabs_x2.zip"       , 48);
	indexOptionalArchive(Aurora::kArchiveZIP, "scripts_x2.zip"       , 49);
	indexOptionalArchive(Aurora::kArchiveZIP, "soundsets_x2.zip"     , 50);
	indexOptionalArchive(Aurora::kArchiveZIP, "sounds_x2.zip"        , 51);
	indexOptionalArchive(Aurora::kArchiveZIP, "speedtree_x2.zip"     , 52);
	indexOptionalArchive(Aurora::kArchiveZIP, "templates_x2.zip"     , 53);
	indexOptionalArchive(Aurora::kArchiveZIP, "vo_x2.zip"            , 54);

	warning("TODO: Mysteries of Westgate (MoW) resource files");
	warning("TODO: Patch resource files");

	progress.step("Indexing extra sound resources");
	indexMandatoryDirectory("ambient"   , 0,  0, 60);
	indexOptionalDirectory ("ambient_x1", 0,  0, 61);
	indexOptionalDirectory ("ambient_x2", 0,  0, 62);
	progress.step("Indexing extra music resources");
	indexMandatoryDirectory("music"     , 0,  0, 63);
	indexOptionalDirectory ("music_x1"  , 0,  0, 64);
	indexOptionalDirectory ("music_x2"  , 0,  0, 65);
	progress.step("Indexing extra movie resources");
	indexMandatoryDirectory("movies"    , 0,  0, 66);
	progress.step("Indexing extra effects resources");
	indexMandatoryDirectory("effects"   , 0,  0, 67);
	progress.step("Indexing extra character resources");
	indexMandatoryDirectory("localvault", 0,  0, 68);
	progress.step("Indexing extra UI resources");
	indexMandatoryDirectory("ui"        , 0, -1, 69);

	progress.step("Indexing Windows-specific resources");
	indexMandatoryArchive(Aurora::kArchiveEXE, "nwn2main.exe", 70);

	progress.step("Indexing override files");
	indexOptionalDirectory("override", 0, 0, 100);

	progress.step("Loading game cursors");
	initCursors();

	progress.step("Loading main talk table");
	TalkMan.addMainTable("dialog");

	progress.step("Registering file formats");
	registerModelLoader(new NWN2ModelLoader);
	FontMan.setFormat(Graphics::Aurora::kFontFormatTTF);

	progress.step("Successfully initialized the engine");
}

void NWN2Engine::initCursors() {
	CursorMan.add("cursor0" , "default"  , "up"  );
	CursorMan.add("cursor1" , "default"  , "down");

	CursorMan.setDefault("default", "up");
}

} // End of namespace NWN2

} // End of namespace Engines
