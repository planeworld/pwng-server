# Planeworld Next Generation (PWNG)

## Development

| Status | |
|--------|-|
| Server | ![build](https://github.com/planeworld/pwng-server/actions/workflows/ci.yml/badge.svg) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/363343886c1c4561ba600c900fa28e82)](https://www.codacy.com/gh/planeworld/pwng-server/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=planeworld/pwng-server&amp;utm_campaign=Badge_Grade) ![lastcommit](https://img.shields.io/github/last-commit/planeworld/pwng-server) |
| [Desktop Client](https://github.com/planeworld/pwng-client) | ![build](https://github.com/planeworld/pwng-client/actions/workflows/ci.yml/badge.svg) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/8d8325f947844b9f86d0947d28b6692f)](https://www.codacy.com/gh/planeworld/pwng-client/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=planeworld/pwng-client&amp;utm_campaign=Badge_Grade) ![lastcommit](https://img.shields.io/github/last-commit/planeworld/pwng-client) |

### What happened and what's to come

#### Server

- [ ] Procedurally generate planets based on Poisson distribution
- [ ] Accept subscriptions to specific star systems
- [x] 2021-08-27: Using the broker, JSON-RCP requests will now generate correct response, i.e result or error
- [x] 2021-08-25: Finally, a message broker has been implemented to centralise message handling. In the future this allows for easier implementation of choosable updade frequencies.
- [x] 2021-07-01: The server transmits two timestamps now: The first one is a simulation time, which just increments seconds (as a floating point number, so milliseconds are covered) and overflows when reaching a year, counting years in an unsigned integer variable. This is the virtual simulation time which is independend of any system clock and therefore load / processing power. Second is a realtime timestamp which transmits microseconds since epoch (1970-01-01, 00:00 UTC) and should be quite portable. This timestamp is mainly usable for syncing clients and interpolation/extrapolation of frames for smooth realtime visuals.
- [x] 2021-06-24: There's a very basic integration of box2d for now. Longterm goal ist, to have local box2d world instances where objects need sophisticated physics, i.e. joint-based rigid body interactions. In this proof-of-concept, a very simple, local tire model has been implemented, which consists of the rim (mass) and the tire (several radially distributed point masses) that are connected via springs/dampers (distance joint). For now, it's simply at the coordinate systems origin, later, it will be bound to a parent (e.g. moon, local region of a moon)

#### Desktop Client
- [ ] Use image pyramids and blur shaders to make the galaxy more smooth/dense looking
- [x] 2021-09-03: An additional branch *glfw* has been added which can be alternatively chosen in order to use GLFW instead of SDL2 for window initialisation, OpenGL context creation, and input handling.
- [x] 2021-06-21: The client draws the experimental proof of concept box2d tire model when subscribing to dynamic data
- [x] 2021-06-13: The user can now subscribe and unsubscibe to star systems by name using the GUI. The request is send, but the server doesn't answer yet. Eventually, star system subscriptions will replace the current subscription to dynamic data. Automatic subscription will later subscibe based on viewport and zoom level.
- [x] 2021-05-30: Rendering to texture in a different resolution allows for lower resolutions in comparison to SSAA as well. There is now a GUI slider to choose the render resolution factor in the range [0.1, 4.0]. For factors > 1.0 the aforementioned blur shader is parameterized dynamically as a lowpass filter to ensure proper subsampling. A very simplistic LOD is implemented, too, but will be improved later on by replacing the stock shaders in order to reduce draw calls. 
- [x] 2021-05-27: The scene is now rendered to a texture via FBO. This allows for rendering to higher resolutions than window/screen size. Hence, SSAA (super sampling anti aliasing) is achieved by applying a gaussian blur shader and then rendering the texture to native window/screen resolution. For now, the default value is targeted at 4xSSAA. The maximum texture size is queried and dynamically limits super sampling factor based on window resolution.
- [x] 2021-05-15: Static simulation objects such as stars and star systems of the galaxy are now fully separated from dynamic objects (for the moment that are the manually added Sun, Earth and Moon.
- [x] 2021-05-15: The camera can hook on all objects, static and dynamic objects

## What is Planeworld

Planeworld is a 2D galaxy/universe simulation engine (server component) and a [desktop client](https://github.com/planeworld/pwng-client) that can connect via websocket. The following screenshot (Figure 1) shows a procedurally generated galaxy with two spiral arms. Stars are are all within the main sequence (stellar classes M - O) for now. Stellar class distribution as well as masses, temperatures and radii are generated using probability distributions with respect to the information available.

![galaxy representation](screenshots/galaxy_2021-04-09.png?raw=true)

*Figure 1: Procedurally generated galaxy*

The second screenshot (Figure 2) shows details of some region in one of the galaxy arms. Note, that sizes of stars are exaggerated via slider in the GUI for a more symbolic representation.

![galaxy representation_detail](screenshots/galaxy_detail_2021-04-09.png?raw=true)

*Figure 2: Details of stars (size exaggerated via GUI for a more symbolic representation)*

### Planeworld history: A journey of a hobby coder

Well, the original [Planeworld](https://github.com/planeworld/planeworld) has been playground for everything I wanted to implement, but it started with a much narrower scope: In 1997 I didn't like graphics programming any more, because all of my beautifully handcrafted polygon drawing algorithms, Gouraud-"shaders" and ASM-optimisations on the edge between processor and coprocessor where somehow deprecated by something new: dedicated (3D) graphics cards, the first GPU's, such as S3's ViRGE and 3dfx' Voodoo cards. So I decided to go for something else: physics, or rigid body dynamics, to be more precise.
I decided very early to only go for 2D, because that would simplify the problem and would ensure, that I as a single hobby developer could cope with state of the art algorithms.
Time went on, with rigid body dynamics came larger worlds, procedural generation, noise functions, planet generation, athmospheric simulation, galaxies... I had a lot of topics, always started some new aspect, going from micro to macro (as in: falling boxes to procedural planets). I wanted everything to be dynamic, flexible, modular. And it was implementing for the sake of implementing and learning. So I had an internal command interface where external scripting languages such as Lua could attach very easily, implemented multithreading with command queues, and tried to bring my simple OpenGL fixed function pipeline to a more recent core profile shader based graphics engine. Many ideas needed refactoring. One such thing was a clean client-server architecture, and with emerging open source platforms I realised, that there were a lot of things that others did a lot better than me. 

So, sometime ago I just needed a break, started some new pet project [BattleSub](https://github.com/bfeldpw/battlesub) to get to know a little more about GPU shaders and fluid dynamics. I wanted to make fast progress and decided to use middlewares, such as [Magnum](https://github.com/mosra/magnum) and [EnTT](https://github.com/skypjack/entt). So...

### Why Planeworld NG

Based on my positive experiences from [BattleSub](https://github.com/bfeldpw/battlesub) with [Magnum](https://github.com/mosra/magnum) and [EnTT](https://github.com/skypjack/entt) and decades of lessons learned concerning Planeworld, I decided to restart my passion project by consequently using a client server mechanic, and utilising EnTT and Magnum Graphics, as well as some others (credits will follow). While the server component doesn't rely on a graphical interface, check out Magnum Graphics used by the desktop client, too.

## Installation and running

By default, pwng-server and pwng-client will be installed locally with all their dependencies. While the code should be portable, for now it is only tested against Linux. Hence, helper scripts are tailored for Linux shells for now.

There are 4 scripts located within the *./scripts* subdirectory:
- *clean_all* removes all build files and installations
- *build_dependencies* builds all dependencies by fetching, compiling, and installing them locally
- *build [DEBUG]* builds pwng-server/pwng-client, the optional *DEBUG* parameter will build in debug mode, accordingly.
- *run* starts pwng-server/pwng-client by setting the environment to the local installation and executing the binary.

### Dependencies
The server has no external dependencies that need to be manually installed. All dependencies will be automatically installed locally by using the *build_dependencies* script in *./scripts/*.

The client depends on SDL2 which has to be manually installed on the system. There is also the possibility to use GLFW3 instead of SDL2, choose the *glfw* branch in that case. All other dependencies will be automatically installed locally by using the *build_dependencies* script in *./scripts/*.

## Behind the scenes

### EnTT

At the server side, EnTT is used for two main purposes at the moment:
1. Each object, e.g. a star, is an entity with several components. This might be metadata such as temperature and stellar class or kinematics/kinetics information like position, velocity, acceleration. Objects with the latter data pass the integrator system, which integrates all accelerations and subsequently velocities to their final position.
2. Each connection to a client is an entity, too. This allows for component-based subscriptions. For each type of subscription there will be an accordant component. If a client requests a certain subscription, the representing entity will simply get the relevant subscription component attached. This way, all subscriptions can be handled by iterating over the component views.

### Magnum

The client heavily relies on the excellent [Magnum](https://github.com/mosra/magnum) middleware.
1. At the moment, stars are just drawn as circles with the stock shader. The number of segments is based on the circles size, which basically is a very naive 3-stage LOD. In the future, a custom shader will draw dots if size is small in a single draw call. For circles, instancing can be implemented. Furthermore, since the stars of the galaxy are static for now, galaxies can be prerenderd to a texture in order to only draw a textured quad for lower LODs.
2. The main scene (without UI) is rendered to a texture. This allows for arbitrary resolutions. If the resolution is lower than the native window resolution, the texture is just rendered to screen. For higher resolutions, i.e. SSAA (super sampling anti-aliasing), rendering consists of two stages: First, the scene is rendered to a high resolution texture. Second, a custom Gaussian blur shader is used for lowpass filtering with several passes, depending on super sampling factor. Hence, the final rendering to screen (which effectively is a downsampling of the image) follows the Nyquist-Shannon theorem to avoid aliasing and thus, to produce the desired anti-aliased result. 

## Client-Server Communication Protocol

The communication protocol will be specified in the wiki: [Application Protocol Draft](https://github.com/planeworld/pwng-server/wiki)
