# OoTRandomizer

This is a randomizer for _The Legend of Zelda: Ocarina of Time_ for the Nintendo 64.

**WARNING:** This branch is a modified version of the randomizer. It is **not** officially supported and may be **very unstable**.
When asking questions or reporting issues in the main Randomizer Discord, please make sure to include the full version number in the `A.B.C Fenhl-D` format — that last part is important!
You can also [open an issue](https://github.com/fenhl/OoT-Randomizer/issues/new) on this fork or contact me directly on Discord (`@fenhl`) for any help, report or request.

This branch is available to use online at <https://ootrandomizer.com/generatorDev?version=devFenhl_>.

This branch (`dev-fenhl`) is based on [Roman971](https://github.com/Roman971)'s branch ([`Dev-R`](https://github.com/Roman971/OoT-Randomizer)) which is in turn based on the main [`Dev`](https://github.com/OoTRandomizer/OoT-Randomizer) branch.

Differences between `dev-fenhl` and [`Dev-R`](https://github.com/Roman971/OoT-Randomizer):

* New settings and options:
  * New plando-only “Chest, Pot, Crate, & Beehive Appearance Does Not Match Contents” setting which makes each chest etc. randomly choose any appearance except the correct one ([#1950](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1950))
  * New “Vanilla Locations” option for the “Shuffle Songs” setting ([#1882](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1882))
  * New “Bosses” option for the “Mix Entrance Pools” setting (based on [#1820](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1820))
  * New “Triforce Hunt Mode” setting with “Normal”, “Easter Egg Hunt”, “Ice%”, and “Triforce Blitz” options (Easter Egg Hunt based on [#1804](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1804), Triforce Blitz based on [Elagatua's `Dev` branch](https://github.com/Elagatua/OoT-Randomizer/tree/Dev))
  * New “Open Deku Tree” setting separate from “Open Forest” ([#1536](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1536))
  * “Closed Forest Requires Gohma” is a separate setting. With it enabled, items that can be used to escape the forest won't appear in the forest, and the randomizer will try to place at least one slingshot for each player in the forest. The setting is compatible with all forms of entrance randomizer by restricting entrances inside the forest area to only be shuffled among themselves. ([#1531](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1531))
  * New “Full” options for the settings “Randomize Owl Drops”, “Randomize Warp Song Destinations”, and “Randomize Overworld Spawns” (which is split into “Randomize Child Overworld Spawn” and “Randomize Adult Overworld Spawn” for this reason) that include more types of entrances (based on [#1179](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1179) and [#1287](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1287))
  * New “On (Savewarp to Overworld)” option for the “Shuffle Thieves' Hideout Entrances” setting (based on [fenhl#7](https://github.com/fenhl/OoT-Randomizer/pull/7))
  * New “Shuffle Items” and “Shuffle Other Items” settings which can be disabled to generate vanilla seeds, only shuffle songs, only shuffle entrances, etc. (Currently not compatible with Master Quest)
  * Work-in-progress “Language” setting to translate the game into French or German
  * New “[EXPERIMENTAL] Allow Access to Shadow and Spirit Temples From Boss Doors” setting
  * “Enable Specific Glitch-Useful Cutscenes” has been renamed to “Glitch-Useful Behaviors” and also controls whether the water in the well is present as adult
  * New “Shuffle Blue Warps” setting
  * New “Mutually Exclusive One-Ways” setting which makes the hint area restriction apply to one-way entrances of different types
  * The “Key Appearance Matches Dungeon” setting has been reworked into “Distinct Item Models”, with additional options that are on by default but can be disabled to reintroduce pairs of items with the same model, including frog songs/warp songs and small keys/keyrings
  * Ganon's Castle can now be a pre-completed dungeon; new “Ganon's Castle Can Be Pre-completed” setting which can be disabled to make “Pre-completed Dungeon Count” exclude Ganon's Castle
  * New option “Specific Dungeon Rewards” for the “Rainbow Bridge Requirement”, “Ganon's Boss Key”, and “LACS Condition” settings
  * New option “Everything Else” for the “Exclude Item Types” setting controlling randomized starting items
* New hint types based on [Elagatua's `Dev` branch](https://github.com/Elagatua/OoT-Randomizer/tree/Dev):
  * `goal-count` tells you how many items are on the path to a goal.
  * `goal-legacy` is a variant of `goal` with many subtle differences. Notably, hints will be placed in the goal's world, not the world where the item can be found. It is used in the “Triforce Blitz S2” hint distribution.
  * `goal-legacy-single` is identical to `goal-legacy`, and is used in the “Triforce Blitz S2” hint distribution to place these hints with varying numbers of copies per hint.
  * `playthrough-location`, also known as “Wanderer”, hints the area of an item that appears in the spoiler log playthrough but is not on the Way of the Hero.
  * `unlock-woth` hints the name of two Way of the Hero items where the first is required to obtain the second.
  * `unlock-playthrough`, also known as “Wanderer Unlock”, hints the name of two items that appear in the spoiler log playthrough where the first is required to obtain the second. The first item may or may not be on the Way of the Hero, but the second definitely isn't.
  * `wanderer` randomly generates a `playthrough-location` or `unlock-playthrough` hint.
* New hint distributions:
  * “Chaos!!! (dev-fenhl)” is like “Chaos!!!” but including hint types not available on [`Dev-R`](https://github.com/Roman971/OoT-Randomizer).
  * “Chaos!!! (dev-fenhl, no goal hints)” is like “Chaos!!! (no goal hints)” but including hint types not available on [`Dev-R`](https://github.com/Roman971/OoT-Randomizer).
  * “Chaos!!! (TOoTR compat)” is like “Chaos!!!” but without entrance hints and without the Dual hints for the bosses.
  * “Ice%” consists of only Sometimes hints, and replaces the hookshot hint in Dampé's diary with a blue fire arrows hint.
  * “Mixed Pools Tournament” is the hint distribution being used for the upcoming [4th Mixed Pools Tournament](https://midos.house/event/mp/4).
  * “SAWS” is the hint distribution used for the Standard Anti-Weekly Settings presets (see below).
  * “SGL 2023” is the hint distribution used for the SpeedGaming Live 2023 tournaments (see below).
  * “Triforce Blitz S2” is the hint distribution used for the [Triforce Blitz Season 2 Tournament](https://midos.house/event/tfb/2), taken from [Elagatua's `Dev` branch](https://github.com/Elagatua/OoT-Randomizer/tree/Dev). Note that the tournament itself was played on that branch, not this one.
* New settings presets:
  * “Fenhl's Casual” is my preferred flavor of playing OoTR, with everything shuffled, full entrance randomizer, chaos hints, warp song note shuffle, half damage, no Master Quest, and no tricks enabled. The rainbow bridge requires all dungeon rewards and Ganon's boss key requires all 100 gold skulltula tokens.
  * “Fenhl's Casual (TOoTR compat)” is Fenhl's Casual but with the following changes for compatibility with [TOoTR](https://tootr.mracsys.com/):
    * “Open Door of Time” is set to “Song of Time” instead of “3 Stones + OoT + SoT”
    * Hint distribution is “Chaos!!! (TOoTR compat)” instead of “Chaos!!! (dev-fenhl)” ([mracsys/tootr#73](https://github.com/mracsys/tootr/issues/73), [mracsys/tootr#76](https://github.com/mracsys/tootr/issues/76), and lack of support for Triforce Blitz hints)
  * “Vanilla” generates a seed that's as close to the vanilla game as possible with current randomizer features. Unlike the vanilla seed available on <https://ootrandomizer.com/>, this preset uses glitchless logic to produce a useful spoiler log playthrough.
  * “Fast Vanilla” is “Vanilla” but with speed-ups like fast bunny hood, fast chest cutscenes, or “Skip Some Minigame Phases” enabled.
  * “Vanilla (Master Quest)” and “Fast Vanilla (Master Quest)” are “Vanilla” and “Fast Vanilla” but with all dungeons from Master Quest.
  * “Ice%” is an extremely fast-paced game mode where the goal is to reach the Iron Boots chest.
  * 4th Mixed Pools Tournament” is used for [an upcoming tournament](https://midos.house/event/mp/4) with nearly full mixed pools entrance randomizer.
  * “Standard Anti-Weekly Settings (Beginner)” disables every location that's enabled in “S9 Tournament” and enables every location that's disabled there, as well as changing some miscellaneous settings. See [the official document](https://docs.google.com/document/d/1vS7JrYjlWFkXfUnT5BJSQ19ywJa-TiB7RPK7Uz0JLsU/edit) for details.
  * “Standard Anti-Weekly Settings (Advanced)” adds “Shuffle Rupees & Hearts”, “Shuffle Pots”, “Shuffle Crates”, “Shuffle Beehives”, and “Shuffle Silver Rupees”, has a minimal item pool, and adds extra ice traps.
  * “Triforce Blitz S2” is a fast-paced game mode with very powerful hints used for [a tournament](https://midos.house/event/tfb/2), taken from [Elagatua's `Dev` branch](https://github.com/Elagatua/OoT-Randomizer/tree/Dev). Note that the tournament itself was played on that branch, not this one. See [the official website](https://www.triforceblitz.com/) for details.
  * “SDG Bingo Tournament 3” is a variant of “Bingo” used for an ongoing tournament. Note that the tournament itself is being played on version 8.0 of the randomizer, not this branch. See [the official document](https://docs.google.com/document/d/1fpDPSBGH9YQeC9W3P1SMDE7FhoikVgbFTJapqKnMmL4/edit) for details.
* Other changes:
  * New hint distribution option `boss_goal_names` (defaults to `true`) that can be disabled to force dungeon reward names to be used in Goal hints instead of boss names even if dungeon rewards aren't shuffled. ([#2416](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2416))
  * Spoiler logs explicitly list the type of each hint and plandos can specify hint types to place on gossip stones ([#2393](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2393))
  * Plandos can specify different settings for each world ([#2055](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2055))
  * The text box no longer shows the player's own gold skulltula token count when finding a token for another player (part of [#2055](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2055))
  * Support for multiworld on EverDrive ([#2042](https://github.com/OoTRandomizer/OoT-Randomizer/issues/2042))
  * Some settings have been renamed for clarity ([#1560](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1560))
  * The conditions for forcing one-way entrances that lead to the Bolero, Nocturne, and Requiem warp pads have been adjusted to increase variety with some settings, such as “Guarantee Reachable Locations” set to “All Goals”, “Shuffle Dungeon Rewards”, or “Mix Entrance Pools” (based on [#1440](https://github.com/OoTRandomizer/OoT-Randomizer/pull/1440))
  * New Dual hints that are off by default but can be enabled by custom hint distributions.
  * Gold skulltula tokens can be on excluded locations if there are no checks requiring them (such as in SAWS)
  * The GUI tweaks made by [`Dev-R`](https://github.com/Roman971/OoT-Randomizer) are further adjusted to balance consistency with [main `Dev`](https://github.com/OoTRandomizer/OoT-Randomizer) and ease of use.
  * The Lens of Truth can be in a foolish area if Treasure Chest Game keys are shuffled and all relevant “lensless” tricks are enabled.
  * Updated settings preset for the Standard Weeklies which now use the `boss_goal_names` hint distribution option.
* Bug fixes:
  * Potentially fix a bug causing an item to turn into a Gerudo mask when picked up at the same time a Triforce piece is received from the network ([#2406](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2406))
  * Fix Dual and Dual Always hints hinting locations in precompleted dungeons ([#2397](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2397))
  * Fix generator crash with bingo hints when all bottles (other than Ruto's letter) are milk bottles ([#2392](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2392))
  * Fix misc. location hints not being counted as always hints for the purpose of Barren and Named Item hints ([#2391](https://github.com/OoTRandomizer/OoT-Randomizer/pull/2391))
  * Fix conditions for `Deku Theater Rewards` becoming a Dual Always hint.

Differences between [`Dev-R`](https://github.com/Roman971/OoT-Randomizer) and [`Dev`](https://github.com/OoTRandomizer/OoT-Randomizer):

* Various GUI tweaks and improvements along with setting tooltip/option changes
* 2 new advanced ER settings: "Mixed Entrance Pools" and "Decouple Entrances"
* Picking up Gold Skulltula Tokens in non-Tokensanity displays a self-closing textbox which no longer freezes the player.
* New cosmetic setting to randomize the color of some additional equipment and items.

## Index

* [Installation](#installation)
* [General Description](#general-description)
  * [Getting Stuck](#getting-stuck)
  * [Settings](#settings)
  * [Known Issues](#known-issues)
* [Changelog](#changelog)

## Installation

It is strongly suggested users use the web generator from here:

<https://ootrandomizer.com/generatorDev?version=devFenhl_>

If you wish to run the script raw, clone this repository and either run ```Gui.py``` for a
graphical interface or ```OoTRandomizer.py``` for the command line version. They both require Python 3.9+.
To use the GUI, [NodeJS](https://nodejs.org/download/release/v20.17.0/) (v20 LTS, with npm) will additionally need to be installed. NodeJS v14.14.0 and earlier are no longer supported.
The first time ```Gui.py``` is run it will need to install necessary components, which could take a few minutes. Subsequent instances will run much quicker.
Supported output formats are .z64 (N64/Emulator), .wad (Wii VC, channel IDs NICE/NRKE recommended), Uncompressed ROM (for developmental purposes, offline build only)
and .zpf/.zpfz (patch files, for sharing seeds with others).

This randomizer requires The Legend of Zelda: Ocarina of Time version ```1.0 NTSC-US```. This randomizer includes an in-built decompressor, but if
the user wishes a pre-decompressed ROM may be supplied as input. Please be sure your input ROM filename is either a .n64 or .z64 file. For users
playing via any means other than on real N64 hardware, the use of the "Compress patched ROM" flag is strongly encouraged as uncompressed ROMs are
impossible to inject for the Virtual Console and have random crashing problems on all emulators.

For general use, there are four recommended emulators: [Project64 (v3.0+)](https://wiki.ootrandomizer.com/index.php?title=Project64), [Bizhawk](https://wiki.ootrandomizer.com/index.php?title=Bizhawk), [RetroArch](https://wiki.ootrandomizer.com/index.php?title=Retroarch) and [Dolphin (latest beta)](https://wiki.ootrandomizer.com/index.php?title=Dolphin). All are race-legal when configured appropriately.
In a nutshell the differences are:
* Project64 is the lightest emulator and the easiest to setup, however, you will need the 3.0.0 version or later to run OoTR well (and earlier versions are not permitted for use in OoTR races).
* Bizhawk is the most resource-intensive, but easier to set up than RetroArch and the only race-legal emulator to support [Multiworld](https://wiki.ootrandomizer.com/index.php?title=Multiworld).
* RetroArch is less resource-intensive than Bizhawk and the only of the three N64 emulators to work on platforms other than Windows, but it can be frustrating to set up.
* Dolphin lets you emulate Wii VC, giving you access to faster pauses. It's also lightweight, available on other platforms than Windows, easy to setup (on Windows at least) and offers good native support with most Gamecube Controller Adapters. It does come with more lag, although that can be mitigated through overclocking (this is not permitted for racing).

Please follow [the guides on our wiki](https://wiki.ootrandomizer.com/index.php?title=Setup#Emulators) carefully to ensure a stable game experience and that
[the settings requirements for races](https://wiki.ootrandomizer.com/index.php?title=Racing#Emulator_Settings_Requirements) are met. OoTR can also be run on
an N64 using an [EverDrive](https://wiki.ootrandomizer.com/index.php?title=Everdrive), or on [Wii Virtual Console](https://wiki.ootrandomizer.com/index.php?title=Wii_Virtual_Console). For questions and tech support we kindly refer you to our [Discord](https://discord.gg/q6m6kzK).

## General Description

This program takes _The Legend of Zelda: Ocarina of Time_ and randomizes the locations of the items for a new, more dynamic play experience.
Proper logic is used to ensure every seed is possible to complete without the use of glitches and will be safe from the possibility of softlocks with any possible usage of keys in dungeons.

The randomizer will ensure a glitchless path through the seed will exist, but the randomizer will not prevent the use of glitches for those players who enjoy that sort of thing, though we offer no guarantees that all glitches will have identical behavior to the original game.
Glitchless can still mean that clever or unintuitive strategies may be required involving the use of things like Hover Boots, the Hookshot, or other items that may not have been as important in the original game.

Each major dungeon will earn you a random Spiritual Stone or Medallion once completed.
The particular dungeons where these can be found, as well as other relevant dungeon information can be viewed in the pause menu by holding the D-Pad buttons on the C-Item Menu.
Note, however, that the unlock conditions for dungeon information are settings-dependent.

As a service to the player in this very long game, many cutscenes have been greatly shortened or removed, and text is as often as possible either omitted or sped up. It is likely that someone somewhere will miss the owl's interjections; to that person, I'm sorry I guess?

### Getting Stuck

With a game the size of _Ocarina of Time_, it's quite easy for new Randomizer players to get stuck in certain situations with no apparent path to progressing. 
Before reporting an issue, please make sure to check out [our Logic wiki page](https://wiki.ootrandomizer.com/index.php?title=Logic). 
We also have many community members who can help out in our [Discord](https://discord.gg/8nmX7fa).

### Settings

The OoT Randomizer offers many different settings to customize your play experience.
A comprehensive list can be found [here](https://wiki.ootrandomizer.com/index.php?title=Readme).

#### Plandomizer

"Plan"-domizer is a feature that gives some additional control over the seed generation using a separate distribution file. In such a file you can:
* Place items at specific locations or restrict items from being placed at specific locations.
* Add or remove items from the item pool.
* Select items to start with.
* Set specific dungeons to be vanilla vs Master Quest.
* Set which trials are required.
* Set any regular settings.

Caveat: Plandomizer settings will override most settings in the main OoTR generator settings, particularly list-based settings like enabled tricks or starting inventory. For example, if the Plandomizer distribution file contains an empty list of starting items, and the generator settings include additional starting equipment, the player will start with none of them instead. You will have to edit the Plandomizer file to change such settings, or **delete** completely the line in the Plandomizer file with the given setting to allow the main generator to alter the setting.

See [the Plandomizer wiki page](https://wiki.ootrandomizer.com/index.php?title=Plandomizer) for full details.

#### Custom Models

Save the .ZOBJ file of the desired model in `data/Models/Adult` or `data/Models/Child`. The file must be in .ZOBJ format (the compressed .PAK files are not compatible), but most Modloader64 models will work. Exceptions are models which are larger than the base Link models (the randomizer will give an error message) and those created on the new pipeline (technically load but the textures get wonky). Please see notes regarding known model files that are floating around [in this spreadsheet](https://docs.google.com/spreadsheets/d/1xbJnYw8lGR_qAkpvOQXlzvSUobWdX6phTm1SRbXy4TQ/edit#gid=1223582726) before asking why a model doesn't work.

Once the models are saved, the program may be opened and the model(s) selected under the `Cosmetics` tab.

If the model's skeleton is similar enough to Link, the randomizer will use Link's skeleton. If it is substantially different, then a note will be placed on the pause screen to make it clear that the skeleton was changed.

### Known issues

Unfortunately, a few known issues exist. These will hopefully be addressed in future versions.

* The fishing minigame sometimes refuses to allow you to catch fish when playing specifically on Bizhawk. Save and Hard Reset (NOT savestate) and return to fix the
issue. You should always Hard Reset to avoid this issue entirely.
* Versions older than 3.0 of Project64 have known compatablity issues with OoTR. To avoid this either 
[update to v3.0 and follow the rest of our Project64 guide](https://wiki.ootrandomizer.com/index.php?title=Project64) or change to one of our other two supported emulators.
* Executing the collection delay glitch on various NPCs may have unpredictable and undesirable consequences.
* This randomizer is based on the 1.0 version of _Ocarina of Time_, so some of its specific bugs remain.

## Changelog

The changelog has moved to [CHANGELOG.md](CHANGELOG.md).

For an overview of settings additions by release, see [the wiki](https://wiki.ootrandomizer.com/index.php?title=Readme#Generation_Options).
