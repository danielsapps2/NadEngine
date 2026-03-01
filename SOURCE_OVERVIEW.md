# Minicraft Source Overview

This document explains how the project is structured, what design philosophy holds it together, and how you would implement similar systems yourself.

It is not a literal history of the author's work. The repository only has two commits, and the first one already contains the complete game. What follows is a reconstruction from the current source layout and dependencies.

Key source anchors:

- `src/com/mojang/ld22/Game.java`
- `src/com/mojang/ld22/gfx/Screen.java`
- `src/com/mojang/ld22/InputHandler.java`
- `src/com/mojang/ld22/level/Level.java`
- `src/com/mojang/ld22/level/levelgen/LevelGen.java`
- `src/com/mojang/ld22/level/tile/Tile.java`
- `src/com/mojang/ld22/entity/Entity.java`
- `src/com/mojang/ld22/entity/Mob.java`
- `src/com/mojang/ld22/entity/Player.java`
- `src/com/mojang/ld22/item/Item.java`
- `src/com/mojang/ld22/crafting/Crafting.java`
- `src/com/mojang/ld22/screen/Menu.java`

## Core Philosophy

The project is built like a very small engine with content layered on top of it.

The governing ideas are:

1. Keep the rendering model primitive and deterministic.
2. Make the world a tile grid first, and let nearly all gameplay emerge from tile interaction.
3. Build a generic entity model with a few hooks, then specialize it for player, enemies, furniture, and dropped items.
4. Use procedural generation to create large replayable worlds, but validate the result so it remains playable.
5. Express game progression as combinations of simple systems rather than as bespoke scripted sequences.

This is why the code feels compact. The game does not have many separate subsystems doing unrelated things. It has a few foundational abstractions that are reused everywhere.

## Likely Build Order

If you wanted to build a similar game, the most defensible order is:

1. Window creation, fixed update loop, framebuffer, and sprite blitting.
2. Keyboard input abstraction.
3. Tile map representation and camera scrolling.
4. Generic level container and tile registry.
5. Procedural map generation and map validation.
6. Generic entity base class and collision rules.
7. Mobile actor base class with health, hurt state, knockback, and swimming logic.
8. Player movement and basic interaction verbs.
9. World tiles with special behavior: trees, rocks, water, farming, stairs, lava, ores.
10. Items, resources, and dropped item entities.
11. Enemies, spawning rules, and boss layer.
12. Furniture and crafting stations.
13. Menus, title flow, death flow, win flow, and GUI.
14. Build tooling and packaging.

This order matters because each system in Minicraft depends on the one below it. The project is not content-first. It is foundation-first.

## System Overview

### 1. Game Loop and Rendering

The game starts in `Game.java`. That file is responsible for:

- Owning the window-facing canvas.
- Running a fixed-timestep loop.
- Keeping a software pixel buffer.
- Delegating update and render work to the world and menus.

The ideology here is simple: separate simulation time from rendering time, and keep the world update deterministic. The loop ticks at a fixed rate, then renders whatever the current state is.

How to build this system:

1. Decide on a tiny internal resolution and scale it up when presenting it.
2. Draw into a software pixel buffer instead of drawing gameplay directly to the OS window.
3. Run updates at a fixed cadence so movement, timers, and AI stay consistent.
4. Make rendering a pure read of current state.
5. Keep this layer ignorant of game-specific rules except for orchestration.

Why this matters:

- A low internal resolution makes a full-world redraw cheap.
- A software renderer keeps the mental model simple.
- Fixed update timing makes combat, stamina, spawn pacing, and animation easier to reason about.

### 2. Screen and Palette Model

`Screen.java` is not a scene graph. It is a raw pixel surface with a sprite-sheet blitter and a light overlay pass.

The ideology is:

- Do almost nothing automatically.
- Treat every visual as a tile or sprite copied into pixels.
- Keep visual style constrained by a palette system.

How to build it:

