![build](https://github.com/planeworld/pwng-server/actions/workflows/ci.yml/badge.svg)

## What is Planeworld

Planeworld is a galaxy/universe simulation engine (server component) and a desktop client that can connect via websocket. The following screenshot shows a procedurally generated galaxy with two spiral arms. Stars are are all within the main sequence (stellar classes M - O) for now. Stellar class distribution as well as masses, temperatures and radii are generated using probability distributions with respect to the information available.

![Very early galaxy representation](screenshots/galaxy_2021-04-09.png?raw=true)

### A journey of a hobby coder

Well, the original [Planeworld](https://github.com/planeworld/planeworld) has been playground for everything I wanted to implement, but it started with a much narrower scope: In 1997 I didn't like graphics programming any more, because all of my beautifully handcrafted polygon drawing algorithms, Gouraud-"shaders" and ASM-optimisations on the edge between processor and coprocessor where somehow deprecated by something new: graphics dedicated cards, the first GPU's, such as 3dfx' voodoo cards. So I decided to go for something else: physics, or rigid body dynamics, to be more precise.
I decided very early to only go for 2D, because that would simplify the problem and would ensure, that I as a single hobby developer could cope with state of the art algorithms.
Time went on, with rigid body dynamics came larger worlds, procedural generation, noise functions, planet generation, athmospheric simulation, galaxies... I had a lot of topics, always started some new aspect, going from micro to macro (as in: falling boxes to procedural planets). I wanted everything to be dynamic, flexible, modular. And it was implementing for the sake of implementing and learning. So I had an internal command interface where external scripting languages such as Lua could attach very easily, implemented multithreading with command queues, and tried to bring my simple OpenGL fixed function pipeline to a more recent core profile shader based graphics engine. Many ideas needed refactoring. One such thing was a clean client-server architecture. 

Sometime ago I just needed a break, started some pet project [BattleSub](https://github.com/bfeldpw/battlesub) to get to know a little more about GPU shaders and fluid dynamics. I wanted to make fast progress and decided to use middlewares, such as [Magnum](https://github.com/mosra/magnum) and [EnTT](https://github.com/skypjack/entt). So...

### Why Planeworld NG?

Based on my positive experiences from [BattleSub](https://github.com/bfeldpw/battlesub) with [Magnum](https://github.com/mosra/magnum) and [EnTT](https://github.com/skypjack/entt) and decades of lessons learned concerning Planeworld, I decided to restart my passion project [Planeworld](https://github.com/planeworld/planeworld) by consequently using a client server mechanic, an ECS (EnTT) and a graphics (and much more) middleware (Magnum Graphics). While the server component doesn't rely on a graphical interface, check out Magnum Graphics used by the desktop client, too.

## Client-Server Communication Protocol

The communication protocol will be specified in the wiki: [Application Protocol Draft](https://github.com/planeworld/pwng-server/wiki)
