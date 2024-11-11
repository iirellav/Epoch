# Epoch

Epcoh is a game engine I'm working on during my free time. The goal is to learn as much as possible and grow as a programmer, but also to be able to create and maybe even publish some small games.
My main source of inspiration and resource is [Hazel](https://hazelengine.com/) by **Studio Cherno**.

***

## Getting Started

For now, Windows is the only supported platform.

<ins>**1. Downloading the repository:**</ins>

The repository can either be downloaded directly or cloned. When cloning, recursive cloning isn't needed as no submodules are currently used.
This will probably be changed in the future to minimize the repositories size.

<ins>**2. Generate project:**</ins>

Running the [Generate.bat](https://github.com/isak-morand/Epoch/blob/main/Generate.bat) file will generate all the needed files.

<ins>**2. Building and running:**</ins>

Before you build you should know about the configurations. `Debug` runs really slow and is only used for debugging. `Release` is the main config used for development. `Dist` is used to build distributation builds that run without the console, not used for development except for testing purposes.

Before we can build and run, we need to manually build the Epoch-ScriptCore. To do so, right-click on `Solution Explorer -> Core -> Epoch-ScriptCore` and press `Build`.

You can now build and run the engine/editor.

I suggest reading [this](https://github.com/isak-morand/Epoch/wiki/Create-your-first-game:-A-guide-to-make-a-flappy-bird-ripoff#create-your-project) to get started.