1. Load one sprite sheet and treat it as a tile atlas.
2. Render by copying small fixed-size tiles into a pixel buffer.
3. Store colors in a compact encoded form rather than full arbitrary colors everywhere.
4. Add a second pass for lighting if needed.
5. Keep camera offset as a simple subtraction during rendering.

Why this matters:

- The renderer stays fast because it is repetitive and predictable.
- The art style stays coherent because the palette rules are narrow.
- Lighting becomes a compositing effect, not a different rendering architecture.

### 3. Input as an Intent Layer

`InputHandler.java` converts raw keyboard events into higher-level intent flags like `up`, `down`, `attack`, and `menu`.

The ideology is to avoid scattering raw key checks through gameplay code.

How to build it:

1. Create named actions rather than exposing raw keyboard codes.
2. Track both held state and one-shot click state.
3. Advance input state once per game tick.
4. Let game systems query actions instead of reading hardware events directly.

Why this matters:

- Menus and gameplay can share the same intent model.
- The player logic stays readable.
- You can later remap controls without rewriting core gameplay.

### 4. World as Tiles First

The real center of the game is not the player. It is the tile world.

`Level.java` stores:

- Tile ids.
- Tile metadata.
- Lists of entities.
- Spatial buckets for nearby entity lookup.
- Per-layer configuration such as depth and monster density.

The ideology is that the map itself is the primary simulation surface. Everything meaningful happens because entities stand on, collide with, or mutate tiles.

How to build it:

1. Represent the world as arrays of tile ids and per-tile state.
2. Treat the tile registry as a catalog of behavior, not only artwork.
3. Give the level ownership of entities and entity lookup.
4. Keep rendering and ticking centered on visible tile ranges rather than whole-map brute force when possible.
5. Let the level mediate between tile logic and entity logic.

Why this matters:

- Terrain becomes gameplay, not background.
- World mutation is cheap because a tile is just an id and a small state value.
- Procedural generation has a clear output format.

### 5. Tile Registry as Behavior Catalog

`Tile.java` is a common pattern worth copying. It is a global registry of tile types where each tile can override behavior hooks:

- render
- mayPass
- hurt
- steppedOn
- interact
- use
- light emission

The ideology is that terrain rules belong to terrain types, not to a single giant switch statement in the world class.

How to build it:

1. Define a base tile type with a small set of behavior hooks.
2. Register all concrete tile types in one place.
3. Store only the tile id in the map.
4. Resolve behavior by asking the tile object for its rules.
5. Use per-tile metadata only for state that changes over time, like growth or damage.

Why this matters:

- Adding a new terrain type does not require rewriting the level engine.
- Harvesting, farming, lighting, and passability all plug into the same structure.
- The world rules remain local and modular.

### 6. Entity Base Before Specific Actors

`Entity.java` handles:

- Position and bounds.
- Intersection tests.
- Movement along one axis at a time.
- Tile collision checks.
- Entity collision checks.
- Basic interaction hooks.

The ideology is to create one neutral movement and collision vocabulary that everything can share.

How to build it:

1. Define a base entity with world position and collision radius.
2. Make movement check tiles first and entities second.
3. Move one axis at a time to keep collision logic simple.
4. Give entities small overridable hooks for interaction and blocking.
5. Keep this class generic enough for furniture, items, enemies, and player.

Why this matters:

- All game objects inhabit one consistent world.
- Collision bugs become easier to isolate.
- Later content can be added by subclassing rather than inventing parallel systems.

### 7. Mob Layer for Combat-Capable Life

`Mob.java` sits between generic entities and living actors.

It adds:

- Health.
- Hurt timing.
- Knockback.
- Facing direction.
- Swimming penalties.
- Death behavior.
- Spawn placement.

The ideology is to separate "thing in the world" from "living actor subject to combat."

How to build it:

1. Start from generic entity movement.
2. Add health and damage response.
3. Add temporary hurt invulnerability to avoid instant stun loops.
4. Add knockback as positional pressure, not just damage numbers.
5. Add shared environmental responses like lava damage or swimming slowdown.

Why this matters:

- The player and enemies can share most combat behavior.
- Enemy implementation becomes mostly AI and drop tuning.
- Survival pressure comes from movement constraints as much as from raw damage.

