[![CI](https://github.com/QuantumEntangledAndy/GTS_Plugin/actions/workflows/build.yml/badge.svg)](https://github.com/QuantumEntangledAndy/GTS_Plugin/actions/workflows/build.yml)

# Size Matters SKSE64 Plugin

This is the source for the Gianttess mod Size matters SKSE plugin.

WIP

---

## Feature Wish List

- [ ] [1] Auto scale height to room
- [ ] [2] Correct bumper for height
- [ ] [3] Talk to actor when crouching
- [X] [4] Accurate Body Collision that inflicts size effects on contact instead of using cloak magiceffects
- [X] [5] Fixed NPC headtracking
- [X] [6] Fixed Player and NPC headtracking
- [ ] [7] Ability to edit HitBoxes so it'll be possible to have Giant Enemies/Have more accurate hitbox
  - AABB was shared between all actors of the same skeleton
  - Found a way to clone the AABB and make them unique to each actor
- [X] [8] Ability to spawn dust effect explosions literally under the feet, not under center of character. Ideally even scale them.
- [X] [9] Maybe proper High-Heel module that dynamically adjusts high heel size based on Root scale?
- [X] [10] In the perfect scenario, repair Animation Speed based on size
- [X] [11] Transfer from SkyrimPlatform to Pure DLL, so SP won't be needed
- [X] [12] Disable swimming when huge enough: make player sink instead of swimming when huge enough or based on size
- [X] [13] Make DLL track Giantess PC/NPC footsteps to shake the screen/place dust without lags. Currently it has a delay because of script latency.
- [X] [14] Vore on button press
- [ ] [15] Blood on the feet/hand after crushing
- [ ] [16] Quest progression ui/current size ui
- [X] [17] Vore anim and other animations
- [ ] [18] Vore belly
- [ ] [19] Integrate new Potions into the world
- [ ] [20] .dll optimization so it will eat less CPU and have better performance
- [ ] [21] Affect stealth by size 
- [ ] [22] Make feet deal damage and effects based on it's speed instead of being able to kill everyone while standing still

## Easier Things
- [ ] Scale
  - [x] Change scale in papyrus
  - [ ] Get height in meters of any actor
  - [ ] Get volume of any actor
- [x] Mechanics  
  - [x] Apply size effects to near by actor
  - [x] Move away from favor active
  - [x] Animation speed adjustment
- [X] Walk/Jump events
  - [x] Camera distance to event
  - [X] Camera shake
  - [X] Feet sounds
