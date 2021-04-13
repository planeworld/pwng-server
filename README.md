![build](https://github.com/planeworld/pwng-server/actions/workflows/ci.yml/badge.svg)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/363343886c1c4561ba600c900fa28e82)](https://www.codacy.com/gh/planeworld/pwng-server/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=planeworld/pwng-server&amp;utm_campaign=Badge_Grade)

## What is Planeworld

Planeworld is a galaxy/universe simulation engine (server component) and a [desktop client](https://github.com/planeworld/pwng-client) that can connect via websocket. The following screenshot (Figure 1) shows a procedurally generated galaxy with two spiral arms. Stars are are all within the main sequence (stellar classes M - O) for now. Stellar class distribution as well as masses, temperatures and radii are generated using probability distributions with respect to the information available.

![galaxy representation](screenshots/galaxy_2021-04-09.png?raw=true)

*Figure 1: Procedurally generated galaxy*

The second screenshot (Figure 2) shows details of some region in one of the galaxy arms. Note, that sizes of stars are exaggerated via slider in the GUI for a more symbolic representation.

![galaxy representation_detail](screenshots/galaxy_detail_2021-04-09.png?raw=true)

*Figure 2: Details of stars (size exaggerated via GUI for a more symbolic representation)*

### Planeworld history: A journey of a hobby coder

Well, the original [Planeworld](https://github.com/planeworld/planeworld) has been playground for everything I wanted to implement, but it started with a much narrower scope: In 1997 I didn't like graphics programming any more, because all of my beautifully handcrafted polygon drawing algorithms, Gouraud-"shaders" and ASM-optimisations on the edge between processor and coprocessor where somehow deprecated by something new: dedicated (3D) graphics cards, the first GPU's, such as S3's ViRGE and 3dfx' Voodoo cards. So I decided to go for something else: physics, or rigid body dynamics, to be more precise.
I decided very early to only go for 2D, because that would simplify the problem and would ensure, that I as a single hobby developer could cope with state of the art algorithms.
Time went on, with rigid body dynamics came larger worlds, procedural generation, noise functions, planet generation, athmospheric simulation, galaxies... I had a lot of topics, always started some new aspect, going from micro to macro (as in: falling boxes to procedural planets). I wanted everything to be dynamic, flexible, modular. And it was implementing for the sake of implementing and learning. So I had an internal command interface where external scripting languages such as Lua could attach very easily, implemented multithreading with command queues, and tried to bring my simple OpenGL fixed function pipeline to a more recent core profile shader based graphics engine. Many ideas needed refactoring. One such thing was a clean client-server architecture. 

Sometime ago I just needed a break, started some pet project [BattleSub](https://github.com/bfeldpw/battlesub) to get to know a little more about GPU shaders and fluid dynamics. I wanted to make fast progress and decided to use middlewares, such as [Magnum](https://github.com/mosra/magnum) and [EnTT](https://github.com/skypjack/entt). So...

### Why Planeworld NG?

Based on my positive experiences from [BattleSub](https://github.com/bfeldpw/battlesub) with [Magnum](https://github.com/mosra/magnum) and [EnTT](https://github.com/skypjack/entt) and decades of lessons learned concerning Planeworld, I decided to restart my passion project by consequently using a client server mechanic, and utilising EnTT and Magnum Graphics, as well as some others (credits will follow). While the server component doesn't rely on a graphical interface, check out Magnum Graphics used by the desktop client, too.

## Behind the scenes

## EnTT

At the server side, EnTT is used for two main purposes at the moment:
1. Each object, e.g. a star, is an entity with several components. This might be metadata such as temperature and stellar class or kinematics/kinetics information like position, velocity, acceleration. Objects with the latter pass the integrator system, which integrates all acceleration and subsequently velocities to their final position.
2. Each connection to a client is an entity, too. This allows for component-based subscriptions. For each type of subscription there will be an accordant component. If a client requests a certain subscription, the representing entity will simply get the relevant subscription component attached. This way, all subscriptions can be handled by iterating over the component views.

## Client-Server Communication Protocol

The communication protocol will be specified in the wiki: [Application Protocol Draft](https://github.com/planeworld/pwng-server/wiki)