### 8. Player as a Composition Point

`Player.java` is where many systems meet:

- Input.
- Stamina.
- Attacks.
- Item use.
- Tile interaction.
- Entity interaction.
- Inventory.
- Level transitions.

The ideology is that the player should not directly own special-case world rules. The player should mostly route actions into tiles, items, and entities.

How to build it:

1. Let input create movement and action requests.
2. Resolve actions in a strict order: use entity, use tile, use active item, then default attack.
3. Put stamina or action cost at the player layer so all verbs share one economy.
4. Keep movement, combat, and interaction spatially local with short reach boxes.
5. Let the player call into the world instead of embedding world-specific logic inside the player.

Why this matters:

- New tools and tiles can expand the game without bloating player code too much.
- Combat and harvesting feel unified because they share the same action pipeline.
- The game creates depth from context, not from complicated input grammar.

### 9. Items, Resources, and Furniture

Items in Minicraft are intentionally abstract. The base `Item` is little more than a set of hooks. Concrete meaning is defined by subclasses and by resources in `item/resource/Resource.java`.

The ideology is:

- Keep items lightweight.
- Let many gameplay mechanics be item-mediated interactions.
- Represent progression through better tools and conversion recipes, not through large stat trees.

How to build it:

1. Create a minimal item interface with hooks for inventory rendering, world interaction, and attack bonuses.
2. Split resources from tools and furniture so each category has a clear role.
3. Make dropped items world entities so gathering feels physical.
4. Use tools to gate which tiles can be harvested efficiently.
5. Use furniture as world-placed production interfaces rather than abstract UI-only stations.

Why this matters:

- Progression becomes tangible and spatial.
- Resource loops can stay simple yet expressive.
- The world feels inhabited because production exists inside it.

### 10. Crafting as Progression Graph

`Crafting.java` is mostly data. That is important.

The ideology is that crafting is not a subsystem with heavy logic. It is a graph of conversions and station gates:

- workbench for early tools and structures
- furnace and oven for material conversion
- anvil for higher-tier gear

How to build it:

1. Decide what each station unlocks in the progression ladder.
2. Express recipes as ingredient lists and outputs.
3. Keep recipe execution generic and keep content in data declarations.
4. Tie stations to the world so crafting requires placement and access.
5. Make resource transformation reinforce exploration across layers.

Why this matters:

- Progression can be tuned without rewriting systems.
- The player has reasons to gather, descend, smelt, and upgrade.
- Crafting serves world exploration rather than replacing it.

### 11. Menus and Flow

Menus in this project are thin overlays on top of the same rendering and input systems used everywhere else.

The ideology is to avoid building a separate UI framework. Menus are just another mode of the game.

How to build it:

1. Create a tiny menu base with `tick` and `render`.
2. Let the game own the current menu.
3. Pause or redirect world simulation depending on menu state.
4. Keep inventory and crafting menus as list navigators, not drag-and-drop interfaces.
5. Use menus to expose state that already exists in the world model.

Why this matters:

- UI complexity stays contained.
- Control scheme remains consistent.
- Jam-scale scope remains manageable.

## Level Generation Deep Dive

This is the most instructive system in the repository.

`LevelGen.java` is not using a general-purpose terrain library. It uses a hand-rolled fractal field generator and then composes multiple independent fields into a map through thresholding, stamping, and validation.

The philosophy is:

1. Generate broad natural-looking structure with noise.
2. Convert that structure into gameplay terrain using hard rules.
3. Add local variation in post-processing passes.
4. Reject worlds that are statistically wrong or likely unfun.

That last step is crucial. The generator is not trying to model geology. It is trying to produce playable survival maps.

### What Kind of Noise It Uses

The `LevelGen` constructor creates a grid of random samples at a coarse spacing and repeatedly refines it by:

- averaging corners to create center values
- averaging nearby values to create edge values
- adding random displacement at each scale
- halving step size until detail reaches the tile scale

This is best thought of as a diamond-square style fractal value field with wraparound sampling.

