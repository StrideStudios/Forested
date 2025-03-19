#include "Forested.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, Forested, "Forested" );

DEFINE_LOG_CATEGORY(Forested);
DEFINE_LOG_CATEGORY(ForestedEditor);

// 'Real Artists Ship' - Steve Jobs
//means get it done
//Animations with multiple of 60 frames seem to jitter or lag a little bit

//Lore/Story built into gameplay

/* TODO:
new game

TODO: selectable object channel instead of trace channel, use visibility pass to get?

cleanup grass material and landscape material

handle water component with no physics (just river movement)
load actors loading on startup
axe blend after hit will not switch items

structure:
introduction
discovery
solution

Fridays are the days the forest invades
you can trade golden acorns in a repurposed supply box (like from firewatch)
the inside of the lid has instructions (in an old cartoon-like art style)


https://forums.unrealengine.com/t/ue5-networking-should-this-go-into-the-gamemode/1191299/14
Roy Wierer

3.6.1  GameMode

- Exists only on the server.
- Does not replicate.
- Acts similar to a Controller in MVC.
- Holds private game rules and all game logic, little to no data.
- Instructs a GameState when a rule is met.
- Writes game related public data to the GameState if relevant for all clients.

 3.6.2  GameState

- Exists on the server and all clients.
- Replicates down from server to clients.
- Acts similar to a Model in MVC.
- Holds public game related data relevant to all clients, little to no logic.
- Held data examples are match timer, team score, winning team, current cards in deck.
- Can be instructed by the GameMode to switch stage types such as lobby, start of match, end of match, score board.

fix inits on this and player inventory (more this) and probably transfer over stuff from bp_player to here
work on migrating to soft references to lower memory stuff

tree products break when hitting the ground if too small (In Item Actor and spawn on fall?)
RenderActor->GetClass() == NextRenderActor (different slots instead?)

base premise is to use trees to get more upgrades and such

knife equipping in main hand, right click opens small (radial?) crafting menu
durability is no longer a concern, but sharpness is
?get better at crafting over time
?should have spray bottle
?no basket
?abandoned houses. Town
?charcoal crafting
?scrap under glowing flowers
?scrap launcher jams (how)
mailbox & shop & selling
kiln/forge
log holder/mold holder
log splitting for firewood

GOLDEN ACORNS (or something similar)
grow on trees and sparkle
can be found but rarely
used to unlock things including the ending area

stick launcher and stick quiver
stick quiver check ammo by reaching arm back and number displays along with symbol of stick
quiver is optional, otherwise sticks are stored in slots (given later)

//Mailbox
take orders to get money from people (maybe goods instead or both)
preferably can't stock up items and just constantly fulfill orders

Tools + Modifications:
* axe:
different types of head/handle
different materials
* stick launcher:
* stick quiver:
* trowel:
* ?machete:
* ?basket:

Hazards:
* Moving Deadly Trees (need name)
* Thorns
* Something else? Fire?

Materials:
* Wood
* Metal
* Sap
* resin/pitch

Trees:

Basic:
* Elm
* Birch
* Oak

Sap:
* Pine
* Maple
* Sycamore

Levels to unlock trees or perhaps requires a golden acorn?

Fruit:
* Apple
* Orange
* Lemon
* Pear
* Peach
* Mango

Produces Product That needs to be picked: will be destroyed if cut down
will regrow every certain amount of time

Increases Damage and Sharpness durability
Metals:
* Copper
* Tin
* Bronze
* Iron
* Steel

Increases speed/handling
Woods:
* Elm
* Birch
* Oak

The rest are just offshoots of those three



Missions/Loose mission design like new vegas

* Bunker/Laboratory dilapidated/destroyed, leads to an underground cavern with a grapple like in alien
some sort of weird fluid/ore that explains the odd tree behavior and such
something starts a fire and begins to burn down the forest
final push until escaping the valley

*/

/*
 *	Poster Art:
 *	Arm with axe cutting in half (/ or |) Looks like paper being cut in half
 *	left side nighttime, right side daytime
 *	Big bold letters: Forested
 */