Important characteristics:

- It is cheap.
- It creates coherent blobs instead of white noise speckle.
- It gives large-scale shapes first, then smaller detail.
- Wraparound sampling avoids hard seams at map edges in the field itself.

The field alone is not the map. It is only raw material.

### Why Multiple Noise Fields Are Combined

One noise field usually gives soft continents or blobs. That is not enough for an interesting survival map.

Minicraft generates several independent fields and compares them. The common pattern is:

- generate field A
- generate field B
- compare them with absolute difference
- sometimes introduce a third field to disrupt regularity

This matters because the difference between two smooth fields produces ridges, pockets, channels, and broken regions more interesting than a single thresholded blob map.

The system is not asking, "What is the elevation here?"

It is asking, "How strongly do these overlapping patterns disagree here, and should that disagreement become water, rock, dirt, or open space?"

That is a very game-oriented way to use noise.

### Top Layer Strategy

The surface map is built in phases:

1. Generate base terrain fields.
2. Combine them into a scalar value.
3. Apply a strong edge falloff so the world trends toward water at the boundaries.
4. Threshold the result into water, rock, and grass.
5. Stamp sand patches over some grass.
6. Stamp tree clusters over some grass.
7. Stamp flower patches with per-tile color metadata.
8. Scatter cacti onto sand.
9. Place stairs only where a local 3x3 region satisfies strict terrain rules.
10. Validate the final counts and regenerate if the map lacks critical terrain types.

The design ideology is layered authorship:

- Noise creates believable macro shape.
- Rule-based passes create biome identity.
- Validation preserves gameplay affordances.

This is stronger than relying on one clever noise function.

### Underground Strategy

Underground generation is not just the top map with different colors. It uses several overlapping fields to separate:

- solid rock mass
- diggable dirt regions
- water or lava basins
- ore distribution

Then it adjusts output by depth:

- deeper levels reduce stairs availability
- deeper levels shift liquid from water to lava
- ore tier changes with depth
- monster density and feel change by layer

This shows a key procedural design lesson:

Do not only randomize space. Randomize the meaning of space by layer.

The underground generator also places ores in a later pass rather than baking them directly into the first thresholding step. That is good design because resources are not just terrain; they are rewards and progression gates.

### Sky Layer Strategy

The sky level is even simpler:

- use noise to define cloud mass versus empty fall space
- invert the logic compared with ground generation
- stamp cloud cactus as a local feature
- place stairs on stable cloud patches

The important idea is not realism. It is thematic contrast with familiar mechanics:

- same engine
- same tile model
- different traversal risk

### Edge Falloff and Why It Matters

A repeated pattern in the generator is the edge-distance adjustment.

The map computes how far a point is from the center, exaggerates that distance sharply, and uses it to push terrain toward emptier boundary states.

The effect is:

- surface maps taper toward water
- sky maps taper toward void
- playable density stays concentrated toward the interior

This is a practical technique with two benefits:

1. It makes the world feel island-like and bounded.
2. It protects the generator from filling the entire map with equally dense content.

Without edge falloff, noise often produces maps that are technically random but spatially monotonous.

### Validation as a First-Class Design Tool

The generator does not trust itself.

After producing a candidate map, it counts important tile types and rejects the world if it lacks enough of them.

Examples of things it validates:

- enough rock
- enough sand
- enough grass
- enough trees
- enough cloud
- enough stairs
- enough ore for the given depth

This is one of the most valuable lessons in the codebase.

Noise does not guarantee playability. Validation does.

If you build a procedural game, you should think in terms of:

- generation
- measurement
- rejection or repair

not just generation.

### Stairs and Inter-Layer Connectivity

A procedural world with multiple layers is only as good as the links between layers.

Minicraft handles this in two stages:

1. The generator places downward stairs where the surrounding terrain is suitable.
2. `Level.java` reads the parent level and mirrors those points as upward stairs in the connected layer.

Then it adjusts nearby tiles to make the stair landing sensible for that depth.

This is a strong systems idea:

Generate layers independently where possible, but synchronize the transition points so the world remains legible and navigable.

### The Hidden Development Tool

`LevelGen.java` includes a `main` method that renders generated maps into a quick preview window.

This reveals an important practical philosophy:

- procedural systems need fast visual feedback
- you should be able to inspect many generations quickly
- iteration speed matters more than theoretical elegance

When building your own version, plan for a generator preview mode early.

## How to Recreate a Similar Level Generator

If you want to build a similar system from scratch, this is the cleanest implementation plan.

### Phase 1: Raw Field Generator

Goal:
Create one coherent scalar field over the map.

Steps:

1. Pick a map size that works cleanly with repeated halving.
2. Seed coarse random values at regular intervals.
3. Repeatedly refine the field by interpolating and perturbing.
4. Tune roughness so large structure dominates over tiny noise.
5. Add a viewer so you can inspect the raw field.

Success criteria:

- broad contiguous regions
- no single-tile speckle dominance
- a clear sense of macro shape

### Phase 2: Terrain Classification

Goal:
Convert one or more scalar fields into terrain identities.

Steps:

1. Generate at least two independent fields.
2. Compare them to produce more interesting variation than one field alone.
3. Add strong edge falloff.
4. Choose thresholds that map scalar ranges to tile categories.
5. Visualize the thresholded result immediately.

Success criteria:

- clear land and obstacle regions
- navigable patches
- visual variety at multiple scales

### Phase 3: Post-Process Feature Stamping

Goal:
Layer biome flavor and resource distribution over the base terrain.

Steps:

1. Stamp sand or dirt regions into otherwise generic areas.
2. Stamp vegetation clusters onto valid base terrain.
3. Scatter small decorative or harvestable features.
4. Place resource nodes in thematically appropriate host terrain.
5. Keep each pass constrained by terrain rules.

Success criteria:

- map reads as biomes rather than abstract blobs
- resources feel discoverable, not arbitrary
- decoration strengthens traversal decisions

### Phase 4: Progression and Connectivity

Goal:
Make the map support the larger game loop.

Steps:

1. Place stairs or exits on safe, legible terrain.
2. Ensure layer transitions correspond across maps.
3. Adjust each depth's material and hazard profile.
4. Make sure resource tiers align with progression needs.
5. Reserve some unique content for special layers.

Success criteria:

- the player can always move through the progression path
- each layer has a distinct strategic purpose
- transitions feel intentional, not arbitrary

### Phase 5: Validation and Regeneration

Goal:
Protect the player from bad procedural outcomes.

Steps:

1. Count critical terrain and resource categories after generation.
2. Set minimum acceptable thresholds.
3. Reject maps that fail the thresholds.
4. Add repair logic only if rejection becomes too expensive.
5. Continuously test generation at scale.

Success criteria:

- few unwinnable seeds
- few dull seeds
- stable map quality across many generations

## Major Lessons to Carry Into Your Own Project

### Build Systems That Feed Each Other

The best thing about Minicraft is not any individual mechanic. It is how the mechanics reinforce one another:

- noise creates terrain
- terrain gates resources
- resources unlock tools
- tools expand terrain interaction
- stairs unlock deeper terrain
- deeper terrain yields stronger resources

This loop is why the game feels larger than its code size.

### Prefer Small Reusable Hooks Over Giant Managers

Tiles, items, entities, and menus all use tiny overridable hooks. That keeps the architecture extensible without making it abstract for abstraction's sake.

### Validate Procedural Output

Procedural generation is not finished when you sample noise. It is finished when the result supports the intended play loop.

### Use Constraint as Style

The renderer, palette, tile size, input grammar, and UI all operate under strong limits. Those limits create clarity and coherence.

## If You Want to Build Your Own Similar Game

A good philosophy is:

1. Start with a toy engine, not a content pile.
2. Make one reliable loop for movement, rendering, and interaction.
3. Let tiles carry much of the game design.
4. Use procedural generation to support replayability, but validate it aggressively.
5. Add content only after the foundation already feels satisfying.

That is the deepest pattern visible in this repository.
